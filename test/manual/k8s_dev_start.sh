#! /bin/bash

cd ~/appview-k8s-demo && ./stop.sh
docker rmi -f $(docker images -a -q)
docker volume prune
cd ~/appview && make all
ver=`~/appview/bin/linux/appview version --tag`
echo $ver
cd ~/appview && docker build -t cribl/appview:$ver -f docker/base/Dockerfile .
export APPVIEW_VER=$ver
cd ~/appview-k8s-demo && ./start.sh cribl
