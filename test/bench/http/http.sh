#!/bin/bash
# Requires nginx, appview, ab
# Configure /etc/nginx/nginx.conf with: worker_processes 1

unviewd_nginx() {
    printf "Unviewd Nginx:\n"
    sudo nginx -s stop
    sudo nginx
    PID=$(ps -ef | grep www-data | grep -v grep | awk '{ print $2 }')
    START=$(cat /proc/$PID/schedstat | awk '{ print $1 }')
    ab -c 1 -n 10000 http://127.0.0.1:80/ > /dev/null 2>&1
    END=$(cat /proc/$PID/schedstat | awk '{ print $1 }')
    printf "Start:\t%s\n" $START 
    printf "End:\t%s\n" $END 
    printf "Diff:\t%s\n" $(($END-$START)) 
}

viewed_nginx() {
    printf "\nViewed Nginx:\n"
    sudo nginx -s stop
    sudo ../../../bin/linux/x86_64/appview run -- nginx
    PID=$(ps -ef | grep www-data | grep -v grep | awk '{ print $2 }')
    START=$(cat /proc/$PID/schedstat | awk '{ print $1 }')
    ab -c 1 -n 10000 http://127.0.0.1:80/ > /dev/null 2>&1
    END=$(cat /proc/$PID/schedstat | awk '{ print $1 }')
    printf "Start:\t%s\n" $START 
    printf "End:\t%s\n" $END 
    printf "Diff:\t%s\n" $(($END-$START)) 
}

unviewd_nginx
viewed_nginx
