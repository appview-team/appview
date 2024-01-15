#! /bin/bash

APPVIEW_LIB=./lib/linux/$(uname -m)/libappview.so

# List of forbidden symbols
declare -a verboten_syms=(
"backtrace"
"secure_getenv"
"setenv"
)

declare -i EXIT_STATUS=0
echo "================================="
echo "      Verboten  Symbol Test      "
echo "================================="

for sym in "${verboten_syms[@]}"
do
    if nm "$APPVIEW_LIB" | grep -w "$sym";
    then
        EXIT_STATUS=1
        echo "Test failed symbol $sym should not be present in $APPVIEW_LIB"
    fi
done

if (( "$EXIT_STATUS" == 0 )); then
    echo "Success"
else
    echo "Failed"
fi

exit ${EXIT_STATUS}
