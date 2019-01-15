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
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>       // stat()
#include <fcntl.h>

/* Archivo con el pid del syslog, puede variar segun la plataforma
 *  * tomar este valor desde un archivo de configuracion */
#define PID_FILE "/var/run/syslogd.pid"

/* bytes maximos permitidos en la linea del archivo que contiene
 * el pid del syslog */
#define MAXPID 16

/* Variables globales para la deteccion de cambio de syslog pid */
long old_mtime = 0;
pid_t old_syslog = 0;
