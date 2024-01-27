#!/bin/sh

if [ ! -f /etc/nginx/appview-demo.com.crt ]; then
    openssl req -nodes -new -x509 -newkey rsa:2048 -keyout /etc/nginx/appview-demo.com.key -out /etc/nginx/appview-demo.com.crt -days 420 -subj '/O=AppView/C=US/CN=appview-demo.com' >/dev/null 2>/dev/null
fi

if [ -z "${APPVIEW_NO_NGINX}" ]; then
    appview nginx
fi

exec "$@"
