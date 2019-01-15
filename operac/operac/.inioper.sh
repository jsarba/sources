#!/usr/bin/ksh
trap "" 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18

export CONFIG="/home/operac/etc/operac.conf"
export WORK=`cat $CONFIG|sed -e '/^#/d'|sed -e '/^$/d'|grep WORKDIR|awk -F\  '{print $2}'`
export PATH=/usr/bin:/etc:/usr/sbin:/usr/ucb:/usr/bin/X11:/sbin:${WORK}/bin:${WORK}/sbin:${WORK}/lib:${WORK}/etc:${WORK}/logs

rm -rf ${WORK}/logs/.db*

umask 002
${WORK}/sbin/menu.sh ${WORK}/etc/menuoper
