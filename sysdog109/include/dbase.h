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

#include "typedb.h"

extern db_struct dbi;

// This feature is obsolete on OracleDbase
#ifdef _STORAGE_MYSQL
// Mysql Options (connect-timeout)
const u_int DB_TIMEOUT = 1;
#endif

// Cantidad de conexion simultaneas a la base 
// TODO_WARN: agregar para definir en el configure
#define DB_CONN 3

// tiempo de espera para volver a intentar la conexion
#define TRYTIME 60 // En segundos 

#define MAX_QUERY 4096

static int db_seq = 0;
static int db_real = DB_CONN;

#ifdef _STORAGE_MYSQL 
#include <mysql/mysql.h>
MYSQL dbase[DB_CONN];
static MYSQL spool_db[2];
#endif

#ifdef _STORAGE_MSSQL 
#include <sqldb.h>
typedef struct _MsSqlSt
{
	LOGINREC *login;
	DBPROCESS *dbproc;
} MSSQL;
MSSQL dbase[DB_CONN];
static MSSQL spool_db[2];
static const char ESCAPED_STR[]="`'&;";
#endif

#ifdef _STORAGE_ORACLE
#include <oci.h>
typedef struct _OracleSt
{
    OCIEnv *ociEnv;
    OCIError *ociErr;
    OCISvcCtx *ociSrv;
    OCIStmt *ociStat;
} ORACLE;

ORACLE dbase[DB_CONN];
static ORACLE spool_db[2];
static const char ESCAPED_STR[]="`'&;";
#endif
