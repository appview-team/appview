#! /bin/bash

rm -f /tmp/influx*
#nohup ./myscript 0<&- &> my.admin.log.file &
#APPVIEW_EVENT_DEST=file:///tmp/influxd.event nohup appview /go/influxd_stat &
./iserver.sh stat
APPVIEW_EVENT_DEST=file:///tmp/influxc.event appview ./influx_stress_stat
pkill -f appview

cnt=`grep -c http.req /tmp/influxd.event`
#echo "$cnt"
test "$cnt" -lt 2000 && echo "ERROR: Server" && exit 1
echo "Success: Server"

cnt=`grep -c http.req /tmp/influxc.event`
#echo "$cnt"
test "$cnt" -lt 2000 && echo "ERROR: Server" && exit 1
echo "Success: Client"
