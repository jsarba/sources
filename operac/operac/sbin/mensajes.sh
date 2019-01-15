#!/usr/bin/ksh
###################################################################
# Script: mensajes.sh						  #
# Autor: Ariel Lombardero                                         #
# Funcion: Mensajes de errores                                    #
# Fecha: 8-11-2002                                                #
###################################################################
#
# El parametro $1 es el codigo de error de $HOME/lib/mensajes.
# El parametro $2 es un numero. Si vale 0, no es un mensaje de error.
# Caso contrario, sí lo es.
#
# Recorro tabla de errores
mens=`grep $1 /home/operac/lib/mensajes | awk -F: '{print $2}'` 2>/dev/null
if [[ $? = 0 ]] ; then
	echo
	if [[ $2 > 0 ]] ; then
		tput bel
	fi
	echo $mens
else
	echo
	tput bel
	echo "ATENCION: DIRECTORIO O LIBRERIA DE MENSAJES PERDIDA"
fi
sleep 2
