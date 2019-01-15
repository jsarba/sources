#!/usr/bin/ksh 
###################################################################
# Script: bkp_pl.sh                                               #
# Autor: Ariel Lombardero                                         #
# Funcion: Backup medio dia de Plantas                            #
# Fecha: 02-12-2003                                               #
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

# Variables
S="               "
s="    "
m_titulo=$1
alias fecha='date +"%d/%m/%y %H:%M:%S"'
dia=$(date +"%d%m%y")
user=$(whoami)"@"$(hostname)
cinta=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep TAPE|awk '{print $2}')

logback=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep BCK_P_LOG|awk '{print $2}')
logerr=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep BCK_P_ERR|awk '{print $2}')
logfile=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep BACKUPPL|awk '{print $2}'|sed -e "s/#/$dia/")

host=$(hostname)
instancia=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep BP_INSTANCE|awk '{print $2}')
copias=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep BP_COPIES|awk '{print $2}')

#
# Confirmar
. ingredat.sh "Esta seguro de realizar Backup 1/2 dia (S/N)?" "S s N n" 1
sino=$m_variable

if [[ $sino = "S" || $sino = "s" ]] ; then
  #
  # Inicialización del log.
  echo "################################################################################" >>$logfile
  echo "$(fecha) $user inicia el Backup del medio dia." >>$logfile
  /usr/bin/tctl -f "$cinta" rewind
  if [[ $? -eq 0 ]] ; then
     #
     # Proceso de backup
     nohup exec_bp $host $instancia $copias 1>$logback 2>$logerr &
     echo "El backup fue lanzado en Background. Por favor, revisar Log respectivo."| tee -a $logfile
  else
      echo "$(fecha) ERROR: La cinta no está disponible !" | tee -a $logfile
  fi
  echo "Presione una tecla . . .\c"
  read
fi

