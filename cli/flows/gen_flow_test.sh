#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

ARCH=$(uname -m)
GOBIN=$(pwd)/.gobin/$ARCH

BINDIR=../bin/linux/$ARCH
APPVIEW=${BINDIR}/appview

GO_BINDATA=${GOBIN}/go-bindata

$APPVIEW run -p -- curl --http1.1 -so /dev/null https://cribl.io/
cwd=$(pwd); cd $($APPVIEW hist -d) && $GO_BINDATA -pkg flows -o ${DIR}/flow_file_test.go -fs . payloads; cd -
