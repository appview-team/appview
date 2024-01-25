#! /bin/bash
export APPVIEW_EVENT_DEST=file:///opt/test/logs/events.log

DEBUG=0  # set this to 1 to capture the EVT_FILE for each test
FAILED_TEST_LIST=""
FAILED_TEST_COUNT=0

starttest(){
    CURRENT_TEST=$1
    echo "=============================================="
    echo "             Testing $CURRENT_TEST            "
    echo "=============================================="
    ERR=0
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

    echo "******************* $RESULT *******************"
    echo ""
    echo ""
}

run() {
    CMD="$@"
    echo "\`${CMD}\`"
    OUT=$(${CMD} 2>&1)
    RET=$?
}

outputs() {
    if ! grep "$1" <<< "$OUT" >/dev/null; then
        echo "FAIL: Expected \"$1\" in output of \`$CMD\`, got $OUT"
        ERR+=1
    else
	echo "PASS: Output as expected"
    fi
}

doesnt_output() {
    if grep "$1" <<< "$OUT" >/dev/null; then
        echo "FAIL: Didn't expect \"$1\" in output of \`$CMD\`"
        ERR+=1
    else
	echo "PASS: Output as expected"
    fi
}

dbgOutput() {
    echo "[debug output] $OUT"
}

is_file() {
    if [ ! -f "$1" ] ; then
        echo "FAIL: File $1 does not exist"
        ERR+=1
    else
	echo "PASS: File exists"
    fi
}

is_dir() {
    if [ ! -d "$1" ] ; then
        echo "FAIL: Directory $1 does not exist"
        ERR+=1
    else
	echo "PASS: Directory exists"
    fi
}

returns() {
    if [ "$RET" != "$1" ]; then
        echo "FAIL: Expected \`$CMD\` to return $1, got $RET"
        ERR+=1
    else
	echo "PASS: Return value as expected"
    fi
}

viewedProcessNumber() {
    local procFound=$(($(appview ps | wc -l) - 1 ))

    echo $procFound
}

wordPresentInFile() {
    local word=$1
    local fileName=$2

    count=$(grep $word $fileName | wc -l)
    if [ $count -eq 0 ] ; then
        echo "FAIL: Value $1 is not present int $2"
        ERR+=1
    else
	    echo "PASS: Value $1 present in $2"
    fi
}

# wait maximum 30 seconds
waitForCmdviewedProcessNumber() {
    local expViewed=$1
    local retry=0
    local maxRetry=30
    local delay=1
    until [ "$retry" -ge "$maxRetry" ]
    do
        count=$(viewedProcessNumber)
        if [ "$count" -eq "$expViewed" ] ; then
            return
        fi
        retry=$((retry+1)) 
        sleep "$delay"
    done
    echo "FAIL: waiting for the number $expViewed viewed process $count"
    ERR+=1
}

################# START TESTS ################# 

#
# Attach by pid
#
starttest "Attach by pid"

# Run sleep
sleep 1000 & 
sleep_pid=$!

sleep 1

# Attach to sleep process
run appview attach $sleep_pid
dbgOutput
returns 0

# Wait for attach to execute
waitForCmdviewedProcessNumber 1

# Detach to sleep process by PID
run appview detach $sleep_pid
dbgOutput
outputs "Detaching from pid ${sleep_pid}"
returns 0

# Wait for detach to execute
waitForCmdviewedProcessNumber 0

# Reattach to sleep process by PID
run appview attach $sleep_pid
dbgOutput
outputs "Reattaching to pid ${sleep_pid}"
returns 0

# Wait for reattach to execute
waitForCmdviewedProcessNumber 1

# End sleep process
kill $sleep_pid

# Assert .appview directory exists
is_dir /root/.appview

# Assert sleep session directory exists (in /tmp)
is_dir /tmp/sleep_*${sleep_pid}*

# Assert sleep config file exists
is_file /tmp/sleep_*${sleep_pid}*/appview.yml

# Compare sleep config.yml files (attach and reattach) with expected.yml
for viewedirpath in /tmp/sleep_*${sleep_pid}_*; do
    viewedir=$(basename "$viewedirpath")
    cat $viewedirpath/appview.yml | sed -e "s/$viewedir/SESSIONPATH/" | diff - /expected.yml
    if [ $? -eq 0 ]; then
        echo "PASS: AppView sleep config as expected"
    else
        echo "Configuration file"
        cat $viewedirpath/appview.yml
        echo "FAIL: AppView sleep config not as expected"
        ERR+=1
    fi
done

endtest


#
# Attach by name
#
starttest "Attach by name"

# Run sleep
sleep 1000 & 
sleep_pid=$!

# Attach to sleep process
run appview attach sleep
outputs "Attaching to process ${sleep_pid}"
returns 0

endtest


#
# AppView ps
#
starttest "AppView ps"

# Wait for attach to execute
waitForCmdviewedProcessNumber 1

# AppView ps
run appview ps
outputs "ID	PID	USER	COMMAND
1	${sleep_pid} 	root	sleep 1000"
returns 0

endtest


#
# AppView start
#
starttest "AppView start"

# AppView start
run appview start
returns 0

endtest


#
# AppView detach by name
#
starttest "AppView detach by name"

echo "1" | appview detach sleep
RET=$?
returns 0

endtest

# Give time to consume configuration file (without sleep)
timeout 4s tail -f /dev/null


#
# AppView reattach by name
#
starttest "AppView reattach by name"

# reattach by name
run appview attach sleep
outputs "Reattaching to pid ${sleep_pid}"
returns 0

# Kill sleep process
kill $sleep_pid

endtest


#
# AppView detach all
#
starttest "AppView detach all"

# Run sleep
sleep 1000 & 
sleep_pid1=$!

# Run another sleep
sleep 1000 & 
sleep_pid2=$!

# Attach to sleep processes
run appview attach $sleep_pid1
returns 0
run appview attach $sleep_pid2
returns 0

# Wait for attach to execute
waitForCmdviewedProcessNumber 2

# Detach from sleep processes
yes | appview detach --all 2>&1
RET=$?
returns 0

endtest


##
## AppView daemon
##
#starttest "AppView daemon"
#
## Start a netcat listener
#nc -l -p 9109 > crash.out &
#sleep 1
#
## Start the appview daemon
#run appview daemon --filedest localhost:9109 &
#daemon_pid=$!
#sleep 2
#
## Run top
#top -b -d 1 > /dev/null &
#top_pid=$!
#sleep 1
#
## Attach to top
#run appview attach --backtrace --coredump $top_pid
#sleep 1
#
## Crash top
#kill -s SIGSEGV $top_pid
#sleep 5
#
## Check crash and snapshot files exist
#is_file /tmp/appview/${top_pid}/snapshot_*
#is_file /tmp/appview/${top_pid}/info_*
#is_file /tmp/appview/${top_pid}/core_*
#is_file /tmp/appview/${top_pid}/cfg_*
#is_file /tmp/appview/${top_pid}/backtrace_*
#
## Check files were received by listener
#wordPresentInFile "snapshot_" "crash.out"
#wordPresentInFile "info_" "crash.out"
#wordPresentInFile "cfg_" "crash.out"
#wordPresentInFile "backtrace_" "crash.out"
#
## Kill appview daemon process
#kill $daemon_pid
#
#endtest


##
## AppView snapshot (same namespace)
##
#starttest "AppView snapshot"
#
#top -b -d 1 > /dev/null &
#top_pid=$!
#sleep 2
#
#APPVIEW_SNAPSHOT_COREDUMP=true APPVIEW_SNAPSHOT_BACKTRACE=true appview --ldattach $top_pid
#returns 0
#sleep 2
#
#kill -s SIGSEGV $top_pid
#sleep 2
#
#run appview snapshot $top_pid
#returns 0
#sleep 2
#
#is_file /tmp/appview/${top_pid}/snapshot_*
#is_file /tmp/appview/${top_pid}/info_*
#is_file /tmp/appview/${top_pid}/core_*
#is_file /tmp/appview/${top_pid}/cfg_*
#is_file /tmp/appview/${top_pid}/backtrace_*
#
#endtest


#
# AppView Update from stdin (same namespace)
#
starttest "AppView update from stdin"

top -b -d 1 > /dev/null &
top_pid=$!
sleep 2

appview --ldattach $top_pid
returns 0
sleep 2

appview update $top_pid < /opt/test/bin/update_log_dest.yml
returns 0
sleep 3

kill $top_pid
sleep 2

is_file /tmp/top_events.log

endtest


#
# AppView Update from file path (same namespace)
#
starttest "AppView update from file path"

top -b -d 1 > /dev/null &
top_pid=$!
sleep 2

appview --ldattach $top_pid
returns 0
sleep 2

appview update $top_pid --config /opt/test/bin/update_log_dest.yml
returns 0
sleep 3

kill $top_pid
sleep 2

is_file /tmp/top_events.log

endtest


#
# AppView Report
#
starttest "AppView report"

unset APPVIEW_EVENT_DEST

appview run -- curl -Lso /dev/null https://wttr.in
sleep 1

run appview report
outputs "wttr.in"
outputs "/usr/lib/ssl/openssl.cnf"
outputs "/etc/ssl/certs/ca-certificates.crt"
outputs "/dev/null"

endtest
export APPVIEW_EVENT_DEST=file:///opt/test/logs/events.log


################# END TESTS ################# 

#
# Print test results
#
if (( $FAILED_TEST_COUNT == 0 )); then
    echo ""
    echo ""
    echo "************ ALL CLI TESTS PASSED ************"
else
    echo "************ SOME CLI TESTS FAILED ************"
    echo "Failed tests: $FAILED_TEST_LIST"
fi
echo ""

exit ${FAILED_TEST_COUNT}
