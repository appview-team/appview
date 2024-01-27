#ifndef __IPC_RESP_H__
#define __IPC_RESP_H__

#include "cJSON.h"

/*
 * meta_req_t describes the metadata part of ipc request command retrieves from IPC communication
 * IMPORTANT NOTE:
 * meta_req_t must be inline with client: metaReqCmd
 * Please extend `cmdMetaName` structure in ipc_resp.c
 */
typedef enum {
    META_REQ_JSON,            // JSON request (complete)
    META_REQ_JSON_PARTIAL,    // JSON request (partial)
} meta_req_t;

/*
 * ipc_appview_req_t describes the appview request command retrieves from IPC communication
 * IMPORTANT NOTE:
 * ipc_appview_req_t must be inline with client: appviewReqCmd
 * NEW VALUES MUST BE PLACED AFTER LAST SUPPORTED CMD AND BEFORE IPC_CMD_UNKNOWN
 * Please extend `cmdAppViewName` structure in ipc_resp.c
 */
typedef enum {
    IPC_CMD_GET_SUPPORTED_CMD,    // Retrieves the supported commands, introduced in: 1.3.0
    IPC_CMD_GET_APPVIEW_STATUS,     // Retrieves appview status of application (enabled or disabled), introduced in: 1.3.0
    IPC_CMD_GET_APPVIEW_CFG,        // Retrieves the current configuration, introduced in: 1.3.0
    IPC_CMD_SET_APPVIEW_CFG,        // Update the current configuration, introduced in: 1.3.0
    IPC_CMD_GET_TRANSPORT_STATUS, // Retrieves the transport status, introduced in: 1.3.0
    IPC_CMD_GET_PROC_DETAILS,     // Retrieves the process details, introduced in: 1.4.0
    // Place to add new message
    IPC_CMD_UNKNOWN,              // MUST BE LAST - points to unsupported message
} ipc_appview_req_t;

/*
 * Internal status of parsing the incoming message request, which can be:
 * - success after joining all the frames in the message
 * - error during parsing frames (processing will not wait for all frames)
 * - status is used both for ipc message and appview message
 */
typedef enum {
    REQ_PARSE_GENERIC_ERROR,            // Error: unknown error
    REQ_PARSE_ALLOCATION_ERROR,         // Error: memory allocation fails or empty queue
    REQ_PARSE_RECEIVE_ERROR,            // Error: general error during receive the message
    REQ_PARSE_RECEIVE_TIMEOUT_ERROR,    // Error: timeout during receive the message
    REQ_PARSE_MISSING_APPVIEW_DATA_ERROR, // Error: missing appview frame in request
    REQ_PARSE_JSON_ERROR,               // Error: request it not based on JSON format
    REQ_PARSE_REQ_ERROR,                // Error: msg frame issue with req field
    REQ_PARSE_UNIQ_ERROR,               // Error: msg frame issue with uniq field
    REQ_PARSE_REMAIN_ERROR,             // Error: msg frame issue with remain field
    REQ_PARSE_APPVIEW_REQ_ERROR,          // Error: appview frame issue with uniq field
    REQ_PARSE_APPVIEW_SIZE_ERROR,         // Error: appview frame issue with size
    REQ_PARSE_PARTIAL,                  // Request was succesfully parsed partial
    REQ_PARSE_OK,                       // Request was succesfully parsed
} req_parse_status_t;

// Forward declaration
typedef struct appviewRespWrapper appviewRespWrapper;

// This must be inline with respStatus in ipccmd.go
typedef enum {
    IPC_RESP_OK = 200,              // Response OK
    IPC_RESP_OK_PARTIAL_DATA = 206, // Response OK Partial Data
    IPC_BAD_REQUEST = 400,          // Invalid message syntax from client
    IPC_RESP_SERVER_ERROR = 500,    // Internal Server Error
    IPC_RESP_NOT_IMPLEMENTED = 501, // Method not implemented
} ipc_resp_status_t;

// String representation of appview response
char *ipcRespAppViewRespStr(appviewRespWrapper *);

// Wrapper for AppView responses
appviewRespWrapper *ipcRespGetAppViewCmds(const cJSON *);
appviewRespWrapper *ipcRespGetAppViewStatus(const cJSON *);
appviewRespWrapper *ipcRespGetAppViewCfg(const cJSON *);
appviewRespWrapper *ipcRespSetAppViewCfg(const cJSON *);
appviewRespWrapper *ipcRespGetTransportStatus(const cJSON *);
appviewRespWrapper *ipcRespGetProcessDetails(const cJSON *);
appviewRespWrapper *ipcRespStatusNotImplemented(const cJSON *);
appviewRespWrapper *ipcRespStatusAppViewError(ipc_resp_status_t);

// Wrapper destructor
void ipcRespWrapperDestroy(appviewRespWrapper *);

#endif // __IPC_RESP_H__
