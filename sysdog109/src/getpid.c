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

#include "getpid.h"

/* obtenemos el timestamp de modificacion del archivo que
 * contiene el pid del syslog */
long get_mtime(void)
{
	struct stat filestat;
	
	/* el loop sera hasta que el archivo de pid vuelva a 
	 * generarse. Loop de espera */
	while ( stat(PID_FILE, &filestat) == -1 );
		
	return ( filestat.st_mtime != old_mtime ) ? filestat.st_mtime : 0;
}

/* solo si la fecha de modificacion cambio, comparamos el valor
 * del pid original con el nuevo */
pid_t get_syslogpid(void)
{
	pid_t pid = -1;
	char line[MAXPID];
	int fd;
	ssize_t readbytes;

	if ( (fd=open(PID_FILE, O_RDONLY|O_NONBLOCK, 0)) == -1 )
		return -1;
	
	readbytes = read(fd, line, MAXPID-1);

	if ( readbytes > 0 ) 
		pid = (pid_t)strtoul(line, NULL, 0);

	close(fd);
	return ( pid != old_syslog ) ? pid : 0 ;
}
	
/* es la funcion principal que utiliza 
 * dicha funcion devolvera un pid valido o se quedara
 * infinitamente hasta encontrar uno */
pid_t syslogpid(void)
{
	long new_mtime;
	pid_t new_syslog;
	
	for ( ; ; )
	{
		new_mtime = get_mtime();
		if ( new_mtime )
		{
			old_mtime = new_mtime;
			new_syslog = get_syslogpid();

			if ( new_syslog )
			{
				old_syslog = new_syslog;
				return new_syslog;
			}
		} 
	}
}
