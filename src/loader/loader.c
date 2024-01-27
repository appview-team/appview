#define _GNU_SOURCE

#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "attach.h"
#include "inject.h"
#include "loader.h"
#include "libdir.h"
#include "loaderutils.h"
#include "nsinfo.h"
#include "nsfile.h"
#include "ns.h"
#include "patch.h"
#include "setup.h"
 
// TODO use rootdir instead of nspid for Service and Unservice. Deprecate --namespace?
int
cmdService(char *serviceName, pid_t nspid)
{
    uid_t eUid = geteuid();
    gid_t eGid = getegid();
    uid_t nsUid = eUid;
    uid_t nsGid = eGid;

    if (!serviceName) {
        return EXIT_FAILURE;
    }

    // Change mnt namespace if nspid is provided
    if (nspid != -1) {
        nsUid = nsInfoTranslateUidRootDir("", nspid);
        nsGid = nsInfoTranslateGidRootDir("", nspid);

        if (nsSetNsRootDir("", nspid, "mnt") == FALSE) {
            return EXIT_FAILURE;
        }
    }

    // Set up the service
    if (setupService(serviceName, nsUid, nsGid)) {
        fprintf(stderr, "error: failed to setup service\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
cmdUnservice(pid_t nspid)
{
    // Change mnt namespace if nspid is provided
    if (nspid != -1) {
        if (nsSetNsRootDir("", nspid, "mnt") == FALSE) {
            return EXIT_FAILURE;
        }
    }

    // Unservice
    if (setupUnservice()) {
        fprintf(stderr, "error: failed to setup unservice\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
 * If attaching to a process in the current namespace:
 * - follow regular attach logic to ultimately PTRACE attach
 * If attaching to a process in a child namespace:
 * - fork and exec a child appview process in the child PID and MNT namespace (nsForkAndExec)
 * - write the appview loader and configuration (joinChildNamespace)
 * - follow regular attach logic to ultimately PTRACE attach
 * If attaching to a process in a parent/sibling namespace (--rootdir specified)
 * it is not possible to change PID ns to that of a parent, so:
 * - change to the target mnt namespace (nsAttach)
 * - write the appview loader and configuration
 * - create a shell script, executed by cron, to perform the appview attach
 */
int
cmdAttach(pid_t pid, const char* rootdir)
{
    int res = EXIT_FAILURE;
    char *appviewLibPath;
    char path[PATH_MAX] = {0};
    char env_path[PATH_MAX];
    uid_t eUid = geteuid();
    gid_t eGid = getegid();
    uid_t nsUid = eUid;
    uid_t nsGid = eGid;
    elf_buf_t *appview_ebuf = NULL;
    elf_buf_t *ebuf = NULL;

    if (rootdir) {
        return nsAttach(pid, rootdir);
    }

    nsUid = nsInfoTranslateUidRootDir("", pid);
    nsGid = nsInfoTranslateGidRootDir("", pid);

    appview_ebuf = getElf("/proc/self/exe");
    if (!appview_ebuf) {
        perror("setenv");
        goto out;
    }

    // Extract and patch libappview from appview static. Don't attempt to extract from appview dynamic
    if (is_static(appview_ebuf->buf)) {
        if (libdirExtract(NULL, 0, nsUid, nsGid, LIBRARY_FILE)) {
            fprintf(stderr, "error: failed to extract library\n");
            goto out;
        }

        appviewLibPath = (char *)libdirGetPath(LIBRARY_FILE);

        if (patchLibrary(appviewLibPath, FALSE) == PATCH_FAILED) {
            fprintf(stderr, "error: failed to patch library\n");
            goto out;
        }
    } else {
        appviewLibPath = (char *)libdirGetPath(LIBRARY_FILE);
    }

    if (access(appviewLibPath, R_OK|X_OK)) {
        fprintf(stderr, "error: library %s is missing, not readable, or not executable\n", appviewLibPath);
        goto out;
    }

    if (setenv("APPVIEW_LIB_PATH", appviewLibPath, 1)) {
        perror("setenv(APPVIEW_LIB_PATH) failed");
        goto out;
    }

    // Set APPVIEW_PID_ENV
    setPidEnv(getpid());

    /*
     * Get the pid as int
     * Validate that the process exists
     * Perform namespace switch if required
     * Is the library currently loaded in the target process
     * Attach using ptrace or a dynamic command, depending on lib presence
     * Return at end of the operation
     */
    pid_t nsAttachPid = 0;

    snprintf(path, sizeof(path), "/proc/%d", pid);
    if (access(path, F_OK)) {
        printf("error: --ldattach, --lddetach PID not a current process: %d\n", pid);
        goto out;
    }

    uint64_t rc = findLibrary("libappview.so", pid, FALSE, NULL, 0);

    /*
    * If the expected process exists in different PID namespace (container)
    * we do a following switch context sequence:
    * - load static loader file into memory
    * - [optionally] save the configuration file pointed by APPVIEW_CONF_PATH into memory
    * - switch the namespace from parent
    * - save previously loaded static loader into new namespace
    * - [optionally] save previously loaded configuration file into new namespace
    * - fork & execute static loader attach one more time with updated PID
    */
    if (nsInfoGetPidNs(pid, &nsAttachPid) == TRUE) {
        // must be root to switch namespace
        if (eUid) {
            printf("error: --ldattach requires root\n");
            goto out;
        }

        res = nsForkAndExec(pid, nsAttachPid, TRUE);
        goto out;
    /*
    * Process can exists in same PID namespace but in different mnt namespace
    * we do a simillar sequence like above but without switching PID namespace
    * and updating PID.
    */
    } else if (nsInfoIsPidInSameMntNs(pid) == FALSE) {
        // must be root to switch namespace
        if (eUid) {
            printf("error: --ldattach requires root\n");
            goto out;
        }
        res =  nsForkAndExec(pid, pid, TRUE);
        goto out;
    }

    // create /dev/shm/${PID}.env when attaching, for the library to load
    snprintf(env_path, sizeof(env_path), "/attach_%d.env", pid);
    int fd = nsFileShmOpen(env_path, O_RDWR|O_CREAT, S_IRUSR|S_IRGRP|S_IROTH, nsUid, nsGid, eUid, eGid);
    if (fd == -1) {
        fprintf(stderr, "nsFileShmOpen failed\n");
        goto out;
    }

    // add the env vars we want in the library
    dprintf(fd, "APPVIEW_LIB_PATH=%s\n", libdirGetPath(LIBRARY_FILE));

    int i;
    for (i = 0; environ[i]; i++) {
        if (strlen(environ[i]) > 6 && strncmp(environ[i], "APPVIEW_", 6) == 0) {
            dprintf(fd, "%s\n", environ[i]);
        }
    }

    // done
    close(fd);

    // rc from findLibrary
    if (rc == -1) {
        fprintf(stderr, "error: can't get path to executable for pid %d\n", pid);
        res = EXIT_FAILURE;
    } else if (rc == 0) {
        // proc exists, libappview does not exist, a load & attach
        res = load_and_attach(pid, appviewLibPath);
    } else {
        // libappview exists, a first time attach or a reattach
        res = attach(pid);
    }

    // remove the env var file
    snprintf(env_path, sizeof(env_path), "/attach_%d.env", pid);
    shm_unlink(env_path);

out:
    if (ebuf) {
        freeElf(ebuf->buf, ebuf->len);
        free(ebuf);
    }

    if (appview_ebuf) {
        freeElf(appview_ebuf->buf, appview_ebuf->len);
        free(appview_ebuf);
    }

    exit(res);
}

int
cmdDetach(pid_t pid, const char *rootdir)
{
    if (pid < 1) {
        return EXIT_FAILURE;
    }

    // Change mnt namespace if rootdir is provided
    if (rootdir) {
        if (nsSetNsRootDir(rootdir, 1, "mnt") == FALSE) {
            fprintf(stderr, "nsDetach mnt failed\n");
            return EXIT_FAILURE;
        }
    }

    // Detach
    if (detach(pid)) {
        fprintf(stderr, "error: failed to detach\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
cmdRun(pid_t pid, pid_t nspid, int argc, char **argv)
{
    char *appviewLibPath;
    uid_t eUid = geteuid();
    gid_t eGid = getegid();
    uid_t nsUid = eUid;
    uid_t nsGid = eGid;
    elf_buf_t *appview_ebuf = NULL;
    elf_buf_t *ebuf = NULL;

    if (nspid != -1) {
        nsUid = nsInfoTranslateUidRootDir("", nspid);
        nsGid = nsInfoTranslateGidRootDir("", nspid);
    }

    appview_ebuf = getElf("/proc/self/exe");
    if (!appview_ebuf) {
        perror("setenv");
        goto out;
    }

    // Extract and patch libappview from appview static. Don't attempt to extract from appview dynamic
    if (is_static(appview_ebuf->buf)) {
        if (libdirExtract(NULL, 0, nsUid, nsGid, LIBRARY_FILE)) {
            fprintf(stderr, "error: failed to extract library\n");
            goto out;
        }

        appviewLibPath = (char *)libdirGetPath(LIBRARY_FILE);

        if (patchLibrary(appviewLibPath, FALSE) == PATCH_FAILED) {
            fprintf(stderr, "error: failed to patch library\n");
            goto out;
        }
    } else {
        appviewLibPath = (char *)libdirGetPath(LIBRARY_FILE);
    }

    if (access(appviewLibPath, R_OK|X_OK)) {
        fprintf(stderr, "error: library %s is missing, not readable, or not executable\n", appviewLibPath);
        goto out;
    }

    if (setenv("APPVIEW_LIB_PATH", appviewLibPath, 1)) {
        perror("setenv(APPVIEW_LIB_PATH) failed");
        goto out;
    }

    // Set APPVIEW_PID_ENV
    setPidEnv(getpid());


    /*
     * What kind of app are we trying to appview?
     */

    char *inferior_command = NULL;

    inferior_command = getpath(argv[0]); 
    if (!inferior_command) {
        fprintf(stderr,"appview could not find or execute command `%s`.  Exiting.\n", argv[0]);
        goto out;
    }

    ebuf = getElf(inferior_command);

    if (ebuf && (is_go(ebuf->buf) == TRUE)) {
        if (setenv("APPVIEW_APP_TYPE", "go", 1) == -1) {
            perror("setenv");
            goto out;
        }
    } else {
        if (setenv("APPVIEW_APP_TYPE", "native", 1) == -1) {
            perror("setenv");
            goto out;
        }
    }


    /*
     * If the app we want to appview is Dynamic
     * Just exec it with LD_PRELOAD and we're done
     */

    if ((ebuf == NULL) || (!is_static(ebuf->buf))) {
        if (ebuf) freeElf(ebuf->buf, ebuf->len);

        if (setenv("LD_PRELOAD", appviewLibPath, 0) == -1) {
            perror("setenv");
            goto out;
        }

        if (setenv("APPVIEW_EXEC_TYPE", "dynamic", 1) == -1) {
            perror("setenv");
            goto out;
        }

        goto out;
    }


    /*
     * If the app we want to appview is Static
     * There are determinations to be made
     */

    if (setenv("APPVIEW_EXEC_TYPE", "static", 1) == -1) {
        perror("setenv");
        goto out;
    }

    // Add a comment here
    if (getenv("LD_PRELOAD") != NULL) {
        unsetenv("LD_PRELOAD");
        goto out;
    }

    program_invocation_short_name = basename(argv[0]);

    // If it's not a Go app, we don't support it
    // so just exec it without appview
    if (!is_go(ebuf->buf)) {
        // We're getting here with upx-encoded binaries
        // and any other static native apps...
        // Start here when we support more static binaries
        // than go.
        goto out;
    }

    // If appview itself is static, we need to call appview dynamic
    // because we need to use dlopen
    if (is_static(appview_ebuf->buf)) {

        // Get appviewdyn from the appview executable
        unsigned char *start; 
        size_t len;
        if ((len = getAsset(LOADER_FILE, &start)) == -1) {
            fprintf(stderr, "error: failed to retrieve loader\n");
            goto out;
        }

        // Patch the appviewdyn executable on the heap (for musl support)
        if (patchLoader(start, nsUid, nsGid) == PATCH_FAILED) {
            fprintf(stderr, "error: failed to patch loader\n");
            goto out;
        }

#if 0   // Write appviewdyn to /tmp for debugging
        int fd_dyn; 
        if ((fd_dyn = open("/tmp/appviewdyn", O_CREAT | O_RDWR | O_TRUNC)) == -1) {
            perror("cmdRun:open");
            goto out;
        }
        int rc = write(fd_dyn, start, len);
        if (rc < len) {
            perror("cmdRun:write");
            goto out;
        }
        close(fd_dyn);
#endif
        // Write appviewdyn to shared memory
        char path_to_fd[PATH_MAX];
        int fd = memfd_create("", 0);
        if (fd == -1) {
            perror("memfd_create");
            goto out;
        }
        ssize_t written = write(fd, start, len);
        if (written != g_appviewdynsz) {
            fprintf(stderr, "error: failed to write loader to shm\n");
            goto out;
        }

        // Exec appviewdyn from shared memory
        // Append "appviewdyn" to argv first
        int execArgc = 0;
        char *execArgv[argc + 1];
        execArgv[execArgc++] = "appviewdyn";
        for (int i = 0; i < argc; i++) {
            execArgv[execArgc++] = argv[i];
        }
        execArgv[execArgc++] = NULL;
        sprintf(path_to_fd, "/proc/self/fd/%i", fd);
        execve(path_to_fd, &execArgv[0], environ);
        perror("execve");

    // If appview itself is dynamic, dlopen libappview and sys_exec the app
    // and we're done
    } else {
        // we should never be here. we dont run appview dynamic.
    }
 
out:
    /*
     * Cleanup and exec the user app (where possible)
     * If there are errors, the app will run without appview
     */
    if (ebuf) {
        freeElf(ebuf->buf, ebuf->len);
        free(ebuf);
    }

    if (appview_ebuf) {
        freeElf(appview_ebuf->buf, appview_ebuf->len);
        free(appview_ebuf);
    }

    if (inferior_command) {
        execve(inferior_command, &argv[0], environ);
        perror("execve");
    }

    exit(EXIT_FAILURE);
}

// Handle the install command
int
cmdInstall(const char *rootdir)
{
    uid_t eUid = geteuid();
    gid_t eGid = getegid();
    uid_t nsUid = eUid;
    uid_t nsGid = eGid;
    unsigned char *library_file = NULL;
    size_t library_file_len;
    unsigned char *loader_file = NULL;
    size_t loader_file_len;

    // Extract library from appview binary into memory while in origin namespace
    if ((library_file_len = getAsset(LIBRARY_FILE, &library_file)) == -1) {
        fprintf(stderr, "cmdInstall getAsset library failed\n");
        return EXIT_FAILURE;
    }

    // Extract loader from appview binary into memory while in origin namespace
    if ((loader_file_len = getAsset(STATIC_LOADER_FILE, &loader_file)) == -1) {
        fprintf(stderr, "cmdInstall getAsset loader failed\n");
        return EXIT_FAILURE;
    }

    // Change mnt namespace if rootdir is provided
    if (rootdir) {
        nsUid = nsInfoTranslateUidRootDir(rootdir, 1);
        nsGid = nsInfoTranslateGidRootDir(rootdir, 1);

        // Use pid 1 to locate ns fd
        if (nsSetNsRootDir(rootdir, 1, "mnt") == FALSE) {
            fprintf(stderr, "cmdInstall mnt failed\n");
            return EXIT_FAILURE;
        }
    }

    // Extract the library
    if (libdirExtract(library_file, library_file_len, nsUid, nsGid, LIBRARY_FILE)) {
        fprintf(stderr, "cmdInstall library extract failed\n");
        return EXIT_FAILURE;
    }

    // Extract the loader
    if (libdirExtract(loader_file, loader_file_len, nsUid, nsGid, STATIC_LOADER_FILE)) {
        fprintf(stderr, "cmdInstall loader extract failed\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Handle the rules command
int
cmdRules(const char *configRulesPath, const char *rootdir)
{
    uid_t eUid = geteuid();
    gid_t eGid = getegid();
    uid_t nsUid = eUid;
    uid_t nsGid = eGid;
    size_t configRulesSize = 0;

    if (!configRulesPath) {
        return EXIT_FAILURE;
    }

    // Read rules file in while in origin namespace
    void *configRulesMem = setupLoadFileIntoMem(&configRulesSize, configRulesPath);
    if (configRulesMem == NULL) {
        fprintf(stderr, "error: Loading rules file into memory %s\n", configRulesPath);
        return EXIT_FAILURE;
    }

    // Change mnt namespace if rootdir is provided
    if (rootdir) {
        nsUid = nsInfoTranslateUidRootDir(rootdir, 1);
        nsGid = nsInfoTranslateGidRootDir(rootdir, 1);

        // Use pid 1 to locate ns fd
        if (nsSetNsRootDir(rootdir, 1, "mnt") == FALSE) {
            fprintf(stderr, "nsSetNsRootDir mnt failed\n");
            return EXIT_FAILURE;
        }
    }

    // Install rules file
    if (!setupRules(configRulesMem, configRulesSize, nsUid, nsGid)) {
        fprintf(stderr, "error: failed to install rules file\n");
        return EXIT_FAILURE;
    }

    if (configRulesMem) munmap(configRulesMem, configRulesSize);

    return EXIT_SUCCESS;
}

// Handle the preload command
int
cmdPreload(const char *path, const char *rootdir)
{
    uid_t eUid = geteuid();
    gid_t eGid = getegid();
    uid_t nsUid = eUid;
    uid_t nsGid = eGid;

    // Change mnt namespace if rootdir is provided
    if (rootdir) {
        nsUid = nsInfoTranslateUidRootDir(rootdir, 1);
        nsGid = nsInfoTranslateGidRootDir(rootdir, 1);

        // Use pid 1 to locate ns fd
        if (nsSetNsRootDir(rootdir, 1, "mnt") == FALSE) {
            fprintf(stderr, "nsSetNsRootDir mnt failed\n");
            return EXIT_FAILURE;
        }
    }

    // Set ld.so.preload
    if (!setupPreload(path, nsUid, nsGid)) {
        fprintf(stderr, "error: failed to set ld.so.preload\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Handle the mount command
int
cmdMount(pid_t pid, const char *mountDest, const char *rootdir)
{
    uid_t eUid = geteuid();
    gid_t eGid = getegid();
    uid_t nsUid = eUid;
    uid_t nsGid = eGid;

    if (!mountDest) {
        return EXIT_FAILURE;
    }

    // Change mnt namespace if rootdir is provided
    if (rootdir) {
        nsUid = nsInfoTranslateUidRootDir(rootdir, 1);
        nsGid = nsInfoTranslateGidRootDir(rootdir, 1);

        // Use pid 1 to locate ns fd
        if (nsSetNsRootDir(rootdir, 1, "mnt") == FALSE) {
            fprintf(stderr, "nsSetNsRootDir mnt failed\n");
            return EXIT_FAILURE;
        }
    }

    // Set up the mount
    if (setupMount(pid, mountDest, nsUid, nsGid) == FALSE) {
        fprintf(stderr, "error: failed to mount\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

