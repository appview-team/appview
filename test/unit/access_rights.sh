#! /bin/bash

export APPVIEW_CRIBL_ENABLE=false
export APPVIEW_EVENT_METRIC=true
export APPVIEW_EVENT_DEST=file:///tmp/appview_events.log
export LD_PRELOAD=./lib/linux/$(uname -m)/libappview.so

declare -i ERR=0

echo "================================="
echo "      Access Rights Test         "
echo "================================="

./test/linux/passfdtest -f /tmp/pass.pipe -1
ERR+=$?

./test/linux/passfdtest -f /tmp/pass.pipe -2
ERR+=$?

./test/linux/passfdtest -f /tmp/pass.pipe -3
ERR+=$?

./test/linux/passfdtest -f /tmp/pass.pipe -4
ERR+=$?

if [ $ERR -eq "0" ]; then
    echo "Success"
else
    echo "Test Failed"
fi

exit ${ERR}
