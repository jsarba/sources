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

#include "mempool.h"

void put_msg(DataToDb *sv_msg)
{
	if ( msgin >= SLOTNUM ) msgin = 0;

	if ( (mensaje[msgin] = (DataToDb *)malloc(sizeof(DataToDb ))) == NULL )
		debugf(DEBUG_L2, LOG_CRIT, "malloc error\n");

	debugf(DEBUG_L2, -1, "using slot (%d) at %p\n", msgin, mensaje[msgin]);

	pthread_mutex_lock(&msg_lock); // critic section
		memset((DataToDb *)mensaje[msgin], 0x0, sizeof(DataToDb ));
		// copiamos el mensaje en el slot 
		memcpy(mensaje[msgin]->msgline, sv_msg->msgline, MAX_SYSLOG_LINE);
// Comentado por el cambio en la estructura de datos
		mensaje[msgin]->clase.fac = sv_msg->clase.fac;
		mensaje[msgin]->clase.pri = sv_msg->clase.pri;
//		memcpy(mensaje[msgin]->clase.fac, sv_msg->clase.fac, U12);
//		memcpy(mensaje[msgin]->clase.pri, sv_msg->clase.pri, U12);
		mensaje[msgin]->ipaddr = sv_msg->ipaddr;
		mensaje[msgin]->timestamp = sv_msg->timestamp;
		msgin += 1 % SLOTNUM;
	pthread_mutex_unlock(&msg_lock); // end

	return;
}


bool QueueProducer(DataToDb *st_msg)
{
	// el caso de todos los slots llenos. (el mismo que se esta escribiendo 
	// es el proximo libre)
	if ( nslots <= 1 ) // siempre debemos dejar un slot de transicion
		return FALSE;

	(void )put_msg(st_msg);
	// ya asumimos que hubo al memos un slot libre
	pthread_mutex_lock(&slot_lock);
		nslots--;
	pthread_mutex_unlock(&slot_lock);

	// solo avisamos si el consumidor termino de sacar
	// todos los mensajes encolados en los slots
	if ( prod_done == FALSE )
	{
		pthread_mutex_lock(&item_lock);
			// volvemos a avisar que el consumidor tiene nuevos mensajes
			// en los slots
			prod_done = TRUE;
			pthread_cond_signal(&items);
		pthread_mutex_unlock(&item_lock);
	}

	return TRUE;
}
