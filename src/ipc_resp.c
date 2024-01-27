#define _GNU_SOURCE

#include "com.h"
#include "dbg.h"
#include "ipc_resp.h"
#include "appviewstdlib.h"
#include "runtimecfg.h"

static const char* cmdMetaName[] = {
    [META_REQ_JSON]         = "completeRequestJson",
    [META_REQ_JSON_PARTIAL] = "incompleteRequestJson",
};

static const char* cmdAppViewName[] = {
    [IPC_CMD_GET_SUPPORTED_CMD]    = "getSupportedCmd",
    [IPC_CMD_GET_APPVIEW_STATUS]     = "getAppViewStatus",
    [IPC_CMD_GET_APPVIEW_CFG]        = "getAppViewCfg",
    [IPC_CMD_SET_APPVIEW_CFG]        = "setAppViewCfg",
    [IPC_CMD_GET_TRANSPORT_STATUS] = "getTransportStatus",
    [IPC_CMD_GET_PROC_DETAILS]     = "getProcessDetails",
};

#define CMD_APPVIEW_SIZE  (ARRAY_SIZE(cmdAppViewName))

extern void doAndReplaceConfig(void *);
extern log_t *g_log;
extern mtc_t *g_mtc;
extern ctl_t *g_ctl;

// Wrapper for appview message response
// TODO: replace appviewRespWrapper with cJSON
struct appviewRespWrapper{
    cJSON *resp;                // AppView message response
};

/*
 * Creates the appview response wrapper object
 */
static appviewRespWrapper *
respWrapperCreate(void) {
    appviewRespWrapper *wrap = appview_calloc(1, sizeof(appviewRespWrapper));
    if (!wrap) {
        return NULL;
    }
    wrap->resp = NULL;
    return wrap;
}

/*
 * Destroys the appview response wrapper object
 */
void
ipcRespWrapperDestroy(appviewRespWrapper *wrap) {
    if (wrap->resp) {
        cJSON_Delete(wrap->resp);
    }

    appview_free(wrap);
}

/*
 * Returns the appview message response string representation
 */
char *
ipcRespAppViewRespStr(appviewRespWrapper *wrap) {
    return cJSON_PrintUnformatted(wrap->resp);
}

/*
 * Creates the wrapper for generic response (appview message and ipc message)
 * Used by following requests: IPC_CMD_UNKNOWN, IPC_CMD_SET_APPVIEW_CFG
 */
appviewRespWrapper *
ipcRespStatus(ipc_resp_status_t status) {
    appviewRespWrapper *wrap = respWrapperCreate();
    if (!wrap) {
        return NULL;
    }
    cJSON *resp = cJSON_CreateObject();
    if (!resp) {
        goto allocFail;
    }
    wrap->resp = resp;
    if (!cJSON_AddNumberToObjLN(resp, "status", status)) {
        goto allocFail;
    }

    return wrap;

allocFail:
    ipcRespWrapperDestroy(wrap);
    return NULL; 
}

/*
 * Creates descriptor for meta and appview command used in IPC_CMD_GET_SUPPORTED_CMD
 */
static cJSON*
createCmdDesc(int id, const char *name) {
    cJSON *cmdDesc = cJSON_CreateObject();
    if (!cmdDesc) {
        return NULL;
    }

    if (!cJSON_AddNumberToObject(cmdDesc, "id", id)) {
        cJSON_Delete(cmdDesc);
        return NULL;
    }

    if (!cJSON_AddStringToObject(cmdDesc, "name", name)) {
        cJSON_Delete(cmdDesc);
        return NULL;
    }

    return cmdDesc;
}

/*
 * Creates the wrapper for response to IPC_CMD_GET_SUPPORTED_CMD
 * TODO: use unused attribute later
 */
appviewRespWrapper *
ipcRespGetAppViewCmds(const cJSON * unused) {
    APPVIEW_BUILD_ASSERT(IPC_CMD_UNKNOWN == ARRAY_SIZE(cmdAppViewName), "cmdAppViewName must be inline with ipc_appview_req_t");

    appviewRespWrapper *wrap = respWrapperCreate();
    if (!wrap) {
        return NULL;
    }
    cJSON *resp = cJSON_CreateObject();
    if (!resp) {
        goto allocFail;
    }
    wrap->resp = resp;
    if (!cJSON_AddNumberToObjLN(resp, "status", IPC_RESP_OK)) {
        goto allocFail;
    }

    cJSON *metaCmds = cJSON_CreateArray();
    if (!metaCmds) {
        goto allocFail;
    }

    for (int id = 0; id < ARRAY_SIZE(cmdMetaName); ++id){
        cJSON *singleCmd = createCmdDesc(id, cmdMetaName[id]);
        if (!singleCmd) {
            goto metaCmdFail;
        }
        cJSON_AddItemToArray(metaCmds, singleCmd);
    }
    cJSON_AddItemToObjectCS(resp, "commands_meta", metaCmds);

    cJSON *appviewCmds = cJSON_CreateArray();
    if (!appviewCmds) {
        goto metaCmdFail;
    }

    for (int id = 0; id < ARRAY_SIZE(cmdAppViewName); ++id){
        cJSON *singleCmd = createCmdDesc(id, cmdAppViewName[id]);
        if (!singleCmd) {
            goto appviewCmdFail;
        }
        cJSON_AddItemToArray(appviewCmds, singleCmd);
    }
    cJSON_AddItemToObjectCS(resp, "commands_appview", appviewCmds);

    return wrap;

metaCmdFail:
    cJSON_Delete(metaCmds);

appviewCmdFail:
    cJSON_Delete(appviewCmds);

allocFail:
    ipcRespWrapperDestroy(wrap);
    return NULL; 
}

/*
 * Creates the wrapper for response to IPC_CMD_GET_APPVIEW_STATUS
 * TODO: use unused attribute later
 */
appviewRespWrapper *
ipcRespGetAppViewStatus(const cJSON *unused) {
    appviewRespWrapper *wrap = respWrapperCreate();
    if (!wrap) {
        return NULL;
    }
    cJSON *resp = cJSON_CreateObject();
    if (!resp) {
        goto allocFail;
    }
    wrap->resp = resp;
    if (!cJSON_AddNumberToObjLN(resp, "status", IPC_RESP_OK)) {
        goto allocFail;
    }
    if (!cJSON_AddBoolToObjLN(resp, "viewed", (g_cfg.funcs_attached))) {
        goto allocFail;
    }
    return wrap;

allocFail:
    ipcRespWrapperDestroy(wrap);
    return NULL; 
}

/*
 * Creates the wrapper for response to IPC_CMD_GET_APPVIEW_CFG
 * TODO: use unused attribute later
 */
appviewRespWrapper *
ipcRespGetAppViewCfg(const cJSON *unused) {
    appviewRespWrapper *wrap = respWrapperCreate();
    if (!wrap) {
        return NULL;
    }
    cJSON *resp = cJSON_CreateObject();
    if (!resp) {
        goto allocFail;
    }
    wrap->resp = resp;

    cJSON *cfg = jsonConfigurationObject(g_cfg.staticfg);
    if (!cfg) {
        if (!cJSON_AddNumberToObjLN(resp, "status", IPC_RESP_SERVER_ERROR)) {
            goto allocFail;
        }
        return wrap;
    }

    cJSON_AddItemToObjectCS(resp, "cfg", cfg);
    
    if (!cJSON_AddNumberToObjLN(resp, "status", IPC_RESP_OK)) {
        goto allocFail;
    }
    
    return wrap;

allocFail:
    ipcRespWrapperDestroy(wrap);
    return NULL;
}

/*
 * Creates the wrapper for response to IPC_CMD_UNKNOWN
 * TODO: use unused attribute later
 */
appviewRespWrapper *
ipcRespStatusNotImplemented(const cJSON *unused) {
    return ipcRespStatus(IPC_RESP_NOT_IMPLEMENTED);
}

/*
 * Process the request IPC_CMD_SET_APPVIEW_CFG
 */
static bool
ipcProcessSetCfg(const cJSON *appviewReq) {
    bool res = FALSE;
    // Verify if appview request is based on JSON-format
    cJSON *cfgKey = cJSON_GetObjectItem(appviewReq, "cfg");
    if (!cfgKey || !cJSON_IsObject(cfgKey)) {
        return res;
    }
    char *cfgStr = cJSON_PrintUnformatted(cfgKey);
    config_t *cfg = cfgFromString(cfgStr);
    doAndReplaceConfig(cfg);
    appview_free(cfgStr);
    res = TRUE;
    return res;
}

/*
 * Creates the wrapper for response to IPC_CMD_SET_APPVIEW_CFG
 */
appviewRespWrapper *
ipcRespSetAppViewCfg(const cJSON *appviewReq) {
    if (ipcProcessSetCfg(appviewReq)) {
        return ipcRespStatus(IPC_RESP_OK);
    }
    return ipcRespStatus(IPC_RESP_SERVER_ERROR);
}

/*
 * The interface*Func are functions accessors definitions, used to retrieve information about:
 * - enablement of specific interface
 * - transport status of specific interface
 */
typedef bool               (*interfaceEnabledFunc)(void);
typedef transport_status_t (*interfaceTransportStatusFunc)(void);

/*
 * singleInterface is structure contains the interface object
 */
struct singleInterface {
    const char *name;
    interfaceEnabledFunc enabled;
    interfaceTransportStatusFunc status;
};

/*
 * logTransportEnabled retrieves status if "log" interface is enabled
 */
static bool
logTransportEnabled(void) {
    return TRUE;
}

/*
 * logTransportStatus retrieves the status of "log" interface
 */
static transport_status_t
logTransportStatus(void) {
    return logConnectionStatus(g_log);
}

/*
 * payloadTransportEnabled retrieves status if "payload" interface is enabled
 */
static bool
payloadTransportEnabled(void) {
    return (ctlPayStatus(g_ctl) == PAYLOAD_STATUS_DISABLE) ? FALSE : TRUE;
}

/*
 * payloadTransportStatus retrieves the status of "payload" interface
 */
static transport_status_t
payloadTransportStatus(void) {
    return ctlConnectionStatus(g_ctl, CFG_LS);
}

/*
 * criblTransportEnabled retrieves status if "cribl" interface is enabled
 */
static bool
criblTransportEnabled(void) {
    return cfgLogStreamEnable(g_cfg.staticfg);
}

/*
 * eventsTransportEnabled retrieves the status of "events" interface
 */
static bool
eventsTransportEnabled(void) {
    return cfgEvtEnable(g_cfg.staticfg);
}

/*
 * eventsTransportStatus retrieves the status of "events" interface
 */
static transport_status_t
eventsTransportStatus(void) {
    return ctlConnectionStatus(g_ctl, CFG_CTL);
}

/*
 * metricTransportEnabled retrieves status if "metric" interface is enabled
 */
static bool
metricTransportEnabled(void) {
    return mtcEnabled(g_mtc);
}

/*
 * metricsTransportStatus retrieves the status of "metric" interface
 */
static transport_status_t
metricsTransportStatus(void) {
    return mtcConnectionStatus(g_mtc);
}

// Used for interface indexes!  Don't change the numbers!
enum InterfaceId {
    INDEX_LOG     = 0,
    INDEX_PAYLOAD = 1,
    INDEX_CRIBL   = 2,
    INDEX_EVENTS  = 3,
    INDEX_METRICS = 4,
};

static const
struct singleInterface appview_interfaces[] = {
    [INDEX_LOG]     = {.name = "log",     .enabled = logTransportEnabled,     .status = logTransportStatus},
    [INDEX_PAYLOAD] = {.name = "payload", .enabled = payloadTransportEnabled, .status = payloadTransportStatus},
    [INDEX_CRIBL]   = {.name = "cribl",   .enabled = criblTransportEnabled,   .status = eventsTransportStatus},
    [INDEX_EVENTS]  = {.name = "events",  .enabled = eventsTransportEnabled,  .status = eventsTransportStatus},
    [INDEX_METRICS] = {.name = "metrics", .enabled = metricTransportEnabled,  .status = metricsTransportStatus},
};

/*
 * Creates the wrapper for response to IPC_CMD_GET_TRANSPORT_STATUS
 * TODO: use unused attribute later
 */
appviewRespWrapper *
ipcRespGetTransportStatus(const cJSON *unused) {
    appviewRespWrapper *wrap = respWrapperCreate();
    if (!wrap) {
        return NULL;
    }
    cJSON *resp = cJSON_CreateObject();
    if (!resp) {
        goto allocFail;
    }
    wrap->resp = resp;
    if (!cJSON_AddNumberToObjLN(resp, "status", IPC_RESP_OK)) {
        goto allocFail;
    }

    cJSON *interfaces = cJSON_CreateArray();
    if (!interfaces) {
        goto allocFail;
    }

    for (int index = 0; index < ARRAY_SIZE(appview_interfaces); ++index) {
        int interfaceIndex = index;
        // Skip preparing the interface info if it is disabled
        if (appview_interfaces[interfaceIndex].enabled() == FALSE) {
            continue;
        }

        // if the cribl is last one there is no point to handle events and metrics
        if (interfaceIndex == INDEX_CRIBL) {
            index = ARRAY_SIZE(appview_interfaces);
        }

        cJSON *singleInterface = cJSON_CreateObject();
        if (!singleInterface) {
            goto interfaceFail;
        }

        transport_status_t status = appview_interfaces[interfaceIndex].status();

        if (!cJSON_AddStringToObject(singleInterface, "name", appview_interfaces[interfaceIndex].name)) {
            cJSON_Delete(singleInterface);
            goto interfaceFail;
        }

        if (!cJSON_AddStringToObject(singleInterface, "config", status.configString)) {
            cJSON_Delete(singleInterface);
            goto interfaceFail;
        }

        if (status.isConnected == TRUE) {
            if (!cJSON_AddTrueToObject(singleInterface, "connected")) {
                cJSON_Delete(singleInterface);
                goto interfaceFail;
            }
        } else {
            if (!cJSON_AddFalseToObject(singleInterface, "connected")) {
                cJSON_Delete(singleInterface);
                goto interfaceFail;
            }
            if (!cJSON_AddNumberToObject(singleInterface, "attempts", status.connectAttemptCount)) {
                cJSON_Delete(singleInterface);
                goto interfaceFail;
            }

            if (status.failureString) {
                if (!cJSON_AddStringToObject(singleInterface, "failure_details", status.failureString)) {
                    cJSON_Delete(singleInterface);
                    goto interfaceFail;
                }
            }
        }
        cJSON_AddItemToArray(interfaces, singleInterface);
    }
    cJSON_AddItemToObjectCS(resp, "interfaces", interfaces);
    return wrap;

interfaceFail:
    cJSON_Delete(interfaces);

allocFail:
    ipcRespWrapperDestroy(wrap);
    return NULL; 
}

/*
 * Creates the wrapper for response to IPC_CMD_GET_PROC_DETAILS
 * TODO: use unused attribute later
 */
appviewRespWrapper *
ipcRespGetProcessDetails(const cJSON *unused) {
    appviewRespWrapper *wrap = respWrapperCreate();
    if (!wrap) {
        return NULL;
    }
    cJSON *resp = cJSON_CreateObject();
    if (!resp) {
        goto allocFail;
    }
    wrap->resp = resp;
    if (!cJSON_AddNumberToObjLN(resp, "status", IPC_RESP_OK)) {
        goto allocFail;
    }

    if (!cJSON_AddNumberToObjLN(resp, "pid", g_proc.pid)) {
        goto allocFail;
    }

    if (!cJSON_AddStringToObject(resp, "uuid", g_proc.uuid)) {
        goto allocFail;
    }

    if (!cJSON_AddStringToObject(resp, "id", g_proc.id)) {
        goto allocFail;
    }

    if (!cJSON_AddStringToObject(resp, "machine_id", g_proc.machine_id)) {
        goto allocFail;
    }

    return wrap;

allocFail:
    ipcRespWrapperDestroy(wrap);
    return NULL; 
}
/*
 * Creates the wrapper for failed case in processing appview msg
 */
appviewRespWrapper *
ipcRespStatusAppViewError(ipc_resp_status_t status) {
    return ipcRespStatus(status);
}
