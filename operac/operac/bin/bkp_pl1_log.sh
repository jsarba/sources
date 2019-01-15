#!/usr/bin/ksh 
###################################################################
# Script: bkp_pl1_log.sh                                          #
# Autor: Ariel Lombardero                                         #
# Funcion: Log de administracion de impresiones                   #
# Fecha: 23-12-2003                                               #
###################################################################
#
# Variables
S="               "
s="    "
m_titulo=$1
dia=$(date +"%d%m%y")
user=$(whoami)"@"$(hostname)

logimpr1=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep BACKUPPL|awk '{print $2}'|sed -e "s/#/*/")
/usr/bin/ls -x $logimpr1 | awk -F"${WORK}/logs/" '{print $2,$3,$4}' >/tmp/$$.log
#
# Ingresar Fecha
. ingredat.sh "LOGS disponibles:\n$(/usr/bin/cat /tmp/$$.log)\n\n$S Ingrese Fecha (ddmmaa) o ENTER para hoy ($dia)" "Fecha" 1
dia1=$m_variable
rm -f /tmp/$$.log

logimpr=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep BACKUPPL|awk '{print $2}'|sed -e "s/#/$dia1/")

if [[ -s $logimpr ]] ; then
	clear
	pg $logimpr
	i_existe=1
else
	mensajes.sh 4 1
fi
