#!/usr/bin/ksh 
###################################################################
# Script: bkp_pl_log.sh                                           #
# Autor: Ariel Lombardero                                         #
# Funcion: Log de Backup medio dia de Plantas                     #
# Fecha: 04-12-2003                                               #
###################################################################
#
veri_planta()
# Proceso de verificacion server de Planta
#
{
   central=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep CENTRAL|awk '{print $2}')
   if [[ $central != "no" ]] ; then
      echo "\nEste equipo no corresponde a una planta. . ."
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

logback=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep BCK_P_LOG|awk '{print $2}')

ps -ef | grep "backup.ksh" | grep -v grep >/dev/null
if [[ $? = 0 ]] ; then
   clear
   tail -f $logback
else
   pg $logback
fi
