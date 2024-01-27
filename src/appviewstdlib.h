#ifndef __APPVIEW_STDLIB_H__
#define __APPVIEW_STDLIB_H__

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <dirent.h>
#include <grp.h>
#include <link.h>
#include <locale.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <mqueue.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/*
* Following macro is commonly used in several places
* Note: If the set of common used macro used will grow
* please consider moving these macros to separate file
*/

/*
* Size of array
*/
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

/*
* Constant String length
*/ 
#define C_STRLEN(a)  (sizeof(a) - 1)

extern int  appviewlibc_fcntl(int, int, ... /* arg */);
extern int  appviewlibc_open(const char *, int, ...);
extern long appviewlibc_syscall(long, ...);
extern int  appviewlibc_printf(const char *, ...);
extern int  appviewlibc_dprintf(int, const char *, ...);
extern int  appviewlibc_fprintf(FILE *, const char *, ...);
extern int  appviewlibc_snprintf(char *, size_t, const char *, ...);
extern int  appviewlibc_sscanf(const char *, const char *, ...);
extern int  appviewlibc_fscanf(FILE *, const char *, ...);
extern int  appviewlibc_sprintf(char *, const char *, ...);
extern int  appviewlibc_asprintf(char **, const char *, ...);
extern int  appviewlibc_mq_open(const char *, int, ...);
extern int  appviewlibc_errno_val;
extern FILE appviewlibc___stdin_FILE;
extern FILE appviewlibc___stdout_FILE;
extern FILE appviewlibc___stderr_FILE;
extern unsigned short ** appviewlibc___ctype_b_loc(void);
extern int32_t ** appviewlibc___ctype_tolower_loc(void);

#define appview_fcntl    appviewlibc_fcntl
#define appview_open     appviewlibc_open
#define appview_syscall  appviewlibc_syscall
#define appview_printf   appviewlibc_printf
#define appview_dprintf  appviewlibc_dprintf
#define appview_fprintf  appviewlibc_fprintf
#define appview_snprintf appviewlibc_snprintf
#define appview_sscanf   appviewlibc_sscanf
#define appview_fscanf   appviewlibc_fscanf
#define appview_sprintf  appviewlibc_sprintf
#define appview_asprintf appviewlibc_asprintf
#define appview_mq_open  appviewlibc_mq_open
#define appview_errno    appviewlibc_errno_val
#define appview_stdin    (&appviewlibc___stdin_FILE)
#define appview_stdout   (&appviewlibc___stdout_FILE)
#define appview_stderr   (&appviewlibc___stderr_FILE)

/*
 * Notes on the use of errno.
 * There are 2 errno values; errno and appview_errno.
 *
 * errno is set by the application. It should be
 * used, for the most part, by interposed functions where
 * results of application behavior needs to be checked. It
 * should only ever be set by libappview in specific cases.
 *
 * appview_errno is used by the internal libc.
 * This value is not thread specific, thread safe, as we avoid the
 * use of the %fs register and TLS behavior with the internal libc.
 *
 * Use appview_errno only for functions called from the periodic
 * thread, during libappview constructor, or from appview.
 *
 * Other functions, primarily those called from interposed functions, can
 * not safely reference appview_errno.
 */
// Other
extern void appviewSetGoAppStateStatic(int);
extern int appviewGetGoAppStateStatic(void);

// Custom operations

void  appview_init_vdso_ehdr(void);
void  appview_op_before_fork(void);
void  appview_op_after_fork(int);

// Memory management handling operations
void* appview_memalign(size_t, size_t);
void* appview_malloc(size_t);
void* appview_calloc(size_t, size_t);
void* appview_realloc(void *, size_t);
void  appview_free(void *);
void* appview_mmap(void *, size_t, int, int, int, off_t);
int   appview_munmap(void *, size_t);
FILE* appview_open_memstream(char **, size_t *);
void* appview_memset(void *, int, size_t);
void* appview_memmove(void *, const void *, size_t);
int   appview_memcmp(const void *, const void *, size_t);
int   appview_mprotect(void *, size_t, int);
void* appview_memcpy(void *, const void *, size_t);
int   appview_mlock(const void *, size_t);
int   appview_msync(void *, size_t, int);
int   appview_mincore(void *, size_t, unsigned char *);
int   appview_memfd_create(const char *, unsigned int);

// File handling operations
FILE*          appview_fopen(const char *, const char *);
int            appview_fclose(FILE *);
FILE*          appview_fdopen(int, const char *);
int            appview_close(int);
ssize_t        appview_read(int, void *, size_t);
size_t         appview_fread(void *, size_t, size_t, FILE *);
ssize_t        appview_write(int, const void *, size_t);
size_t         appview_fwrite(const void *, size_t, size_t, FILE *);
char *         appview_fgets(char *, int, FILE *);
ssize_t        appview_getline(char **, size_t *, FILE *);
int            appview_puts(const char *);
int            appview_setvbuf(FILE *, char *, int, size_t);
int            appview_fflush(FILE *);
char*          appview_dirname(char *);
DIR*           appview_opendir(const char *);
struct dirent* appview_readdir(DIR *);
int            appview_closedir(DIR *);
int            appview_access(const char *, int);
FILE*          appview_fmemopen(void *, size_t, const char *);
long           appview_ftell(FILE *);
int            appview_fseek(FILE *, long, int);
off_t          appview_lseek(int, off_t, int);
int            appview_unlink(const char *);
int            appview_dup2(int, int);
char*          appview_basename(char *);
int            appview_stat(const char *, struct stat *);
int            appview_chmod(const char *, mode_t);
int            appview_fchmod(int, mode_t);
int            appview_feof(FILE *);
int            appview_fileno(FILE *);
int            appview_flock(int, int);
int            appview_fstat(int, struct stat *);
int            appview_mkdir(const char *, mode_t);
int            appview_chdir(const char *);
int            appview_rmdir(const char *);
char*          appview_get_current_dir_name(void);
char*          appview_getcwd(char *, size_t);
int            appview_lstat(const char *, struct stat *);
int            appview_rename(const char *, const char *);
int            appview_remove(const char *);
int            appview_pipe2(int [2], int);
void           appview_setbuf(FILE *, char *);

// String handling operations
char*              appview_realpath(const char *, char *);
ssize_t            appview_readlink(const char *, char *, size_t);
char*              appview_strdup(const char *);
int                appview_vasprintf(char **, const char *, va_list);
size_t             appview_strftime(char *, size_t, const char *, const struct tm *);
size_t             appview_strlen(const char *);
size_t             appview_strnlen(const char *, size_t);
char *             appview_strerror(int);
int                appview_strerror_r(int, char *, size_t);
double             appview_strtod(const char *, char **);
long               appview_strtol(const char *, char **, int);
long long          appview_strtoll(const char *, char **, int);
unsigned long      appview_strtoul(const char *, char **, int);
unsigned long long appview_strtoull(const char *, char **, int);
char*              appview_strchr(const char *, int);
char*              appview_strrchr(const char *, int);
char*              appview_strstr(const char *, const char *);
int                appview_vsnprintf(char *, size_t, const char *, va_list);
int                appview_vfprintf(FILE *, const char *, va_list);
int                appview_vprintf(const char *, va_list);
int                appview_strcmp(const char *, const char *);
int                appview_strncmp(const char *, const char *, size_t);
int                appview_strcasecmp(const char *, const char *);
char *             appview_strchrnul(const char *, int);
char *             appview_strcpy(char *, const char *);
char *             appview_strncpy(char *, const char *, size_t);
char *             appview_stpcpy(char *, const char *);
char *             appview_stpcpy(char *, const char *);
char *             appview_stpncpy(char *, const char *, size_t);
size_t             appview_strcspn(const char *, const char *);
char *             appview_strcat(char *, const char *);
char *             appview_strncat(char *, const char *, size_t);
char *             appview_strpbrk(const char *, const char *);
char *             appview_strcasestr(const char *, const char *);
char *             appview_strtok(char *, const char *);
char *             appview_strtok_r(char *, const char *, char **);
const char *       appview_gai_strerror(int);

// Network handling operations
int             appview_gethostname(char *, size_t);
int             appview_getsockname(int, struct sockaddr *, socklen_t *);
int             appview_getsockopt(int, int, int, void *, socklen_t *);
int             appview_setsockopt(int, int, int, const void *, socklen_t);
int             appview_socket(int, int, int);
int             appview_accept(int, struct sockaddr *, socklen_t *);
int             appview_bind(int, const struct sockaddr *, socklen_t);
int             appview_connect(int, const struct sockaddr *, socklen_t);
int             appview_listen(int, int);
void            appview_rewind(FILE *);
ssize_t         appview_send(int, const void *, size_t, int);
ssize_t         appview_sendmsg(int, const struct msghdr *, int);
ssize_t         appview_recv(int, void *, size_t, int);
ssize_t         appview_recvmsg(int, struct msghdr *, int);
ssize_t         appview_recvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);
int             appview_shutdown(int, int);
int             appview_poll(struct pollfd *, nfds_t, int);
int             appview_select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
int             appview_getaddrinfo(const char *, const char *, const struct addrinfo *, struct addrinfo **);
int             appview_copyaddrinfo(struct sockaddr *, socklen_t, struct addrinfo **);
void            appview_freeaddrinfo(struct addrinfo *);
int             appview_getnameinfo(const struct sockaddr *, socklen_t, char *, socklen_t, char *, socklen_t, int);
int             appview_getpeername(int, struct sockaddr *, socklen_t *);
struct hostent* appview_gethostbyname(const char *);
const char*     appview_inet_ntop(int, const void *, char *, socklen_t);
uint16_t        appview_ntohs(uint16_t);
uint16_t        appview_htons(uint16_t);

// Misc
int           appview_atoi(const char *);
int           appview_isspace(int);
int           appview_isprint(int);
int           appview_isdigit(int);
void          appview_perror(const char *);
int           appview_gettimeofday(struct timeval *, struct timezone *);
int           appview_timer_create(clockid_t, struct sigevent *, timer_t *);
int           appview_timer_settime(timer_t, int, const struct itimerspec *, struct itimerspec *);
int           appview_timer_delete(timer_t);
time_t        appview_time(time_t *);
struct tm*    appview_localtime_r(const time_t *, struct tm *);
struct tm*    appview_gmtime_r(const time_t *, struct tm *);
unsigned int  appview_sleep(unsigned int);
int           appview_usleep(useconds_t);
int           appview_nanosleep(const struct timespec *, struct timespec *);
int           appview_sigaction(int, const struct sigaction *, struct sigaction *);
int           appview_sigemptyset(sigset_t *);
int           appview_sigfillset(sigset_t *);
int           appview_sigdelset(sigset_t *, int);
int           appview_pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
int           appview_pthread_barrier_init(pthread_barrier_t *, const pthread_barrierattr_t *, unsigned);
int           appview_pthread_barrier_destroy(pthread_barrier_t *);
int           appview_pthread_barrier_wait(pthread_barrier_t *);;
int           appview_ns_initparse(const unsigned char *, int, ns_msg *);
int           appview_ns_parserr(ns_msg *, ns_sect, int, ns_rr *);
int           appview_getgrgid_r(gid_t, struct group *, char *, size_t, struct group **);
int           appview_getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);
pid_t         appview_getpid(void);
pid_t         appview_getppid(void);
uid_t         appview_getuid(void);
uid_t         appview_geteuid(void);
gid_t         appview_getegid(void);
int           appview_seteuid(uid_t);
int           appview_setegid(gid_t);
gid_t         appview_getgid(void);
pid_t         appview_getpgrp(void);
void*         appview_dlopen(const char *, int);
void*         appview_dlsym(void *, const char *);
int           appview_dlclose(void *);
long          appview_ptrace(int, pid_t, void *, void *);
pid_t         appview_waitpid(pid_t, int *, int);
char*         appview_getenv(const char *);
int           appview_setenv(const char *, const char *, int);
struct lconv* appview_localeconv(void);
int           appview_shm_open(const char *, int, mode_t);
int           appview_shm_unlink(const char *);
long          appview_sysconf(int);
int           appview_mkstemp(char *);
int           appview_clock_gettime(clockid_t, struct timespec *);
int           appview_getpagesize(void);
int           appview_uname(struct utsname *);
int           appview_arch_prctl(int, unsigned long);
int           appview_getrusage(int, struct rusage *);
int           appview_atexit(void (*)(void));
int           appview_tcsetattr(int, int, const struct termios *);
int           appview_tcgetattr(int, struct termios *);
void*         appview_shmat(int, const void *, int);
int           appview_shmdt(const void *);
int           appview_shmget(key_t, size_t, int);
int           appview_sched_getcpu(void);
int           appview_ftruncate(int, off_t);
int           appview_rand(void);
void          appview_srand(unsigned int);
int           appview_setns(int, int);
int           appview_chown(const char *, uid_t, gid_t);
int           appview_fchown(int, uid_t, gid_t);
int           appview_getc(FILE *);
int           appview_putc(int, FILE *);
int           appview_symlink(const char *, const char *);
int           appview_mq_close(mqd_t);
int           appview_mq_send(mqd_t, const char *, size_t, unsigned int);
ssize_t       appview_mq_receive(mqd_t, char *, size_t, unsigned int *);
int           appview_mq_unlink(const char *);
int           appview_mq_getattr(mqd_t, struct mq_attr *);


#endif // __APPVIEW_STDLIB_H__
