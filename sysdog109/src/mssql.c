/*
    SyslogDog. Concentrator of systems logs
    Copyright (C) 2007 Luciano Bello.

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

#include "dbase.h"

bool MsSql_Connect(MSSQL *instance)
{       
	instance->login=dblogin();
	DBSETLPWD  (instance->login,dbi.db_pass);
	DBSETLUSER (instance->login,dbi.db_user);
	DBSETLAPP  (instance->login,"sysdog");
	instance->dbproc=dbopen(instance->login,dbi.db_server);
	if ( ! instance->dbproc ) return FALSE;
	if ( ! dbuse(instance->dbproc,dbi.db_name)) return FALSE;
	return TRUE;
}  

bool mssql_init(MSSQL *con_ptr)
{
	if( dbinit() != 0 ) return TRUE;
	else return FALSE;
}

int mssql_escape_string(char *dest, char *src, int src_lenght)
{
    int ret = 0;

        // TODO_WARN: Ningun mensaje contendra un retorno de linea??
    while ( *src && (int )*src != '\n' && (int )*src != '\r' )
    {
        if ( strchr(ESCAPED_STR, *src) != NULL )
                        *dest++ = '\"';
                else
            *dest++ = *src;

        *src++;
                ret++;
    }
    *dest = '\0';

    return ret;
}

bool MsSql_Write(DataToDb *msg, MSSQL *inst)
{
	char query[MAX_QUERY] = { 0 };
	char escape[MAX_SYSLOG_LINE] = { 0 };

	mssql_escape_string(escape, msg->msgline, strlen(msg->msgline));
	snprintf(query, sizeof(query), 
	"INSERT INTO %s (date, facility, priority, hostname, message) VALUES('%lld', '%d', '%d', '%s', '%s')", dbi.db_table, msg->timestamp, msg->clase.fac, msg->clase.pri, toAddr(msg->ipaddr), escape);

	printf("----%s----\n",query);
	dbcmd(inst->dbproc, query);
	dbsqlexec(inst->dbproc);

	return TRUE;
}

bool mssql_ping(MSSQL *con_ptr)
{
//TODO
                        return 0;
}

void mssql_close(MSSQL *con_ptr)
{
  //TODO 
}

bool MsSqlConn(MSSQL *conn, int action)
{
	switch(action)
	{
		case OPEN:
			if ( mssql_init(conn) && MsSql_Connect(conn)) return TRUE;
			break;

		case CLOSE:
			if ( mssql_ping(conn) == 0 ) mssql_close(conn);
			return TRUE;
			break;

		case RECONNECT:
			(void )MsSqlConn(conn, CLOSE); 
			return MsSqlConn(conn, OPEN);
			break;
	}
	return FALSE;
}

