#!/bin/bash --login

if [ "$1" = "test" ]; then
  exec /usr/local/appview/appview-test
fi

exec "$@"
