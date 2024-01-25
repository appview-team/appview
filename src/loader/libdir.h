#ifndef _APPVIEW_LIBDIR_H
#define _APPVIEW_LIBDIR_H 1

#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

extern unsigned long g_libappviewsz;
extern unsigned long g_appviewdynsz;

// File types
typedef enum {
    LIBRARY_FILE,       // libappview.so
    LOADER_FILE,        // appviewdyn
    STATIC_LOADER_FILE  // appview
} libdirfile_t;

typedef enum {
    MKDIR_STATUS_CREATED = 0,           // Path was created
    MKDIR_STATUS_EXISTS = 1,            // Path already points to existing directory
    MKDIR_STATUS_ERR_PERM_ISSUE = 2,    // Error: Path already points to existing directory but user can not create file there
    MKDIR_STATUS_ERR_NOT_ABS_DIR = 3,   // Error: Path does not points to a directory
    MKDIR_STATUS_ERR_OTHER = 4,         // Error: Other
} mkdir_status_t;

mkdir_status_t libdirCreateDirIfMissing(const char *, mode_t, uid_t, gid_t);
int libdirCreateFileIfMissing(unsigned char *, size_t, libdirfile_t, const char *, bool, mode_t, uid_t, gid_t);
int libdirSetLibraryBase(const char *);                                      // Override default library base search dir
int libdirExtract(unsigned char *, size_t, uid_t, gid_t, libdirfile_t);                    // Extracts library file to default path
const char *libdirGetPath(libdirfile_t);                                     // Get full path to existing library file
size_t getAsset(libdirfile_t, unsigned char **);

// Unit Test helper
int libdirInitTest(const char *, const char *, const char *); // Override defaults

#endif // _APPVIEW_LIBDIR_H
