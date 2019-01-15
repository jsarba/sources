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

#define MAX_DBHOST	PARTLINE
#define MAX_DBNAME	U32
#define MAX_DBTBL	U16
#define MAX_DBUSER	U16
#define MAX_DBPASS	U16

typedef struct _DbInfo
{
    char db_server[MAX_DBHOST];
	    char db_name[MAX_DBNAME]; // obsolete for oracle storage
		    char db_table[MAX_DBTBL];
			    char db_user[MAX_DBUSER];
				    char db_pass[MAX_DBPASS];
} db_struct;

