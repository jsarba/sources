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

#include "ipparse.h"
#include "net.c"

int add_node(int dbrw, ip_long address, int db_num)
{
	int size;
	list_c *newnode;

	size = sizeof(list_c) + 1;

	if ( (newnode = (list_c *)calloc(size, sizeof(list_c ))) == NULL )
		return -1;
	
	newnode->item.address = address;
	newnode->item.dbwrite = dbrw;
	newnode->item.database = db_num;
	newnode->next = NULL;
	
	if (head_ptr == NULL) head_ptr = newnode;
	else last_ptr->next = newnode;

	last_ptr = newnode;
	return 0;
}


data_c *get_node(void)
{
	list_c *tmplist;

	tmplist = *actual_ptr;
	if ( tmplist == NULL ) return NULL;

	tmpnode = tmplist->item;
	actual_ptr = &(tmplist->next);
	return &tmpnode;
}


int rew_nodels(void)
{
	actual_ptr = &head_ptr;

	if ( head_ptr == NULL ) return -1; // la lista esta vacia.
	else return 0; 
}


int UnloadIpConfig(void)
{
	list_c *cleaner;

	if ( rew_nodels() != 0 ) return 0;
	do
	{
		if ( (cleaner = *actual_ptr) == NULL ) break;
		actual_ptr = &(cleaner->next);
		free(cleaner);
	} while ( cleaner->next != NULL );

	head_ptr = NULL;
	last_ptr = NULL;
	actual_ptr = &head_ptr;

	return 1;
}


int LoadIpConfig(void)
{
	FILE *fd = NULL;
	int con;
	char line[MAXLINE] = { 0 };
	char addr[PARTLINE] = { 0 };
	char dbase[U16] = { 0 };
	struct in_addr host;

	if ( (fd = fopen(CLT_FILE, "r")) == NULL ) return 0;
		
	(void )fseek(fd, 0L, SEEK_SET);

	while ( !feof(fd) )
	{
		if ( !aRead(line, MAXLINE-1, 4, fd) ) continue;

		// TODO_WARN: optimizar esto!!!
		// podriamos haber utilizado la tecnica usada en 
		// config.c
		memccpy(&addr[0], line, 0x3a, PARTLINE);
		con = strlen(addr);
		addr[con-1] = 0x0;
		host.s_addr = getip(addr); // resolvemos el hostname de la lista

		memccpy(&dbase[0], line+con, 0x3a, U16);
		con += strlen(dbase);

		// definir la base de datos
		if ( strlen(dbase) <= 1 ) memcpy(dbase, DEFAULT_DB, strlen(DEFAULT_DB));
		else dbase[strlen(dbase)-1] = 0x0;

		if ( strncmp(line+con, LOCALWRITE, strlen(LOCALWRITE)) == 0 )
			con = 1; // parametro es "local" ( Escribe en disco )
		else
			con = 0;

		// tercer argumento es el numero de base
		if ( add_node(con, host.s_addr, 0) != 0 ) return 0;
	}

	Client_List = TRUE;
	fclose(fd);
	return 1;
}


int ReloadIpConfig(void)
{
	(void )UnloadIpConfig();
	return LoadIpConfig();
}


/* Comprobamos que la direccion IP este permitida 
 * Marcamos si escribimos en disco 
 * El tercer argumento sera solo informativo para la funcion de llamada
*/
int IsAddrPermit(pid_t pid, long addr_ori, struct in_addr *client)
{
	int ret = -1; // Deny all clients by default
	data_c *infonode;

	client->s_addr = 0;
	if ( addr_ori != 0 )	// Only IPv4 (ia32)
		client->s_addr = (uint32_t )peek(pid, (void *)(addr_ori + 4));
	else
		client->s_addr = getip(LOCALHOST);

	if ( client->s_addr == -1 || client->s_addr == 0 ) return ret;
	if ( rew_nodels() == -1 || Client_List == FALSE ) return ret; // Deny all

	while ( (infonode = get_node() ) != NULL )
		if ( infonode->address == client->s_addr )
		{
			ret = infonode->dbwrite;
			break;
		}

	return ret; 
}
