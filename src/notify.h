#ifndef __NOTIFY_H__
#define __NOTIFY_H__

// User config as definedin env vars
#define NOTIFY_IQ_VAR "APPIQ_NOTIFY"
#define NOTIFY_IQ_SEND "APPIQ_NOTIFY_SEND"
#define NOTIFY_IQ_EXIT "APPIQ_EXIT_ON_NOTIFY"
#define NOTIFY_IQ_LIBS "APPIQ_NOTIFY_LIBS"
#define NOTIFY_IQ_PRELOAD "APPIQ_NOTIFY_PRELOAD"
#define NOTIFY_IQ_FILES "APPIQ_NOTIFY_FILES"
#define NOTIFY_IQ_FUNCS "APPIQ_NOTIFY_FUNCS"
#define NOTIFY_IQ_NET "APPIQ_NOTIFY_NET"
#define NOTIFY_IQ_DNS "APPIQ_NOTIFY_DNS"

#define SLACK_TOKEN_VAR "SLACKBOT_TOKEN"
#define SLACK_CHANNEL "SLACKBOT_CHANNEL"
#define SLACK_API_HOST "slack.com"
#define SLACK_API_PATH "/api/chat.postMessage"
#define SLACK_CHANNEL_ID "#notifications"
#define HTTPS_PORT 443

// default behavior
#define DEFAULT_ENABLE TRUE
#define DEFAULT_SEND TRUE
#define DEFAULT_EXIT FALSE
#define DEFAULT_FILES TRUE
#define DEFAULT_NET TRUE
#define DEFAULT_FUNCS TRUE
#define DEFAULT_LIBS TRUE
#define DEFAULT_PRELOAD TRUE
#define DEFAULT_DNS TRUE

typedef enum {
    NOTIFY_LIBS,
    NOTIFY_PRELOAD,
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
