#ifndef __NOTIFY_H__
#define __NOTIFY_H__

// User config as defined in env vars
// This env var is required
#define SLACK_TOKEN_VAR "APPVIEW_SLACKBOT_TOKEN"

// These env vars will default if not set
#define NOTIFY_IQ_VAR "APPVIEW_NOTIFY"
#define NOTIFY_IQ_SEND "APPVIEW_NOTIFY_SEND"
#define NOTIFY_IQ_EXIT "APPVIEW_EXIT_ON_NOTIFY"
#define NOTIFY_IQ_LIBS "APPVIEW_NOTIFY_LIBS"
#define NOTIFY_IQ_FILES "APPVIEW_NOTIFY_FILES"
#define NOTIFY_IQ_FUNCS "APPVIEW_NOTIFY_FUNCS"
#define NOTIFY_IQ_NET "APPVIEW_NOTIFY_NET"
#define NOTIFY_IQ_EXFIL "APPVIEW_NOTIFY_EXFIL"
#define NOTIFY_IQ_DNS "APPVIEW_NOTIFY_DNS"
#define NOTIFY_IQ_FILE_READ "APPVIEW_NOTIFY_FILE_READ"
#define NOTIFY_IQ_FILE_WRITE "APPVIEW_NOTIFY_FILE_WRITE"
#define NOTIFY_IQ_SYS_DIRS "APPVIEW_NOTIFY_SYS_DIRS"
#define NOTIFY_IQ_IP_WHITE "APPVIEW_NOTIFY_IP_WHITE"
#define NOTIFY_IQ_IP_BLACK "APPVIEW_NOTIFY_IP_BLACK"
#define NOTIFY_WHITE_BLOCK "APPVIEW_WHITE_LIST_BLOCK"
#define SLACK_CHANNEL "APPVIEW_SLACKBOT_CHANNEL"

// Constants
#define SLACK_API_HOST "slack.com"
#define SLACK_API_PATH "/api/chat.postMessage"
#define HTTPS_PORT 443
#define MAX_FILE_ENTRIES 32
#define MAX_IP_ENTRIES 16

// default behavior
#define SLACK_CHANNEL_ID "#notifications"
#define DEFAULT_ENABLE TRUE
#define DEFAULT_SEND TRUE
#define DEFAULT_EXIT FALSE
#define DEFAULT_FILES TRUE
#define DEFAULT_NET TRUE
#define DEFAULT_FUNCS TRUE
#define DEFAULT_LIBS TRUE
#define DEFAULT_DNS TRUE
#define DEFAULT_EXFIL TRUE
#define DEFAULT_FILE_WRITE "/usr/bin,/usr/lib,/var/lib,/lib/run,/sbin,/etc/ssh/sshd"
#define DEFAULT_FILE_READ ".ssh,/etc/passwd"
#define DEFAULT_SYS_DIRS "/usr/lib/,/var/lib/,/lib/run/,/sbin/,/usr/bin/,/bin/"
#define DEFAULT_IP_WHITE NULL
#define DEFAULT_IP_BLACK NULL
#define DEFAULT_WHITE_BLOCK FALSE

typedef enum {
    NOTIFY_INIT,
    NOTIFY_LIBS,
    NOTIFY_FILES,
    NOTIFY_FUNC,
    NOTIFY_NET,
    NOTIFY_DNS
} notify_type_t;

typedef struct {
    bool enable;
    bool exit;
    bool send;
    bool libs;
    bool preload;
    bool files;
    bool functions;
    bool network;
    bool exfil;
    bool dns;
    bool white_block;
    char *file_read[MAX_FILE_ENTRIES];
    char *file_write[MAX_FILE_ENTRIES];
    char *sys_dirs[MAX_FILE_ENTRIES];
    char *ip_white[MAX_IP_ENTRIES];
    char *ip_black[MAX_IP_ENTRIES];
} notify_info_t;

extern notify_info_t g_notify_def;
extern bool notify(notify_type_t, const char *);

#endif // __NOTIFY_H__
