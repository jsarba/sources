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

#include <stdio.h>
#include <stdarg.h>         // va_arg()
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#ifndef SYSLOG_NAMES
#ifdef HAVE_SYS_SYSLOG_H
#include <sys/syslog.h>
#else
#include "syslog.h"
#endif
#endif

#ifdef _STORAGE_MYSQL
#include <mysql/mysql.h>
#endif

#include "net.h"
#include "datalib.h"
#include "common.h"

#ifdef _STORAGE_ORACLE
#include <oci.h>
typedef struct _OracleSt
{
    OCIEnv *ociEnv;
    OCIError *ociErr;
    OCISvcCtx *ociSrv;
    OCIStmt *ociStat;
} ORACLE;
#endif

#ifdef _STORAGE_MSSQL
#include <sqldb.h>
typedef struct _MsSqlSt
{
	LOGINREC *login;
	DBPROCESS *dbproc;
} MSSQL;
#endif

// Definicion de la facility para los eventos del mismo daemon sldog
// No se puede recorrer la estructura facilitynames[] en busca de este valor
const int PACKAGE_FAC = 999;
//
const char *LOGFILE = PACKAGE_DIR"/var/log/sldog.log";
pid_t sldog;

#define	ERR_level 10

#ifdef __LEVEL1
	#undef ERR_level
	#define ERR_level 30
#elif __LEVEL2
	#undef ERR_level
	#define ERR_level 70
#endif

#ifdef SYSLOG_NAMES
typedef struct _code {
    char    *c_name;
    int c_val;
} CODE;
extern CODE prioritynames[];
#endif

extern ip_long getip(char *);
extern int ManageDB(void **, int, int);
extern void InsertMessage(DataToDb *, void *, int arg);
extern inline int QPRI(const int );
extern int prod_done;
