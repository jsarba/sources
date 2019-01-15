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

/* Lectura de configuracion en archivos de texto.
 * 
 * ptr Puntero donde se guardara la cadena de caracteres leidos
 * size Lenght de ptr
 * min_sz Cantidad minima de caracteres para que la linea sea valida
 * stream File descriptor del archivo involucrado
 *
 * Devuelve la cantidad de caracteres leidos por linea
 * 0 si la linea no es valida
*/

size_t aRead(char *ptr, const size_t size, const size_t min_sz, FILE *stream)
{
	register size_t count = 0;
	char touch;
	memset(ptr, 0x0, size);

	do
	{
		if ( fread(&touch, 1, sizeof(char), stream) == 0) break;
		if ( (int )touch == '\n' || (int )touch == '\r' ) break;
		memcpy(ptr+count, &touch, 1);
	} while ( count++ < size );

	if ( strlen(ptr) < 4 ||
		(int )ptr[0] == '#' || 
		(int )ptr[0] == 0x20 ) return 0;

	return count;
}


FILE *aOpen(const char *file, char *attr)
{
	FILE *fd;

	fd = fopen(file, attr);

	if ( fd == NULL )
		debugf(DEBUG_L1, LOG_ERR, "File %s error\n", file);

	return fd;
}
