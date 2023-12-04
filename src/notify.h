#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#define SLACK_TOKEN_VAR "SCOPE_SLACKBOT_TOKEN"
#define SLACK_CHANNEL "SCOPE_SLACKBOT_CHANNEL"
#define SLACK_API_HOST "slack.com"
#define SLACK_API_PATH "/api/chat.postMessage"
#define SLACK_CHANNEL_ID "#notifications"
#define HTTPS_PORT 443

extern bool notify(const char *);

#endif // __NOTIFY_H__
