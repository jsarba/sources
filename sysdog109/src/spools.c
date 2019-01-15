/*
    SyslogDog. Concentrator of systems logs
    Copyright (C) 2005  Jonathan Sarba.

    This file is part of SyslogDog.
    SyslogDog is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    SyslogDog is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SyslogDog; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "mempool.c"

size_t SpoolOnDisk(DataToDb *mess)
{
	size_t wrote; 

	// TODO_WARN: comprobacion de errores !!
	wrote = fwrite(mess, 1, sizeof(DataToDb ), fd_spool);	
	fseek(fd_spool, 0L, SEEK_END);

	return wrote;
}


void DumpToDisk(int signal)
{
	size_t mem_slots = SLOTNUM-nslots;
	register int i;

	if ( active_th1 == TRUE ) return;
	if ( mem_slots == 0 ) pthread_exit(0);

	fseek(fd_spool, 0L, SEEK_END);
	for (i = 0; i < mem_slots; i++)
	{
		// TODO_WARN: Si se corta la conexion en medio
		// de la escritura estariamos perdiendo mensajes
		(void )SpoolOnDisk(mensaje[i]);
		if ( mensaje[i] ) free(mensaje[i]);
	}
	if ( i > 0 )
		debugf(DEBUG_L1, LOG_NOTICE, "[TH1] %d message/s from memory spool, saved to disk.\n", i);

	pthread_exit(0);
}


// thread 1
// encargado de manejar el spool de memoria
static void *ChildConsumer(void *arg)
{
	register int i = 0;
	sigset_t intmask;

	sigemptyset(&intmask);
	sigaddset(&intmask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &intmask, NULL);
	// signal(SIGUSR1, DumpToDisk);

	for ( ;; )
	{
		// espera a que tenga nuevos mensajes en los slots
		pthread_mutex_lock(&item_lock);
			while ( nslots == SLOTNUM && !prod_done )
				pthread_cond_wait(&items, &item_lock);
		pthread_mutex_unlock(&item_lock);

		if ( ManageDB((void **)&spool_db[0], 1, OPEN) == -1 )	
		{
			sleep(TRYTIME);
			continue;
		}

		while ( i < SLOTNUM )
		{
			if ( nslots >= SLOTNUM ) break;

			// sincronizemos los mensajes entrantes y
			// los salientes;
			pthread_mutex_lock(&msg_lock);
				if ( !WriteToDB(mensaje[i], (void *)&spool_db[0], 1) ) 
				{
					pthread_mutex_unlock(&msg_lock);
					sleep(TRYTIME);
					(void )ManageDB((void **)&spool_db[0], 1, RECONNECT);
					continue;
				}
				if ( mensaje[i] ) free(mensaje[i]);
			pthread_mutex_unlock(&msg_lock);

			pthread_mutex_lock(&slot_lock);
			// este aviso es solamente si pudo escribir el mensaje
			// en caso contrario los slots permaneceran llenos hasta 
				nslots++;
			pthread_mutex_unlock(&slot_lock);
			
			i = ( i + 1 ) % SLOTNUM;
		}
		(void )ManageDB((void **)&spool_db[0], 1, CLOSE);
		debugf(DEBUG_L1, LOG_NOTICE, "[TH1] %d message/s from memory spool, saved sucessfully.\n", i);
		prod_done = FALSE;
	}
	return NULL;
}


// Pendiente: todo el chequeo de errores !!!
// devuelve el fd del archivo destino
FILE *copyfile(FILE *fdsrc, const char *dest, int from, int whence)
{
	FILE *fddst;
	register int i;
	DataToDb node;
	size_t size = sizeof(DataToDb);

	//TODO_WARN: comprobacion de errores!!!
	fddst = aOpen(dest, "a+");

	for (i=from; i < whence; i++)
	{
		memset(&node, 0x0, size);
		fseek(fdsrc, i*size, SEEK_SET);
		fread(&node, 1, size, fdsrc);
		fwrite(&node, 1, size, fddst);
	}

	return fddst;
}


void CleanSpool(void)
{
	unlink(SPOOL_FILE);
	unlink(SPOOL_TEMP);
	if ( fd_spool ) fclose(fd_spool);
	fd_spool = aOpen(SPOOL_FILE, "a+");
}


/* devuelve el item 'i' del spool */
inline size_t GetItem(FILE *fd, int i, DataToDb *item)
{
	fseek(fd, i*sizeof(DataToDb), SEEK_SET);
	return fread(item, 1, sizeof(DataToDb ), fd);
}


inline size_t GetNumItems(const char *file)
{
	struct stat filestat;

	if ( lstat(file, &filestat) != -1 ) 
		return (filestat.st_size / sizeof(DataToDb));

	return 0;
}


// thread 2
// el encargado de depurar el archivo de spool
static void *ManageSpoolDisk(void *arg)
{
	FILE *fdtmp = NULL;
	size_t num_items;
	DataToDb *msg_spool;
	sigset_t intmask;

	sigemptyset(&intmask);
	sigaddset(&intmask, SIGINT);
	pthread_sigmask(SIG_BLOCK, &intmask, NULL);

	if ( (msg_spool = (DataToDb *)malloc(sizeof(DataToDb ))) == NULL )
		debugf(DEBUG_L2, LOG_CRIT, "[TH2] SlotAlloc error\n");

	for ( ; ; )
	{
		int i = 0;

		pthread_mutex_lock(&spool_lock);
			while ( active_th2 == FALSE )
				pthread_cond_wait(&spools, &spool_lock);
		pthread_mutex_unlock(&spool_lock);

		if ( active_th2 == -1 ) break;
		if ( ManageDB((void **)&spool_db[1], 1, OPEN) == -1 )
		{
			active_th2 = FALSE;
			continue;
		}

		if ( (num_items = GetNumItems(SPOOL_TEMP)) > 0 )
			fdtmp = aOpen(SPOOL_TEMP, "r+");
		else if ( (num_items = GetNumItems(SPOOL_FILE)) > 0 )
		{
			fdtmp = copyfile(fd_spool, SPOOL_TEMP, 0, num_items);
			if ( !fd_spool ) 
				fd_spool = aOpen(SPOOL_FILE, "a+");
		}

		if ( num_items > 0 ) 
			debugf(DEBUG_L1, LOG_NOTICE, "[TH2] %d message/s found on spool file.\n", num_items);

		for ( ; i < num_items && active_th2 == TRUE ; i++ )
		{
			memset((DataToDb *)msg_spool, 0x0, sizeof(DataToDb ));
			(void )GetItem(fdtmp, i, msg_spool);
			if ( !WriteToDB(msg_spool, (void *)&spool_db[1], 2) ) break;
		}

		if ( fdtmp ) fclose(fdtmp);
		if ( i >= (num_items-1) ) 
		{
			(void )CleanSpool();
			debugf(DEBUG_L1, LOG_NOTICE, "[TH2] %d message/s from spool file, saved succesfully\n", i);
		}

		(void )ManageDB((void **)&spool_db[1], 1, CLOSE);
		active_th2 = FALSE;
	}
	free(msg_spool);
	return NULL;
}


// dispatcher
int manage_spool(void)
{
	fd_spool = aOpen(SPOOL_FILE, "a+");

	if ( pthread_create(&constid, NULL, &ChildConsumer, NULL) != 0 )
		return -1;                  

	if ( pthread_create(&spoolid, NULL, &ManageSpoolDisk, NULL) != 0 ) 
		return -1;                      

	return 1;
}
