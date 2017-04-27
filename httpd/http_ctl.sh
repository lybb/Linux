#! /bin/bash

ROOT_PATH=`pwd`
BIN=$ROOT_PATH/httpd
CONF=${ROOT_PATH}/conf/httpd.conf
PID=$ROOT_PATH/httpd.pid

proc=`basename $0`
function usage()
{
	printf "%s [start(-s) | stop(-t) | restart(-rt)]\n" "$proc"
}

function start_ctl()
{
	[[ -f $PID ]] && {
		printf "httpd is exist, pid is $(cat $PID)\n"
		return
	}

	ip=$(grep -E '^IP:' $CONF | awk -F: '{print $2}')
	port=$(grep -E '^PORT:' $CONF | awk -F: '{print $2}')
	$BIN $ip $port
	pidof $(basename $BIN) > $PID
	printf "start done, pid is : $(cat $PID)...\n"
}

function stop_ctl()
{
	[[ ! -f $PID ]] && {
		printf "httpd is not exist!\n"
	}

	pid=$(cat $PID)
	kill -9 $pid
	rm -f $PID
	printf "stop success...\n"
}

[[ $# -ne 1 ]] && {
	usage
	exit 1	
}

case $1 in
	start | -s)
		start_ctl
	;;
	stop | -t)
		stop_ctl
	;;
	restart | -rt)
		stop_ctl
		start_ctl
	;;
	* )
		usage
		exit 2
		;;
esac
