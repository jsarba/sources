#!/usr/bin/ksh
#
# triface-v1.8.tgz
#
# Jonathan Sarba <jsarba@petrobrasenergia.com>
# 7 Octubre de 2005
# 
# Pasajes SICOM - version 1.8
#
#

WORK_HOME=/instancias/sicom_central/
export WORK_HOME

. ${WORK_HOME}/etc/triface.conf
. ${WORK_HOME}/lib/triface.lib

# sudo restringido
SUDO=${WORK_HOME}/lib/js_sudo
# Comandos
FIND=`which find`
TAR=`which tar`
CKSUM=`which cksum`
COMPRESS=`which compress`
FILE=`which file`
CHOWN="/usr/bin/chown"

HEADER="${IDENT} (${NODE_TYPE}):"

LogPrint()
{
	echo "${HEADER} $1"
	echo "${HEADER} $1" 1>>${FILE_LOG} 2>&1
}

MailNotify()
{
	result=$1
	tid=$2
	objects=$3
	mails_file=${WORK_HOME}/etc/triface.mail
	temp_file=${TMP}/.notify.$$

	LogPrint "Notifying users."
	echo "Timestamp: `echo ${IDENT} | awk '{print $1}`" > $temp_file ; echo >> $temp_file
	echo "TransferID: $tid" >> $temp_file; echo >> $temp_file
	echo "Origen: Remitente($NODE_TYPE)" >> $temp_file ; echo >> $temp_file 
	echo "Destino: $NODE_DEST($NODE_DEST_TYPE)" >> $temp_file ; echo >> $temp_file
	echo "Objetos transferidos: $objects" >> $temp_file
	echo >> $temp_file
	for user in $(cat $mails_file|sed -e '/^#/d'|sed -e '/^$/d'|grep "$result"| awk -F\: '{print $2}')
	do
		cat $temp_file | mail -s "${IDENT}: Transference $tid $result." $user
	done
	rm -rf $temp_file 2>/dev/null 1>&2
}

LockDir()
{
	touch $2/.lock
	chmod $1 $2
}

CleanTemp()
{
	rm -rf ${TMP}/.ldf_testigo.$$ 2>/dev/null
	rm -rf ${TMP}/.df_testigo.$$ 2>/dev/null
	rm -rf ${TMP}/.crc_testigo.$$ 2>/dev/null
}

UnLockDir()
{
	chmod $1 $2
	rm -rf $2/.lock
}

CheckSum()
{
	CRC_TESTIGO="${TMP}/.crc_testigo.$$"
	for file in `cat ${LDF_TESTIGO} 2>/dev/null`
	do
		${SUDO} ${CKSUM} $2/$file | awk -vvar=$file '{print var":"$1}' >>${CRC_TESTIGO} 2>/dev/null
	done
}

DirScan()
{
	cd $1 2>/dev/null 1>&2

	if [[ $? -ne 0 ]] ; 
	then
		LogPrint "Can't access $1 directory";
		LogPrint "Transference $$ error."
		return -1; exit;
	fi

	if [[ ! -r "${LDF_TESTIGO}" ]] ; then
		TESTIGO_f=`${SUDO} ${FIND} . -type f| grep -v "^./.lock$" | grep -v "^./.repeat"| wc -l|sed -e 's/\ //g' 2>/dev/null`

		if [[ ${TESTIGO_f} -ne 0 ]] ; then
			${SUDO} ${FIND} . -type f | grep -v "^./.lock$" | grep -v "^./.repeat" 1>${LDF_TESTIGO} 2>/dev/null
			echo ${TESTIGO_f} 1>${DF_TESTIGO} 2>/dev/null
		fi
	else
		TESTIGO_f="`cat $DF_TESTIGO`"
	fi

	# PENDIENTE
	CheckSum ${LDF_TESTIGO} "`pwd`"

	cd - 1>/dev/null 2>&1
}

CopyFiles()
{
	for node in ${NODE_DEST}
	do
		cp -f ${LDF_TESTIGO} ${TESTIGO_FINAL} 2>/dev/null

		R_TESTIGO=$(ssh -p1004 ${node} "export WORK_HOME=${WORK_HOME};cd ${IFACE_D_PUBLIC} 2>/dev/null 1>&2;${SUDO} `which find` . -type f|wc -l|sed -e 's/\ //g' 2>/dev/null" 2>/dev/null)
		if [[ $R_TESTIGO -gt 0 ]] ; then
			LogPrint "${R_TESTIGO} accumulated files on destination."
		fi

		# scp para copiar el archivo testigo
		LogPrint "Sending control files to ${node} (${NODE_DEST_TYPE})"
		if [[ ! -z "$DF_TESTIGO" ]] ; then
			scp -p -P1004 $LDF_TESTIGO $node:${D_TMP}/
			if [[ $? -ne 0 ]] ; then
				touch ${TREX_DIR}/.repeat
				LogPrint "Errors while was sending the files. Transference $$ error."
				return -1 ; exit ; 
			fi
			scp -p -P1004 $DF_TESTIGO $node:${D_TMP}/ 
			if [[ $? -ne 0 ]] ; then
				touch ${TREX_DIR}/.repeat
				LogPrint "Errors while was sending the files. Transference $$ error."
				return -1 ; exit ;
			fi
		else
			LogPrint ".?f_testigo.* files not found, you need to reinitiate the transference"
			CleanTemp
			UnLockDir 750 ${TREX_DIR}
			LogPrint "Transference $$ error."
			return -1 ; exit ;
		fi

		# Obsoleto
		# ssh -p1004 ${node} "cd ${IFACE_D_PUBLIC} 2>/dev/null 1>&2; chmod -R u+w * 2>/dev/null"

		# TAR_METHOD
		cd ${TREX_DIR} 2>/dev/null 1>&2
		${SUDO} ${TAR} pcf ${TMP}/package.$$.tar $(cat ${LDF_TESTIGO}) 2>/dev/null
		cd - 2>/dev/null 1>&2

		# scp copiar los archivos
		if [[ ! -z "$TESTIGO_f" ]] ; then
			LogPrint "Sending ${TESTIGO_f} files to ${node} (${NODE_DEST_TYPE})"

			scp -P1004 ${TMP}/package.$$.tar $node:${D_TMP}/;

			#OLD_METHOD
			#scp -p -r -P1004 ${TREX_DIR}/* $node:${IFACE_D_PUBLIC}/;

			if [[ $? -ne 0 ]] ; then
				touch ${TREX_DIR}/.repeat
				LogPrint "Transference $$ error."
				return -1 ; exit 
			fi

			# TAR_METHOD
			ssh -p1004 ${node} "export WORK_HOME=${WORK_HOME}; cd ${IFACE_D_PUBLIC} 2>/dev/null 1>&2;${SUDO} `which tar` pxf ${D_TMP}/package.$$.tar 2>/dev/null;rm -rf ${D_TMP}/package.$$.tar 2>/dev/null"

			LogPrint "Analyzing the integrity of ${TESTIGO_f} files"
			ssh -p1004 ${node} "export WORK_HOME=${WORK_HOME}; cd ${IFACE_D_PUBLIC} 2>/dev/null 1>&1; ${SUDO} `which find` . -type f -exec cksum {} \; 2>/dev/null" 2>/dev/null 1>/${TMP}/.crc_testigo.none
			if [[ $? -ne 0 ]] ; then
				touch ${TREX_DIR}/.repeat
				LogPrint "Transference $$ error."
				return -1 ; exit
			fi

			for FILE in $(cat $TESTIGO_FINAL)
			do
				S_CRC=$(${SUDO} ${CKSUM} ${TREX_DIR}/${FILE} 2>/dev/null| awk -F\  '{print $1}')
				D_CRC=$(cat ${TMP}/.crc_testigo.none 2>/dev/null| grep " $FILE$" | awk '{print $1}')
				if [[ "0$S_CRC" -ne "0$D_CRC" ]] ; then
					echo ""
					LogPrint "File $FILE CRC32 failed, you need to reinitiate the transference"
					ERROR=1
				fi
				printf "."
			done
			rm -rf ${TMP}/.crc_testigo.none 2>/dev/null
			echo ""
			# TAR_METHOD
			rm -rf ${TMP}/package.$$.tar
			if [[ $ERROR -eq 1 ]] ; then
				touch ${TREX_DIR}/.repeat
				rm -rf ${CRC_TESTIGO}
				return -1 ; exit
			fi
		fi
	done
}

BackUp()
{
	SUBDIR=$(date +"%Y")"/"$(date +"%m")
	mkdir -p $2/${SUBDIR}/ 2>/dev/null
	cd $1 1>/dev/null 2>&1
	${SUDO} ${TAR} -cf $2/${SUBDIR}/transfer_${tstamp}_$$.tar .
	${SUDO} ${COMPRESS} $2/${SUBDIR}/transfer_${tstamp}_$$.tar
	cd - 1>/dev/null 2>&1
}

################################
# Comprobemos que no se esten realizando trabajos previos remotos
for node in ${NODE_DEST}
do
	ssh -p 1004 ${node} "ls -at ${IFACE_D_PUBLIC}/.lock 2>/dev/null" 2>/dev/null 1>&2
	if [[ $? -eq 0 ]] ; then
		LogPrint "Remote host is procesing old transferences. Transference $$ error"
		return -1; exit
	fi
done


################################
# Comprobamos Internal Lock 
if [[ -r ${IFACE_PUBLIC}/.lock ]] ; then
	LogPrint "Local transference in progress. Transference $$ error"
	return -1; exit
fi

################################
# NO _ DEV - case
if [[ "${NODE_TYPE}" != "DEV" ]] ; then
    if [[ "$1" -gt 0 ]] ; then
        DF_TESTIGO=${TMP}/.df_testigo.$1
        LDF_TESTIGO=${TMP}/.ldf_testigo.$1
    else
        DF_TESTIGO=`ls -at ${TMP}/.?f_testigo.* 2>/dev/null | tail -1`
        LDF_TESTIGO=`ls -at ${TMP}/.l?f_testigo.* 2>/dev/null|tail -1`
    fi

    if [[ ! -r "${DF_TESTIGO}" ]] ; then
        LogPrint "Cannot open $DF_TESTIGO"
        return -1; exit;
    fi

    if [[ ! -r "${LDF_TESTIGO}" ]] ; then
        LogPrint "Cannot open $LDF_TESTIGO."
        return -1 ; exit
    fi

    if [[ ! -s "${LDF_TESTIGO}" ]] ; then
        LogPrint "$LDF_TESTIGO is empty"
        return -1 ; exit;
    fi

	for file in $(cat ${LDF_TESTIGO})
	do
		${SUDO} ${FILE} ${IFACE_PUBLIC}/${file} 2>/dev/null 1>&2
		if [[ $? -ne 0 ]] ; then
		LogPrint "file ${IFACE_PUBLIC}/${file} not found; ${DF_TESTIGO} removed"
		rm -rf ${DF_TESTIGO}
		return -1 ; exit;
		fi
	done
else
	DF_TESTIGO="${TMP}/.df_testigo.$$"
	LDF_TESTIGO="${TMP}/.ldf_testigo.$$"
fi

if [[ -r ${TREX_DIR}/.lock ]] ; then
	if [[ ! -r ${TREX_DIR}/.repeat ]] ; then
		LogPrint "Remote transference in progress. Transference $$ error"
		return -1 ; exit
	fi
	DF_TESTIGO=`ls -at ${TMP}/.?f_testigo.* 2>/dev/null | tail -1`
	LDF_TESTIGO=`ls -at ${TMP}/.l?f_testigo.* 2>/dev/null|tail -1`
	DirScan ${TREX_DIR}
	LogPrint "Starting transference $$ using file ${LDF_TESTIGO}"
else
	DirScan ${IFACE_PUBLIC};

	if [[ "$TESTIGO_f" -eq 0 ]] ; then
		CleanTemp
		return 0 ; exit
	fi
	LogPrint "Starting transference $$" 

	LockDir 0700 ${IFACE_PUBLIC}
	LockDir 0700 ${TREX_DIR}
	LogPrint "Moving ${TESTIGO_f} files to transfer directory"

	cd ${IFACE_PUBLIC}/ 2>/dev/null 1>&2 

	${SUDO} ${TAR} pcf - . | ( cd ${TREX_DIR} ; ${SUDO} ${TAR} pxf - ) 2>/dev/null
	if [[ "$PURGE_PUB" = "yes" ]] ; then
		${SUDO} ${CHOWN} -R ${USER} ${IFACE_PUBLIC}/*
		rm -rf * 2>/dev/null
	fi

	cd - 2>/dev/null 1>&2

	if [[ $? -ne 0 ]] ; then
		LogPrint "Local transference with errors. Transference $$ error"
		UnLockDir ${PUBLIC_PERMS} ${IFACE_PUBLIC}
		CleanTemp
		return -1 ; exit
	fi

	UnLockDir ${PUBLIC_PERMS} ${IFACE_PUBLIC}
	TESTIGO2_f=`${SUDO} ${FIND} ${TREX_DIR}/ -type f 2>/dev/null >/dev/null| grep -v "^./.lock"| grep -v "^./.repeat" | wc -l`

	if [[ "${TESTIGO2_f}" -ne 0 ]] ; then
		LogPrint "You have "`expr ${TESTIGO_f} - ${TESTIGO2_f}`" old files cumulated on ${TREX_DIR}";
	fi
fi


TESTIGO_FINAL=`ls -at ${TMP}/.ldf_testigo.final.* 2>/dev/null |tail -1`
if [[ -z $TESTIGO_FINAL ]] ; then
	TESTIGO_FINAL=${TMP}/.ldf_testigo.final.$$
fi

if [[ ${NODE_TYPE} = PROD ]] ; then
	MailNotify success $$ $TESTIGO_f
	LogPrint "Transference $$ successful"
	RET="received with"
else
	CopyFiles;
	RET="copied to ${NODE_DEST}(${NODE_DEST_TYPE}) with"
fi

#
# Historial
echo "${IDENT}: "`cat ${TESTIGO_FINAL} | wc -l | sed -e 's/\ //g'`" objects ${RET} Catalog TID $$" >> ${HIST_LOG}
mv -f ${TESTIGO_FINAL} ${HISTORY}/${tstamp}_catalogo_$$

# backup y  borrado de los archivos en trex_dir
if [[ "$BACKUP" = "yes" ]] ; then
	BackUp ${TREX_DIR} ${IFACE_BACKUP} 
	LogPrint "Backup taken with identification $$" 
fi

if [[ "$PURGE_TREX" = "yes" ]] ; then
	${SUDO} ${CHOWN} -R ${USER} ${TREX_DIR}/*
	rm -rf ${TREX_DIR}/*
fi

UnLockDir 750 ${TREX_DIR}

if [[ "$PURGE_PUB" = "yes" ]] ; then
	rm -rf ${TMP}/.crc* ${TMP}/.ldf* ${TMP}/.df* 2>/dev/null
fi

rm -rf ${TESTIGO_FINAL} 2>/dev/null
rm -rf ${LDF_TESTIGO} 2>/dev/null
rm -rf ${DF_TESTIGO} 2>/dev/null
rm -rf ${CRC_TESTIGO} 2>/dev/null
rm -rf ${TREX_DIR}/.repeat 2>/dev/null
MailNotify success $$ $TESTIGO_f
LogPrint "Transference $$ successful"

return 1
