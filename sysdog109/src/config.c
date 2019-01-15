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

#include "config.h"

bool ConfigAccess(void)
{
	struct stat filest;

	if ( lstat(CFG_FILE, &filest) == -1) return FALSE;

	// El archivo db-sldog.conf necesita los permisos:
	// File Permision octal: 0600
	// User ID: 0
	// Group ID: 0
	if ( (filest.st_mode & 0xfff) != 0x180 || filest.st_uid != 0 || filest.st_gid != 0 ) 
		debugf(DEBUG_L1, LOG_CRIT, "Wrong permissions on config file %s\n", CFG_FILE);

	return TRUE;
}


static void CopyConfigParameter(char *part)
{
	struct in_addr host;

	if ( (int )part[strlen(part)-1] == ':' ) 
		part[strlen(part)-1] = 0x0;

	switch(entry)
	{
		case 0:	// DB_HOST
			host.s_addr = getip(part);
			memcpy(dbi.db_server, (char *)inet_ntoa(host), MAX_DBHOST); 
			break;
		case 1: memcpy(dbi.db_name, part, MAX_DBNAME); break; // DB_NAME
		case 2: memcpy(dbi.db_table, part, MAX_DBTBL); break; // DB_TABLE
		case 3: memcpy(dbi.db_user, part, MAX_DBUSER); break; // DB_USER
		case 4: memcpy(dbi.db_pass, part, MAX_DBPASS); break; // DB_PASS
	}			
	entry++;

	return;
}

// Funcion recursiva;
// para obtener parametro por parametro la linea de datos de la BASE	
void ConfigParameter(char *ptr)
{
	char partline[PARTLINE];
	size_t pos;

	if ( strlen(ptr) == 0 ) return;

	memset(partline, 0x0, PARTLINE);
	memccpy(&partline[0], ptr, 0x3a, PARTLINE-1);
	pos = strlen(partline);

	(void )CopyConfigParameter(partline);
	(void )ConfigParameter(ptr+pos);

	return;
}


bool LoadDBConfig(void)
{
	char line[MAXLINE] = { 0 };
	FILE *filed = NULL;

	if ( !ConfigAccess() || 
		((filed = fopen(CFG_FILE, "r")) == NULL) ||
		filed == NULL ) 
			return FALSE;

	while ( !feof(filed) )
	{	// TODO_WARN: al momento solo tomamos una sola linea de datos
		// 			en el archivo que contiene la informacion de la base
		entry = 0;

		if ( !aRead(line, MAXLINE-1, U16, filed) ) continue;
		(void )ConfigParameter(line);
	}

	fclose(filed);
	return TRUE;
}
