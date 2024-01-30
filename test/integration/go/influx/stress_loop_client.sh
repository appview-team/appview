#! /bin/bash

influx_verbose=0
appview_path="../../../../bin/linux/$(uname -m)/appview"
influx_path="./"
dbfile="$influx_path/db/meta/meta.db"

echo "==============================================="
echo "             Testing influx static stress      "
echo "==============================================="

rm -f $influx_path/db/*.event

if (( $influx_verbose )); then
#    APPVIEW_LOG_LEVEL=debug APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_METRIC=true APPVIEW_METRIC_ENABLE=false APPVIEW_EVENT_DEST=file://$influx_path/db/influxd.event $appview_path $influx_path/influxd_dyn --config $influx_path/stress_local.conf &
    APPVIEW_LOG_LEVEL=debug APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_METRIC=true APPVIEW_METRIC_ENABLE=false APPVIEW_EVENT_DEST=file://$influx_path/db/influxd.event $appview_path $influx_path/influxd_stat --config $influx_path/stress_local.conf &
#    $influx_path/influxd_stat --config $influx_path/stress_local.conf &
#    APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_METRIC=true APPVIEW_METRIC_ENABLE=false $appview_path $influx_path/influxd_stat --config $influx_path/stress_local.conf &
else
#    APPVIEW_LOG_LEVEL=debug APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_METRIC=true APPVIEW_METRIC_ENABLE=false APPVIEW_EVENT_DEST=file://$influx_path/db/influxd.event $appview_path $influx_path/influxd_dyn --config $influx_path/stress_local.conf 2> /dev/null &
    APPVIEW_LOG_LEVEL=debug APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_METRIC=true APPVIEW_METRIC_ENABLE=false APPVIEW_EVENT_DEST=file://$influx_path/db/influxd.event $appview_path $influx_path/influxd_stat --config $influx_path/stress_local.conf 2> /dev/null &
#	$influx_path/influxd_stat --config $influx_path/stress_local.conf 2>/dev/null &
#    APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_METRIC=true APPVIEW_METRIC_ENABLE=false $appview_path $influx_path/influxd_stat --config $influx_path/stress_local.conf 2> /dev/null&    
fi
    
until test -e "$dbfile" ; do
	sleep 1 
done

sleep 5

i=1

while [ 1 -eq 1 ] ;
do
    echo "   *************** Go stress $i times ***************"

    $influx_path/stress_test insert -r 30s -f
#    $influx_path/stress_test insert -r 30s    
#    APPVIEW_EVENT_HTTP=true APPVIEW_EVENT_DEST=file://$influx_path/db/influxc.event $appview_path $influx_path/stress_test insert -r 30s -f

    echo "*************** Stress test complete ***************"
    echo ""
    ((i=i+1))
done
