# Jonathan Sarba <jsarba@petrobrasenergia.com>
#
# 12 de mayo de 2005
#
# Bajar o subir una base de datos

S="\t"
s="\t"
m_titulo=$1
dia=$(date +"%d%m%y")
user=$(whoami)"@"$(hostname)
identificacion=`date +"%H:%M:%S"`" "${user}

# oratab=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep ORATAB|awk '{print $2}')
dbowner=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep DBOWNER|awk '{print $2}')
forze_sql=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep FORZE_SQL|awk '{print $2}')
db_action="${WORK}/sbin/db_action"

if [[ ! -r /etc/oratab ]] ; then
 		        echo "NO SE ENCONTRO ORATAB O NO TIENE PERMISOS PARA LEERLO!"
        		read
			exit 0       
fi
oratab=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep ORATAB|awk '{print $2}')

if [[ ${2} = "subir" ]] ; then
	logfile=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep STARTDB|awk '{print $2}'|sed -e "s/#/$dia/")
	START=1
else
	logfile=${WORK}/logs/$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep STOPDB|awk '{print $2}'|sed -e "s/#/$dia/")
	START=0
fi	

tempfile=${WORK}/logs/.dblist.$$
run_sqlplus=${WORK}/logs/.dbaction-sql.$$
run_svrmgrl=${WORK}/logs/.dbaction-svr.$$

cat ${oratab} | awk -F\: '{print $1}'| sed -e '/^#/d' > ${tempfile}

. ${WORK}/sbin/ingredat.sh "Bases disponibles:\n\n$(/usr/bin/cat ${tempfile}| sed -e 's/^/\\t\\t/')\n\n$S Ingrese Base o ENTER para volver" "`cat ${tempfile}`" 1

rm -rf ${tempfile}

   if [ ! -z "$m_variable" ] ; then
	export ORACLE_SID=$m_variable
	export ORACLE_HOME=`cat ${oratab} |grep -w $ORACLE_SID |awk -F\: '{print $2}'`
	echo " Espere..."
	case "$START" in
		"0")
			echo "${identificacion} [${db_action} $m_variable $dbowner stop]" >> ${logfile}
			if [[ ${forze_sql} = "si" ]] ; then
				touch ${run_svrmgrl}
			else	
				${db_action} $m_variable $dbowner stop 0 > ${run_svrmgrl}
			fi

			if [ -z "`cat ${run_svrmgrl} | grep "Connected"`" ] ; then
				${db_action} $m_variable $dbowner stop 1 > ${run_sqlplus}
				file=${run_sqlplus}
			else
				file=${run_svrmgrl}
			fi
			;;
		"1")
			echo "${identificacion} [${db_action} $m_variable $dbowner start]" >> ${logfile}
			if [[ ${forze_sql} = "si" ]] ; then
				touch ${run_svrmgrl}
			else
				${db_action} $m_variable $dbowner start	0 > ${run_svrmgrl}
			fi

			if [ -z "`cat ${run_svrmgrl} | grep "Connected"`" ] ; then
				${db_action} $m_variable $dbowner start	1 > ${run_sqlplus}
				file=${run_sqlplus}
			else
				file=${run_svrmgrl}
			fi
			;;
	esac	

	echo "-------------------------------------------------------------"
	cat ${file} | sed -e "s/^/$identificacion /" | tee -a ${logfile}	
	rm -rf ${run_sqlplus}
	rm -rf ${run_svrmgrl}
	echo "-------------------------------------------------------------"
	echo " Presione <ENTER> para continuar... \n "
	read
fi

