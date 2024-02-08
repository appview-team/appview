# Detecting Potential Security Issues in Applications

AppView has the ability to detect potential security issues.
These are grouped into a few categories:
- Files
- Network
- Operating System Security Policy
- GOT hooking

A number of the mechanisms are defined by detection steps
in Mitre Attack enterprise techniques.

There are 2 means for exposing results of security oriented
detection mechanisms:
- External tools
  - A number of external tools can be supported.
  - Initially Slack is the primary external tool supported for notification.
- Security events
  - AppView events of type security are created from security detection mechanisms.
  - The command 'appview events' will display results.
  - Refer to AppView docs for how to expose events.

Initially, a number of environment variables are used to manage configuration of
security detection mechanisms. The intent is to add related control to the
AppView config file as well. Refer to the AppView Library docs for details of
the environment variables used with security detection.

# Examples
These examples use shell commands to make this easy to reproduce.
In order to receive notifications in Slack an environment variable needs to be set.

## Getting started with detection examples
From a bash prompt start with:
...
export APPVIEW_SLACKBOT_TOKEN=xxx
...
Where xxx is the token for the Slack workspace to be used.
Refer to https://api.slack.com/tutorials/tracks/getting-a-token for a
description of how to located your API token from Slack.

The default Slack channel for security notifications is #notifications.
You will likely need to create the channel in order to use the default.
In order to use a different Slack channel set the environment variable
APPVIEW_SLACKBOT_CHANNEL to the channel to be used for notifications.

## Notifications and Security Events for File Security

### Notify on file write to a filename in the write array
...
APPVIEW_NOTIFY_FILE_WRITE="/tmp/test" appview run -- vi /tmp/test_write.txt "+:wq"
appview events | grep sec
...

### Notify on file read from a filename in the read array
...
echo "Test reads" > /tmp/test_read.txt
APPVIEW_NOTIFY_FILE_READ="/tmp/test" appview run -- cat /tmp/test_read.txt
appview events | grep sec
...

### Notify on spaces at the end of a file name
...
echo "Test spaces" > "/tmp/test_space.txt  "
APPVIEW_NOTIFY_FILE_READ="/tmp/test" appview run -- cat "/tmp/test_space.txt  "
appview events | grep sec
...

### Notify on setuid or setgid file permissions
...
echo "Test UID" > /tmp/test_suid.txt
chown +s /tmp/test_suid.txt
appview run -- cat /tmp/test_suid.txt
appview events | grep sec
...

### Notify on write attempts to executable files
...
echo "Test exec writes" > /tmp/test_ew.txt
chmod +x /tmp/test_ew.txt
appview run -- vi /tmp/test_ew.txt "+:wq"
appview events | grep sec
...

### Notify on the use of files in system dirs that have g/a write permissions
...
echo "Test sys writes" > /tmp/test_sw.txt
chmod ga+w /tmp/test_sw.txt
APPVIEW_NOTIFY_SYS_DIRS="/tmp/" appview run -- vi /tmp/test_write.txt "+:wq"
appview events | grep sec
...

### Notify on access to files that are owned by unknown users
...
echo "Test UID" > /tmp/test_user.txt
sudo chown 5000:6000 /tmp/test_user.txt
appview run -- cat /tmp/test_user.txt
appview events | grep sec
...

## Notifications and Security Events from Modifications to Operating System Security Policy
### Notify on attempts to modify security policy
Initially the detection mechanisms operate on the system functions setrlimit() and prctl(PR_SET_SECCOMP).
More detection mechanisms will be added.

Create a bash shell script with the following:
...
#! /bin/bash

ulimit -f 2048
..
We create a shell script for this because the command ulimit is a bash shell builtin command.

Execute the shell script created above:
...
LD_PRELOAD=Path_to_libview/libappview.so ./myScript.sh
...

Where Path_to_libview is the path representing the location where libappview.so has been installed.
Where myScript.sh represents the script created from above.

## Notifications and Security Events for GOT Hooking
### Notify on GOT errors
ddd

## Notifications and Security Events for Network Security
### Notify on DNS inconsistency
ddd

### Notify on network connections to an IP address black list entry
Find the IP address to use in the black list.
For example, get the IP address for wttr.in:
..
dig wttr.in
...

Note that the IP address, as of this writing, is 5.9.243.187.

Set the IP black list to the IP address of wttr.in and connect:
...
APPVIEW_NOTIFY_IP_BLACK="5.9.243.187" appview run -- curl wttr.in
appview events | grep sec
...

You will see the error:
curl: (7) Couldn't connect to server

### Notify on network connections to an IP address white list entry
Note that the IP white list will allow a connection to be made even if it would be blocked by a black list entry.
Extending the behavior of the black list above:

...
APPVIEW_NOTIFY_IP_BLACK="5.9.243.187" APPVIEW_NOTIFY_IP_WHITE="5.9.243.187" appview run -- curl wttr.in
...

You will see normal output from wttr.in
No notifications or security events should be emitted.

### Notify on files ex-filtrated through network connections
ddd
