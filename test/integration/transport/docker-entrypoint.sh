#!/bin/bash

openssl req -newkey rsa:2048 -nodes -keyout /tmp/appview.key -x509 -days 365 -out /tmp/appview.crt -subj '/C=US/ST=GA/L=Canton/O=AppView/OU=IT/CN=appview'

mkdir /tmp/out
/opt/cribl/bin/cribl start

if [ "$1" = "test" ]; then
  exec /opt/test/bin/appview-test
fi

exec "$@"
