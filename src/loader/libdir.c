#define _XOPEN_SOURCE 500 // for FTW
#define _GNU_SOURCE

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#include "libdir.h"
#include "libver.h"
#include "loaderutils.h"
#include "loader.h"
#include "nsfile.h"
#include "patch.h"
#include "appviewtypes.h"
#include "setup.h"

#ifndef APPVIEW_VER
#error "Missing APPVIEW_VER"
#endif

#ifndef APPVIEW_LIBAPPVIEW_SO
#define APPVIEW_LIBAPPVIEW_SO "libappview.so"
#endif

#ifndef APPVIEW_DYN_NAME
#define APPVIEW_DYN_NAME "appviewdyn"
#endif

#ifndef APPVIEW_NAME
#define APPVIEW_NAME "appview"
#endif

#define APPVIEW_NAME_SIZE (16)
#define LIBAPPVIEW "github.com/appview-team/appview/run._buildLibappviewSo"
#define APPVIEWDYN "github.com/appview-team/appview/run._buildAppviewdyn"

// private global state
static struct
{
    char ver[PATH_MAX];          // contains raw version
    char install_base[PATH_MAX]; // full path to the desired install base directory
    char tmp_base[PATH_MAX];     // full path to the desired tmp base directory
} g_libdir_info = {
    .ver = APPVIEW_VER,                    // default version
    .install_base = "/usr/lib/appview", // default install base
    .tmp_base = "/tmp/appview",         // default tmp base
};

// internal state object structure
struct appview_obj_state{
    char binaryName[APPVIEW_NAME_SIZE];    // name of the binary
    char binaryBasepath[PATH_MAX];       // full path to the actual binary base directory i.e. /tmp/appview or /usr/lib/appview
    char binaryPath[PATH_MAX];           // full path to the actual binary file i.e. /tmp/appview/dev/libappview.so
};

// internal state for library
static struct appview_obj_state libappviewState = {
    .binaryName = APPVIEW_LIBAPPVIEW_SO,
};

// internal state for loader
static struct appview_obj_state appviewdynState = {
    .binaryName = APPVIEW_DYN_NAME,
};

// internal state for loader
static struct appview_obj_state appviewState = {
    .binaryName = APPVIEW_NAME,
};

// Representation of the .note.gnu.build-id ELF segment
typedef struct
{
    Elf64_Nhdr nhdr;  // Note section header
    char name[4];     // "GNU\0"
    char build_id[0]; // build-id bytes, length is nhdr.n_descsz
} note_t;

// from https://github.com/mattst88/build-id/blob/master/build-id.c
#define ALIGN(val, align) (((val) + (align)-1) & ~((align)-1))

// ----------------------------------------------------------------------------
// Internal
// ----------------------------------------------------------------------------

static struct appview_obj_state *
getObjState(libdirfile_t objFileType)
{
    switch (objFileType) {
    case LIBRARY_FILE:
        return &libappviewState;
        break;
    case STATIC_LOADER_FILE:
        return &appviewState;
        break;
    case LOADER_FILE:
        return &appviewdynState;
        break;
    }
    // unreachable
    return NULL;
}

/*
 * Update the @start parameter to point to the binary specified by @objFileType
 * Note: When the @objFileType parameter specifies the STATIC_LOADER_FILE,
 * the caller should munmap the memory afterwards.
 */
size_t
getAsset(libdirfile_t objFileType, unsigned char **start)
{
    if (!start) return -1;

    size_t len = -1;
    uint64_t *libsym;
    unsigned char *libptr;
    elf_buf_t *ebuf = NULL;
    size_t appviewSize;
    char path[PATH_MAX] = {0};

    if ((ebuf = getElf("/proc/self/exe")) == NULL) {
        fprintf(stderr, "error: can't read /proc/self/exe\n");
        goto out;
    }

    switch (objFileType) {
    case LIBRARY_FILE:
        if ((libsym = getSymbol(ebuf->buf, LIBAPPVIEW))) {
            libptr = (unsigned char *)*libsym;
        } else {
            fprintf(stderr, "%s:%d no addr for _buildLibappviewSo\n", __FUNCTION__, __LINE__);
            goto out;
        }

        *start = (unsigned char *)libptr;
        len =  g_libappviewsz;
        break;

    case LOADER_FILE:
        if ((libsym = getSymbol(ebuf->buf, APPVIEWDYN))) {
            libptr = (unsigned char *)*libsym;
        } else {
            fprintf(stderr, "%s:%d no addr for _buildViewedyn\appviewdyn: %s\n",
                    __FUNCTION__, __LINE__, APPVIEWDYN);
            goto out;
        }

        *start = (unsigned char *)libptr;
        len =  g_appviewdynsz;
        break;

    case STATIC_LOADER_FILE:
        if (readlink("/proc/self/exe", path, sizeof(path) - 1) == -1) {
            fprintf(stderr, "%s:%d readlink failed\n", __FUNCTION__, __LINE__);
            goto out;
        }
        libptr = (unsigned char *)setupLoadFileIntoMem(&appviewSize, path);
        if (libptr == NULL) {
            goto out;
        }

        *start = libptr;
        len = appviewSize;
        break;

    default:
        fprintf(stderr, "error: invalid objFileType\n");
        goto out;

    }

out:
    if (ebuf) {
        freeElf(ebuf->buf, ebuf->len);
        free(ebuf);
    }

    return len;
}

static int
libdirCreateSymLinkIfMissing(char *path, char *target, bool overwrite, mode_t mode, uid_t nsUid, gid_t nsGid)
{
    int ret;
    int errnoVal;

    // Check if file exists
    if (!access(path, R_OK) && !overwrite) {
        return 0; // File exists
    }

    uid_t currentEuid = geteuid();
    gid_t currentEgid = getegid();
    
    ret = nsFileSymlink(target, path, nsUid, nsGid, currentEuid, currentEgid, &errnoVal);
    if (ret && (errnoVal != EEXIST)) { 
        fprintf(stderr, "libdirCreateSymLinkIfMissing: symlink %s failed\n", path);
        return ret;
    }

    return 0;
}

/*
 * Create a file of type @objFileType if it does not exist yet
 * Optionally specify @file and @file_len arguments to use an asset that's already 
 * been retrieved ; otherwise, the asset will be retrieved
 */
int
libdirCreateFileIfMissing(unsigned char *file, size_t file_len, libdirfile_t objFileType, const char *path, bool overwrite, mode_t mode, uid_t nsEuid, gid_t nsEgid)
{
    int ret = 0;
    int fd;
    char temp[PATH_MAX];
    unsigned char *start = NULL;
    unsigned char *buf;
    size_t len;

    // Check if file exists
    if (!access(path, R_OK) && !overwrite) {
        return ret; // File exists
    }

    if (file) {
        buf = file;
        len = file_len;
    } else {
        if ((len = getAsset(objFileType, &start)) == -1) {
            ret = -1;
            goto out;
        }
        buf = start;
    }

    // Write file
    int tempLen = snprintf(temp, PATH_MAX, "%s.XXXXXX", path);
    if (tempLen < 0) {
        fprintf(stderr, "error: snprintf failed.\n");
        ret = -1;
        goto out;
    }
    if (tempLen >= PATH_MAX) {
        fprintf(stderr, "error: extract temp too long.\n");
        ret = -1;
        goto out;
    }

    uid_t currentEuid = geteuid();
    gid_t currentEgid = getegid();

    if ((fd = nsFileMksTemp(temp, nsEuid, nsEgid, currentEuid, currentEgid)) < 1) {
        // No permission
        unlink(temp);
        ret = -1;
        goto out;
    }

    if (write(fd, buf, len) != len) {
        close(fd);
        unlink(temp);
        perror("libdirCreateFileIfMissing: write() failed");
        ret = -1;
        goto out;
    }

    if (fchmod(fd, mode)) {
        close(fd);
        unlink(temp);
        perror("libdirCreateFileIfMissing: fchmod() failed");
        ret = -1;
        goto out;
    }

    close(fd);

    if (nsFileRename(temp, path, nsEuid, nsEgid, currentEuid, currentEgid)) {
        unlink(temp);
        perror("libdirCreateFileIfMissing: rename() failed");
        ret = -1;
        goto out;
    }

out:
    if (objFileType == STATIC_LOADER_FILE && start) munmap(start, len);
    return ret;
}

// Verify if following absolute path points to directory
// Returns operation status
static mkdir_status_t
libdirCheckIfDirExists(const char *absDirPath, uid_t uid, gid_t gid)
{
    struct stat st = {0};
    if (!stat(absDirPath, &st)) {
        if (S_ISDIR(st.st_mode)) {      
            // Check for file creation abilities in directory  
            if (((st.st_uid == uid) && (st.st_mode & S_IWUSR)) ||
                ((st.st_gid == gid) && (st.st_mode & S_IWGRP)) ||
                (st.st_mode & S_IWOTH)) {
                return MKDIR_STATUS_EXISTS;
            }
            return MKDIR_STATUS_ERR_PERM_ISSUE;
        }
        return MKDIR_STATUS_ERR_NOT_ABS_DIR;
    }
    return MKDIR_STATUS_ERR_OTHER;
}

// ----------------------------------------------------------------------------
// External
// ----------------------------------------------------------------------------

// Override default values (function is used only for unit test)
int
libdirInitTest(const char *installBase, const char *tmpBase, const char *rawVersion)
{
    memset(&g_libdir_info, 0, sizeof(g_libdir_info));
    memset(&libappviewState, 0, sizeof(libappviewState));
    strcpy(libappviewState.binaryName, APPVIEW_LIBAPPVIEW_SO);
          
    if (installBase) {
        int len = strlen(installBase);
        if (len >= PATH_MAX) {
            fprintf(stderr, "error: installBase path too long.\n");
            return -1;
        }
        strncpy(g_libdir_info.install_base, installBase, len);
    } else {
        strcpy(g_libdir_info.install_base, "/usr/lib/appview");
    }

    if (tmpBase) {
        int len = strlen(tmpBase);
        if (len >= PATH_MAX){
            fprintf(stderr, "error: tmpBase path too long.\n");
            return -1;
        }
        strncpy(g_libdir_info.tmp_base, tmpBase, len);
    } else {
        strcpy(g_libdir_info.tmp_base, "/tmp/appview");
    }

    if (rawVersion) {
        int len = strlen(rawVersion);
        if (len >= PATH_MAX){
            fprintf(stderr, "error: rawVersion too long.\n");
            return -1;
        }
        strncpy(g_libdir_info.ver, rawVersion, len);
    } else {
        strcpy(g_libdir_info.ver, APPVIEW_VER);
    }

    return 0;
}

// Create a directory in following absolute path creating any intermediate directories as necessary
// Returns operation status
mkdir_status_t
libdirCreateDirIfMissing(const char *dir, mode_t mode, uid_t nsEuid, gid_t nsEgid)
{
    int mkdirRes = -1;
    /* Operate only on absolute path */
    if (dir == NULL || *dir != '/') {
        return MKDIR_STATUS_ERR_NOT_ABS_DIR;
    }

    mkdir_status_t res = libdirCheckIfDirExists(dir, nsEuid, nsEgid);

    /* exit if path exists */
    if (res != MKDIR_STATUS_ERR_OTHER) {
        return res;
    }

    char *tempPath = strdup(dir);
    if (tempPath == NULL) {
        goto end;
    }

    uid_t euid = geteuid();
    gid_t egid = getegid();

    /* traverse the full path */
    for (char *p = tempPath + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';
            errno = 0;

            struct stat st = {0};
            if (stat(tempPath, &st)) {
                mkdirRes = nsFileMkdir(tempPath, mode, nsEuid, nsEgid, euid, egid);
                if (!mkdirRes) {
                    /* We ensure that we setup correct mode regarding umask settings */
                    if (chmod(tempPath, mode)) {
                        goto end;
                    }
                } else {
                    /* nsFileMkdir fails */
                    goto end;
                }
            }

            *p = '/';
        }
    }
    struct stat st = {0};
    if (stat(tempPath, &st)) {
        /* if last element was not created in the loop above */
        mkdirRes = nsFileMkdir(tempPath, mode, nsEuid, nsEgid, euid, egid);
        if (mkdirRes) {
            goto end;
        }
    }

    /* We ensure that we setup correct mode regarding umask settings */
    if (chmod(tempPath, mode)) {
        goto end;
    }

    res = MKDIR_STATUS_CREATED;

end:
    free(tempPath);
    return res;
}

// Sets base_dir of the full path to the library
// The full path takes a following format:
//  <base_dir>/<version>/<library_name>
// The <version> and <library_name> is set internally by this function
// E.g:
//  for /usr/lib/appview/dev/libappview.so:
//    - <base_dir> - "/usr/lib/appview"
//    - <version> - "dev"
//    - <library_name> - "libappview.so"
//  for /tmp/appview/1.2.0/libappview.so:
//    - <base_dir> - "/tmp"
//    - <version> - "1.2.0"
//    - <library_name> - "libappview.so"
// Returns 0 if the full path to a library is accessible
int
libdirSetLibraryBase(const char *base)
{
    const char *normVer = libverNormalizedVersion(g_libdir_info.ver);
    char tmp_path[PATH_MAX] = {0};

    int pathLen = snprintf(tmp_path, PATH_MAX, "%s/%s/%s", base, normVer, APPVIEW_LIBAPPVIEW_SO);
    if (pathLen < 0) {
        fprintf(stderr, "error: snprintf() failed.\n");
        return -1;
    }
    if (pathLen >= PATH_MAX) {
        fprintf(stderr, "error: path too long.\n");
        return -1;
    }

    if (!access(tmp_path, R_OK)) {
        strncpy(libappviewState.binaryBasepath, base, PATH_MAX);
        return 0;
    }

    return -1;
}

/*
* Retrieve the full absolute path of the specified binary libappview.so.
* Returns path for the specified binary, NULL in case of failure.
*/
const char *
libdirGetPath(libdirfile_t objFileType)
{
    const char *normVer = libverNormalizedVersion(g_libdir_info.ver);

    struct appview_obj_state *state = getObjState(objFileType);
    if (!state) {
        return NULL;
    }

    if (state->binaryPath[0]) {
        return state->binaryPath;
    }

    if (state->binaryBasepath[0]) {
        // Check custom base first
        char tmp_path[PATH_MAX] = {0};
        int pathLen = snprintf(tmp_path, PATH_MAX, "%s/%s/%s", state->binaryBasepath, normVer, state->binaryName);
        if (pathLen < 0) {
            fprintf(stderr, "error: libdirGetPath: custom base snprintf() failed.\n");
            return NULL;
        }
        if (pathLen >= PATH_MAX) {
            fprintf(stderr, "error: libdirGetPath: custom base path too long.\n");
            return NULL;
        }

        if (!access(tmp_path, R_OK)) {
            strncpy(state->binaryPath, tmp_path, PATH_MAX);
            return state->binaryPath;
        }
    }

//    const char *cribl_home = getenv("CRIBL_HOME");
//    if (cribl_home) {
//        char tmp_path[PATH_MAX] = {0};
//        int pathLen = snprintf(tmp_path, PATH_MAX, "%s/appview/%s/%s", cribl_home, normVer, state->binaryName);
//        if (pathLen < 0) {
//            fprintf(stderr, "error: libdirGetPath: $CRIBL_HOME/appview/... snprintf() failed.\n");
//            return NULL;
//        }
//        if (pathLen >= PATH_MAX) {
//            fprintf(stderr, "error: libdirGetPath: $CRIBL_HOME/appview/... path too long.\n");
//            return NULL;
//        }
//
//        if (!access(tmp_path, R_OK)) {
//            strncpy(state->binaryPath, tmp_path, PATH_MAX);
//            return state->binaryPath;
//        }
//    }

    if (g_libdir_info.install_base[0]) {
        // Check install base next
        if (objFileType == LIBRARY_FILE) {
            // Special case for the library when we're dealing with the install path. It exists at /usr/lib/libappview.so.
            // Check symlink to the library exists and is valid, and if so, return it
            if (!access(APPVIEW_LIBAPPVIEW_PATH, R_OK)) {
                strncpy(state->binaryPath, APPVIEW_LIBAPPVIEW_PATH, PATH_MAX);
                return state->binaryPath;
            }
        } else {
            char tmp_path[PATH_MAX] = {0};
            int pathLen = snprintf(tmp_path, PATH_MAX, "%s/%s/%s", g_libdir_info.install_base, normVer, state->binaryName);
            if (pathLen < 0) {
                fprintf(stderr, "error: libdirGetPath: install base snprintf() failed.\n");
                return NULL;
            }
            if (pathLen >= PATH_MAX) {
                fprintf(stderr, "error: libdirGetPath: install base path too long.\n");
                return NULL;
            }
            if (!access(tmp_path, R_OK)) {
                strncpy(state->binaryPath, tmp_path, PATH_MAX);
                return state->binaryPath;
            }
        }
    }

    if (g_libdir_info.tmp_base[0]) {
        // Check tmp base next
        char tmp_path[PATH_MAX] = {0};
        int pathLen = snprintf(tmp_path, PATH_MAX, "%s/%s/%s", g_libdir_info.tmp_base, normVer, state->binaryName);
        if (pathLen < 0) {
            fprintf(stderr, "error: libdirGetPath: tmp base snprintf() failed.\n");
            return NULL;
        }
        if (pathLen >= PATH_MAX) {
            fprintf(stderr, "error: libdirGetPath: tmp base path too long.\n");
            return NULL;
        }
        if (!access(tmp_path, R_OK)) {
            strncpy(state->binaryPath, tmp_path, PATH_MAX);
            return state->binaryPath;
        }
    }

    return NULL;
}

/*
* Extract (physically create) libappview.so to the filesystem.
* The extraction will not be performed:
* - if the file is present and it is official version
* - if the custom path was specified before by `libdirSetLibraryBase`
* Returns 0 in case of success, other values in case of failure.
*/
int
libdirExtract(unsigned char *asset_file, size_t asset_file_len, uid_t nsUid, gid_t nsGid, libdirfile_t objFileType)
{
    char path[PATH_MAX] = {0};
    char path_musl[PATH_MAX] = {0};
    char path_glibc[PATH_MAX] = {0};
    size_t pathlen = 0;
    char *target;
    mode_t mode = 0755;
    mkdir_status_t res;
    bool useTmpPath = FALSE;

    // Which version of AppView are we dealing with (official or dev)
    const char *loaderVersion = libverNormalizedVersion(APPVIEW_VER);
    bool isDevVersion = libverIsNormVersionDev(loaderVersion);
    bool overwrite = isDevVersion;

    // Create the destination directory if it does not exist
    
//    // Try to create $CRIBL_HOME/appview (if set)
//    const char *cribl_home = getenv("CRIBL_HOME");
//    if (cribl_home) {
//        int pathLen = snprintf(path, PATH_MAX, "/%s/appview/%s/", cribl_home, loaderVersion);
//        if (pathLen < 0) {
//            fprintf(stderr, "error: libdirExtract: $CRIBL_HOME/appview/... snprintf() failed.\n");
//            return -1;
//        }
//        if (pathLen >= PATH_MAX) {
//            fprintf(stderr, "error: libdirExtract: $CRIBL_HOME/appview/... path too long.\n");
//            return -1;
//        }
//        res = libdirCreateDirIfMissing(path, mode, nsUid, nsGid);
//    }

    // If CRIBL_HOME not defined, or there was an error, create usr/lib/appview
//    if (!cribl_home || res > MKDIR_STATUS_EXISTS) {
        memset(path, 0, PATH_MAX);
        int pathLen = snprintf(path, PATH_MAX, "/usr/lib/appview/%s/", loaderVersion);
        if (pathLen < 0) {
            fprintf(stderr, "error: libdirExtract: usr/lib/appview/... snprintf() failed.\n");
            return -1;
        }
        if (pathLen >= PATH_MAX) {
            fprintf(stderr, "error: libdirExtract: /usr/lib/appview/... path too long.\n");
            return -1;
        }
        res = libdirCreateDirIfMissing(path, mode, nsUid, nsGid);
//    }

    // If all else fails, create /tmp/appview
    if (res > MKDIR_STATUS_EXISTS) {
        useTmpPath = TRUE;
        mode = 0777;
        memset(path, 0, PATH_MAX);
        int pathLen = snprintf(path, PATH_MAX, "/tmp/appview/%s/", loaderVersion);
        if (pathLen < 0) {
            fprintf(stderr, "error: libdirExtract: /tmp/appview/... snprintf() failed.\n");
            return -1;
        }
        if (pathLen >= PATH_MAX) {
            fprintf(stderr, "error: libdirExtract: /tmp/appview/... path too long.\n");
            return -1;
        }
        res = libdirCreateDirIfMissing(path, mode, nsUid, nsGid);
    }

    if (res > MKDIR_STATUS_EXISTS) {
        fprintf(stderr, "setupInstall: libdirCreateDirIfMissing failed\n");
        return -1;
    }

    // Create the file if it does not exist or needs to be overwritten

    switch (objFileType) {
    case LIBRARY_FILE:
        pathlen = strlen(path);

        // Extract libappview.so.glibc (bundled libappview defaults to glibc loader)
        strncpy(path_glibc, path, pathlen);
        strncat(path_glibc, "libappview.so.glibc", sizeof(path_glibc) - 1);
        if (libdirCreateFileIfMissing(asset_file, asset_file_len, objFileType, path_glibc, overwrite, mode, nsUid, nsGid)) {
            fprintf(stderr, "libdirExtract: saving %s failed\n", path_glibc);
            return -1;
        }

        // Extract libappview.so.musl
        strncpy(path_musl, path, pathlen);
        strncat(path_musl, "libappview.so.musl", sizeof(path_musl) - 1);
        if (libdirCreateFileIfMissing(asset_file, asset_file_len, objFileType, path_musl, overwrite, mode, nsUid, nsGid)) {
            fprintf(stderr, "libdirExtract: saving %s failed\n", path);
            return -1;
        }

        // Patch the libappview.so.musl file for musl
        patch_status_t patch_res;
        if ((patch_res = patchLibrary(path_musl, TRUE)) == PATCH_FAILED) {
            fprintf(stderr, "libdirExtract: patch %s failed\n", path_musl);
            return -1;
        }

        // Create symlink
        
        // Determine which version it should point to
        target = isMusl() ? path_musl : path_glibc;

        // Determine where it should be created
        if (useTmpPath) {
            // Symlink to be created at /tmp/appview/<ver>/libappview.so
            strncat(path, "libappview.so", sizeof(path) - 1);
        } else {
            // Symlink to be created at /usr/lib/libappview.so
            memset(path, 0, PATH_MAX);
            int pathLen = snprintf(path, PATH_MAX, APPVIEW_LIBAPPVIEW_PATH);
            if (pathLen < 0) {
                fprintf(stderr, "error: libdirExtract: snprintf() failed.\n");
                return -1;
            }
            if (pathLen >= PATH_MAX) {
                fprintf(stderr, "error: libdirExtract: path too long.\n");
                return -1;
            }
        }

        // Always remove old symlink (in case it points to an older lib)
        if ((remove(path) < 0) && (errno != ENOENT)) {
            fprintf(stderr, "error: libdirExtract: remove failed %d.\n", errno);
            return -1;
        }

        // Create new symlink
        if (libdirCreateSymLinkIfMissing(path, target, overwrite, mode, nsUid, nsGid)) {
            fprintf(stderr, "libdirExtract: symlink %s failed\n", path);
            return -1;
        }
        break;

    case LOADER_FILE:
        // Create the dynamic loader file if it does not exist; or needs to be overwritten
        strncat(path, "appviewdyn", sizeof(path) - 1);
        if (libdirCreateFileIfMissing(asset_file, asset_file_len, objFileType, path, overwrite, mode, nsUid, nsGid)) {
            fprintf(stderr, "libdirExtract: saving %s failed\n", path);
            return -1;
        }
        break;

    case STATIC_LOADER_FILE:
        // Create the loader file if it does not exist; or needs to be overwritten
        strncat(path, "appview", sizeof(path) - 1);
        if (libdirCreateFileIfMissing(asset_file, asset_file_len, objFileType, path, overwrite, mode, nsUid, nsGid)) {
            fprintf(stderr, "libdirExtract: saving %s failed\n", path);
            return -1;
        }
        break;

    default:
        fprintf(stderr, "error: invalid objFileType\n");
        return -1;

    }

    return 0;
}
