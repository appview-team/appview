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

<span id="notifications-events-got"></span>

## Notifications and Security Events for GOT Hooking

### Notify on GOT errors
ddd

<span id="notifications-events-network"></span>

## Notifications and Security Events for Network Security

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

### Notify on DNS inconsistency
This example manually creates a DNS query so that it can be manipulated in order to create something that looks like what malware might utilize in order to exfiltrate data with DNS. It is overly simplified in order to illustrate the point. In no way is this meant to mimic any specific malware. The query is sent to the Google DNS server at 8.8.8.8. A standard DNS header is defined. The question specifies an IPv4 address. The query is for wttr.in. It takes the form of "\004wttr\002in" as defined in RFC1035. Binary data is added to the end of the query. This is meant to mimic a few malware definitions which have utilized DNS to exfiltrate data, much of the time to a custom endpoint. Examples can be found in Mitre Attack. The outgoing packet must conform to the DNS spec while adding detail to be exfiltrated.

#### Run the demo
Edit a file called dns-example.c and paste the code below into that file.

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define DNS_SERVICE "8.8.8.8"
#define DNS_PORT 53
#define QNAME "\004wttr\002in"

// DNS full header definition per RFC 1035
/*
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

Question Field
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QNAME                      |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QTYPE                      |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QCLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

 */

struct dns_header
{
    unsigned short id; // identification number
    unsigned char qr :1;     // query/response flag
    unsigned char opcode :4; // purpose of message
    unsigned char aa :1;     // authoritative answer
    unsigned char tc :1;     // truncated message
    unsigned char rd :1;     // recursion desired

    unsigned char ra :1;     // recursion available
    unsigned char z :1;      // its z! reserved
    unsigned char rcode :4;  // response code
    unsigned char cd :1;     // checking disabled
    unsigned char ad :1;     // authenticated data
 
    unsigned short qdcount;  // number of question entries
    unsigned short ancount;  // number of answer entries
    unsigned short nscount;  // number of authority entries
    unsigned short arcount;  // number of resource entries
};

typedef struct dns_query_t {
    struct dns_header qhead;
    unsigned char name[];
} dns_query;

typedef struct question_t
{
    unsigned short qtype;
    unsigned short qclass;
} question;

int
main (int argc, char *argv[])
{
    int sockfd;
    int qlen;
    char *qname;
    unsigned char bad_exfil[8] = {0x12,0x34,0x56,0x78,0x9a,0x9b,0x9c,0x0};
    struct sockaddr_in serverAddr;
    struct dns_header *dnsq = NULL;
    char buffer[MAX_BUFFER_SIZE];

    // Point to the buffer
    dnsq = (struct dns_header *)buffer;

    dnsq->id = (unsigned short) htons(getpid());
    dnsq->qr = 0;     // A query
    dnsq->opcode = 0; // A standard query
    dnsq->aa = 0;     // Not Authoritative
    dnsq->tc = 0;     // Message is not truncated
    dnsq->rd = 1;     // Recursion Requested
    dnsq->ra = 0;     // Defined by the server
    dnsq->z = 0;
    dnsq->ad = 0;
    dnsq->cd = 0;
    dnsq->rcode = 0;
    dnsq->qdcount = htons(1); // 1 question
    dnsq->ancount = 0;
    dnsq->nscount = 0;
    dnsq->arcount = 0;

    // End of the DNS header where the question section starts
    qname =(char *)&buffer[sizeof(struct dns_header)];
    snprintf(qname, sizeof(QNAME), QNAME);
    strcat(qname, bad_exfil);

    question *qinfo = (question *)(qname + strlen(qname) + 1);
    qinfo->qtype = htons(1);   // type A
    qinfo->qclass = htons(1);  // class IN

    qlen = (int)(qname - buffer) + sizeof(question);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DNS_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(DNS_SERVICE);

    // Send the data
    if (sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("DNS query to %s for %s\n", DNS_SERVICE, qname);

    // Close the socket
    close(sockfd);

    exit(0);
}
```


Compile the code.
```
gcc -g dns-example.c -o dnsd
```

Execute the DNS request with appview
```
appview ./dnsd
appview events
```

You will receive a security event notifying of a DNS issue:


**dnsd sec sec.dns dns_name:��� host:XXX pid:232907 proc:dnsd reason:"DNS request with an illegal label character" unit:process**

Assuming you have notifications enabled to be sent to Slack, you will receive a notification in the configured Slack channel:

**Process dnsd (pid 232907) on host XXX encountered DNS request with an illegal label character**

### Notify on files ex-filtrated through network connections
ddd
