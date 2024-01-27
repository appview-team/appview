#! /bin/bash

DEBUG=0  # set this to 1 to capture the EVT_FILE for each test

FAILED_TEST_LIST=""
FAILED_TEST_COUNT=0

EVT_FILE="/opt/test/logs/events.log"

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
        cp $EVT_FILE $EVT_FILE.$CURRENT_TEST
    fi

    rm $EVT_FILE
    touch $EVT_FILE
}


################ BEGIN TESTS ################

#
# appview_cli_static
# 
starttest appview_cli_static

# Assert that appview is built as a static executable in order to be portable to Alpine/musl.
file $(realpath $(which appview)) | grep "statically linked"
if [ $? -eq 0 ]; then
	echo "PASS AppView is a static executable"
else
	echo "FAIL AppView isn't static!"
	ERR+=1
fi

evaltest

endtest

#
# appview_cli_runs
#
starttest appview_cli_runs

# Assert that the appview executable runs on alpine.
if [ appview ]; then
	echo "PASS AppView CLI is runnable"
else
	echo "FAIL AppView CLI won't run"
	ERR+=1
fi

evaltest

endtest


################ END TESTS ################


unset APPVIEW_PAYLOAD_ENABLE
unset APPVIEW_PAYLOAD_HEADER

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
