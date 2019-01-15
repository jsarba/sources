#!/usr/bin/ksh 
###################################################################
# Script: dsmsched_log.sh                                         #
# Autor: PAscual Adrian y Cristian Carunchio                      # 
# Binario: Jonathan Sarba                                         #
# Funcion: Restart Servicio TSM                                   #
# Fecha: 05-17-2005                                               #
###################################################################

dia=$(date +"%d%m%y")
user=$(whoami)"@"$(hostname)
identificacion=`date +"%H:%M:%S"`" "${user}

logfile=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep SRV_LOGS|awk '{print $2}'|sed -e "s/#/$dia/")

###################################################################
#                                                                 #
# Proceso de verificacion server de Planta                        #
###################################################################
veri_planta()
{
   central=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep CENTRAL|awk '{print $2}')
   if [[ $central = "no" ]] ; then
      echo "\nEste equipo corresponde a una planta. . ."
      sleep 3
      return 1
   fi
}
      
###################################################################
#                                                                 #
# CUERPO PRINCIPAL                                                #
###################################################################

veri_planta
if [[ $? -eq 1 ]] ; then
   return
fi

# Listado de procesos TSM 
tsm_proc=$(ps -ef | grep dsm | wc -l)	
echo "      -------------------------------------------------------------"
if [ $tsm_proc = 1 ]; then
	echo "         NO HAY SERVICIOS DE TSM ACTIVOS"
	echo "      -------------------------------------------------------------"
else
	echo "          ESTAN ACTIVOS LOS SIGUIENTES SERVICIOS DE TSM "
	ps -ef | grep dsm 		
	echo "      -------------------------------------------------------------"
fi

echo "       Esta seguro de reiniciar los servicios de TSM (S/N)? \c" 
read sino

if [[ $sino = "S" || $sino = "s" ]] ; then
	srv_rstart 1
	ps -ef | grep dsm
	echo "${identificacion} [Se reincio el servicio TSM]" >> ${logfile}
	echo "      -------------------------------------------------------------"
	echo "          Presione una tecla para continuar....."
	echo "      -------------------------------------------------------------"	
	read
fi
