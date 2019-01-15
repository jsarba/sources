#
# Jonathan Sarba
# Administracion de colas de impresion
# 23/12/2003
# 
#!/bin/sh

#
# Manejador de trabajos de impresion
#
MgtJobs()
{
	QUEUES=6
	CLINES=`expr ${QUEUES} \* 3`
	
	err=/tmp/.${PID}/.err
	jobs=/tmp/.${PID}/.jlp
	rm -rf ${jobs} ${err} 2>/dev/null
	${lpq} -l -P${printer[${cola}]} >> $jobs
	lines=`cat $jobs | wc -l`
	ret=0
	q=5
	while [ $q -le $lines ]
	do
		clear
		cat $jobs | head -4 
		${readline} $jobs ${q} ${CLINES}
		echo "-----------------------------------------------------------------"
		echo " Cancelar trabajo [x-Salir | enter-Siguiente] (#): \c"
		read ret 2>/dev/null
		if [[ $ret = "x" || $ret = "X" ]] ; then
			return
		fi
		if [[ $ret = +([0-9]) ]] ; then
			if [[ $ret -gt 0 ]]; then
				${cancel} ${ret} 2>$err 
				if [[ -n `cat ${err}` ]] ; then
					echo
					echo " ATENCION $ret: No se puede borrar el trabajo"
					echo "${HEAD} No se puede borrar el trabajo ${ret} de la cola de impresion ${printer[${cola}]}" >> ${logfile}
					cat ${err} >> ${logfile}
					echo
					sleep 2
				else
					echo "${HEAD} Borrar trabajo ${ret} de la cola de impresion ${printer[${cola}]}" >> ${logfile}
				fi
				break
			fi
		fi
		let q=q+${CLINES}
	done
	MgtJobs
}

#
# Carga de la matriz con el estado de las colas de impresion
#
LoadList()
{
        printers=/tmp/.${PID}/.llp
        rm -rf ${printers}
        ${lsallq} -c | grep ":" | sort >> $printers
        x=0
        for i in $(cat $printers)
        do
                let x=x+1
                printer[$x]=`echo $i`
                status[$x]=$(${lpstat} -a${printer[$x]} | head -3 | tail -1 | awk -F\  '{print $3}')
                jobs[$x]=`${lpstat} -a${printer[$x]}|wc -l`
                jobs[$x]=`expr ${jobs[$x]} - 3`
                if [ ${jobs[$x]} = 0 ] ; then
                        jobs[$x]="-"
                fi
                echo ".\c"
        done
}

#
# Listar impresoras / estado / trabajos
#
ListPrinters()
{ 
	clear
	echo " $1"
	echo "-----------------------------------------------------------------"
	echo "## - Cola\t\t\t  Estado\tTrabajos"
	echo "-----------------------------------------------------------------"
	y=1
	while [ ${y} -le ${x} ]
	do
		printf "%02d - %s\t" ${y} ${printer[$y]}
		if [[ ${#printer[$y]} < 19 ]] ; then
			echo "\t\c"
			if [[ ${#printer[$y]} < 11 ]] ; then
				echo "\t\c"
			fi
		fi
		echo " ( ${status[$y]} )\t${jobs[$y]}"
		let y=y+1
	done
	echo "-----------------------------------------------------------------"
	echo " Cola [x-Salir|r-Refresh|?-(enable|disable)](? #): \c"
	read cola 2>/dev/null

	if [[ -n $cola && ( $cola = "x" || $cola = "X" ) ]] ; then
		rm -rf /tmp/.${PID} 2>/dev/null
		exit
	fi
	if [[ $cola = "r" || $cola = "R" ]] ; then
		echo
		echo " Recargando listado de colas\c"
		LoadList
		ListPrinters
	fi
}

#
# Variables Globales
#
# extern var WORK (.initoper.sh)
#

PID="$$"
DIA=`date +"%d%m%y"`
HEAD=`date +"%d/%m/%y %H:%M:%S "``whoami`"@"`hostname`

logfile=${WORK}/logs/`cat ${CONFIG}|sed -e '/^#/d'|sed -e '/^$/d'|grep ADMPRINT|awk '{print $2}'|sed -e "s/#/$DIA/"`

lpstat=/usr/bin/lpstat
lpq=/usr/bin/lpq
lsallq=/usr/bin/lsallq
enable=/usr/bin/enable
disable="/usr/bin/disable -c"
cancel=/usr/bin/cancel
readline="${WORK}/sbin/readline"

#
# Main
#
rm -rf /tmp/.${PID} 2>/dev/null
mkdir -m 700 /tmp/.${PID} 2>/dev/null
err=/tmp/.${PID}/.err-ed
rm -rf ${err} 2>/dev/null
echo
echo "      Cargando colas de impresion\c"
LoadList
while [ true ]
do
	ListPrinters "$1"
	
	# Si es un numero
	if [[ $cola = +([0-9]) ]] ; then
		if [[ $cola -gt 0 && $cola -le $x ]] ; then
			if [[ ${jobs[$cola]} = "-" ]] ; then
				clear
				${lpq} -l -P${printer[${cola}]} | pg
			else
				MgtJobs
			fi
		fi
	else
		ena=`echo $cola | grep -i 'ena' 2>/dev/null`
		dis=`echo $cola | grep -i 'dis' 2>/dev/null`

		number=`echo $cola | awk -F\  '{print $2}'`

		if [[ -n $number ]] ; then
			if [[ -n $ena ]] ; then
				${enable} ${printer[${number}]} 2> $err
				if [[ -n `cat ${err}` ]]; then
					echo ${HEAD}" Error al levantar la cola de impresion ${printer[${number}]}" >> $logfile
					cat ${err} >> $logfile
				else
					echo ${HEAD}" Levantar cola de impresion ${printer[${number}]}" >> $logfile
				fi
			elif [[ -n $dis ]] ; then
				${disable} ${printer[${number}]} 2> $err
				if [[ -n `cat ${err}` ]]; then
					echo ${HEAD}" Error al bajar la cola de impresion ${printer[${number}]}" >> $logfile
					cat ${err} >> $logfile
				else
					echo ${HEAD}" Baja cola de impresion ${printer[${number}]}" >> $logfile
				fi
			fi
			if [[ -n `cat ${err}` ]]; then
				echo
				echo " ATENCION ${printer[${number}]}: No se pudo realizar la operacion"
				echo
				sleep 2
			else
				echo
				echo " Recargando listado de colas\c"
				LoadList
			fi
		else
			echo
			echo " ATENCION $cola: No esta definida"
			echo
			sleep 3
		fi
	fi
done
