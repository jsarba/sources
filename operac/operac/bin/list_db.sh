# Jonathan Sarba
# Octubre 10 de 2004
#
# Lista el estado de las bases locales

clear
m_titulo=$1
user=$(whoami)"@"$(hostname)
db_state="${WORK}/sbin/db_state"
tempfile="${WORK}/logs/.db_state.$$"

oratab=$(cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep ORATAB|awk '{print $2}')
db_list=`cat ${oratab} | awk -F\: '{print $1} ' | sed -e '/^#/d'`

echo "Usuario: ${user}" 
echo "-------------------------------------------------------------"
echo "$m_titulo"
echo "============================================================="
printf "Estado   \tBase de Datos\n"
echo "-------------------------------------------------------------"

for db in $db_list
do
	export ORACLE_SID=$db
	export ORACLE_HOME=`cat ${oratab} |sed -e '/^#/d' |grep -w $ORACLE_SID |awk -F\: '{print $2}'`
	${db_state} $db > ${tempfile}

	cat ${tempfile} | grep -i denied 1>/dev/null 2>/dev/null

	if [ $? -eq 0 ] ; then
		EXTRA="Logon failed"
	fi

	cat ${tempfile} | grep "^dbuped" 1>/dev/null 2>/dev/null

	if [ $? -eq 0 ]  ; then
		printf "( READY )\t%s (%s)\n" ${ORACLE_SID} "${EXTRA}"
	else 
		printf "( DOWN )\t%s (%s)\n" ${ORACLE_SID} "${EXTRA}"

	rm -rf ${tempfile}
	EXTRA=""
fi
	
done

echo "-------------------------------------------------------------"
echo " Presione <ENTER> para continuar... \n "
read
