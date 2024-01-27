#!/bin/bash

ERR=0

#export APPVIEW_EVENT_DEST=file:///opt/test/logs/events.log
#/opt/test/bin/test_appview.sh
#ERR+=$?

#export APPVIEW_EVENT_DEST=file:///go/events.log
#/go/test_go.sh
#ERR+=$?

export APPVIEW_EVENT_DEST=file:///opt/test/logs/events.log
/opt/test/bin/test_tls.sh
ERR+=$?

exit ${ERR}
