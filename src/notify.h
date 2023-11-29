#ifndef __NOTIFY_H__
#define __NOTIFY_H__

// User config as defined in env vars
// This env var is required
#define SLACK_TOKEN_VAR "SCOPE_SLACKBOT_TOKEN"

// These env vars will default if not set
#define NOTIFY_IQ_VAR "SCOPE_NOTIFY"
#define NOTIFY_IQ_SEND "SCOPE_NOTIFY_SEND"
#define NOTIFY_IQ_EXIT "SCOPE_EXIT_ON_NOTIFY"
#define NOTIFY_IQ_LIBS "SCOPE_NOTIFY_LIBS"
#define NOTIFY_IQ_FILES "SCOPE_NOTIFY_FILES"
#define NOTIFY_IQ_FUNCS "SCOPE_NOTIFY_FUNCS"
#define NOTIFY_IQ_NET "SCOPE_NOTIFY_NET"
#define NOTIFY_IQ_DNS "SCOPE_NOTIFY_DNS"
#define SLACK_CHANNEL "SCOPE_SLACKBOT_CHANNEL"

// Constants
#define SLACK_API_HOST "slack.com"
#define SLACK_API_PATH "/api/chat.postMessage"
#define HTTPS_PORT 443

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

typedef enum {
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
    bool dns;
} notify_info_t;


extern bool notify(notify_type_t, const char *);

#endif // __NOTIFY_H__
