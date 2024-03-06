---
title: Threat Detection
---

<span id="detecting-issues"></span>

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

<span id="detection-examples"></span>

## Detection examples

These examples use shell commands to make this easy to reproduce.
In order to receive notifications in Slack an environment variable needs to be set.

From a bash prompt start with:
```
export APPVIEW_SLACKBOT_TOKEN=xxx
```
Where xxx is the token for the Slack workspace to be used.
Refer to https://api.slack.com/tutorials/tracks/getting-a-token for a
description of how to located your API token from Slack.

The default Slack channel for security notifications is #notifications.
You will likely need to create the channel in order to use the default.
In order to use a different Slack channel set the environment variable
APPVIEW\_SLACKBOT\_CHANNEL to the channel to be used for notifications.

<span id="notifications-events-file"></span>

## Notifications and Security Events for File Security

### Notify on file write to a filename in the write array
```
APPVIEW_NOTIFY_FILE_WRITE="/tmp/test" appview run -- vi /tmp/test_write.txt "+:wq"
appview events | grep sec
```

### Notify on file read from a filename in the read array
```
echo "Test reads" > /tmp/test_read.txt
APPVIEW_NOTIFY_FILE_READ="/tmp/test" appview run -- cat /tmp/test_read.txt
appview events | grep sec
```

### Notify on spaces at the end of a file name
```
echo "Test spaces" > "/tmp/test_space.txt  "
APPVIEW_NOTIFY_FILE_READ="/tmp/test" appview run -- cat "/tmp/test_space.txt  "
appview events | grep sec
```

### Notify on setuid or setgid file permissions
```
echo "Test UID" > /tmp/test_suid.txt
chown +s /tmp/test_suid.txt
appview run -- cat /tmp/test_suid.txt
appview events | grep sec
```

### Notify on write attempts to executable files
```
echo "Test exec writes" > /tmp/test_ew.txt
chmod +x /tmp/test_ew.txt
appview run -- vi /tmp/test_ew.txt "+:wq"
appview events | grep sec
```

### Notify on the use of files in system dirs that have g/a write permissions
```
echo "Test sys writes" > /tmp/test_sw.txt
chmod ga+w /tmp/test_sw.txt
APPVIEW_NOTIFY_SYS_DIRS="/tmp/" appview run -- vi /tmp/test_write.txt "+:wq"
appview events | grep sec
```

### Notify on access to files that are owned by unknown users
```
echo "Test UID" > /tmp/test_user.txt
sudo chown 5000:6000 /tmp/test_user.txt
appview run -- cat /tmp/test_user.txt
appview events | grep sec
```

<span id="notifications-events-os"></span>

## Notifications and Security Events from Modifications to Operating System Security Policy

### Notify on attempts to modify security policy
Initially the detection mechanisms operate on the system functions setrlimit() and prctl(PR\_SET\_SECCOMP).
More detection mechanisms will be added.

Create a bash shell script with the following:
```
#! /bin/bash

ulimit -f 2048
```
We create a shell script for this because the command ulimit is a bash shell builtin command.

Execute the shell script created above:
```
LD_PRELOAD=Path_to_libview/libappview.so ./myScript.sh
```

Where Path\_to\_libview is the path representing the location where libappview.so has been installed.
Where myScript.sh represents the script created from above.

<span id="notifications-events-network"></span>

## Notifications and Security Events for Network Security

### Notify on DNS inconsistency
ddd

### Notify on network connections to an IP address black list entry
Find the IP address to use in the black list.
For example, get the IP address for wttr.in:
```
dig wttr.in
```

Note that the IP address, as of this writing, is 5.9.243.187.

Set the IP black list to the IP address of wttr.in and connect:
```
APPVIEW_NOTIFY_IP_BLACK="5.9.243.187" appview run -- curl wttr.in
appview events | grep sec
```

You will see the error:
curl: (7) Couldn't connect to server

### Notify on network connections to an IP address white list entry
Note that the IP white list will allow a connection to be made even if it would be blocked by a black list entry.
Extending the behavior of the black list above:

```
APPVIEW_NOTIFY_IP_BLACK="5.9.243.187" APPVIEW_NOTIFY_IP_WHITE="5.9.243.187" appview run -- curl wttr.in
```

You will see normal output from wttr.in
No notifications or security events should be emitted.

### Notify on files ex-filtrated through network connections
ddd

<span id="notifications-events-got"></span>

## Notifications and Security Events for GOT Hooking

### A Brief Summary of the PLT/GOT
The PLT and GOT are sections within an ELF file that support dynamic linking. There are two types of binaries on any system: statically linked and dynamically linked. Statically linked binaries are self-contained, containing all of the code necessary for them to run within a single file, and do not depend on any external libraries. Dynamically linked binaries, the default, do not include a lot of functions, but rely on system libraries to provide a portion of the functionality. For example, when your binary uses printf to print some data, the actual implementation of printf is part of the system C library. Typically, on current GNU/Linux systems, this is provided by libc.so.6. A GOT entry provides direct access to the absolute address of a symbol, such as printf, without compromising position-independence and sharability. Because the executable file and shared objects have separate GOTs, a symbol may appear in several tables. The dynamic linker processes all the GOT relocations before giving control to any code in the process image, thus ensuring the absolute addresses are available during execution.

Much as the Global Offset Table redirects position-independent address calculations to absolute locations, the Procedure Linkage Table (PLT) redirects position-independent function calls to absolute locations. The linkage editor cannot resolve execution transfers (such as function calls) from one executable or shared object to another, so instead it arranges for the program to transfer control to entries in the PLT. The dynamic linker determines the absolute addresses of the destinations and stores them in the GOT, from which they are loaded by the PLT entry. Executable files and shared object files have separate PLTs.

### What is GOT Poisoning and Function Redirection
Function redirection is a mechanism that involves a function used normally by the process to be redirected to point to a nefarious function living in a parasite library. One example involves obfuscation of files. Any given malware may need to place files on a system while not allowing those files to be readily viewed. To accomplish this, the functions that read files from a directory, such as readdir, are redirected to a nefarious function contained in a parasite library. The nefarious readdir removes any reference to malware files and the user is unaware of their existence. 

There are number of ways in which function redirection is accomplished. Here we provide a brief definition of two high level approaches for deploying a nefarious function. One, an attacker-controlled, parasitic shared library is forced to be loaded into the target process, and causes any number of functions to be redirected. Two, a parasitic library is loaded when the process starts using dynamic loader functionality to cause an addition library to be loaded along with the required libraries. The simple example described below uses the second approach.


### Notify on GOT errors
The code below is compiled and creates a shared library. The resulting library is intended to be loaded by the dynamic loader and results in the function readdir being redirected to a nefarious readdir function that blocks the view of a defined file.

A nefarious readdir function is defined with default visibility. This causes the dynamic loader to create PLT/GOT entries for the nefarious readdir when this library is loaded. The dynamic linker is configured to preload this library before all other required libraries are loaded. This results in the nefarious readdir being the new default and used for all references in the process being started.

The constructor obtains the original address of readdir. The original readdir is called by the now default nefarious readdir, the results examined for the file to be obfuscated and removed. The result is that a user is not aware of the presence of the obfuscated file.

#### Run the demo
Edit a file called got-example.c and paste the code below into that file.


```
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>

#define BLOCKED_FILE "mycreds"

typedef struct dirent *(*ThisReaddir)(DIR *);
ThisReaddir g_readdir;

__attribute__((constructor)) void
init_obf(void)
{
    g_readdir = (ThisReaddir)dlsym(RTLD_NEXT, "readdir");
}

__attribute__((visibility("default"))) struct dirent *
readdir(DIR *dirp)
{
    if (!g_readdir) return NULL;

    struct dirent *dent = g_readdir(dirp);

    // if the current dir entry is blocked, ignore and get the next one
    if (dent && (strncmp(dent->d_name, BLOCKED_FILE, strlen(dent->d_name)) == 0)) {
        //printf("%s:%d %s\n", __FUNCTION__, __LINE__, dent->d_name);
        dent = g_readdir(dirp);...


    }

    return dent;
}
```

Compile the code.
```
gcc -fPIC -g -shared got-example.c -o libobf.so
```

Create a test file. The Example code obfuscates the existence of a file called mycreds.
```
touch /tmp/mycreds
```

Execute "ls" to see that the file mycreds exist
```
ls /tmp
```

Execute ls with the parasitic library preloaded
```
export LD_PRELOAD="`pwd`/libobf.so"
ls /tmp
```

Now the results from /tmp do not include the file mycreds. It has been obfuscated and does not appear to exist.


Enable AppView to detect GOT overlays
```
export LD_PRELOAD="`pwd`/libobf.so:/tmp/appview/dev/libappview.so"
```
Note that you want the parasitic library first in the preload list


Execute ls with appview
```
appview ls /tmp
appview events
```
You will receive a security event notifying of a GOT issue:

**sec sec.file file:/XXX/libobf.so host:XXX pid:203752 proc:ls reason:"a modification to the GOT to use lib /XXX/libobf.so for function readdir" unit:process write_bytes:0**

Assuming you have notifications enabled to be sent to Slack
```
ls /tmp
```
You will receive a notification in the configured Slack channel:

**Process ls (pid 203752) on host XXX encountered a modification to the GOT to use lib /XXX/libobf.so for function readdir**
