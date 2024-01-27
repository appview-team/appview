#!/bin/bash

ERR=0

fail() { ERR+=1; echo >&2 "fail:" $@; }
ARCH=$(uname -m )

CRIBL_ENV_CONF="/etc/sysconfig/cribl"

FULL_CRIBL_CFG="LD_PRELOAD=/usr/lib/${ARCH}-linux-gnu/cribl/libappview.so\nAPPVIEW_HOME=/etc/appview/cribl"

#Check modified configuration
appview service cribl --force --cribldest tls://example_instance.cribl.cloud:10090 &> /dev/null

if [ ! -f /etc/appview/cribl/appview.yml ]; then
    fail "missing appview.yml"
fi

if [ ! -f $CRIBL_ENV_CONF ]; then
    fail "missing $CRIBL_ENV_CONF"
fi

if [ ! -d /var/log/appview ]; then
    fail "missing /var/log/appview/"
fi

if [ ! -d /var/run/appview ]; then
    fail "missing /var/run/appview/"
fi

count=$(grep 'LD_PRELOAD' $CRIBL_ENV_CONF | wc -l)
if [ $count -ne 1 ] ; then
    fail "missing LD_PRELOAD in $CRIBL_ENV_CONF"
fi

count=$(grep 'APPVIEW_HOME' $CRIBL_ENV_CONF | wc -l)
if [ $count -ne 1 ] ; then
    fail "missing APPVIEW_HOME in $CRIBL_ENV_CONF"
fi

pcregrep -q -M $FULL_CRIBL_CFG $CRIBL_ENV_CONF
if [ $? -ne "0" ]; then
    fail "missing $FULL_CRIBL_CFG"
    cat $CRIBL_ENV_CONF
fi

count=$(grep 'example_instance' /etc/appview/cribl/appview.yml | wc -l)
if [ $count -ne 1 ] ; then
    fail "Wrong configuration in appview.yml"
fi

#Remove the cribl configuration file
rm $CRIBL_ENV_CONF
touch $CRIBL_ENV_CONF
echo "EXAMPLE_ENV=FOO" >> $CRIBL_ENV_CONF

#Check default configuration
appview service cribl --force &> /dev/null

count=$(grep 'example_instance' /etc/appview/cribl/appview.yml | wc -l)
if [ $count -ne 0 ] ; then
    fail "Wrong configuration in appview.yml"
fi

count=$(grep 'LD_PRELOAD' $CRIBL_ENV_CONF | wc -l)
if [ $count -ne 1 ] ; then
    fail "missing LD_PRELOAD in $CRIBL_ENV_CONF"
fi

count=$(grep 'APPVIEW_HOME' $CRIBL_ENV_CONF | wc -l)
if [ $count -ne 1 ] ; then
    fail "missing APPVIEW_HOME in $CRIBL_ENV_CONF"
fi

pcregrep -q -M $FULL_CRIBL_CFG $CRIBL_ENV_CONF
if [ $? -ne "0" ]; then
    fail "missing $FULL_CRIBL_CFG"
    cat $CRIBL_ENV_CONF
fi

if [ $ERR -gt 0 ]; then
    echo "$ERR test(s) failed"
    exit $ERR
else
    echo "All test passed"
fi
