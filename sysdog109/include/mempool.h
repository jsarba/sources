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

#define SLOTNUM 4096 // 1.1k por slot

const char *SPOOL_TEMP = PACKAGE_DIR"/var/spool/sldog/.msgtmp";
const char *SPOOL_FILE = PACKAGE_DIR"/var/spool/sldog/msg.spool";

int msgin = 0;
int nslots = SLOTNUM;

// ni bien comienza chequea que no halla mensajes viejos
// en el spool de disco
static int active_th2 = TRUE;

// solo utilizada para comprobar que la se#al SIGUSR1 
// es enviada por la funcion detach
static int active_th1 = TRUE;

int prod_done = 0;

FILE *fd_spool;
DataToDb *mensaje[SLOTNUM];

static pthread_mutex_t msg_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t slot_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t item_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t spool_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t items = PTHREAD_COND_INITIALIZER;
pthread_cond_t spools = PTHREAD_COND_INITIALIZER;

pthread_t constid;
pthread_t spoolid;
