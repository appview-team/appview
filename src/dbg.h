#ifndef __DBG_H__
#define __DBG_H__

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "log.h"
#include "appviewtypes.h"

typedef struct _dbg_t dbg_t;

extern dbg_t* g_dbg;

// Constructors Destructors
void                 dbgInit(void);
void                 dbgDestroy(void);

// Accessors
unsigned long long   dbgCountAllLines(void);
unsigned long long   dbgCountMatchingLines(const char*);
void                 dbgDumpAll(FILE*);

// Setters
void                 dbgAddLine(const char* key, const char* fmt, ...);

// Variables
extern uint64_t g_cbuf_drop_count;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define DBG_FILE_AND_LINE __FILE__ ":" TOSTRING(__LINE__)

#define PRINTF_FORMAT(fmt_id, arg_id) __attribute__((format(printf, (fmt_id), (arg_id))))
#define UNREACHABLE() (__builtin_unreachable())

#ifdef static_assert
#define APPVIEW_BUILD_ASSERT(cond, msg) ({static_assert(cond, msg);})
#else
#define APPVIEW_BUILD_ASSERT(cond, msg)
#endif
//
//  The DBG macro is used to keep track of unexpected/undesirable
//  conditions as instrumented with DBG in the source code.  This is done
//  by storing the source file and line of every DBG macro that is executed.
//
//  At the most basic level, a count is incremented for each file/line.
//  In addition to this the time, errno, and optionally a string are stored
//  for the earliest and most recent time each file/line is hit.
//
//  Example uses:
//     DBG(NULL);                                    // No optional string
//     DBG("Should never get here");                 // Boring string
//     DBG("Hostname/port: %s:%d", hostname, port)   // Formatted string

#define DBG(...) dbgAddLine(DBG_FILE_AND_LINE, ## __VA_ARGS__)

//
//  Dynamic commands allow this information to be output from an actively
//  running process, with process ID <pid>.  It just runs dbgDumpAll(),
//  outputting the results to the file specified by APPVIEW_CMD_DBG_PATH.
//  To do this with default configuration settings, run this command and
//  output should appear in /tmp/mydbg.txt within a APPVIEW_SUMMARY_PERIOD:
//
//     echo "APPVIEW_CMD_DBG_PATH=/tmp/mydbg.txt" >> /tmp/appview.<pid>
//




// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// logging utilities
//

extern log_t *g_log;
extern proc_id_t g_proc;
extern bool g_constructor_debug_enabled;
extern bool g_ismusl;
extern bool g_isstatic;
extern bool g_isgo;
extern bool g_issighandler;
extern char g_libpath[];

void appviewLog(cfg_log_level_t, const char *, ...) PRINTF_FORMAT(2,3);
void appviewLogHex(cfg_log_level_t, const void *, size_t, const char *, ...) PRINTF_FORMAT(4,5);
void appviewLogDropItOnTheFloor(const char *, ...);

#define appviewLogError(...) appviewLog(CFG_LOG_ERROR, __VA_ARGS__)
#define appviewLogWarn(...)  appviewLog(CFG_LOG_WARN,  __VA_ARGS__)
#define appviewLogInfo(...)  appviewLog(CFG_LOG_INFO,  __VA_ARGS__)
#ifdef DEBUG
#define appviewLogDebug(...) appviewLog(CFG_LOG_DEBUG, __VA_ARGS__)
#define appviewLogTrace(...) appviewLog(CFG_LOG_TRACE, __VA_ARGS__)
#else
#define appviewLogDebug(...) appviewLogDropItOnTheFloor(__VA_ARGS__)
#define appviewLogTrace(...) appviewLogDropItOnTheFloor(__VA_ARGS__)
#endif

#define appviewLogHexError(...) appviewLogHex(CFG_LOG_ERROR, __VA_ARGS__)
#define appviewLogHexWarn(...)  appviewLogHex(CFG_LOG_WARN,  __VA_ARGS__)
#define appviewLogHexInfo(...)  appviewLogHex(CFG_LOG_INFO,  __VA_ARGS__)
#ifdef DEBUG
#define appviewLogHexDebug(...) appviewLogHex(CFG_LOG_DEBUG, __VA_ARGS__)
#define appviewLogHexTrace(...) appviewLogHex(CFG_LOG_TRACE, __VA_ARGS__)
#else
#define appviewLogHexDebug(...) appviewLogDropItOnTheFloor (__VA_ARGS__)
#define appviewLogHexTrace(...) appviewLogDropItOnTheFloor (__VA_ARGS__)
#endif

// Bit operations

#define APPVIEW_BIT_SET(base, bit_val)   ((base) |= (1ULL<<(bit_val)))
#define APPVIEW_BIT_CLEAR(base, bit_val) ((base) &= ~(1ULL<<(bit_val)))
#define APPVIEW_BIT_SET_VAR(base, bit_val, val) ((!!(val)) ? APPVIEW_BIT_SET(base, bit_val) : APPVIEW_BIT_CLEAR(base, bit_val))
#define APPVIEW_BIT_CHECK(base, bit_val) (!!((base) & (1ULL<<(bit_val))))

#endif // __DBG_H__
