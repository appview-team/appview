#! /bin/bash

DEBUG=0  # set this to 1 to capture the EVT_FILE for each test
ARCH=`uname -m`
FAILED_TEST_LIST=""
FAILED_TEST_COUNT=0

EVT_FILE="/go/events.log"
ERR_FILE="stderr.txt"
LOG_FILE="/tmp/appview.log"
touch $EVT_FILE

GO_MAJOR_VER=(`echo $GOLANG_VERSION | cut -d '.' -f2`)

starttest(){
    CURRENT_TEST=$1
    echo "==============================================="
    echo "             Testing $CURRENT_TEST             "
    echo "==============================================="
    ERR=0
}

evaltest(){
    echo "             Evaluating $CURRENT_TEST"
}

endtest(){
    if [ $ERR -eq "0" ]; then
        RESULT=PASSED
    else
        cat $LOG_FILE
        RESULT=FAILED
        FAILED_TEST_LIST+=$CURRENT_TEST
        FAILED_TEST_LIST+=" "
        FAILED_TEST_COUNT=$(($FAILED_TEST_COUNT + 1))
    fi

    echo "*************** $CURRENT_TEST $RESULT ***************"
    echo ""
    echo ""

    # copy the EVT_FILE to help with debugging
    if (( $DEBUG )) || [ $RESULT == "FAILED" ]; then
        cp $EVT_FILE $EVT_FILE.$CURRENT_TEST
        cp $LOG_FILE $LOG_FILE.$CURRENT_TEST
    fi

    rm $EVT_FILE
    rm -f $ERR_FILE
    rm -f $LOG_FILE
}

is_file() {
    if [ ! -f "$1" ] ; then
        echo "FAIL: File $1 does not exist"
        ERR+=1
    else
	echo "PASS: File exists"
    fi
}

export APPVIEW_PAYLOAD_ENABLE=true
export APPVIEW_PAYLOAD_HEADER=true

evalPayload(){
    PAYLOADERR=0
    echo "Testing that payload files don't contain tls for $CURRENT_TEST"
    for FILE in $(ls /tmp/*in /tmp/*out 2>/dev/null); do
        # Continue if there aren't any .in or .out files
        if [ $? -ne "0" ]; then
            continue
        fi

        hexdump -C $FILE | cut -c11-58 | \
                     egrep "7d[ \n]+0a[ \n]+1[4-7][ \n]+03[ \n]+0[0-3]"
        if [ $? -eq "0" ]; then
            echo "$FILE contains tls"
            PAYLOADERR=$(($PAYLOADERR + 1))
        fi
    done

    # There were failures.  Move them out of the way before continuing.
    if [ $PAYLOADERR -ne "0" ]; then
        echo "Moving payload files to /tmp/payload/$CURRENT_TEST"
        mkdir -p /tmp/payload/$CURRENT_TEST
        cp /tmp/*in /tmp/payload/$CURRENT_TEST
        cp /tmp/*out /tmp/payload/$CURRENT_TEST
        rm /tmp/*in /tmp/*out
    fi

    return $PAYLOADERR
}


#
# syscalls_unlinkat
#
starttest syscalls_unlinkat
cd /go/syscalls

appview -z ./unlinkat

evaltest
sleep 1
grep unlinkat $EVT_FILE | grep fs.delete > /dev/null
ERR+=$?

endtest

#
# syscalls_opfalse
#
starttest opfalse
cd /go/syscalls

appview -z ./opfalse

evaltest

grep opfalse $EVT_FILE | grep fs.delete > /dev/null
if [ $? -eq "0" ]; then
    ERR+=1
fi

grep opfalse $EVT_FILE | grep fs.error > /dev/null
if [ $? -ne "0" ]; then
    ERR+=1
fi

endtest

#
# syscalls_readdir
#
starttest syscalls_readdir
cd /go/syscalls

appview -z ./readdir

evaltest

grep readdir $EVT_FILE | grep fs.open > /dev/null
grep readdir $EVT_FILE | grep fs.close > /dev/null
ERR+=$?

endtest


#
# plainServerDynamic
#
starttest plainServerDynamic
cd /go/net
PORT=80

appview -z ./plainServerDynamic ${PORT} &

# this sleep gives the server a chance to bind to the port
# before we try to hit it with curl
sleep 1
curl http://localhost:${PORT}/hello
ERR+=$?

sleep 0.5
# This stops plainServerDynamic
pkill -f plainServerDynamic

# this sleep gives plainServerDynamic a chance to report its events on exit
sleep 1

evaltest

grep plainServerDynamic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep plainServerDynamic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
grep plainServerDynamic $EVT_FILE | grep net.close > /dev/null
ERR+=$?
grep plainServerDynamic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep plainServerDynamic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep plainServerDynamic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep plainServerDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep plainServerDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# plainServerDynamicPie
#
starttest plainServerDynamicPie
cd /go/net
PORT=81

appview -z ./plainServerDynamicPie ${PORT} &

# this sleep gives the server a chance to bind to the port
# before we try to hit it with curl
sleep 1
curl http://localhost:${PORT}/hello
ERR+=$?

sleep 0.5
# This stops plainServerDynamicPie
pkill -f plainServerDynamicPie

# this sleep gives plainServerDynamicPie a chance to report its events on exit
sleep 1

evaltest

grep plainServerDynamicPie $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep plainServerDynamicPie $EVT_FILE | grep net.open > /dev/null
ERR+=$?
grep plainServerDynamicPie $EVT_FILE | grep net.close > /dev/null
ERR+=$?
grep plainServerDynamicPie $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep plainServerDynamicPie $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep plainServerDynamicPie $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep plainServerDynamicPie $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep plainServerDynamicPie $EVT_FILE | grep http.resp > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


# Commented out for now, unable to repro test failures only seen in CI
##
## appview snapshot (same namespace)
##
#starttest "appview_snapshot"
#cd /go/net
#PORT=82
#
#appview run --backtrace -- ./plainServerStatic ${PORT} &
#sleep 2
#psd_pid=`pidof appviewdyn`
#
#kill -s SIGFPE $psd_pid
#sleep 2
#
#appview snapshot $psd_pid
#sleep 2
#
#evaltest
#
#is_file /tmp/appview/${psd_pid}/snapshot
#is_file /tmp/appview/${psd_pid}/info
## is_file /tmp/appview/${psd_pid}/core
#is_file /tmp/appview/${psd_pid}/cfg
#is_file /tmp/appview/${psd_pid}/backtrace
#
## Kill psd process
#kill $psd_pid
#
#endtest


#
# plainServerStatic
#
starttest plainServerStatic
cd /go/net
PORT=83
appview -z ./plainServerStatic ${PORT} &

# this sleep gives the server a chance to bind to the port
# before we try to hit it with curl
sleep 1
curl http://localhost:${PORT}/hello
ERR+=$?

sleep 0.5
# This stops plainServerStatic
pkill -f plainServerStatic

# this sleep gives plainServerStatic a chance to report its events on exit
sleep 1

evaltest

grep plainServerStatic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep plainServerStatic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
grep plainServerStatic $EVT_FILE | grep net.close > /dev/null
ERR+=$?
grep plainServerStatic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep plainServerStatic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep plainServerStatic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep plainServerStatic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep plainServerStatic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# tlsServerDynamic
#
starttest tlsServerDynamic
cd /go/net
PORT=4430
appview -z ./tlsServerDynamic ${PORT} &

# this sleep gives the server a chance to bind to the port
# before we try to hit it with curl
sleep 1
curl -k --key server.key --cert server.crt https://localhost:${PORT}/hello
ERR+=$?

sleep 0.5
# This stops tlsServerDynamic
pkill -f tlsServerDynamic

# this sleep gives tlsServerDynamic a chance to report its events on exit
sleep 1

evaltest

grep tlsServerDynamic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep net.close > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# tlsServerStatic
#
starttest tlsServerStatic
cd /go/net
PORT=4431
STRUCT_PATH=/go/net/go_offsets.txt
APPVIEW_GO_STRUCT_PATH=$STRUCT_PATH appview -z ./tlsServerStatic ${PORT} &

# this sleep gives the server a chance to bind to the port
# before we try to hit it with curl
sleep 1
curl -k --key server.key --cert server.crt https://localhost:${PORT}/hello
ERR+=$?

sleep 5
# This stops tlsServerStatic
pkill -f tlsServerStatic

# this sleep gives tlsServerStatic a chance to report its events on exit
sleep 1

evaltest

grep tlsServerStatic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep tlsServerStatic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
# dont wait for this, it's not always guaranteed in the test app's timeframe
#grep tlsServerStatic $EVT_FILE | grep net.close > /dev/null
#ERR+=$?
grep tlsServerStatic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep tlsServerStatic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep tlsServerStatic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep tlsServerStatic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# plainClientDynamic
#
starttest plainClientDynamic
cd /go/net
appview -z ./plainClientDynamic
ERR+=$?

# this sleep gives plainClientDynamic a chance to report its events on exit
sleep 1

evaltest

grep plainClientDynamic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep plainClientDynamic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
# dont wait for this, it's not always guaranteed in the test app's timeframe
#grep plainClientDynamic $EVT_FILE | grep net.close > /dev/null
#ERR+=$?
grep plainClientDynamic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep plainClientDynamic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep plainClientDynamic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep plainClientDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep plainClientDynamic $EVT_FILE | grep console > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# plainClientStatic
#
starttest plainClientStatic
cd /go/net
appview -z ./plainClientStatic
ERR+=$?

# this sleep gives plainClientStatic a chance to report its events on exit
sleep 1

evaltest

grep plainClientStatic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep plainClientStatic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
# dont wait for this, it's not always guaranteed in the test app's timeframe
#grep plainClientStatic $EVT_FILE | grep net.close > /dev/null
#ERR+=$?
grep plainClientStatic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep plainClientStatic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep plainClientStatic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep plainClientStatic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep plainClientStatic $EVT_FILE | grep console > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# plainClientStaticStripped
#
starttest plainClientStaticStripped
cd /go/net
appview -z ./plainClientStaticStripped
ERR+=$?

# this sleep gives plainClientStaticStripped a chance to report its events on exit
sleep 1

evaltest

# We don't support Go stripped executables < 1.13
if [ $GO_MAJOR_VER -lt 13 ]; then
    grep "Continuing without AppView." ${LOG_FILE}
    ERR+=$?
else
    grep plainClientStaticStripped $EVT_FILE | grep net.app > /dev/null
    ERR+=$?
    grep plainClientStaticStripped $EVT_FILE | grep net.open > /dev/null
    ERR+=$?
    # dont wait for this, it's not always guaranteed in the test app's timeframe
    #grep plainClientStaticStripped $EVT_FILE | grep net.close > /dev/null
    #ERR+=$?
    grep plainClientStaticStripped $EVT_FILE | grep fs.open > /dev/null
    ERR+=$?
    grep plainClientStaticStripped $EVT_FILE | grep fs.close > /dev/null
    ERR+=$?
    grep plainClientStaticStripped $EVT_FILE | grep http.req > /dev/null
    ERR+=$?
    grep plainClientStaticStripped $EVT_FILE | grep http.resp > /dev/null
    ERR+=$?
    grep plainClientStaticStripped $EVT_FILE | grep console > /dev/null
    ERR+=$?
fi

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# tlsClientDynamic
#
starttest tlsClientDynamic
cd /go/net
appview -z ./tlsClientDynamic
ERR+=$?

# this sleep gives tlsClientDynamic a chance to report its events on exit
sleep 1

evaltest

grep tlsClientDynamic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
# dont wait for this, it's not always guaranteed in the test app's timeframe
#grep tlsClientDynamic $EVT_FILE | grep net.close > /dev/null
#ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep console > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# tlsClientStatic
#
starttest tlsClientStatic
cd /go/net
APPVIEW_GO_STRUCT_PATH=$STRUCT_PATH appview -z ./tlsClientStatic
ERR+=$?

# this sleep gives tlsClientStatic a chance to report its events on exit
sleep 5

evaltest

grep tlsClientStatic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep tlsClientStatic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
# dont wait for this, it's not always guaranteed in the test app's timeframe
#grep tlsClientStatic $EVT_FILE | grep net.close > /dev/null
#ERR+=$?
grep tlsClientStatic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep tlsClientStatic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep tlsClientStatic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep tlsClientStatic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep tlsClientStatic $EVT_FILE | grep console > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


# 
# tlsClientDynamicHTTP1 
#
starttest tlsClientDynamicHTTP1
cd /go/net
GODEBUG=http2client=0,http2server=0 appview -z ./tlsClientDynamic
ERR+=$?

# this sleep gives tlsClientDynamic a chance to report its events on exit
sleep 1

evaltest

grep tlsClientDynamic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
# dont wait for this, it's not always guaranteed in the test app's timeframe
#grep tlsClientDynamic $EVT_FILE | grep net.close > /dev/null
#ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep tlsClientDynamic $EVT_FILE | grep console > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


# 
# tlsServerDynamicHTTP1 
#
starttest tlsServerDynamicHTTP1
cd /go/net
PORT=4433
GODEBUG=http2client=0,http2server=0 appview -z ./tlsServerDynamic ${PORT} &

# this sleep gives the server a chance to bind to the port
# before we try to hit it with curl
sleep 1
curl -k --key server.key --cert server.crt https://localhost:${PORT}/hello
ERR+=$?

sleep 0.5
# This stops tlsServerDynamic
pkill -f tlsServerDynamic

# this sleep gives tlsServerDynamic a chance to report its events on exit
sleep 1

evaltest

grep tlsServerDynamic $EVT_FILE | grep net.app > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep net.open > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep net.close > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep fs.open > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep fs.close > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep http.req > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?
grep tlsServerDynamic $EVT_FILE | grep http.resp > /dev/null
ERR+=$?

if [ $ERR -ge 1 ]; then
    cat $EVT_FILE
fi

evalPayload
ERR+=$?

endtest


#
# test_go_struct_server
#
starttest test_go_struct_server
cd /go

./test_go_struct.sh $STRUCT_PATH /go/net/tlsServerStatic Server
ERR+=$?

evaltest

touch $EVT_FILE
endtest


#
# test_go_struct_client
#
starttest test_go_struct_client
cd /go

./test_go_struct.sh $STRUCT_PATH /go/net/tlsClientStatic Client
ERR+=$?

evaltest

touch $EVT_FILE
endtest


#
# fileThread
#
starttest fileThread
cd /go/thread
appview -z ./fileThread
ERR+=$?
evaltest

grep fileThread $EVT_FILE > /dev/null
ERR+=$?

evalPayload
ERR+=$?

endtest


#
# signalHandlerDynamic
#
starttest signalHandlerDynamic
cd /go/signals
appview -z ./signalHandlerDynamic 2>${ERR_FILE}&
APPVIEW_PID=$!
ERR+=$?

sleep 1
kill -SIGCHLD $APPVIEW_PID &

# verify that process still exists
if ! ps -p $APPVIEW_PID > /dev/null; then
    echo "$APPVIEW_PID ps first fail signalHandlerDynamic"
    ERR+=1
fi

while kill -0 ${APPVIEW_PID} &> /dev/null; do
  kill -SIGKILL ${APPVIEW_PID}
  sleep 1
done

count=$(grep 'bad g' $ERR_FILE | wc -l)
if [ $count -ne 0 ] ; then
    ERR+=1
fi

endtest

#
# signalHandlerStatic
#
starttest signalHandlerStatic
cd /go/signals
appview -z ./signalHandlerStatic &>${ERR_FILE}&
APPVIEW_PID=$!
ERR+=$?
sleep 0.1

kill -SIGCHLD ${APPVIEW_PID}
sleep 1

# verify that process still exists
if ! ps -p ${APPVIEW_PID}; then
    echo "$APPVIEW_PID ps first fail signalHandlerStatic"
    ERR+=1
fi

count=$(grep 'bad g' $ERR_FILE | wc -l)
if [ $count -ne 0 ] ; then
    echo "$APPVIEW_PID bad g seen in $ERR_FILE before sigkill"
    cat $ERR_FILE
    ERR+=1
fi

while kill -0 ${APPVIEW_PID}; do
  echo "sending SIGKILL"
  kill -SIGKILL ${APPVIEW_PID}
  sleep 1
done

count=$(grep 'bad g' $ERR_FILE | wc -l)
if [ $count -ne 0 ] ; then
    echo "$APPVIEW_PID bad g seen in $ERR_FILE after sigkill"
    cat $ERR_FILE
    ERR+=1
fi

endtest

#
# signalHandlerStaticStripped
#
starttest signalHandlerStaticStripped
cd /go/signals
appview -z ./signalHandlerStaticStripped &>${ERR_FILE}&
APPVIEW_PID=$!
ERR+=$?
sleep 0.1

kill -SIGCHLD ${APPVIEW_PID}
sleep 1

# verify that process still exists
if ! ps -p ${APPVIEW_PID}; then
    echo "$APPVIEW_PID ps first fail signalHandlerStaticStripped"
    ERR+=1
fi

count=$(grep 'bad g' $ERR_FILE | wc -l)
if [ $count -ne 0 ] ; then
    echo "$APPVIEW_PID bad g seen in $ERR_FILE before sigkill"
    cat $ERR_FILE
    ERR+=1
fi

while kill -0 ${APPVIEW_PID}; do
  echo "sending SIGKILL"
  kill -SIGKILL ${APPVIEW_PID}
  sleep 1
done

count=$(grep 'bad g' $ERR_FILE | wc -l)
if [ $count -ne 0 ] ; then
    echo "$APPVIEW_PID bad g seen in $ERR_FILE after sigkill"
    cat $ERR_FILE
    ERR+=1
fi

endtest

#
# cgoDynamic
#
starttest cgoDynamic
cd /go/cgo
LD_LIBRARY_PATH=. appview -z ./cgoDynamic
ERR+=$?
evaltest

grep cgoDynamic $EVT_FILE > /dev/null
ERR+=$?

evalPayload
ERR+=$?

endtest


#
# cgoStatic
#
starttest cgoStatic
cd /go/cgo
appview -z ./cgoStatic
ERR+=$?
evaltest

grep cgoStatic $EVT_FILE > /dev/null
ERR+=$?

evalPayload
ERR+=$?

endtest

#
#  influxdb tests
#
if [ $ARCH != "aarch64" ]; then
dbfile="/go/influx/db/meta/meta.db"
influx_verbose=0

influx_start_server() {
    rm -f /go/influx/db/*.event

    if (( $influx_verbose )); then
        APPVIEW_EVENT_DEST=file:///go/influx/db/influxd.event appview -z $1 &
    else
        APPVIEW_EVENT_DEST=file:///go/influx/db/influxd.event appview -z $1 2>/dev/null &
    fi

    until test -e "$dbfile" ; do
	    sleep 1
    done

}

influx_eval() {
    sleep 2
    pkill -f $2

    pexist="influxd"
    until test -z "$pexist" ; do
	    sleep 2
	    pexist=`ps -ef | grep influxd | grep config`
    done

    evaltest
    cnt=`grep -c http.req /go/influx/db/influxd.event`
    if (test "$cnt" -lt $1); then
	    echo "ERROR: Server count is $cnt"
	    ERR+=1
    else
	    echo "Server Success count is $cnt"
    fi

    grep http.req /go/influx/db/influxd.event > /dev/null
    ERR+=$?

    if [ -e  "/go/influx/db/influxc.event" ]; then
        cnt=`grep -c http.req /go/influx/db/influxc.event`
        if (test "$cnt" -lt $1); then
	        echo "ERROR: Client count is $cnt"
	        ERR+=1
        else
	        echo "Client Success count is $cnt"
        fi
    fi

    rm -r /go/influx/db/*
    touch /go/influx/db/data.txt
    touch $EVT_FILE

    evalPayload
    ERR+=$?

    endtest

}

#
# influx static stress test
#
# we've seen that the stress test does not run repeatedly
# in a container/EC2 environment. There is a standalone
# script that will run stress, Commentend out here.
#
sleep 1

# This test has race conditions where it closes sockets
# while other threads are reading/writing data on them.
# APPVIEW_HTTP_SERIALIZE_ENABLE ensures that a close and
# read/write can't happen at the same time.
export APPVIEW_HTTP_SERIALIZE_ENABLE=true
unset APPVIEW_PAYLOAD_ENABLE
starttest influx_static_stress

influx_start_server "/go/influx/influxd_stat --config /go/influx/influxdb.conf"
sleep 3
APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/stress_test insert -n 1000000 -f


influx_eval 50 appview

unset APPVIEW_HTTP_SERIALIZE_ENABLE
export APPVIEW_PAYLOAD_ENABLE=true

#
# influx static TLS test
#
sleep 1
starttest influx_static_tls

influx_start_server "/go/influx/influxd_stat --config /go/influx/influxdb_ssl.conf"

APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event /go/influx/influx_stat -ssl -unsafeSsl -host localhost -execute 'CREATE DATABASE goats'
APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_stat -ssl -unsafeSsl -host localhost -execute 'SHOW DATABASES'

if (test $influx_verbose -eq 0); then
    sleep 1
fi

APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_stat -ssl -unsafeSsl -host localhost -import -path=/go/influx/data.txt -precision=s
APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_stat -ssl -unsafeSsl -host localhost -execute 'SHOW DATABASES'

influx_eval 2 appview


#
# influx dynamic TLS test
#
sleep 1
starttest influx_dynamic_tls

influx_start_server "/go/influx/influxd_dyn --config /go/influx/influxdb_ssl.conf"

APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_dyn -ssl -unsafeSsl -host localhost -execute 'CREATE DATABASE goats'
APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_dyn -ssl -unsafeSsl -host localhost -execute 'SHOW DATABASES'

influx_eval 2 influxd

#
# influx static clear test
#
sleep 1
starttest influx_static_clear

influx_start_server "/go/influx/influxd_stat --config /go/influx/influxdb.conf"

APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_stat -host localhost -execute 'CREATE DATABASE sheep'
APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_stat -host localhost -execute 'SHOW DATABASES'

influx_eval 2 appview


#
# influx dynamic clear test
#
sleep 1
starttest influx_dynamic_clear

influx_start_server "/go/influx/influxd_dyn --config /go/influx/influxdb.conf"

APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_dyn -host localhost -execute 'CREATE DATABASE sheep'
APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_dyn -host localhost -execute 'SHOW DATABASES'

if (test $influx_verbose -eq 0); then
    sleep 1
fi


APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_dyn -import -path=/go/influx/data.txt -precision=s
APPVIEW_EVENT_DEST=file:///go/influx/db/influxc.event appview -z /go/influx/influx_dyn -host localhost -execute 'SHOW DATABASES'

influx_eval 2 influxd

unset APPVIEW_PAYLOAD_ENABLE
unset APPVIEW_PAYLOAD_HEADER

# endif for aarch64
fi

#
# Done: print results
#
if (( $FAILED_TEST_COUNT == 0 )); then
    echo ""
    echo ""
    echo "*************** ALL TESTS PASSED ***************"
else
    echo "*************** SOME TESTS FAILED ***************"
    echo "Failed tests: $FAILED_TEST_LIST"
    echo "Refer to these files for more info:"
    for FAILED_TEST in $FAILED_TEST_LIST; do
        echo "  $EVT_FILE.$FAILED_TEST"
    done
fi

exit ${FAILED_TEST_COUNT}
