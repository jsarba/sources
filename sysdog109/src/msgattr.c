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

// Por nueva estructura de datos, cambiamos el valor de retorno
static inline int QFAC(const int fac)
{
	register int ret;
	
	for(ret=0; facilitynames[ret].c_val != -1; ret++)
		if ( facilitynames[ret].c_val == (fac<<3) ) 
			return facilitynames[ret].c_val;
//			return ret;
	return -1;
}

// exported to errors.c
inline int QPRI(const int pri)
{
	register int ret;

	for(ret=0; prioritynames[ret].c_val != -1; ret++)
		if ( prioritynames[ret].c_val == pri )
			return prioritynames[ret].c_val;
//			return ret;
	return -1;
}
// --- cambiamos el valor de retorno


int GetMonth(const char *chunk1)
{
	register int month;
	char tmp[U8];

	memset(tmp, 0x0, U8);
	// tomamos los 4 bytes iniciales.
	// deberiamos tener el formato "Nov "
	if ( memccpy(&tmp[0], chunk1, 0x20, 4) != 0)
		for (month=1; MonthName[month] != NULL; month++)
			if ( strncmp(MonthName[month], &tmp[0], 3) == 0 ) return month;

	return 0;
}


int PerfectDay(const char *chunk1, int mes, int year)
{
	int day = 0;
	char tmp[U8];

	memset(tmp, 0x0, U8);
	// Necesito empezar a comparar desde el segundo digito del dia
	// El dia " 2 "
	memcpy(&tmp[0], chunk1, 3);
	day = strtoul(&tmp[0], NULL, 0);

	if ( (year % 4) == 0 && mes == 2 ) dayLimit[2] = 29;
	if ( day > dayLimit[mes] ) return 0;

	return day;
}


int GetPartTime(const char *chunk1, const int lim)
{
	register int t = 0;
	static int mark = 0;
	char tmp[4];
	
	memset(tmp, 0x0, 4);
	memcpy(&tmp[0], chunk1, 3);

	t = strtoul(&tmp[0], NULL, 0);

	if ( lim < 2 && (int )tmp[2] != ':' ) 
		{ mark = TRUE ; return -1; }
	else
		mark = FALSE;

	if ( lim == 2 && mark ) { mark = FALSE; return -1; }
		
	return ( t >= 0 && t <= hourLimit[lim] ) ? t : -1;
}
