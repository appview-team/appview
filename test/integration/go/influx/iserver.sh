#! /bin/bash

exec 0>/dev/null
exec 1>/dev/null
exec 2>/dev/null

APPVIEW_EVENT_DEST=file:///tmp/influxd.event appview ./influxd_$1 &
