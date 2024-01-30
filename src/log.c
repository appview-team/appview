#define _GNU_SOURCE
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "dbg.h"
#include "log.h"
#include "appviewstdlib.h"

struct _log_t
{
    transport_t* transport;
    cfg_log_level_t level;
};

log_t*
logCreate(void)
{
    log_t* log = appview_calloc(1, sizeof(log_t));
    if (!log) {
        DBG(NULL);
        return NULL;
    }

    log->level = DEFAULT_LOG_LEVEL;

    return log;
}

void
logDestroy(log_t** log)
{
    if (!log || !*log) return;
    log_t* l = *log;
    transportDestroy(&l->transport);
    appview_free(l);
    *log = NULL;
}

int
logSend(log_t* log, const char* msg, cfg_log_level_t mlevel)
{
    if (!log || !msg) return -1;

    if ((log->level == CFG_LOG_NONE) || (log->level > mlevel)) return 0;
    
    return transportSend(log->transport, msg, appview_strlen(msg));
}

cfg_log_level_t
logLevel(log_t* log)
{
    return (log) ? log->level : DEFAULT_LOG_LEVEL;
}

void
logFlush(log_t* log)
{
    if (!log) return;
    transportFlush(log->transport);
}

int
logNeedsConnection(log_t* log)
{
    if (!log) return 0;
    return transportNeedsConnection(log->transport);
}

transport_status_t 
logConnectionStatus(log_t* log)
{
    return transportConnectionStatus(log->transport);
}

int
logConnect(log_t* log)
{
    if (!log) return 0;
    return transportConnect(log->transport);
}

int
logConnection(log_t *log)
{
    if (!log) return -1;
    return transportConnection(log->transport);
}

int
logDisconnect(log_t* log)
{
    if (!log) return 0;
    return transportDisconnect(log->transport);
}

int
logReconnect(log_t* log)
{
    if (!log) return 0;
    return transportReconnect(log->transport);
}

void
logTransportSet(log_t* log, transport_t* transport)
{
    if (!log) return;

    // Don't leak if logTransportSet is called repeatedly
    transportDestroy(&log->transport);
    log->transport = transport;
}

void
logLevelSet(log_t* log, cfg_log_level_t level)
{
    if (!log) return;
    log->level = level;
}

