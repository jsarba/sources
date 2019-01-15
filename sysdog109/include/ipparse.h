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
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

#include "datalib.h"
#include "common.h"

const char *CLT_FILE = PACKAGE_DIR"/etc/cip-sldog.conf";
const char *LOCALWRITE = "local";
const char *DEFAULT_DB = "sysdog";

typedef struct _DataClient
{
	int dbwrite;
	int database; // TODO_WARN: escritura en distintas bases
	ip_long address;
} data_c;

static data_c tmpnode;

typedef struct _list_client
{
	data_c item;
	struct _list_client *next;
} list_c;

static int Client_List = FALSE;

static list_c *head_ptr = NULL;
static list_c *last_ptr = NULL;
static list_c **actual_ptr = &head_ptr;

extern long peek(pid_t, void *);
extern size_t aRead(char *, const size_t, const size_t, FILE *);
