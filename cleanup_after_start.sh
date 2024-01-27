#!/bin/bash
#
# Cleanup the environment after appview start 
#

PRELOAD_PATH="/etc/ld.so.preload"
LIBAPPVIEW_PATH="/usr/lib/libappview.so"
PROFILE_APPVIEW_SCRIPT="/etc/profile.d/appview.sh"
USR_APPVIEW_DIR="/usr/lib/appview/"
TMP_APPVIEW_DIR="/tmp/appview/"
CRIBL_APPVIEW_DIR="$CRIBL_HOME/appview/"

echo "Following script will try to remove following files:"
echo "- $PRELOAD_PATH"
echo "- $LIBAPPVIEW_PATH"
echo "- $PROFILE_APPVIEW_SCRIPT"
echo "- $USR_APPVIEW_DIR"
echo "- $TMP_APPVIEW_DIR"
echo "- \$CRIBL_HOME/appview/"

read -p "Continue (y/n)?" choice
case "$choice" in 
  y|Y ) echo "Yes selected - Continuing";;
  n|N ) echo "No selected - Exiting"; exit;;
  * ) echo "Unknown choice - Exiting"; exit;;
esac

if [ "$EUID" -ne 0 ]
  then echo "Please run script with sudo"
  exit
fi

if [ -f $PRELOAD_PATH ] ; then
    rm $PRELOAD_PATH
    echo "$PRELOAD_PATH file was removed"
else
    echo "$PRELOAD_PATH file was missing. Continue..."
fi

# This one is a symbolic link
if [ -L $LIBAPPVIEW_PATH ] ; then
    rm $LIBAPPVIEW_PATH
    echo "$LIBAPPVIEW_PATH file was removed"
else
    echo "$LIBAPPVIEW_PATH file was missing. Continue..."
fi

if [ -f $PROFILE_APPVIEW_SCRIPT ] ; then
    rm $PROFILE_APPVIEW_SCRIPT
    echo "$PROFILE_APPVIEW_SCRIPT file was removed"
else
    echo "$PROFILE_APPVIEW_SCRIPT file was missing. Continue..."
fi

if [ -d "$USR_APPVIEW_DIR" ] ; then
    rm -r $USR_APPVIEW_DIR
    echo "$USR_APPVIEW_DIR directory was removed"
else
    echo "$USR_APPVIEW_DIR directory was missing. Continue..."
fi

if [ -d "$TMP_APPVIEW_DIR" ] ; then
    rm -r $TMP_APPVIEW_DIR
    echo "$TMP_APPVIEW_DIR directory was removed"
else
    echo "$TMP_APPVIEW_DIR directory was missing."
fi

if [ -f $CRIBL_HOME ] && [ -d "$CRIBL_APPVIEW_DIR" ] ; then
    rm -r $CRIBL_APPVIEW_DIR
    echo "$CRIBL_APPVIEW_DIR directory was removed"
else
    echo "\$CRIBL_HOME was not set or \$CRIBL_HOME/appview directory was missing."
fi
