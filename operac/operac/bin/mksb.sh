###################################################################
# Script: mksb.sh                                                 #
# Autor: Ariel Lombardero                                         #
# Funcion: MKSYSB del equipo                                      #
# Fecha: 25-11-20023                                              #
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

logfile=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep MKSYSB|awk '{print $2}'|sed -e "s/#/$dia/")

#
# Confirmar
. ingredat.sh "Esta seguro de ejecutar Mksysb (S/N)?" "S s N n" 1
sino=$m_variable

if [[ $sino = "S" || $sino = "s" ]] ; then
  #
  # Inicialización del log.
  echo "################################################################################" >>$logfile
  echo "$(fecha) $user inicia el Mksysb." >>$logfile
  #
  # Proceso de backup
  echo "Iniciando el Mksysb . . ."
  /usr/bin/tctl -f "$cinta" rewind
  if [[ $? -eq 0 ]] ; then
    if [[ ! -e /tmp/.mksysb_lock ]] ; then
      /usr/bin/touch /tmp/.mksysb_lock
      exec_mksysb "$cinta" | tee -a $logfile
      /usr/bin/rm -f /tmp/.mksysb_lock
      /usr/bin/tctl -f "$cinta" rewoffl
      if [[ $? -ne 0 ]] ; then
        echo "$(fecha) Cuidado! la cinta no se expulso correctamente." | tee -a $logfile
      fi
    else
      echo "$(fecha) ERROR: Mksysb ejecutandose o cancelado previamente!" | tee -a $logfile
    fi
  else
    echo "$(fecha) ERROR: La cinta no está disponible !" | tee -a $logfile
  fi
  echo "$(fecha) FIN del Mksysb." >>$logfile
  echo "Presione una tecla . . .\c"
  read
fi

