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

#include "dogdef.h"

#ifdef _STORAGE_MYSQL
#include "mysql.c"
#endif
#ifdef _STORAGE_MSSQL
#include "mssql.c"
#endif
#ifdef _STORAGE_ORACLE
#include "oracle.c"
#endif

#include "trydb.c"
#include "afuncs.c"
#include "spools.c"
#include "actions.c"
#include "msgattr.c"
#include "msgparse.c"
#include "msgget.c"

void* sync_sign_handler(void* unused)
{
  int sig;
  debugf(DEBUG_L1, LOG_NOTICE,"started signal handling thread\n");
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGHUP);
//  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTTIN);
  while (sigwait(&set,&sig)==0)
  {
    switch(sig)
    {
     case SIGHUP:
       debugf(DEBUG_L1, LOG_NOTICE, "Re-Loading ip client information\n");
       if ( !ReloadIpConfig() )
           debugf(DEBUG_L1, LOG_WARNING, "Cannot load ip client information\n");
       break;
     case SIGINT:
       debugf(DEBUG_L1, LOG_WARNING, "SyslogDog is under attack!!!\n");
       break;
     case SIGTTIN:
       detach(SIGTTIN);
       break;
     case SIGUSR1:
       DumpToDisk(SIGUSR1);
       break;
     case SIGUSR2:
       discard_th3(SIGUSR2);
       break;
    }
  }
  return 0;
}


int main(int argc, char **argv)
{
	sigset_t set;
        sigfillset(&set);
        pthread_sigmask(SIG_SETMASK, &set, NULL);
        pthread_t sync_sign_th;
        pthread_create (&sync_sign_th, NULL, &sync_sign_handler, NULL);	
        
        int mark;
	//struct sigaction act;
#ifndef _NODAEMON
	pid_t dad;
#endif
	//act.sa_handler = detach;
	//sigemptyset(&act.sa_mask);
	//act.sa_flags = 0;
	
#ifndef _NODAEMON
	if ( (dad = fork()) == -1 ) debugf(DO_EXIT, LOG_CRIT, "can't fork\n");
	if ( dad == 0 )
	{
#endif
	sldog = getpid();
	if ( !LoadDBConfig() )
        	debugf(DEBUG_L1 | DO_EXIT, -1, "Can't load database information\n");

	//if ( sigaction(SIGINT, &act, NULL) < 0 )
	//	debugf(DEBUG_L2, LOG_CRIT, "Could not get old handler for SIGINT\n");

	if ( (db_real = ManageDB((void **)&dbase, DB_CONN, OPEN)) >= 0 )
		debugf(DEBUG_L1, LOG_NOTICE, "%d DB Connections established\n", db_real);
	else
		debugf(DEBUG_L1, LOG_WARNING, "Can't connect to database\n");

    if ( manage_spool() == -1 ) 
		debugf(DEBUG_L2, LOG_CRIT, "Child error\n");

    if ( (d_todb=(DataToDb *)malloc(sizeof(DataToDb))) == NULL )
        debugf(DEBUG_L2|DO_EXIT, LOG_CRIT, "Calloc error\n");
	
	// repitiendo e invirtiendo la operacion de la carga de clientes
	// evitamos que el syslogd quede stopeado si no se puede resolver 
	// alguno de los clientes de la lista
	debugf(DEBUG_L1, LOG_NOTICE, "Loading ip client list...\n");
	if ( !LoadIpConfig() )
		debugf(DEBUG_L1, LOG_WARNING, "Cannot load ip client information\n");
	debugf(DEBUG_L1, LOG_NOTICE, "Waiting syslog server...\n");
	old_syslog = syslogpid();

	if ( c_ptrace(PTRACE_ATTACH) != 0 )
	{
		if ( d_todb != NULL ) free(d_todb); // arreglar esta condicion
		debugf(DO_EXIT|DEBUG_L1, LOG_CRIT, "PTRACE_ATTACH on pid %d failed\n", old_syslog);
	}


	wait_pid(EXPECT_STOPPED, SIGSTOP);
	c_ptrace(PTRACE_SYSCALL);

	for( ; ; )
	{
		debugf(DEBUG_L1, LOG_NOTICE, "Process %d(syslogd) Attached\n", old_syslog);
		mark = traceloop();

		if ( mark == -1 )
		{
			c_ptrace(PTRACE_DETACH);
			// ante la caida del syslogd la lista de clientes
			// no es recargada nuevamente.
			debugf(DEBUG_L1, LOG_NOTICE, "Waiting syslog server...\n");
			old_syslog = syslogpid();

			if ( c_ptrace(PTRACE_ATTACH) != 0 )
				debugf(DO_EXIT|DEBUG_L1, LOG_CRIT, "PTRACE_ATTACH on pid %d failed\n", old_syslog);
			
			mark = 0;
		}
	} 
#ifndef _NODAEMON
	}
#endif
	exit(0);
}
