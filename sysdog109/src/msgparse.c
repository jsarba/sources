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

int GetCode(char *header_code)
{
	char facility[U8];
	int tmp, fac, pri;
	int sz, ret = 0;

	memset(&facility[0], 0x0, U8);

	/*
	*	from RFC 3164
	*	...
	*	The PRI part MUST have three, four, or five characters and will be
	*	bound with angle brackets as the first and last characters.  The PRI
	*	part starts with a leading "<" ('less-than' character), followed by a
	*	number, which is followed by a ">" ('greater-than' character).
	*	...
	* Copiemos hasta el siguiente ">"
	* si superamos los 5 bytes pueden entrar mensajes NIS/RPC etc.
	*/
	if ( memccpy(&facility[0], &header_code[1], (int )'>', 4) == NULL ) 
	{
		ret = -1;
		goto fake_msg;
	}

	tmp = strtoul(&facility[0], NULL, 0);
	if ( tmp <= 0 ) goto fake_msg;

// Cambio de la estructura de datos, estas funciones cambio el valor de retorno
	fac = QFAC(LOG_FAC(tmp));
	pri = QPRI(LOG_PRI(tmp));

	if ( fac == -1 || pri == -1 ) goto fake_msg;

	// TODO_WARN: si fac o pri supera la cantidad de elementos de los
	// array facilitynames o prioritynames

// Las columnas de priority/facility cambia a integers
// Comentado ya que la estructura de datos pasa de char a int
	if ( facilitynames[fac].c_val != -1 )
		d_todb->clase.fac = fac;
//		strncpy(d_todb->clase.fac, facilitynames[fac].c_name, U12);

	if ( prioritynames[pri].c_val != -1 )
	{
//		strncpy(d_todb->clase.pri, prioritynames[pri].c_name, U12);
		d_todb->clase.pri = pri;
		sz = strlen(facility);
		return sz ;
	}

fake_msg:
	/* Consideramos que no se puede clasificar el mensaje */
	d_todb->clase.fac = NONE_FAC;
	d_todb->clase.pri = NONE_PRI;
//	strncpy(d_todb->clase.fac, NONE_FAC, U12);
//	strncpy(d_todb->clase.pri, NONE_PRI, U12);

	return ret; 
}


int GetDateTime(int c1pos, int boolean, char *header_day)
{
	register int c;
	char ts_tmp[TS_LEN+1] = {0};
	int sz = c1pos;
	int mes, day, year;
	int hours[3] = { 0 };
	time_t startlife;
	struct tm *ltime;

	if ( (startlife = time(NULL)) == -1 )
		debugf(DEBUG_L2, LOG_ERR, "Cannot get local time\n");

	ltime = (struct tm *)localtime(&startlife);

	year = ltime->tm_year + YSINCE;
	mes = GetMonth(header_day+sz);
	
  	// tm_mon The number of months since January, in the range 0 to 11.
	if ( boolean == FALSE || mes == 0 ) mes = ltime->tm_mon + 1;
	else sz += 4;

	day = PerfectDay(header_day+sz, mes, year);

	if ( boolean == FALSE || day == 0 ) day = ltime->tm_mday;
	else sz += 3 ; // Ej: "Nov 12 " 

	for (c=0; c < 3; c++)
	{
		hours[c] = GetPartTime(header_day+sz, c);
		if ( boolean == FALSE || hours[c] == -1 )
		{
			hours[0] = ltime->tm_hour;
			hours[1] = ltime->tm_min;
			hours[2] = ltime->tm_sec;
			mes = ltime->tm_mon + 1;
			day = ltime->tm_mday;
			sz = c1pos;
			break; 
		} else
			sz += 3; // Ej: "19:"
	}
	if ( c1pos == sz ) debugf(DEBUG_L2, LOG_NOTICE, "Setting syslog server localtime\n");

	snprintf(ts_tmp, TS_LEN, TS_FMT, // Format Fecha: %04d%02d%02d + Format hora: %02d%02d%02d
		year, // A#o
		mes, // Mes
		day, // dia
		hours[0], // hora
		hours[1], // minutos
		hours[2]); // segundos

	d_todb->timestamp = (d_long )atoll(ts_tmp);

	return sz;
}


int MsgProcess(struct in_addr ipaddr, char *header)
{
	int ret = 0;
	int realmsg = 0;
	int msg_pos = 0;

	if ( (msg_pos = GetCode(header)) == -1 )
		debugf(DEBUG_L2, LOG_WARNING, "Cannot classified the message\n");

	d_todb->ipaddr = ipaddr.s_addr;
	
	// y reasignamos msg_pos para el header
	// Si el mensaje no puede ser clasificado
	// no intentemos tomar la fecha del mismo. (arg 3)
	realmsg = GetDateTime(msg_pos +1, (msg_pos < 0) ? FALSE : TRUE, header);

	if ( strlen(header) == realmsg )
		debugf(DEBUG_L2, LOG_WARNING, "Message invalid or too short, realmsg=%d\n", realmsg);
	else if ( msg_pos == -1 && realmsg == 1 ) // Este podria ser el caso de paquetes NIS/RPC
		debugf(DEBUG_L2, LOG_WARNING, "Message invalid\n");
	else
	{
		strncpy(d_todb->msgline, &header[realmsg], MAX_SYSLOG_LINE);
		(void )InsertMessage(d_todb, (void *)&dbase[db_seq], 0);
		ret = 1;
	}
	/* return */
	return ret;
} 
