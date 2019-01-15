#!/usr/bin/ksh 
###################################################################
# Script: svvg.sh                                                 #
# Autor: Ariel Lombardero                                         #
# Funcion: SAVEVG del equipo                                      #
# Fecha: 27-11-20023                                              #
###################################################################
#
# Variables
S="               "
s="    "
m_titulo=$1
cinta=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep TAPE|awk '{print $2}')
alias fecha='date +"%d/%m/%y %H:%M:%S"'
dia=$(date +"%d%m%y")
user=$(whoami)"@"$(hostname)

logfile=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep SAVEVG|awk '{print $2}'|sed -e "s/#/$dia/")

#
# Cargar Volume Group
. ingredat.sh "Volume Groups Activos:\n$(/usr/sbin/lsvg -o)\n$S Ingrese Volume Group" "$(/usr/sbin/lsvg -o)" 1
if [[ -z $m_variable || $m_variable = " " ]] ; then
  return
fi
vol_group=$m_variable

#
# Confirmar
. ingredat.sh "Esta seguro de ejecutar Savevg (S/N)?" "S s N n" 1
sino=$m_variable

if [[ $sino = "S" || $sino = "s" ]] ; then
  #
  # Inicialización del log.
  echo "################################################################################" >>$logfile
  echo "$(fecha) $user inicia el Savevg de $vol_group." >>$logfile
  #
  # Proceso de backup
  echo "Iniciando el Savevg de $vol_group . . ." 
  /usr/bin/tctl -f "$cinta" rewind
  if [[ $? -eq 0 ]] ; then
    if [[ ! -e /tmp/.savevg_lock ]] ; then
      /usr/bin/touch /tmp/.mksysb_lock
      exec_savevg "$cinta" "$vol_group" | tee -a $logfile
      /usr/bin/rm -f /tmp/.savevg_lock
      /usr/bin/tctl -f "$cinta" rewoffl
      if [[ $? -ne 0 ]] ; then
        echo "$(fecha) Cuidado! la cinta no se expulso correctamente." | tee -a $logfile
      fi
    else
      echo "$(fecha) ERROR: Savevg ejecutandose o cancelado previamente!" | tee -a $logfile
    fi
  else
    echo "$(fecha) ERROR: La cinta no está disponible !" | tee -a $logfile
  fi
  echo "$(fecha) FIN del Savevg de $vol_group." >>$logfile
  echo "Presione una tecla . . .\c"
  read
fi

