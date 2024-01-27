#ifndef __APPVIEWTYPES_H__
#define __APPVIEWTYPES_H__

#include <stdbool.h>

/***********************************************************************
 * Consider updating src/appviewtypes.h if you make changes to this file *
 ***********************************************************************/

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define C_STRLEN(a)  (sizeof(a) - 1)

#define FALSE 0
#define TRUE 1

#define APPVIEW_LIBAPPVIEW_PATH ("/usr/lib/libappview.so")
#define APPVIEW_RULES_USR_PATH ("/usr/lib/appview/appview_rules")
#define APPVIEW_USR_PATH "/usr/lib/appview/"
#define APPVIEW_TMP_PATH "/tmp/appview/"

#define DYN_CONFIG_CLI_DIR "/dev/shm"
#define DYN_CONFIG_CLI_PREFIX "appview_dconf"

#define SM_NAME "appview_anon"

#define APPVIEW_PID_ENV "APPVIEW_PID"

typedef enum {
    SERVICE_STATUS_SUCCESS = 0,         // service operation was success
    SERVICE_STATUS_ERROR_OTHER = 1,     // service was not installed
    SERVICE_STATUS_NOT_INSTALLED = 2    // service operation was failed
} service_status_t;

typedef enum {
    CFG_LOG_TRACE,
    CFG_LOG_DEBUG,
    CFG_LOG_INFO,
    CFG_LOG_WARN,
    CFG_LOG_ERROR,
    CFG_LOG_NONE
} cfg_log_level_t;

typedef struct {
    unsigned long cmdAttachAddr;
    bool viewed;
} export_sm_t;

#endif // __APPVIEWTYPES_H__
