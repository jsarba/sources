#!/usr/bin/ksh 
###################################################################
# Script: dsmsched_log.sh                                         #
# Autor: Ariel Lombardero                                         #
# Funcion: Log del dsmsched del TSM                               #
# Fecha: 09-12-2003                                               #
###################################################################
#
veri_planta()
# Proceso de verificacion server de Planta
#
{
   central=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep CENTRAL|awk '{print $2}')
   if [[ $central = "no" ]] ; then
      echo "\nEste equipo corresponde a una planta. . ."
      sleep 3
      return 1
   fi
}
      
###################################################################
#
# CUERPO PRINCIPAL
#
veri_planta
if [[ $? -eq 1 ]] ; then
   return
fi

logtsm=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep DSMSCHED|awk '{print $2}')

pg $logtsm
