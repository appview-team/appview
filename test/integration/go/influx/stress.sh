#! /bin/bash

influx_verbose=1
appview_path="../../../../bin/linux/$(uname -m)/appview"
influx_path="./"
dbfile="$influx_path/db/meta/meta.db"

echo "==============================================="
echo "             Testing influx static stress      "
echo "==============================================="
ERR=0

rm -f $influx_path/db/*.event

if (( $influx_verbose )); then
    APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_DEST=file://$influx_path/db/influxd.event $appview_path $influx_path/influxd_stat --config $influx_path/stress_local.conf &
else
	APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_DEST=file://$influx_path/db/influxd.event $appview_path $influx_path/influxd_stat --config $influx_path/stress_local.conf 2>/dev/null &
fi
    
until test -e "$dbfile" ; do 
	sleep 1 
done

#sleep 2

APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_DEST=file://$influx_path/db/influxc.event $appview_path $influx_path/stress_test insert -n 1000000 -f
#APPVIEW_EVENT_HTTP=true $appview_path $influx_path/main insert -r 30s -f
#$influx_path/main insert -r 30s -f
#GODEBUG=schedtrace=100 APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_DEST=file://$influx_path/db/influxc.event $appview_path $influx_path/main insert -n 1000000 -f
#APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_DEST=file://$influx_path/db/influxc.event $appview_path $influx_path/influx_stress_stat
#APPVIEW_EVENT_HTTP=true $appview_path $influx_path/influx_stress_stat
ERR+=$?

#sleep 2

pkill -f appview

pexist="influxd"
until test -z "$pexist" ; do
	sleep 1
	pexist=`ps -ef | grep influxd | grep config`
done

cnt=`grep -c http.req $influx_path/db/influxd.event`
if (test "$cnt" -lt 100); then 
	echo "ERROR: Server count is $cnt"
	ERR+=1
else
	echo "Server Success count is $cnt"
fi

if [ -e  "$influx_path/db/influxc.event" ]; then
    cnt=`grep -c http.req $influx_path/db/influxc.event`
    if (test "$cnt" -lt 100); then 
	    echo "ERROR: Client count is $cnt"
	    ERR+=1
    else
	    echo "Client Success count is $cnt"
    fi
fi

rm -r $influx_path/db/*

if [ $ERR -eq "0" ]; then
    RESULT=PASSED
else
    RESULT=FAILED
    FAILED_TEST_LIST+=$CURRENT_TEST
    FAILED_TEST_LIST+=" "
    FAILED_TEST_COUNT=$(($FAILED_TEST_COUNT + 1))
fi

echo "*************** $CURRENT_TEST $RESULT ***************"
echo ""
echo ""
