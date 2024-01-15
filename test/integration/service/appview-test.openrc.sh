#!/bin/bash

ERR=0

fail() { ERR+=1; echo >&2 "fail:" $@; }
ARCH=$(uname -m )

NGINX_ENV_CONF="/etc/conf.d/nginx"

FULL_NGINX_SERVICE_CFG="export\sLD_PRELOAD=/usr/lib/${ARCH}-linux-gnu/cribl/libappview.so\nexport\sAPPVIEW_HOME=/etc/appview/nginx"

rc-service nginx start

#Check modified configuration
appview service nginx --force --cribldest tls://example_instance.cribl.cloud:10090 &> /dev/null

if [ ! -f /etc/appview/nginx/appview.yml ]; then
    fail "missing appview.yml"
fi

if [ ! -f $NGINX_ENV_CONF ]; then
    fail "missing $NGINX_ENV_CONF"
fi

if [ ! -d /var/log/appview ]; then
    fail "missing /var/log/appview/"
fi

if [ ! -d /var/run/appview ]; then
    fail "missing /var/run/appview/"
fi

count=$(grep 'export LD_PRELOAD' $NGINX_ENV_CONF | wc -l)
if [ $count -ne 1 ] ; then
    fail "missing LD_PRELOAD in $NGINX_ENV_CONF"
fi

count=$(grep 'export APPVIEW_HOME' $NGINX_ENV_CONF | wc -l)
if [ $count -ne 1 ] ; then
    fail "missing APPVIEW_HOME in $NGINX_ENV_CONF"
fi

count=$(grep 'example_instance' /etc/appview/nginx/appview.yml | wc -l)
if [ $count -ne 1 ] ; then
    fail "Wrong configuration in appview.yml"
fi

pcregrep -q -M $FULL_NGINX_SERVICE_CFG $NGINX_ENV_CONF
if [ $? -ne "0" ]; then
    fail "missing $FULL_NGINX_SERVICE_CFG"
    cat $NGINX_ENV_CONF
fi

rc-service nginx stop
#Remove the nginx configuration file
rm $NGINX_ENV_CONF
rc-service nginx start

# #Check default configuration
appview service nginx --force &> /dev/null

count=$(grep 'example_instance' /etc/appview/nginx/appview.yml | wc -l)
if [ $count -ne 0 ] ; then
    fail "Wrong configuration in appview.yml"
fi

if [ ! -f $NGINX_ENV_CONF ]; then
    fail "missing $NGINX_ENV_CONF"
fi

count=$(grep 'export LD_PRELOAD' $NGINX_ENV_CONF | wc -l)
if [ $count -ne 1 ] ; then
    fail "missing LD_PRELOAD in $NGINX_ENV_CONF"
fi

count=$(grep 'export APPVIEW_HOME' $NGINX_ENV_CONF | wc -l)
if [ $count -ne 1 ] ; then
    fail "missing APPVIEW_HOME in $NGINX_ENV_CONF"
fi

pcregrep -q -M $FULL_NGINX_SERVICE_CFG $NGINX_ENV_CONF
if [ $? -ne "0" ]; then
    fail "missing $FULL_NGINX_SERVICE_CFG"
    cat $NGINX_ENV_CONF
fi

if [ $ERR -gt 0 ]; then
    echo "$ERR test(s) failed"
    exit $ERR
else
    echo "All test passed"
fi
