#!/usr/bin/bash
#
# pviewed - ps showing only viewed processes
#
# Handy with watch; i.e. `watch -n.5 sudo viewed.sh`
#
# Warning: the `grep /proc/*/maps` won't include results for processes you
# don't have access to so if you run this as a non-root user, you may miss some
# results. Consider running this under `sudu`.
#

LIBS=$(grep libappview /proc/*/maps 2>/dev/null | grep -v 'ldappview')

PIDS=$(sed 's/^\/proc\/\([0-9]*\).*/\1/' <<< $LIBS)

if [ -n "$PIDS" ]; then
    echo "AppView is loaded into the following processes."
    echo
    ps -f --pid $PIDS
else 
    echo "No viewed processes found."
fi

