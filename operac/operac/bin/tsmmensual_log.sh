#!/usr/bin/ksh 
###################################################################
# Script: tsmmensual_log.sh                                       #
# Autor: Ariel Lombardero                                         #
# Funcion: Log del TSM Mensual                                    #
# Fecha: 10-12-2003                                               #
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

logtsm_log=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep TSMMENSLOG|awk '{print $2}')
logtsm_err=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep TSMMENSERR|awk '{print $2}')

clear
echo "LOG DEL ULTIMO BACKUP MENSUAL TSM:"
sleep 3
pg $logtsm_log
clear
echo "ERRORES DEL ULTIMO BACKUP MENSUAL TSM:"
sleep 3
pg $logtsm_err
echo "Presione una tecla. . ."
read
