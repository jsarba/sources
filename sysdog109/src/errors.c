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

#include "errors.h"

FILE *OpenLogFile()
{
	FILE *logf;
	// TODO_WARN: control de errores!!
	if ( (logf = fopen(LOGFILE, "a+")) == NULL )
		fprintf(stderr, "[!!] Can't open log file %s\n", LOGFILE);
	fseek(logf, 0L, SEEK_END);
	return logf;
}


d_long GetTimestamp(char *buf)
{
	time_t unixlife;
	struct tm *ltime;
	char ts_tmp[TS_LEN+1] = { 0 };

	unixlife = time(NULL);
	ltime = (struct tm *)localtime(&unixlife);

	snprintf(ts_tmp, TS_LEN, TS_FMT, 
		ltime->tm_year + YSINCE, ltime->tm_mon + 1, ltime->tm_mday,
		ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

	if ( buf ) memcpy(buf, ts_tmp, TS_LEN);

	return atoll(ts_tmp);
}
	

int LogHeader(char *buf)
{
	int ret;

	(void )GetTimestamp(buf);

	ret = strlen(buf);
	snprintf(buf+ret, MAXLINE, ":%s(%d):", PACKAGE, sldog);

	return ret+1;
}

int EventToDb(char *event, int priori)
{
#ifdef _STORAGE_MYSQL
	MYSQL conn_db;
#endif
#ifdef _STORAGE_MSSQL
	MSSQL conn_db;
#endif
#ifdef _STORAGE_ORACLE
	ORACLE conn_db;
#endif
	int stat;
	DataToDb evento;

	stat = ManageDB((void *)&conn_db, 1, OPEN);

	strncpy(evento.msgline, event, MAX_SYSLOG_LINE);
	evento.timestamp = GetTimestamp(NULL);
// 	Comentado por el cambio de estructura de datos
//	strncpy(evento.clase.fac, PACKAGE, U12);
//	strncpy(evento.clase.pri, prioritynames[priori].c_name, U12);
	evento.clase.fac = PACKAGE_FAC;
	evento.clase.pri = priori;
	evento.ipaddr = getip(LOCALHOST);

	(void )InsertMessage((DataToDb *)&evento, (void *)&conn_db, 3);

	// no queremos intentar cerrar una conexion que nunca pudo establecerse
	if ( stat != -1 )
		(void )ManageDB((void *)&conn_db, 1, CLOSE);

	return 1;
}

/* Standard Error, el primer argumento nos permite 
 * terminar la ejecucion para los casos criticos */
int debugf(const int action, const int priori, const char *fmt, ...)
{
	FILE *logfile;
	register int val;
	int ret = 0;
	int level, pri_name;
	va_list ap;

	val = action & 1;
	level = action - val;

	// TODO_WARN: un infantil manejo en los niveles de errores. obtimizarlo !!!
	if ( level <= ERR_level )
	{
		char lerror[MAXLINE] = {0};
		int position = LogHeader(&lerror[0]);

		va_start(ap, fmt);
			ret = vsnprintf(&lerror[strlen(lerror)], sizeof(lerror)-1, fmt, ap);
		va_end(ap);

#ifndef _NOLOGLOCAL
		logfile = OpenLogFile();
		if ( logfile ) 
		{
			fprintf(logfile, "%s", lerror);
			fseek(logfile, 0L, SEEK_END);
			fclose(logfile);
		}
#endif
#ifdef _NODAEMON
		fprintf(stderr, "%s", lerror);
#endif
#ifndef _NOLOGTODB
		// no hagamos lio....!!! 
		if ( priori != -1 )
		{
			pri_name = QPRI(priori);
			(void )EventToDb(&lerror[position], pri_name);		
		}
#endif
	}

	if ( val ) _exit(0);

	return ret;
}
