###################################################################
# Script: dsmsched_log.sh                                         #
# Autor: PAscual Adrian y Cristian Carunchio                      #
# Binario: Jonathan Sarba                                         #
# Funcion: Restart Listener ORACLE                               #
# Fecha: 05-17-2005                                               #
###################################################################

dia=$(date +"%d%m%y")
user=$(whoami)"@"$(hostname)
identificacion=`date +"%H:%M:%S"`" "${user}
logfile=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep SRV_LOGS|awk '{print $2}'|sed -e "s/#/$dia/")

###################################################################
#                                                                 #
# Proceso de verificacion Listener                               #
###################################################################

chk_list=$(netstat -an | grep LIS | grep 1521 | wc -l)   
echo "      -------------------------------------------------------------"
if [ $chk_list = 0 ] ; then
	echo "                 No esta activo el Listener de Oracle . . ."
	else
	echo "          El servicio de Listener esta activo"
fi    

echo "      -------------------------------------------------------------"

###################################################################
#                                                                 #
# CUERPO PRINCIPAL                                                #
###################################################################

oratab=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep ORATAB|awk '{print $2}')
export ORACLE_HOME=`cat ${oratab}| grep -v "^#"|grep -v "^$" | grep -e "^[a-z]" -e "^[A-Z]"|awk -F\: '{print $2}'| head -1`
dbowner=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep DBOWNER|awk '{print $2}')

echo "       Esta seguro de reiniciar el Listener (S/N)? \c"
read sino
if [[ $sino = "S" || $sino = "s" ]] ; then
	srv_rstart 2 $dbowner
	echo "${identificacion} [Se reincio el servicio Listener]" >> ${logfile}
        echo "      -------------------------------------------------------------"
        echo "          Presione una tecla para continuar....."
        echo "      -------------------------------------------------------------"
        read
fi
