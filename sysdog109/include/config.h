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
#include <sys/types.h>
#include <sys/stat.h>

#include "net.h"
#include "datalib.h"

#include "typedb.h"

#undef SYSLOG_NAMES
#ifdef HAVE_SYS_SYSLOG_H
#include <sys/syslog.h>
#else
#include "syslog.h"
#endif

const char *CFG_FILE = PACKAGE_DIR"/etc/db-sldog.conf";
db_struct dbi;
static int entry = 0;

extern ip_long getip(char *);
extern int debugf(const int, const int, const char *, ...);
extern size_t aRead(char *, const size_t, const size_t, FILE *);
