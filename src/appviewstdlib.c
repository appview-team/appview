#define _GNU_SOURCE
#include "appviewstdlib.h"

#include <limits.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dbg.h"

// Internal standard library references
extern void  appviewlibc_init_vdso_ehdr(unsigned long);
extern void  appviewlibc_lock_before_fork_op(void);
extern void  appviewlibc_unlock_after_fork_op(int);

// Memory management handling operations
extern void *  appviewlibc_memalign(size_t, size_t);
extern void *  appviewlibc_malloc(size_t);
extern void *  appviewlibc_calloc(size_t, size_t);
extern void *  appviewlibc_realloc(void *, size_t);
extern void    appviewlibc_free(void *);
extern void *  appviewlibc_mmap(void *, size_t, int, int, int, off_t);
extern int     appviewlibc_munmap(void *, size_t);
extern FILE *  appviewlibc_open_memstream(char **, size_t *);
extern void *  appviewlibc_memset(void *, int, size_t);
extern void *  appviewlibc_memmove(void *, const void *, size_t);
extern int     appviewlibc_memcmp(const void *, const void *, size_t);
extern int     appviewlibc_mprotect(void *, size_t, int);
extern void *  appviewlibc_memcpy(void *, const void *, size_t);
extern int     appviewlibc_mlock(const void *, size_t);
extern int     appviewlibc_msync(void *, size_t, int);
extern int     appviewlibc_mincore(void *, size_t, unsigned char *);
extern int     appviewlibc_memfd_create(const char *, unsigned int);

// File handling operations
extern FILE *           appviewlibc_fopen(const char *, const char *);
extern int              appviewlibc_fclose(FILE *);
extern FILE *           appviewlibc_fdopen(int, const char *);
extern int              appviewlibc_close(int);
extern ssize_t          appviewlibc_read(int, void *, size_t);
extern size_t           appviewlibc_fread(void *, size_t, size_t, FILE *);
extern ssize_t          appviewlibc_write(int, const void *, size_t);
extern size_t           appviewlibc_fwrite(const void *, size_t, size_t, FILE *);
extern char *           appviewlibc_fgets(char *, int, FILE *);
extern ssize_t          appviewlibc_getline(char **, size_t *, FILE *);
extern int              appviewlibc_puts(const char *);
extern int              appviewlibc_setvbuf(FILE *, char *, int, size_t);
extern int              appviewlibc_fflush(FILE *);
extern char *           appviewlibc_dirname(char *);
extern DIR*             appviewlibc_opendir(const char *);
extern struct dirent *  appviewlibc_readdir(DIR *);
extern int              appviewlibc_closedir(DIR *);
extern int              appviewlibc_access(const char *, int);
extern FILE *           appviewlibc_fmemopen(void *, size_t, const char *);
extern long             appviewlibc_ftell(FILE *);
extern int              appviewlibc_fseek(FILE *, long, int);
extern off_t            appviewlibc_lseek(int, off_t, int);
extern int              appviewlibc_unlink(const char *);
extern int              appviewlibc_dup2(int, int);
extern char *           appviewlibc_basename(char *);
extern int              appviewlibc_stat(const char *, struct stat *);
extern int              appviewlibc_chmod(const char *, mode_t);
extern int              appviewlibc_fchmod(int, mode_t);
extern int              appviewlibc_feof(FILE *);
extern int              appviewlibc_fileno(FILE *);
extern int              appviewlibc_flock(int, int);
extern int              appviewlibc_fstat(int, struct stat *);
extern int              appviewlibc_mkdir(const char *, mode_t);
extern int              appviewlibc_chdir(const char *);
extern int              appviewlibc_rmdir(const char *);
extern char *           appviewlibc_get_current_dir_name(void);
extern char *           appviewlibc_getcwd(char *, size_t);
extern int              appviewlibc_lstat(const char *, struct stat *);
extern int              appviewlibc_rename(const char *, const char *);
extern int              appviewlibc_remove(const char *);
extern int              appviewlibc_pipe2(int [2], int);
extern void             appviewlibc_setbuf(FILE *, char *);

// String handling operations
extern char *              appviewlibc_realpath(const char *, char *);
extern ssize_t             appviewlibc_readlink(const char *, char *, size_t);
extern char *              appviewlibc_strdup(const char *);
extern int                 appviewlibc_vasprintf(char **, const char *, va_list);
extern size_t              appviewlibc_strftime(char *, size_t, const char *, const struct tm *);
extern size_t              appviewlibc_strlen(const char *);
extern size_t              appviewlibc_strnlen(const char *, size_t);
extern char *              appviewlibc_strerror(int);
extern int                 appviewlibc_strerror_r(int, char *, size_t);
extern double              appviewlibc_strtod(const char *, char **);
extern long                appviewlibc_strtol(const char *, char **, int);
extern long long           appviewlibc_strtoll(const char *, char **, int);
extern unsigned long       appviewlibc_strtoul(const char *, char **, int);
extern unsigned long long  appviewlibc_strtoull(const char *, char **, int);
extern char *              appviewlibc_strchr(const char *, int);
extern char *              appviewlibc_strrchr(const char *, int);
extern char *              appviewlibc_strstr(const char *, const char *);
extern int                 appviewlibc_vsnprintf(char *, size_t, const char *, va_list);
extern int                 appviewlibc_vfprintf(FILE *, const char *, va_list);
extern int                 appviewlibc_vprintf(const char *, va_list);
extern int                 appviewlibc_vsscanf(const char *, const char *, va_list);
extern int                 appviewlibc_strcmp(const char *, const char *);
extern int                 appviewlibc_strncmp(const char *, const char *, size_t);
extern int                 appviewlibc_strcasecmp(const char *, const char *);
extern char *              appviewlibc_strchrnul(const char *, int );
extern char *              appviewlibc_strcpy(char *, const char *);
extern char *              appviewlibc_strncpy(char *, const char *, size_t);
extern char *              appviewlibc_stpcpy(char *, const char *);
extern char *              appviewlibc_stpncpy(char *, const char *, size_t);
extern size_t              appviewlibc_strcspn(const char *, const char *);
extern char *              appviewlibc_strcat(char *, const char *);
extern char *              appviewlibc_strncat(char *, const char *, size_t);
extern char *              appviewlibc_strpbrk(const char *, const char *);
extern char *              appviewlibc_strcasestr(const char *, const char *);
extern char *              appviewlibc_strtok(char *, const char *);
extern char *              appviewlibc_strtok_r(char *, const char *, char **);
extern const char *        appviewlibc_gai_strerror(int);

// Network handling operations
extern int               appviewlibc_gethostname(char *, size_t);
extern int               appviewlibc_getsockname(int, struct sockaddr *, socklen_t *);
extern int               appviewlibc_getsockopt(int, int, int, void *, socklen_t *);
extern int               appviewlibc_setsockopt(int, int, int, const void *, socklen_t);
extern int               appviewlibc_socket(int, int, int);
extern int               appviewlibc_bind(int, const struct sockaddr *, socklen_t);
extern int               appviewlibc_accept(int, struct sockaddr *, socklen_t *);
extern int               appviewlibc_connect(int, const struct sockaddr *, socklen_t);
extern int               appviewlibc_listen(int, int);
extern void              appviewlibc_rewind(FILE *);
extern ssize_t           appviewlibc_send(int, const void *, size_t, int);
extern ssize_t           appviewlibc_sendmsg(int, const struct msghdr *, int);
extern ssize_t           appviewlibc_recv(int, void *, size_t, int);
extern ssize_t           appviewlibc_recvmsg(int, struct msghdr *, int);
extern ssize_t           appviewlibc_recvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);
extern int               appviewlibc_shutdown(int, int);
extern int               appviewlibc_poll(struct pollfd *, nfds_t, int);
extern int               appviewlibc_select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
extern int               appviewlibc_getaddrinfo(const char *, const char *, const struct addrinfo *, struct addrinfo **);
extern void              appviewlibc_freeaddrinfo(struct addrinfo *);
extern int               appviewlibc_copyaddrinfo(struct sockaddr *, socklen_t, struct addrinfo **);
extern int               appviewlibc_getnameinfo(const struct sockaddr *, socklen_t, char *, socklen_t, char *, socklen_t, int);
extern int               appviewlibc_getpeername(int, struct sockaddr *, socklen_t *);
extern struct hostent *  appviewlibc_gethostbyname(const char *);
extern const char *      appviewlibc_inet_ntop(int, const void *, char *, socklen_t);
extern uint16_t          appviewlibc_ntohs(uint16_t);
extern uint16_t          appviewlibc_htons(uint16_t);

// Misc handling operations
extern int             appviewlibc_atoi(const char *);
extern int             appviewlibc_isspace(int);
extern int             appviewlibc_isprint(int);
extern int             appviewlibc_isdigit(int);
extern void            appviewlibc_perror(const char *);
extern int             appviewlibc_gettimeofday(struct timeval *, struct timezone *);
extern int             appviewlibc_timer_create(clockid_t, struct sigevent *, timer_t *);
extern int             appviewlibc_timer_settime(timer_t, int, const struct itimerspec *, struct itimerspec *);
extern int             appviewlibc_timer_delete(timer_t);
extern time_t          appviewlibc_time(time_t *);
extern struct tm *     appviewlibc_localtime_r(const time_t *, struct tm *);
extern struct tm *     appviewlibc_gmtime_r(const time_t *, struct tm *);
extern unsigned int    appviewlibc_sleep(unsigned int);
extern int             appviewlibc_usleep(useconds_t);
extern int             appviewlibc_nanosleep(const struct timespec *, struct timespec *);
extern int             appviewlibc_sigaction(int, const struct sigaction *, struct sigaction *);
extern int             appviewlibc_sigemptyset(sigset_t *);
extern int             appviewlibc_sigfillset(sigset_t *);
extern int             appviewlibc_sigdelset(sigset_t *, int);
extern int             appviewlibc_pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
extern int             appviewlibc_pthread_barrier_init(pthread_barrier_t *, const pthread_barrierattr_t *, unsigned);
extern int             appviewlibc_pthread_barrier_destroy(pthread_barrier_t *);
extern int             appviewlibc_pthread_barrier_wait(pthread_barrier_t *);
extern int             appviewlibc_dlclose(void *);
extern int             appviewlibc_ns_initparse(const unsigned char *, int, ns_msg *);
extern int             appviewlibc_ns_parserr(ns_msg *, ns_sect, int, ns_rr *);
extern int             appviewlibc_getgrgid_r(gid_t, struct group *, char *, size_t, struct group **);
extern int             appviewlibc_getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);
extern pid_t           appviewlibc_getpid(void);
extern pid_t           appviewlibc_getppid(void);
extern uid_t           appviewlibc_getuid(void);
extern uid_t           appviewlibc_geteuid(void);
extern gid_t           appviewlibc_getegid(void);
extern int             appviewlibc_seteuid(uid_t);
extern int             appviewlibc_setegid(gid_t);
extern gid_t           appviewlibc_getgid(void);
extern pid_t           appviewlibc_getpgrp(void);
extern void *          appviewlibc_dlopen(const char *, int);
extern int             appviewlibc_dlclose(void *);
extern void *          appviewlibc_dlsym(void *, const char *);
extern long            appviewlibc_ptrace(int, pid_t, void *, void *);
extern pid_t           appviewlibc_waitpid(pid_t, int *, int);
extern char *          appviewlibc_getenv(const char *);
extern int             appviewlibc_setenv(const char *, const char *, int);
extern struct lconv *  appviewlibc_localeconv(void);
extern int             appviewlibc_shm_open(const char *, int, mode_t);
extern int             appviewlibc_shm_unlink(const char *);
extern long            appviewlibc_sysconf(int);
extern int             appviewlibc_mkstemp(char *);
extern int             appviewlibc_clock_gettime(clockid_t, struct timespec *);
extern int             appviewlibc_getpagesize(void);
extern int             appviewlibc_uname(struct utsname *);
extern int             appviewlibc_arch_prctl(int, unsigned long);
extern int             appviewlibc_getrusage(int , struct rusage *);
extern int             appviewlibc_atexit(void (*)(void));
extern int             appviewlibc_tcsetattr(int, int, const struct termios *);
extern int             appviewlibc_tcgetattr(int, struct termios *);
extern void *          appviewlibc_shmat(int, const void *, int);
extern int             appviewlibc_shmdt(const void *);
extern int             appviewlibc_shmget(key_t, size_t, int);
extern int             appviewlibc_sched_getcpu(void);
extern int             appviewlibc_rand(void);
extern void            appviewlibc_srand(unsigned int);
extern int             appviewlibc_ftruncate(int, off_t);
extern int             appviewlibc_setns(int, int);
extern int             appviewlibc_chown(const char *, uid_t, gid_t);
extern int             appviewlibc_fchown(int, uid_t, gid_t);
extern int             appviewlibc_getc(FILE *);
extern int             appviewlibc_putc(int, FILE *);
extern int             appviewlibc_symlink(const char *, const char *);
extern int             appviewlibc_mq_close(mqd_t);
extern int             appviewlibc_mq_send(mqd_t, const char *, size_t, unsigned int);
extern ssize_t         appviewlibc_mq_receive(mqd_t, char *, size_t, unsigned int *);
extern int             appviewlibc_mq_unlink(const char *);
extern int             appviewlibc_mq_getattr(mqd_t, struct mq_attr *);

static int g_go_static;

// TODO consider moving GOAppState API somewhere else
void
appviewSetGoAppStateStatic(int static_state) {
    g_go_static = static_state;
}

int
appviewGetGoAppStateStatic(void) {
    return g_go_static;
}

int
APPVIEW_DlIteratePhdr(int (*callback) (struct dl_phdr_info *info, size_t size, void *data), void *data)
{
    // If we are in a signal handler don't call dl_iterate_phdr.
    // It is not async safe.
    if (g_issighandler == TRUE && g_isgo) return 0;

    // TODO provide implementation for static GO
    // We cannot use dl_iterate_phdr since it uses TLS
    // To retrieve information about go symbols we need to implement own
    return (!appviewGetGoAppStateStatic()) ? dl_iterate_phdr(callback, data) : 0;
}

// Internal library operations

void
appview_init_vdso_ehdr(void) {
    unsigned long ehdr = getauxval(AT_SYSINFO_EHDR);
    appviewlibc_init_vdso_ehdr(ehdr);
}

void
appview_op_before_fork(void) {
    appviewlibc_lock_before_fork_op();
}

void
appview_op_after_fork(int who) {
    appviewlibc_unlock_after_fork_op(who);
}

// Memory management handling operations

void *
appview_memalign(size_t alignment, size_t size) {
    return appviewlibc_memalign(alignment, size);
}

void *
appview_malloc(size_t size) {
    return appviewlibc_malloc(size);
}

void *
appview_calloc(size_t nmemb, size_t size) {
    return appviewlibc_calloc(nmemb, size);
}

void *
appview_realloc(void *ptr, size_t size) {
    return appviewlibc_realloc(ptr, size);
}

void
appview_free(void *ptr) {
    appviewlibc_free(ptr);
}

void *
appview_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return appviewlibc_mmap(addr, length, prot, flags, fd, offset);
}

int
appview_munmap(void *addr, size_t length) {
    return appviewlibc_munmap(addr, length);
}

FILE *
appview_open_memstream(char **ptr, size_t *sizeloc) {
    return appviewlibc_open_memstream(ptr, sizeloc);
}

void *
appview_memset(void *s, int c, size_t n) {
    return appviewlibc_memset(s, c, n);
}

void *
appview_memmove(void *dest, const void *src, size_t n) {
    return appviewlibc_memmove(dest, src, n);
}

int
appview_memcmp(const void *s1, const void *s2, size_t n) {
    return appviewlibc_memcmp(s1, s2, n);
}

int
appview_mprotect(void *addr, size_t len, int prot) {
    return appviewlibc_mprotect(addr, len, prot);
}

void *
appview_memcpy(void *restrict dest, const void *restrict src, size_t n) {
    return appviewlibc_memcpy(dest, src, n);
}

int
appview_mlock(const void *addr, size_t len) {
    return appviewlibc_mlock(addr, len);
}

int
appview_msync(void *addr, size_t length, int flags) {
    return appviewlibc_msync(addr, length, flags);
}

int
appview_memfd_create(const char *name, unsigned int flags) {
    return appviewlibc_memfd_create(name, flags);
}

int
appview_mincore(void *addr, size_t length, unsigned char *vec) {
    return appviewlibc_mincore(addr, length, vec);
}

// File handling operations

FILE *
appview_fopen( const char * filename, const char * mode) {
    return appviewlibc_fopen(filename, mode);
}

int
appview_fclose(FILE * stream) {
    return appviewlibc_fclose(stream);
}

FILE *
appview_fdopen(int fd, const char *mode) {
    return appviewlibc_fdopen(fd, mode);
}

int
appview_close(int fd) {
    return appviewlibc_close(fd);
}

ssize_t
appview_read(int fd, void *buf, size_t count) {
    return appviewlibc_read(fd, buf, count);
}

size_t
appview_fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
    return appviewlibc_fread(ptr, size, nmemb, stream);
}

ssize_t
appview_write(int fd, const void *buf, size_t count) {
    return appviewlibc_write(fd, buf, count);
}

size_t
appview_fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
    return appviewlibc_fwrite(ptr, size, nmemb, stream);
}

char *
appview_fgets(char *restrict s, int n, FILE *restrict stream) {
    return appviewlibc_fgets(s, n, stream);
}

ssize_t
appview_getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream) {
    return appviewlibc_getline(lineptr, n, stream);
}

int
appview_puts(const char *s) {
    return appviewlibc_puts(s);
}

int
appview_setvbuf(FILE *restrict stream, char *restrict buf, int type, size_t size) {
    return appviewlibc_setvbuf(stream, buf, type, size);
}

int
appview_fflush(FILE *stream) {
    return appviewlibc_fflush(stream);
}

char *
appview_dirname(char *path) {
    return appviewlibc_dirname(path);
}

DIR*
appview_opendir(const char *name) {
    return appviewlibc_opendir(name);
}
struct dirent*
appview_readdir(DIR *dirp) {
    return appviewlibc_readdir(dirp);
}

int
appview_closedir(DIR *dirp) {
    return appviewlibc_closedir(dirp);
}

int appview_access(const char *pathname, int mode) {
    return appviewlibc_access(pathname, mode);
}

FILE *
appview_fmemopen(void *buf, size_t size, const char *mode) {
    return appviewlibc_fmemopen(buf, size, mode);
}

long
appview_ftell(FILE *stream) {
    return appviewlibc_ftell(stream);
}

int
appview_fseek(FILE *stream, long offset, int whence) {
    return appviewlibc_fseek(stream, offset, whence);
}

off_t
appview_lseek(int fd, off_t offset, int whence) {
    return appviewlibc_lseek(fd, offset, whence);
}

int
appview_unlink(const char *pathname) {
    return appviewlibc_unlink(pathname);
}

int
appview_dup2(int oldfd, int newfd) {
    return appviewlibc_dup2(oldfd, newfd);
}

char *
appview_basename(char *path) {
    return appviewlibc_basename(path);
}

int
appview_stat(const char *restrict pathname, struct stat *restrict statbuf) {
    return appviewlibc_stat(pathname, statbuf);
}

int
appview_chmod(const char *path, mode_t mode) {
    return appviewlibc_chmod(path, mode);
}

int
appview_fchmod(int fildes, mode_t mode) {
    return appviewlibc_fchmod(fildes, mode);
}

int
appview_feof(FILE *stream) {
    return appviewlibc_feof(stream);
}

int
appview_fileno(FILE *stream) {
    return appviewlibc_fileno(stream);
}

int
appview_flock(int fd, int operation) {
    return appviewlibc_flock(fd, operation);
}

int
appview_fstat(int fd, struct stat *buf) {
    return appviewlibc_fstat(fd, buf);
}

int
appview_mkdir(const char *pathname, mode_t mode) {
    return appviewlibc_mkdir(pathname, mode);
}

int
appview_chdir(const char *path) {
    return appviewlibc_chdir(path);
}

int
appview_rmdir(const char *pathname) {
    return appviewlibc_rmdir(pathname);
}

char *
appview_get_current_dir_name(void){
    return appviewlibc_get_current_dir_name();
}

char *
appview_getcwd(char *buf, size_t size) {
    return appviewlibc_getcwd(buf, size);
}

int
appview_lstat(const char *restrict path, struct stat *restrict buf) {
    return appviewlibc_lstat(path, buf);
}

int
appview_rename(const char *oldpath, const char *newpath) {
    return appviewlibc_rename(oldpath, newpath);
}

int
appview_remove(const char *pathname) {
    return appviewlibc_remove(pathname);
}

int
appview_pipe2(int pipefd[2], int flags) {
    return appviewlibc_pipe2(pipefd, flags);
}

void
appview_setbuf(FILE *restrict stream, char *restrict buf) {
    appviewlibc_setbuf(stream, buf);
}

char *
appview_strcpy(char *restrict dest, const char *src) {
    return appviewlibc_strcpy(dest, src);
}

char *
appview_strncpy(char *restrict dest, const char *restrict src, size_t n) {
    return appviewlibc_strncpy(dest, src, n);
}

char *
appview_stpcpy(char *restrict dest, const char *restrict src) {
    return appviewlibc_stpcpy(dest, src);
}

char *
appview_stpncpy(char *restrict dest, const char *restrict src, size_t n) {
    return appviewlibc_stpncpy(dest, src, n);
}


// String handling operations

char *
appview_realpath(const char *restrict path, char *restrict resolved_path) {
    return appviewlibc_realpath(path, resolved_path);
}

ssize_t
appview_readlink(const char *restrict pathname, char *restrict buf, size_t bufsiz) {
    return appviewlibc_readlink(pathname, buf, bufsiz);
}

char *
appview_strdup(const char *s) {
    return appviewlibc_strdup(s);
}

int
appview_vasprintf(char **strp, const char *fmt, va_list ap) {
    return appviewlibc_vasprintf(strp, fmt, ap);
}

size_t
appview_strftime(char *restrict s, size_t max, const char *restrict format, const struct tm *restrict tm) {
    return appviewlibc_strftime(s, max, format, tm);
}

size_t
appview_strlen(const char *s) {
    return appviewlibc_strlen(s);
}

size_t
appview_strnlen(const char *s, size_t maxlen) {
    return appviewlibc_strnlen(s, maxlen);
}

char *
appview_strerror(int errnum) {
    return appviewlibc_strerror(errnum);
}

int
appview_strerror_r(int err, char *buf, size_t buflen) {
    return appviewlibc_strerror_r(err, buf, buflen);
}

double
appview_strtod(const char *restrict nptr, char **restrict endptr) {
    return appviewlibc_strtod(nptr, endptr);
}

long
appview_strtol(const char *restrict nptr, char **restrict endptr, int base) {
    return appviewlibc_strtol(nptr, endptr, base);
}

long long
appview_strtoll(const char *restrict nptr, char **restrict endptr, int base) {
    return appviewlibc_strtoll(nptr, endptr, base);
}

unsigned long
appview_strtoul(const char *restrict nptr, char **restrict endptr, int base) {
    return appviewlibc_strtoul(nptr, endptr, base);
}

unsigned long long
appview_strtoull(const char *restrict nptr, char **restrict endptr, int base) {
    return appviewlibc_strtoull(nptr, endptr, base);
}

char *
appview_strchr(const char *s, int c) {
    return appviewlibc_strchr(s, c);
}

char *
appview_strrchr(const char *s, int c) {
    return appviewlibc_strrchr(s, c);
}

char *
appview_strstr(const char *haystack, const char *needle) {
    return appviewlibc_strstr(haystack, needle);
}

int
appview_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    return appviewlibc_vsnprintf(str, size, format, ap);
}

int
appview_vfprintf(FILE *stream, const char *format, va_list ap) {
    return appviewlibc_vfprintf(stream, format, ap);
}

int
appview_vprintf(const char *format, va_list ap) {
    return appviewlibc_vprintf(format, ap);
}

int
appview_vsscanf(const char *str, const char *format, va_list ap) {
    return appviewlibc_vsscanf(str, format, ap);
}

int
appview_strcmp(const char *s1, const char *s2) {
    return appviewlibc_strcmp(s1, s2);
}

int
appview_strncmp(const char *s1, const char *s2, size_t n) {
    return appviewlibc_strncmp(s1, s2, n);
}

int
appview_strcasecmp(const char *s1, const char *s2) {
    return appviewlibc_strcasecmp(s1, s2);
}

char *
appview_strchrnul(const char *s, int c) {
    return appviewlibc_strchrnul(s, c);
}

size_t
appview_strcspn(const char *s, const char *reject) {
    return appviewlibc_strcspn(s, reject);
}

char *
appview_strcat(char *restrict dest, const char *restrict src) {
    return appviewlibc_strcat(dest, src);
}

char *
appview_strncat(char *restrict dest, const char *restrict src, size_t n) {
    return appviewlibc_strncat(dest, src, n);
}

char *
appview_strpbrk(const char *s, const char *accept) {
    return appviewlibc_strpbrk(s, accept);
}

char *
appview_strcasestr(const char * haystack, const char * needle) {
    return appviewlibc_strcasestr(haystack, needle);
}

char *
appview_strtok(char *restrict str, const char *restrict delim) {
    return appviewlibc_strtok(str, delim);
}

char *
appview_strtok_r(char *restrict str, const char *restrict delim, char **restrict saveptr) {
    return appviewlibc_strtok_r(str, delim, saveptr);
}

const char *
appview_gai_strerror(int errcode) {
    return appviewlibc_gai_strerror(errcode);
}


// Network handling operations

int
appview_gethostname(char *name, size_t len) {
    return appviewlibc_gethostname(name, len);
}

int
appview_getsockname(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen) {
    return appviewlibc_getsockname(sockfd, addr, addrlen);
}

int
appview_getsockopt(int sockfd, int level, int optname,  void *restrict optval, socklen_t *restrict optlen) {
    return appviewlibc_getsockopt(sockfd, level, optname, optval, optlen);
}

int
appview_setsockopt(int sockfd, int level, int optname,  const void *restrict optval, socklen_t optlen) {
    return appviewlibc_setsockopt(sockfd, level, optname, optval, optlen);
}

int
appview_socket(int domain, int type, int protocol) {
    return appviewlibc_socket(domain, type, protocol);
}

int
appview_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return appviewlibc_bind(sockfd, addr, addrlen);
}

int
appview_accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen) {
    return appviewlibc_accept(sockfd, addr, addrlen);
}

int
appview_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return appviewlibc_connect(sockfd, addr, addrlen);
}

int
appview_listen(int socket, int backlog) {
    return appviewlibc_listen(socket, backlog);
}

void
appview_rewind(FILE *stream) {
    appviewlibc_rewind(stream);
}

ssize_t
appview_send(int sockfd, const void *buf, size_t len, int flags) {
    return appviewlibc_send(sockfd, buf, len, flags);
}

ssize_t
appview_sendmsg(int socket, const struct msghdr *message, int flags) {
    return appviewlibc_sendmsg(socket, message, flags);
}

ssize_t
appview_recv(int sockfd, void *buf, size_t len, int flags) {
    return appviewlibc_recv(sockfd, buf, len, flags);
}

ssize_t
appview_recvmsg(int socket, struct msghdr *message, int flags) {
    return appviewlibc_recvmsg(socket, message, flags);
}

ssize_t
appview_recvfrom(int sockfd, void *restrict buf, size_t len, int flags, struct sockaddr *restrict src_addr, socklen_t *restrict addrlen) {
    return appviewlibc_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

int
appview_shutdown(int sockfd, int how) {
    return appviewlibc_shutdown(sockfd, how);
}

int
appview_poll(struct pollfd *fds, nfds_t nfds, int timeout) {
    return appviewlibc_poll(fds, nfds, timeout);
}

int
appview_select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds, fd_set *restrict exceptfds, struct timeval *restrict timeout) {
    return appviewlibc_select(nfds, readfds, writefds, exceptfds, timeout);
}

int
appview_getaddrinfo(const char *restrict node, const char *restrict service, const struct addrinfo *restrict hints, struct addrinfo **restrict res) {
    return appviewlibc_getaddrinfo(node, service, hints, res);
}

int
appview_copyaddrinfo(struct sockaddr *addr, socklen_t addrlen, struct addrinfo **restrict res) {
    return appviewlibc_copyaddrinfo(addr, addrlen, res);
}

void
appview_freeaddrinfo(struct addrinfo *ai) {
    appviewlibc_freeaddrinfo(ai);
}

int appview_getnameinfo(const struct sockaddr *restrict addr, socklen_t addrlen, char *restrict host, socklen_t hostlen, char *restrict serv, socklen_t servlen, int flags) {
    return appviewlibc_getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags);
}

int
appview_getpeername(int fd, struct sockaddr *restrict addr, socklen_t *restrict len) {
    return appviewlibc_getpeername(fd, addr, len);
}

struct hostent*
appview_gethostbyname(const char *name) {
    return appviewlibc_gethostbyname(name);
}

const char *
appview_inet_ntop(int af, const void *restrict src, char *restrict dst, socklen_t size) {
    return appviewlibc_inet_ntop(af, src, dst, size);
}

uint16_t
appview_ntohs(uint16_t netshort) {
    return appviewlibc_ntohs(netshort);
}

uint16_t
appview_htons(uint16_t hostshort) {
    return appviewlibc_htons(hostshort);
}

// Misc

int
appview_atoi(const char *nptr) {
    return appviewlibc_atoi(nptr);
}

int
appview_isspace(int c) {
    return appviewlibc_isspace(c);
}

int
appview_isprint(int c) {
    return appviewlibc_isprint(c);
}

int
appview_isdigit(int c) {
    return appviewlibc_isdigit(c);
}

void
appview_perror(const char *s) {
    appviewlibc_perror(s);
}

int
appview_gettimeofday(struct timeval *restrict tv, struct timezone *restrict tz) {
    return appviewlibc_gettimeofday(tv, tz);
}

struct tm *
appview_localtime_r(const time_t *timep, struct tm *result) {
    return appviewlibc_localtime_r(timep, result);
}

int
appview_timer_create(clockid_t clockid, struct sigevent *restrict sevp, timer_t *restrict timerid) {
    return appviewlibc_timer_create(clockid, sevp, timerid);
}

int
appview_timer_settime(timer_t timerid, int flags, const struct itimerspec *restrict new_value, struct itimerspec *restrict old_value) {
    return appviewlibc_timer_settime(timerid, flags, new_value, old_value);
}

int
appview_timer_delete(timer_t timerid) {
    return appviewlibc_timer_delete(timerid);
}

time_t
appview_time(time_t *tloc) {
    return appviewlibc_time(tloc);
}

struct tm *
appview_gmtime_r(const time_t *timep, struct tm *result) {
    return appviewlibc_gmtime_r(timep, result);
}

unsigned int
appview_sleep(unsigned int seconds) {
    return appviewlibc_sleep(seconds);
}

int
appview_usleep(useconds_t usec) {
    return appviewlibc_usleep(usec);
}

int
appview_nanosleep(const struct timespec *req, struct timespec *rem) {
    return appviewlibc_nanosleep(req, rem);
}

int
appview_sigaction(int signum, const struct sigaction *restrict act, struct sigaction *restrict oldact) {
    return appviewlibc_sigaction(signum, act, oldact);
}

int
appview_sigemptyset(sigset_t * set) {
    return appviewlibc_sigemptyset(set);
}

int
appview_sigfillset(sigset_t *set) {
    return appviewlibc_sigfillset(set);
}

int
appview_sigdelset(sigset_t *set, int signo) {
    return appviewlibc_sigdelset(set, signo);
}

int
appview_pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg) {
    return appviewlibc_pthread_create(thread, attr, start_routine, arg);
}

int
appview_pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned count) {
    return appviewlibc_pthread_barrier_init(barrier, attr, count);
}

int
appview_pthread_barrier_destroy(pthread_barrier_t *barrier) {
    return appviewlibc_pthread_barrier_destroy(barrier);
}

int
appview_pthread_barrier_wait(pthread_barrier_t *barrier) {
    return appviewlibc_pthread_barrier_wait(barrier);
}

int
appview_ns_initparse(const u_char *msg, int msglen, ns_msg *handle) {
    return appviewlibc_ns_initparse(msg, msglen, handle);
}

int
appview_ns_parserr(ns_msg *handle, ns_sect section, int rrnum, ns_rr *rr) {
    return appviewlibc_ns_parserr(handle, section, rrnum, rr);
}

int
appview_getgrgid_r(gid_t gid, struct group *restrict grp, char *restrict buf, size_t buflen, struct group **restrict result) {
    return appviewlibc_getgrgid_r(gid, grp, buf, buflen, result);
}

int
appview_getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result) {
    return appviewlibc_getpwuid_r(uid, pwd, buf, buflen, result);
}

pid_t
appview_getpid(void) {
    return appviewlibc_getpid();
}

pid_t
appview_getppid(void) {
    return appviewlibc_getppid();
}

uid_t
appview_getuid(void) {
    return appviewlibc_getuid();
}

uid_t
appview_geteuid(void) {
    return appviewlibc_geteuid();
}

gid_t
appview_getegid(void) {
    return appviewlibc_getegid();
}

int
appview_seteuid(uid_t euid) {
    return appviewlibc_seteuid(euid);
}

int
appview_setegid(gid_t egid) {
    return appviewlibc_setegid(egid);
}

gid_t
appview_getgid(void) {
    return appviewlibc_getgid();
}

pid_t
appview_getpgrp(void) {
    return appviewlibc_getpgrp();
}

void *
appview_dlopen(const char *filename, int flags) {
    return appviewlibc_dlopen(filename, flags);
}

int
appview_dlclose(void *handle) {
    return appviewlibc_dlclose(handle);
}

void *
appview_dlsym(void *restrict handle, const char *restrict symbol) {
    return appviewlibc_dlsym(handle, symbol);
}

long
appview_ptrace(int request, pid_t pid, void *addr, void *data) {
    return appviewlibc_ptrace(request, pid, addr, data);
}

pid_t
appview_waitpid(pid_t pid, int *status, int options) {
    return appviewlibc_waitpid(pid, status, options);
}

char *
appview_getenv(const char *name) {
    return appviewlibc_getenv(name);
}

int
appview_setenv(const char *name, const char *value, int overwrite) {
    return appviewlibc_setenv(name, value, overwrite);
}

struct lconv *
appview_localeconv(void) {
    return appviewlibc_localeconv();
}

int
appview_shm_open(const char *name, int oflag, mode_t mode) {
    return appviewlibc_shm_open(name, oflag, mode);
}

int
appview_shm_unlink(const char *name) {
    return appviewlibc_shm_unlink(name);
}

long
appview_sysconf(int name) {
    return appviewlibc_sysconf(name);
}

int
appview_mkstemp(char *template) {
    return appviewlibc_mkstemp(template);
}

int
appview_clock_gettime(clockid_t clk_id, struct timespec *tp) {
    return appviewlibc_clock_gettime(clk_id, tp);
}

int
appview_getpagesize(void) {
    return appviewlibc_getpagesize();
}

int
appview_uname(struct utsname *buf) {
    return appviewlibc_uname(buf);
}

int
appview_arch_prctl(int code, unsigned long addr) {
#if defined(__x86_64)
    return appviewlibc_arch_prctl(code, addr);
#else
    //arch_prctl is supported only on Linux/x86-64 for 64-bit
    return -1;
#endif
}

int
appview_getrusage(int who, struct rusage *usage) {
    return appviewlibc_getrusage(who, usage);
}

int
appview_atexit(void (*atexit_func)(void)) {
    return appviewlibc_atexit(atexit_func);
}

int
appview_tcsetattr(int fildes, int optional_actions, const struct termios *termios_p) {
    return appviewlibc_tcsetattr(fildes, optional_actions, termios_p);
}

int
appview_tcgetattr(int fildes, struct termios *termios_p) {
    return appviewlibc_tcgetattr(fildes, termios_p);
}

void *
appview_shmat(int shmid, const void *shmaddr, int shmflg) {
    return appviewlibc_shmat(shmid, shmaddr, shmflg);
}

int
appview_shmdt(const void *shmaddr) {
    return appviewlibc_shmdt(shmaddr);
}

int
appview_shmget(key_t key, size_t size, int shmflg) {
    return appviewlibc_shmget(key, size, shmflg);
}

int
appview_sched_getcpu(void) {
    return appviewlibc_sched_getcpu();
}

int
appview_ftruncate(int fildes, off_t length) {
    return appviewlibc_ftruncate(fildes, length);
}

int
appview___snprintf_chk(char *str, size_t maxlen, int flag, size_t slen, const char * format, ...) {
    int ret;
    va_list ap;
    va_start(ap, format);
    ret = appview_vsnprintf(str, slen, format, ap);
    va_end(ap);
    return ret;
}

int
appview___vfprintf_chk(FILE *fp, int flag, const char *format, va_list ap) {
    return appview_vfprintf(fp, format, ap);
}

int
appview___vsnprintf_chk(char *s, size_t maxlen, int flag, size_t slen, const char *format, va_list args) {
    return appview_vsnprintf(s, slen, format, args);
}

char *
appview___strcpy_chk(char *dest, const char *src, size_t destlen) {
    return appview_strcpy(dest, src);
}

int
appview__iso99_sscanf(const char *restrict s, const char *restrict fmt, ...) {
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = appview_vsscanf(s, fmt, ap);
    va_end(ap);
    return ret;
}

unsigned short **
appview___ctype_b_loc (void) {
    return appviewlibc___ctype_b_loc();
}

int32_t **
appview___ctype_tolower_loc(void) {
    return appviewlibc___ctype_tolower_loc();
}

int
appview_rand(void) {
    return appviewlibc_rand();
}

void
appview_srand(unsigned int seed) {
    appviewlibc_srand(seed);
}

int
appview_setns(int fd, int nstype) {
    return appviewlibc_setns(fd, nstype);
}

int
appview_chown(const char *pathname, uid_t owner, gid_t group) {
    return appviewlibc_chown(pathname, owner, group);
}

int
appview_fchown(int fd, uid_t owner, gid_t group) {
    return appviewlibc_fchown(fd, owner, group);
}

int
appview_getc(FILE *stream) {
    return appviewlibc_getc(stream);
}

int
appview_putc(int c, FILE *stream) {
    return appviewlibc_putc(c, stream);
}

int
appview_symlink(const char *target, const char *linkpath) {
    return appviewlibc_symlink(target, linkpath);
}

int
appview_mq_close(mqd_t mqdes) {
    return appviewlibc_mq_close(mqdes);
}

int
appview_mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio) {
    return appviewlibc_mq_send(mqdes, msg_ptr, msg_len, msg_prio);
}

ssize_t
appview_mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio) {
    return appviewlibc_mq_receive(mqdes, msg_ptr, msg_len, msg_prio);
}

int
appview_mq_unlink(const char *name) {
    return appviewlibc_mq_unlink(name);
}

int
appview_mq_getattr(mqd_t mqd, struct mq_attr *attr) {
    return appviewlibc_mq_getattr(mqd, attr);
}

char *
appview_secure_getenv(const char *name) {
    return getenv(name);
}
