#! /bin/bash

export APPVIEW_CRIBL_ENABLE=false
export APPVIEW_EVENT_METRIC=true
export APPVIEW_EVENT_DEST=file:///tmp/appview_peer.log
export LD_PRELOAD=./lib/linux/$(uname -m)/libappview.so

declare -i ERR=0

echo "================================="
echo "      UNIX Socket Peer Test      "
echo "================================="

./test/linux/unixpeertest -v -f /tmp/pass.pipe
ERR+=$?

if [ $ERR -eq "0" ]; then
    echo "Success"
else
    echo "Test Failed"
fi

exit ${ERR}
