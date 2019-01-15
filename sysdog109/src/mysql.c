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

#include "dbase.h"

bool MySql_Connect(MYSQL *instance)
{       
	mysql_options(instance, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&DB_TIMEOUT);

	if ( !mysql_real_connect(instance, dbi.db_server, dbi.db_user, dbi.db_pass, dbi.db_name, 0, NULL, 0) )
		return FALSE;

	return TRUE;
}  


bool MySql_Write(DataToDb *msg, MYSQL *inst)
{
	char query[MAX_QUERY] = { 0 };
	char escape[MAX_SYSLOG_LINE] = { 0 };

	mysql_real_escape_string(inst, escape, msg->msgline, strlen(msg->msgline));
	snprintf(query, sizeof(query), 
		"INSERT INTO %s.%s (date, facility, priority, hostname, message) VALUES('%lld', '%d', '%d', '%ld', '%s')",
			dbi.db_name, dbi.db_table, msg->timestamp, msg->clase.fac, msg->clase.pri, msg->ipaddr, escape);
	
	// TODO_WARN: La base ya no devuelve el control porque
	// el disco rigido del server con la base de datos esta lleno.

	if ( mysql_real_query(inst, query, strlen(query)) != 0 )
		return FALSE; // any error?!?!?

	return TRUE; // write ok
}


bool MySqlConn(MYSQL *conn, int action)
{
	switch(action)
	{
		case OPEN:
			if ( mysql_init(conn) != NULL && MySql_Connect(conn)) return TRUE;
			break;

		case CLOSE:
			if ( mysql_ping(conn) == 0 ) mysql_close(conn);
			return TRUE;
			break;

		case RECONNECT:
			(void )MySqlConn(conn, CLOSE); 
			return MySqlConn(conn, OPEN);
			break;
	}
	return FALSE;
}
