#! /bin/bash

DEBUG=0  # set this to 1 to capture the EVT_FILE for each test

FAILED_TEST_LIST=""
FAILED_TEST_COUNT=0

EVT_FILE="/opt/test-runner/logs/events.log"
TEMP_OUTPUT_FILE="/opt/test-runner/temp_output"
DUMMY_RULES_FILE="/opt/test-runner/dummy_rules"
touch $EVT_FILE
touch $TEMP_OUTPUT_FILE

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
        cp -f $EVT_FILE $EVT_FILE.$CURRENT_TEST
    fi

    rm -f $EVT_FILE
}

wait_for_proc_start(){
    local proc_name=$1
    for i in `seq 1 8`;
    do
        if pidof $proc_name > /dev/null; then
            echo "Process $proc_name started"
            return
        fi
        echo "Sleep for the $i time because $proc_name did not started"
        sleep 1
    done

    echo "Process $proc_name did not started"
    ERR+=1
}

cd /opt/test-runner

#
# Detach not viewed process
# 
starttest detachNotViewedProcess

top -b -d 1 > /dev/null &
sleep 1
TOP_PID=`pidof top`
appview --lddetach $TOP_PID 2> $TEMP_OUTPUT_FILE
if [ $? -eq 0 ]; then
    ERR+=1
fi

grep "error: libappview does not exist in this process" $TEMP_OUTPUT_FILE > /dev/null
if [ $? -ne 0 ]; then
    ERR+=1
fi

kill -9 $TOP_PID

endtest

#
# Detach unviewd process (library is loaded)
# 

starttest detachNotViewedProcessLibLoaded

APPVIEW_RULES=$DUMMY_RULES_FILE appview -z top -b -d 1 > /dev/null &
sleep 1
TOP_PID=`pidof top`

THREAD_NO=`ls /proc/$TOP_PID/task | wc -l`
if [ $THREAD_NO -ne 1 ]; then
    # Reporting thread should not be present
    ERR+=1
fi

ls -l /proc/$TOP_PID/fd | grep memfd:appview_anon > /dev/null
if [ $? -ne 0 ]; then
    # memfd shm segment should be present
    ERR+=1
fi

appview --lddetach $TOP_PID > $TEMP_OUTPUT_FILE
if [ $? -ne 0 ]; then
    ERR+=1
fi

grep "Already detached from pid $TOP_PID" $TEMP_OUTPUT_FILE > /dev/null
if [ $? -ne 0 ]; then
    ERR+=1
fi

THREAD_NO=`ls /proc/$TOP_PID/task | wc -l`
if [ $THREAD_NO -ne 1 ]; then
    # Reporting thread should not be present
    ERR+=1
fi

ls -l /proc/$TOP_PID/fd | grep memfd:appview_anon > /dev/null
if [ $? -ne 0 ]; then
    # memfd shm segment should be present
    ERR+=1
fi

kill -9 $TOP_PID

endtest

#
# Attach top unviewd process (library is loaded)
# 

starttest attachNotViewedProcessFirstAttach

APPVIEW_RULES=$DUMMY_RULES_FILE appview -z top -b -d 1 > /dev/null &
sleep 1
TOP_PID=`pidof top`

THREAD_NO=`ls /proc/$TOP_PID/task | wc -l`
if [ $THREAD_NO -ne 1 ]; then
    # Reporting thread should not be present
    ERR+=1
fi

# first attach
appview --ldattach $TOP_PID > $TEMP_OUTPUT_FILE
if [ $? -ne 0 ]; then
    ERR+=1
fi

grep "First attach to pid $TOP_PID" $TEMP_OUTPUT_FILE > /dev/null
if [ $? -ne 0 ]; then
    ERR+=1
fi
sleep 3
THREAD_NO=`ls /proc/$TOP_PID/task | wc -l`
if [ $THREAD_NO -ne 2 ]; then
    # Reporting thread should appear after first attach
    ERR+=1
fi

# Reattach
appview --ldattach $TOP_PID > $TEMP_OUTPUT_FILE
grep "Reattaching to pid $TOP_PID" $TEMP_OUTPUT_FILE > /dev/null
if [ $? -ne 0 ]; then
    ERR+=1
fi

sleep 3
THREAD_NO=`ls /proc/$TOP_PID/task | wc -l`
if [ $THREAD_NO -ne 2 ]; then
    # Reporting thread should still be present after reattach
    ERR+=1
fi

kill -9 $TOP_PID

endtest

#
# Top
#
starttest Top

top -b -d 1 > /dev/null &
sleep 1
TOP_PID=`pidof top`
appview --ldattach $TOP_PID
sleep 1
evaltest

grep '"proc":"top"' $EVT_FILE | grep fs.open > /dev/null
ERR+=$?

grep '"proc":"top"' $EVT_FILE | grep fs.close > /dev/null
ERR+=$?

START_MSG_NO=$(grep "libappviewver" "$EVT_FILE" | wc -l)
if [ $START_MSG_NO -lt 1 ]; then
    echo "Number of start msg wrong after attach $START_MSG_NO"
    ERR+=1
fi

# detach
appview --lddetach $TOP_PID
if [ $? -ne 0 ]; then
    ERR+=1
fi
# Wait for file be consumed and event flushed
sleep 10

EVT_FILESIZE=$(stat -c%s "$EVT_FILE")
START_MSG_NO=$(grep "libappviewver" "$EVT_FILE" | wc -l)
if [ $START_MSG_NO -lt 2 ]; then
    echo "Number of start msg wrong after detach $START_MSG_NO"
    ERR+=1
fi

# reattach
appview --ldattach $TOP_PID
if [ $? -ne 0 ]; then
    echo "Attach wrong status"
    ERR+=1
fi
# Wait for file be consumed and event flushed
sleep 10

REATTACH_FILESIZE=$(stat -c%s "$EVT_FILE")
START_MSG_NO=$(grep "libappviewver" "$EVT_FILE" | wc -l)
if [ $START_MSG_NO -lt 3 ]; then
    echo "Start msg wrong after reattach $START_MSG_NO"
    ERR+=1
fi

if [ $REATTACH_FILESIZE -le $EVT_FILESIZE ]; then
    echo "File size error"
    ERR+=1
fi

kill -9 $TOP_PID

endtest

#
# Python3 Web Server
#
starttest Python3ReattachEnv

python3 -m http.server 2> /dev/null &
sleep 1
PYTHON_PID=`pidof python3`

appview --ldattach $PYTHON_PID
curl http://localhost:8000
sleep 1
evaltest

grep -q '"proc":"python3"' $EVT_FILE > /dev/null
ERR+=$?

grep -q http.req $EVT_FILE > /dev/null
ERR+=$?

grep -q http.resp $EVT_FILE > /dev/null
ERR+=$?

appview --lddetach $PYTHON_PID
ERR+=$?
sleep 10

# Reset Event file size
> $EVT_FILE
EVT_FILE_FILESIZE=$(stat -c%s "$EVT_FILE")
if [ $EVT_FILE_FILESIZE -ne 0 ]; then
    echo "File size should equal 0 after reset"
    ERR+=1
fi

# reattach and update configuration with env variable
EVENT_DEST_NEW="/opt/test-runner/logs/events_new.log"

unset APPVIEW_EVENT_DEST
APPVIEW_EVENT_DEST=file://$EVENT_DEST_NEW appview --ldattach $PYTHON_PID
export APPVIEW_EVENT_DEST=file://$EVT_FILE

sleep 10

curl http://localhost:8000
sleep 1

grep -q http.req $EVENT_DEST_NEW > /dev/null
ERR+=$?

grep -q http.resp $EVENT_DEST_NEW > /dev/null
ERR+=$?

if [ $EVT_FILE_FILESIZE -ne 0 ]; then
    echo "File size should equal 0 after reattach"
    ERR+=1
fi

kill -9 $PYTHON_PID > /dev/null

endtest


#
# Java HTTP Server
#
starttest JavaHttpReattachCfg

cd /opt/java_http
java SimpleHttpServer 2> /dev/null &
sleep 1
JAVA_PID=`pidof java`
appview --ldattach $JAVA_PID
curl http://localhost:8000/status
sleep 1
evaltest

grep -q '"proc":"java"' $EVT_FILE > /dev/null
ERR+=$?

grep -q http.req $EVT_FILE > /dev/null
ERR+=$?

grep -q http.resp $EVT_FILE > /dev/null
ERR+=$?

grep -q fs.open $EVT_FILE > /dev/null
ERR+=$?

grep -q fs.close $EVT_FILE > /dev/null
ERR+=$?

grep -q net.open $EVT_FILE > /dev/null
ERR+=$?

grep -q net.close $EVT_FILE > /dev/null
ERR+=$?

appview --lddetach $JAVA_PID
ERR+=$?
sleep 10

# reattach and update configuration with another configuration file
# Reset Event file size
# > $EVT_FILE
# EVT_FILE_FILESIZE=$(stat -c%s "$EVT_FILE")
# if [ $EVT_FILE_FILESIZE -ne 0 ]; then
#     echo "File size should equal 0 after reset"
#     ERR+=1
# fi
# enable the code after fixing TODO in attachCmd
# CONF_NEW="/opt/test_config/appview_test_cfg.yml"
# EVENT_DEST_NEW="/opt/test-runner/logs/events_from_cfg.log"

# unset APPVIEW_EVENT_DEST
# APPVIEW_CONF_RELOAD=$CONF_NEW appview --ldattach $JAVA_PID
# export APPVIEW_EVENT_DEST=file://$EVT_FILE

# sleep 10

# curl http://localhost:8000
# sleep 1

# grep -q http.req $EVENT_DEST_NEW > /dev/null
# ERR+=$?

# grep -q http.resp $EVENT_DEST_NEW > /dev/null
# ERR+=$?

# EVT_FILE_FILESIZE=$(stat -c%s "$EVT_FILE")
# if [ $EVT_FILE_FILESIZE -ne 0 ]; then
#     echo "File size should equal 0 after reattach"
#     ERR+=1
# fi

kill -9 $JAVA_PID

sleep 1

endtest

#
# attach execve_test
#

starttest execve_test

cd /opt/exec_test/
./exec_test 0 &

wait_for_proc_start "exec_test"
EXEC_TEST_PID=`pidof exec_test`

appview --ldattach ${EXEC_TEST_PID}
if [ $? -ne 0 ]; then
    echo "attach failed"
    ERR+=1
fi

wait ${EXEC_TEST_PID}
sleep 2

egrep '"cmd":"/usr/bin/curl -I https://cribl.io"' $EVT_FILE > /dev/null
if [ $? -ne 0 ]; then
    echo "Curl event not found"
    cat $EVT_FILE
    ERR+=1
fi

endtest

#
# attach execv_test
#

starttest execv_test

cd /opt/exec_test/
./exec_test 1 &

wait_for_proc_start "exec_test"
EXEC_TEST_PID=`pidof exec_test`

appview --ldattach ${EXEC_TEST_PID}
if [ $? -ne 0 ]; then
    echo "attach failed"
    ERR+=1
fi

wait ${EXEC_TEST_PID}
sleep 2

egrep '"cmd":"/usr/bin/wget -S --spider --no-check-certificate https://cribl.io"' $EVT_FILE > /dev/null
if [ $? -ne 0 ]; then
    echo "Wget event not found"
    cat $EVT_FILE
    ERR+=1
fi

endtest


#
# Processes on the doNotAppViewList should not be actively viewed,
# unless we're explicitly instructed to.  By "explicitly instructed to"
# we mean 1) injected into or 2) on the allow list of a rules file.
#
# This is an integration test for src/wrap.c:getSettings()
#

#
# denied_proc_not_viewed_by_default
#
starttest denied_proc_not_viewed_by_default

cd /opt/implicit_deny/
export APPVIEW_RULES=false

# the doNotAppViewList is based on process name, this systemd-networkd
# is not the real thing.
appview -z ./systemd-networkd &
PID=$!

appview inspect --all | grep systemd-networkd
if [ $? -eq "0" ]; then
    echo "systemd-networkd is actively viewed but shouldn't be"
    ERR+=1
fi

kill $PID

endtest

#
# denied_proc_is_viewed_by_inject
#
starttest denied_proc_is_viewed_by_inject

cd /opt/implicit_deny/
export APPVIEW_RULES=false

# the doNotAppViewList is based on process name, this systemd-networkd
# is not the real thing.
./systemd-networkd &
PID=$!
appview attach $PID

appview inspect --all | grep systemd-networkd
if [ $? -ne "0" ]; then
    echo "systemd-networkd is not actively viewed but should be"
    ERR+=1
fi

kill $PID

endtest


#
# denied_proc_is_viewed_by_rules_file
#
starttest denied_proc_is_viewed_by_rules_file

cd /opt/implicit_deny/
export APPVIEW_RULES=${DUMMY_RULES_FILE}2
echo "allow:" >> $APPVIEW_RULES
echo "- procname: systemd-networkd" >> $APPVIEW_RULES

# the doNotAppViewList is based on process name, this systemd-networkd
# is not the real thing.
appview -z ./systemd-networkd &
PID=$!
sleep 1
appview inspect --all | grep systemd-networkd
if [ $? -ne "0" ]; then
    echo "systemd-networkd is not actively viewed but should be"
    ERR+=1
fi

kill $PID
rm $APPVIEW_RULES
unset APPVIEW_RULES

endtest

#
# Implicit allow proc is run in the presence of a filter file
#
# Only run on glibc because musl does not support ld.so.preload
printenv LIB_IS_GLIBC
if [ $? -eq "0" ]; then
    starttest implicit_allow

    # create ld.so.preload
    touch /etc/ld.so.preload
    chmod ga+w /etc/ld.so.preload
    echo /opt/appview/lib/linux/$(uname -m)/libappview.so > /etc/ld.so.preload

    # create a filter file
    cd /opt/implicit_allow
    export APPVIEW_FILTER=${DUMMY_FILTER_FILE}2
    echo "allow:" >> $APPVIEW_FILTER
    echo "- procname: foo" >> $APPVIEW_FILTER

    # 1) ld.so.preload enables libappview to be loaded in all procs
    # 2) the filter file allows only the process foo to be viewed
    # 3) the implicit allow list overrides the filter and allows runc to be viewed
    ./runc &

    appview inspect --all | grep runc
    if [ $? -ne "0" ]; then
        echo "runc is not actively viewed but should be"
        ERR+=1
    fi

    kill `pidof runc`
    rm /etc/ld.so.preload
    rm $APPVIEW_FILTER
    unset APPVIEW_FILTER

    endtest

    starttest exec_preload

    # create ld.so.preload
    touch /etc/ld.so.preload
    chmod ga+w /etc/ld.so.preload
    echo /opt/appview/lib/linux/$(uname -m)/libappview.so > /etc/ld.so.preload

    # create a filter file
    cd /opt/exec_test
    export APPVIEW_FILTER=${DUMMY_FILTER_FILE}2
    echo "allow:" >> $APPVIEW_FILTER
    echo "- procname: exec_test" >> $APPVIEW_FILTER

    echo "using filter file: $APPVIEW_FILTER"
    cat $APPVIEW_FILTER

    # libappview loaded due to ld.so.preload, interposed due to rules file
    ./exec_test 0 &

    wait_for_proc_start "exec_test"
    EXEC_TEST_PID=`pidof exec_test`

    wait ${EXEC_TEST_PID}
    sleep 2

    egrep '"cmd":"/usr/bin/curl -I https://cribl.io"' $EVT_FILE > /dev/null
    if [ $? -ne 0 ]; then
        echo "Curl event not found"
        ERR+=1
    fi

    rm /etc/ld.so.preload
    rm $APPVIEW_FILTER
    unset APPVIEW_FILTER

    endtest

# finish LIB_IS_GLIBC
fi

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
