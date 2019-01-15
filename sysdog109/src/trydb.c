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

int ManageDB(void **ptr, int times, int action)
{
	register int i;
	int err = 0;

	for(i = 0; i < times; i++)
	{
#ifdef _STORAGE_MYSQL
        MYSQL *dbconn = (MYSQL *)ptr + (i * sizeof(void));
        if ( !MySqlConn(dbconn, action) ) err++;
#endif 
#ifdef _STORAGE_MSSQL
        MSSQL *dbconn = (MSSQL *)ptr + (i * sizeof(void));
        if ( !MsSqlConn(dbconn, action) ) err++;
#endif 
#ifdef _STORAGE_ORACLE
        ORACLE *dbconn = (ORACLE *)ptr + (i * sizeof(void));
        if ( !OracleConn(dbconn, action) ) err++;
#endif
	}

	return (err > 0 ) ? -1 : i;
}


void discard_th3(int signal)
{
	if ( active_th3 != -1 ) return;
	pthread_exit(0);
}


//thread generado solo para las conexiones principales
//en caso de perder la conexion.
static void *TryDbConnect(void *arg)
{
	register int i;
    int count = 0;
    sigset_t intmask;

    sigemptyset(&intmask);
    sigaddset(&intmask, SIGINT);
    pthread_sigmask(SIG_BLOCK, &intmask, NULL);

	signal(SIGUSR2, discard_th3);

    while ( count < DB_CONN )
    {	
		//count = ManageDB((void **)&dbase, DB_CONN, OPEN);
		//hacemos el connect directo para evitar la perdida
		//de control del thread
        for (i=0 ; i < DB_CONN ; i++)
#ifdef _STORAGE_MYSQL
			if ( MySql_Connect(&dbase[i]) ) count++;
#endif
#ifdef _STORAGE_MSSQL
			if ( MsSql_Connect(&dbase[i]) ) count++;
#endif
#ifdef _STORAGE_ORACLE
			// los handle de las lib oracle nunca se liberan
			if ( OracleConn(&dbase[i], RECONNECT) ) count++;
#endif

		//el caso de que nunca se establezca la conexion con la base
		//por un lado la se#al genera pthread_exit cuando el thread debe terminar
		if ( active_th3 == -1 ) break;

		if ( count <= 0 )
		{
#ifdef _STORAGE_MYSQL
        	debugf(DEBUG_L1, LOG_CRIT, "[TH3] %s\n", mysql_error(&dbase[0]));
#endif
#ifdef _STORAGE_MSSQL
		//TODO
//        	debugf(DEBUG_L1, LOG_CRIT, "[TH3] %s\n", mssql_error(&dbase[0]));
#endif
		//las libs oracle devuelven el mensaje de error durante el logon
#ifdef _STORAGE_ORACLE
			text txtError[512] = { 0 };
#ifdef _ORACLE10
			sb4 nError;
#else
			ub4 nError;
#endif
			OCIErrorGet((&dbase[0])->ociErr, (ub4)1, (text *)NULL, &nError, txtError, (ub4)sizeof(txtError)-1, OCI_HTYPE_ERROR);
			if ( strlen((char *)txtError) ) debugf(DEBUG_L1, LOG_CRIT, "[TH3] Connect Failed: %s", txtError);
#endif
			// timer para intentar la reconexion
        	sleep(TRYTIME);
		}
    }
	// si en realidad salio del while porque la se#al fue recibida 
	// no nos interesa el estado de la conexion
	if ( active_th3 != -1 ) debugf(DEBUG_L1, LOG_NOTICE, "[TH3] (Re)connected to Database.\n");
	db_real = count;
	reconnect = TRUE;
    return NULL;
}


int WriteToDB(DataToDb *message, void *ptr, int inst)
{
#ifdef _STORAGE_MYSQL
	MYSQL *tmp = (MYSQL *)ptr;
#endif
#ifdef _STORAGE_MSSQL
	MSSQL *tmp = (MSSQL *)ptr;
#endif
#ifdef _STORAGE_ORACLE
    ORACLE *tmp = (ORACLE *)ptr;
#endif

	if ( db_real == -1 )
	{
		if ( inst == 0 && dbtry == 0 )
			pthread_create(&dbtry, NULL, &TryDbConnect, NULL);
			// TODO_WARN: pendiente el mensaje de no puede crear el thread
	} else
	{
		dbtry = 0;

		if ( inst == 0 )
			db_seq = (db_seq + 1) % db_real;

#ifdef _STORAGE_MYSQL
		if ( mysql_ping(tmp) == 0 )
			return MySql_Write(message, tmp);
#endif
#ifdef _STORAGE_MSSQL
		if ( mssql_ping(tmp) == 0 )
			return MsSql_Write(message, tmp);
#endif
#ifdef _STORAGE_ORACLE
		if ( oracle_ping(tmp) == 0 )
			return Oracle_Write(message, tmp);
#endif
		if ( inst == 0 ) db_real = -1;
		else
			(void )ManageDB((void *)tmp, 1, RECONNECT);
	}

	return 0;
}
