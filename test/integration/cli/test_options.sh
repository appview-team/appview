#!/bin/bash
# 
# Cribl AppView Command-Line Option Tests
#

declare -i ERR=0

ARCH=$(uname -m)

run() {
    CMD="$@"
    echo "  \`${CMD}\`"
    OUT=$(${CMD} 2>&1)
    RET=$?
}

outputs() {
    if ! grep "$1" <<< "$OUT" >/dev/null; then
        echo "    * Expected \"$1\" in output of \`$CMD\`"
        ERR+=1
    fi
}

doesnt_output() {
    if grep "$1" <<< "$OUT" >/dev/null; then
        echo "    * Didn't expect \"$1\" in output of \`$CMD\`"
        ERR+=1
    fi
}

returns() {
    if [ "$RET" != "$1" ]; then
        echo "    * Expected \`$CMD\` to return $1, got $RET"
        ERR+=1
    fi
}

echo "================================="
echo "    Command Line Options Test"
echo "================================="

### Constructor Option Handling ###

run ./bin/linux/${ARCH}/appview -a 
outputs "error: missing required value for -a option"
returns 1

run ./bin/linux/${ARCH}/appview --ldattach
outputs "error: missing required value for -a option"
returns 1

run ./bin/linux/${ARCH}/appview -d 
outputs "error: missing required value for -d option"
returns 1

run ./bin/linux/${ARCH}/appview --lddetach
outputs "error: missing required value for -d option"
returns 1

run ./bin/linux/${ARCH}/appview -n
outputs "error: missing required value for -n option"
returns 1

run ./bin/linux/${ARCH}/appview --namespace
outputs "error: missing required value for -n option"
returns 1

run ./bin/linux/${ARCH}/appview -s
outputs "error: missing required value for -s option"
returns 1

run ./bin/linux/${ARCH}/appview --service
outputs "error: missing required value for -s option"
returns 1

run ./bin/linux/${ARCH}/appview -l 
outputs "error: missing required value for -l option"
returns 1

run ./bin/linux/${ARCH}/appview --libbasedir
outputs "error: missing required value for -l option"
returns 1

run ./bin/linux/${ARCH}/appview -p 
outputs "error: missing required value for -p option"
returns 1

run ./bin/linux/${ARCH}/appview --patch
outputs "error: missing required value for -p option"
returns 1

run ./bin/linux/${ARCH}/appview -z 
outputs "could not find or execute command"
returns 1

run ./bin/linux/${ARCH}/appview --passthrough
outputs "could not find or execute command"
returns 1

run ./bin/linux/${ARCH}/appview -z ps
outputs "PID"
returns 0

run ./bin/linux/${ARCH}/appview -z ps -ef
outputs "UID"
returns 0

run ./bin/linux/${ARCH}/appview -z -- ps
outputs "PID"
returns 0

run ./bin/linux/${ARCH}/appview -z -- ps -ef
outputs "UID"
returns 0

run ./bin/linux/${ARCH}/appview -a 1 -d 1
outputs "error: --ldattach and --lddetach cannot be used together"
returns 1

run ./bin/linux/${ARCH}/appview -s nginx -v nginx
outputs "error: --service and --unservice cannot be used together"
returns 1

run ./bin/linux/${ARCH}/appview -a 1 -z echo
outputs "error: --passthrough cannot be used with --ldattach/--lddetach or --namespace or --service/--unservice"
returns 1

run ./bin/linux/${ARCH}/appview -n 1 ls
outputs "error: --namespace option requires --service/--unservice or --mount option"
returns 1

run ./bin/linux/${ARCH}/appview -a dummy_service_value -s 1
outputs "error: --ldattach/--lddetach and --service/--unservice cannot be used together"
returns 1

run ./bin/linux/${ARCH}/appview -l /does_not_exist echo 
returns 1

run ./bin/linux/${ARCH}/appview --libbasedir /does_not_exist echo 
returns 1

run ./bin/linux/${ARCH}/appview -f /does_not_exist echo 
returns 1

run ./bin/linux/${ARCH}/appview -a not_a_pid
outputs "invalid --ldattach PID: not_a_pid"
returns 1

run ./bin/linux/${ARCH}/appview -d not_a_pid
outputs "invalid --lddetach PID: not_a_pid"
returns 1

run ./bin/linux/${ARCH}/appview -a -999
outputs "invalid --ldattach PID: -999"
returns 1

run ./bin/linux/${ARCH}/appview -d -999
outputs "invalid --lddetach PID: -999"
returns 1

run ./bin/linux/${ARCH}/appview -a 999999999
outputs "error: --ldattach, --lddetach PID not a current process"
returns 1

export APPVIEW_LIB_PATH=./lib/linux/${ARCH}/libappview.so
run ./bin/linux/${ARCH}/appview echo
returns 0
export -n APPVIEW_LIB_PATH

export APPVIEW_LIB_PATH=./lib/linux/${ARCH}/libappview.so
run ./bin/linux/${ARCH}/appview -a 999999999
outputs "error: --ldattach, --lddetach PID not a current process: 999999999"
returns 1
export -n APPVIEW_LIB_PATH



### Main Option Handling ###

run ./bin/linux/${ARCH}/appview echo foo
outputs foo
returns 0

run ./bin/linux/${ARCH}/appview run -- echo foo
outputs foo
returns 0

run ./bin/linux/${ARCH}/appview run -- ps -ef # doesn't work without the '--' (-ef parsed by cli instead) and never did
outputs UID
returns 0

run ./bin/linux/${ARCH}/appview run -a some_auth_token -- echo foo
outputs foo
returns 0

run ./bin/linux/${ARCH}/appview 
outputs Cribl AppView Command Line Interface
returns 0

run ./bin/linux/${ARCH}/appview -h
outputs Cribl AppView Command Line Interface
returns 0

run ./bin/linux/${ARCH}/appview --help
outputs Cribl AppView Command Line Interface
returns 0

run ./bin/linux/${ARCH}/appview logs -h
outputs Displays internal AppView logs for troubleshooting AppView itself.
returns 0

run ./bin/linux/${ARCH}/appview logs --help
outputs Displays internal AppView logs for troubleshooting AppView itself.
returns 0

run ./bin/linux/${ARCH}/appview run
outputs Usage:
returns 1

run ./bin/linux/${ARCH}/appview attach
outputs Usage:
returns 1

run ./bin/linux/${ARCH}/appview run -a
outputs error: missing required value for -a option
returns 1





if [ $ERR -eq "0" ]; then
    echo "Success"
else
    echo "Test Failed"
fi

exit ${ERR}
