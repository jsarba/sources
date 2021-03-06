#!/bin/bash
#
# Init file for SyslogDog server daemon
# description: SyslogDog server daemon
#
# processname: sldog
# config: /etc/cip-sldog.conf
# config: /etc/db-sldog.conf

# source function library
. /etc/rc.d/init.d/functions

RETVAL=0
prog="sldog"
SLDOG="/usr/local/sbin${prog}"

PID_NUM=`ps -fe| grep "[/| ]sldog$" | grep -v "grep" | awk -F\  '{print $2}'`

start()
{
	echo -n $"Starting $prog:"
	initlog -c "$SLDOG" && success || failure
	RETVAL=$?
	[ "$RETVAL" = 0 ] && touch /var/lock/subsys/sldog
	echo
}

stop()
{
	echo -n $"Stopping $prog:"
	kill -SIGINT $PID_NUM
	RETVAL=$?
	[ "$RETVAL" = 0 ] && rm -f /var/lock/subsys/sldog
	echo
}

reload()
{
	echo -n $"Reloading $prog:"
	kill -SIGHUP $PID_NUM
	RETVAL=$?
	[ "$RETVAL" = 0 ] && echo "Ok"
	echo
}

reconnect()
{
	echo -n $"Reconnecting $prog:"
	kill -SIGTTIN $PID_NUM
	RETVAL=$?
	[ "$RETVAL" = 0 ] && echo "Ok"
	echo
}


case "$1" in
	start)
		start
		;;
	stop)
		stop
		;;
	restart)
		stop
		sleep 1
		start
		;;
	reload)
		reload
		;;
	reconnect)
		reconnect
		;;
	*)
		echo $"Usage: $0 {start|stop|restart|reload|reconnect}"
		RETVAL=1
esac
exit $RETVAL
