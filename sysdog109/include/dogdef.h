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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>	/* sigaction */
#include <sys/user.h> /* user_regs_struct */
#include <sys/ptrace.h>

#include <asm/unistd.h> /* syscall list */
#include <sys/types.h>
#include <sys/wait.h>

#ifdef HAVE_SYS_SYSLOG_H
#include <sys/syslog.h>
#else
#include "syslog.h"
#endif
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>

#include "net.h"
#include "datalib.h"
#include "common.h"

/* Las syscall que me interesan durante la ejecucion */
/* from /usr/include/linux/net.h*/
#define SYS_RECV        10      /* sys_recv(2)          */
#define SYS_SENDTO      11      /* sys_sendto(2)        */
#define SYS_RECVFROM    12      /* sys_recvfrom(2)      */

#define EXPECT_EXITED   1
#define EXPECT_SIGNALED 2
#define EXPECT_STOPPED  4

/* Solo para ser claros que el -1 significa que debera reatacharse */
#define REATACH -1

/***** variables globales y constantes *****/
const int NONE_FAC = LOG_LOCAL7; // Facility = local7
const int NONE_PRI = INTERNAL_NOPRI; // Priority = none

// TODO_WARN: seria necesario agregar espa#ol?
const char *MonthName[] =
	{ "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL };
const int hourLimit[] = { 23, 59, 59, 0 };

// marcamos cada vez que halla reconectado.
// comenzamos marcandolo como verdadero.
static bool reconnect = TRUE;

static int active_th3 = 1;
static pthread_t dbtry = 0;

// dias del mes (febrero + 1, a#o bisiesto)
int dayLimit[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };
DataToDb *d_todb;

/******* funciones y variables externas ********/
// from errors.c
extern int debugf(const int, const int, const char *, ...);
extern pid_t sldog;

// from getpid.c
extern pid_t syslogpid(void);
extern int old_syslog;

// from ipaddr.c
extern int UnloadIpConfig(void);
extern int LoadIpConfig(void);
extern int ReloadIpConfig(void);
extern int IsAddrPermit(pid_t, long, struct in_addr *);

// from config.c
extern bool LoadDBConfig(void);
