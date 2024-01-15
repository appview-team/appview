#define _GNU_SOURCE

#include "nsfile.h"

int
nsFileShmOpen(const char *name, int oflag, mode_t mode, uid_t nsUid, gid_t nsGid, uid_t restoreUid, gid_t restoreGid) {
    appview_setegid(nsGid);
    appview_seteuid(nsUid);

    int fd = appview_shm_open(name, oflag, mode);

    appview_seteuid(restoreUid);
    appview_setegid(restoreGid);
    return fd;
}

int
nsFileOpen(const char *pathname, int flags, uid_t nsUid, gid_t nsGid, uid_t restoreUid, gid_t restoreGid) {

    appview_setegid(nsGid);
    appview_seteuid(nsUid);

    int fd = appview_open(pathname, flags);

    appview_seteuid(restoreUid);
    appview_setegid(restoreGid);
    return fd;
}

int
nsFileOpenWithMode(const char *pathname, int flags, mode_t mode, uid_t nsUid, gid_t nsGid, uid_t restoreUid, gid_t restoreGid) {

    appview_setegid(nsGid);
    appview_seteuid(nsUid);

    int fd = appview_open(pathname, flags, mode);

    appview_seteuid(restoreUid);
    appview_setegid(restoreGid);
    return fd;
}

int
nsFileMkdir(const char *pathname, mode_t mode, uid_t nsUid, gid_t nsGid, uid_t restoreUid, gid_t restoreGid) {
    appview_setegid(nsGid);
    appview_seteuid(nsUid);

    int res = appview_mkdir(pathname, mode);

    appview_seteuid(restoreUid);
    appview_setegid(restoreGid);
    return res;
}

int
nsFileMksTemp(char *template, uid_t nsUid, gid_t nsGid, uid_t restoreUid, gid_t restoreGid) {
    appview_setegid(nsGid);
    appview_seteuid(nsUid);

    int res = appview_mkstemp(template);

    appview_seteuid(restoreUid);
    appview_setegid(restoreGid);
    return res;
}

int
nsFileRename(const char *oldpath, const char *newpath, uid_t nsUid, gid_t nsGid, uid_t restoreUid, gid_t restoreGid) {
    appview_setegid(nsGid);
    appview_seteuid(nsUid);

    int res = appview_rename(oldpath, newpath);

    appview_seteuid(restoreUid);
    appview_setegid(restoreGid);
    return res;
}

FILE *
nsFileFopen(const char *restrict pathname, const char *restrict mode, uid_t nsUid, gid_t nsGid, uid_t restoreUid, gid_t restoreGid) {
    appview_setegid(nsGid);
    appview_seteuid(nsUid);

    FILE *fp = appview_fopen(pathname, mode);

    appview_seteuid(restoreUid);
    appview_setegid(restoreGid);
    return fp;
}

int
nsFileSymlink(const char *target, const char *linkpath, uid_t nsUid, gid_t nsGid, uid_t restoreUid, gid_t restoreGid, int *errnoVal) {
    appview_setegid(nsGid);
    appview_seteuid(nsUid);

    int res = appview_symlink(target, linkpath);
    if (res == -1) {
        *errnoVal = appview_errno;
    }

    appview_seteuid(restoreUid);
    appview_setegid(restoreGid);
    return res;
}
