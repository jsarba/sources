#!/usr/bin/ksh 
###################################################################
# Script: ctrl_bkp_pl_log.sh                                      #
# Autor: Ariel Lombardero                                         #
# Funcion: Control de logs de los backups de todas las Plantas    #
# Fecha: 04-12-2003                                               #
###################################################################
#
control_BK()
# Parametros recibidos:
# $1: Hostname del equipo a controlar
# $2: Parametro de error del proceso anterior
{
 if [[ $2 = 0 ]] ; then
   clear
   echo "Ejecutando comando remoto en $1... aguarde."
   ping -c2 $1 1>/dev/null 2>&1
   if [ $? -eq 0 ] ; then
      echo
      rsh $1 -l $rshusuario ls -ltr /logs/log/rmt0/backup.*
      if [[ $? -ne 0 ]] ; then
         echo "No posee permiso para comando remoto en $1. Consultar Adm. Unix. . ."
         sleep 4
         return 1
      fi
      echo "Presione una tecla para el tail en $1. . ."
      read
      echo "aguarde . . ."
      rsh $1 -l $rshusuario tail -20 /logs/log/rmt0/backup.log
   else
      echo
      echo "No hay conexión con $1 !!"
   fi
   echo  
   echo "Presione una tecla para seguir . . ."
   read
  else
    return 1
  fi
   
}
#
veri_rshserver()
# Proceso de verificacion server de Planta
#
{
   rshserver=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep RSHSERVER|awk '{print $2}')
   host=$(hostname)
   if [[ $rshserver != $host ]] ; then
      echo "\nEste equipo no es el server RSH. . ."
      sleep 3
      return 1
   fi
}
      
###################################################################
#
# CUERPO PRINCIPAL
#
veri_rshserver
if [[ $? -eq 1 ]] ; then
   return
fi
rshusuario=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep RSHUSUARIO|awk '{print $2}')

control_BK eg3_ds1 0
control_BK eg3_bb1 "$?"
control_BK eg3_cam "$?"
control_BK eg3_sl "$?"
control_BK eg3_cdo "$?"
control_BK eg3_rg "$?"
return 0
