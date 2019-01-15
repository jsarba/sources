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

void InsertMessage(DataToDb *mesi, void *ptr, int arg)
{
	if ( !WriteToDB((DataToDb *)mesi, (void *)ptr, arg) )
	{
		pthread_mutex_lock(&spool_lock);
			active_th2 = FALSE;
		pthread_mutex_unlock(&spool_lock);

		if ( !QueueProducer(mesi) )
			if ( SpoolOnDisk(mesi) <= 0 )
				debugf(DEBUG_L2, LOG_ERR, "Spool file error (need reopen\?\?)\n");
	} else
	{   // el thread1 se dara cuenta que puede escribir
		// y limpiara todos los mensajes de memoria.
		// Solo cuando el thread1 halla limpiado los mensajes de memoria
		// avisamos al thread2 que comienze a limpiar el spool de disco
		if ( nslots == SLOTNUM && active_th2 == FALSE && reconnect == TRUE )
		{
			reconnect = FALSE;
			pthread_mutex_lock(&spool_lock);
				active_th2 = TRUE;
				pthread_cond_signal(&spools);
			pthread_mutex_unlock(&spool_lock);
		}
	}

	return;
}

// kill -21 action  (Re-connect)
void disconnect(int signal)
{
	// signal == 0 => terminacion de ejecucion del sldog
	// por lo tanto no necesitamos hacer un LoadDBConfig
	if ( signal != 0 )
	{
		debugf(DEBUG_L1, LOG_NOTICE, "Re-Loading database information\n");
		if ( !LoadDBConfig() ) 
			debugf(DEBUG_L1, -1, "Cannot load database information\n");
	}

    if ( db_real > 0 )
		debugf(DEBUG_L1, LOG_NOTICE, "%d DB Connections closed\n",
			ManageDB((void **)&dbase, db_real, CLOSE));

	return;
}

// kill -1 action (Reload, client list )
void reload(int signal)
{
	debugf(DEBUG_L1, LOG_NOTICE, "Re-Loading ip client information\n");
	if ( !ReloadIpConfig() )
		debugf(DEBUG_L1, LOG_WARNING, "Cannot load ip client information\n");

	return;
}


void killme(int signal)
{
	debugf(DEBUG_L1, LOG_WARNING, "SyslogDog is under attack!!!\n");
	return;
}


/* Clean Ptrace */
int c_ptrace(int request)
{
	long ret;
	ret = ptrace(request, old_syslog, 0, 0);
	return ret;
}


/* Utilizada solo cuando no estemos forkeando el DOG, ya que
 * es la salida para el trapeo de CTRL+C */
void detach(int signal)
{
	// descuelgo el SyslogDog antes que nada
	c_ptrace(PTRACE_DETACH);
	debugf(DEBUG_L1, LOG_NOTICE, "Detach on process %ld\n", old_syslog);

	/*
	==5368== Invalid read of size 4
	==5368==    at 0x8049B87: UnloadIpConfig (ipparse.c:75)
	==5368==    by 0x8048BB0: detach (action.c:27)
	==5368==    by 0x52BFEFFF: ???
	==5368==    by 0x8048C32: wait_pid (action.c:49)
	==5368==  Address 0x1BA2BC04 is 12 bytes inside a block of size 448 free'd
	==5368==    at 0x1B90036F: free (malloc.c:153)
	==5368==    by 0x8049B83: UnloadIpConfig (ipparse.c:74)
	==5368==    by 0x8048BB0: detach (action.c:27)
	==5368==    by 0x52BFEFFF: ???
	TODO_WARN: Existe una inconsistencia en los free?? fuck!
	*/
	if ( UnloadIpConfig() )
		debugf(DEBUG_L1, LOG_NOTICE, "Unload ip client list\n");

	// termino los threads
	// deben terminar en orden 
	// puede que se haya recibido la se#al de terminar
	// y el thread 3 no este levantado
	if ( dbtry )
	{
		active_th3 = -1;
		pthread_kill(dbtry, SIGUSR2);
		pthread_join(dbtry, NULL);
	}

	active_th2 = -1;
	pthread_cond_signal(&spools);
	pthread_join(spoolid, NULL);

	active_th1 = -1;
	pthread_kill(constid, SIGUSR1);
	pthread_join(constid, NULL);

	// desconecto
	// solo quiero cerrar las conexiones que realmente estan abiertas
	(void )disconnect(0);

	if ( d_todb ) free(d_todb); 
	if ( fd_spool ) fclose(fd_spool);

	debugf(DO_EXIT, -1,  "finished.\n");
}


/* Tomar de la direccion los datos recibidos */
long peek(pid_t ppid, void *preg)
{
	long ret;
	ret = ptrace(PTRACE_PEEKDATA, ppid, preg, 0);
	return ret;
}


void wait_pid(int report, int stopsig)
{
	int status = 0;
	int special;
	pid_t pw;

	pw = waitpid(old_syslog, &status, 0);

	if (WIFEXITED(status) && !(report & EXPECT_EXITED))
		debugf(DEBUG_L2, LOG_CRIT, "child exited%s with status %d\n", WCOREDUMP(status) ? " and dumped core" : "", WEXITSTATUS(status));
	
	if (WIFSTOPPED(status) && !(report & EXPECT_STOPPED))
		debugf(DEBUG_L2, LOG_CRIT, "child stopped by signal %d\n", WSTOPSIG(status)); 
	
	if (WIFSIGNALED(status) && !(report & EXPECT_SIGNALED))
	{
		special = (int )WTERMSIG(status);
		debugf(DEBUG_L2, LOG_CRIT, "child signalled by signal %d %d\n", special);

		if ( special == (int )SIGKILL || special == 0 )
			debugf(DEBUG_L1, LOG_CRIT, "syslog killed. What????\n"); 
			// TODO_WARN: Sumarizar este tipo de eventos.
	}

	if ( WIFSTOPPED(status) && WSTOPSIG(status) != stopsig )
	{
		if ( WSTOPSIG(status) == stopsig ) return;
		ptrace(PTRACE_SYSCALL, old_syslog, 0, (void *)WSTOPSIG(status));
		return wait_pid(report, stopsig);
	}	
}
