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

#include "net.h"

ip_long getip(char *addresock) 
{
	u_long ip;
	struct hostent *IPaddr;

	if ( (IPaddr = gethostbyname(addresock)) != NULL )
		memcpy(&ip, (u_long *)IPaddr->h_addr, IPaddr->h_length);
	else
		ip = inet_addr(addresock);

	return ip;
}

char *toAddr(ip_long ip) 
{
	struct in_addr ip_addr;

	ip_addr.s_addr=ip;
	return inet_ntoa(ip_addr);
}
