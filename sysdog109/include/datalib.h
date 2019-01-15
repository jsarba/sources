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

enum boolean { FALSE=0, TRUE } ;
enum conn_action { CLOSE = FALSE, OPEN, RECONNECT };

/* Definicion para la salida en la funcion debugf */
#define DO_EXIT TRUE

#define DEBUG_L1	10
#define DEBUG_L2	30
#define DEBUG_L3	70

#define U8 8
#define U12 U8 + 4
#define U16 16
#define U32 32
#define MAX_SYSLOG_LINE	1024	/* from syslog source */
#define MAX_IP_ADDR	U16
#define TS_LEN 18 // only for temporal use
#define MAXLINE 256
#define PARTLINE 48

#define ip_long unsigned long int
#define d_long long long

typedef enum boolean bool;

typedef struct _ClassCode
{
	int fac;
	int pri;
} class_code;

typedef struct _DataToDb
{
	char msgline[MAX_SYSLOG_LINE+1];
	d_long timestamp; // time_t != long long !!!!
	ip_long ipaddr;
	class_code clase;	// Static Facility Priority (names)
} DataToDb;

