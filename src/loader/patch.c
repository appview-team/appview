#define _GNU_SOURCE

#include <dirent.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "libdir.h"
#include "loaderutils.h"
#include "nsfile.h"
#include "patch.h"
#include "appviewtypes.h"

#define EXE_TEST_FILE "/bin/cat"
#define LIBMUSL "musl"

#if defined (__x86_64__)
#define MUSL_LD_NAME "ld-musl-x86_64.so.1"
#elif defined (__aarch64__)
#define MUSL_LD_NAME  "ld-musl-aarch64.so.1"
#else
#error Bad arch defined
#endif

/*
 * This code exists solely to support the ability to
 * build a libappview and a appview on a glibc distro
 * that will execute on both a glibc distro and a
 * musl distro.
 *
 * This code is used to create a static exec that will
 * execute on both glibc and musl distros.
 *
 * The process:
 * 1) extract the appview exec and libappview.so
 *    dynamic lib from this object.
 * 2) open an executable file on the current FS and
 *    read the loader string from the .interp section.
 * 3) if it uses a musl ld.so then do musl
 * 4) for musl; create a dir and in that dir create a
 *    soft link to ld-musl.so from ld.linux.so (the
 *    glibc loader).
 * 5) for musl; create or add to the ld lib path
 *    env var to point to the dir created above.
 * 6) for musl; modify the loader string in .interp
 *    of appview to ld-musl.so.
 * 7) execve the extracted appview passing args
 *    from this command line.
 */

static int g_debug = 0;

/*
static int
get_dir(const char *path, char *fres, size_t len) {
    DIR *dirp;
    struct dirent *entry;
    char *dcopy, *pcopy, *dname, *fname;
    int res = -1;

    if (!path || !fres || (len <= 0)) return res;

    pcopy = strdup(path);
    dname = dirname(pcopy);

    if ((dirp = opendir(dname)) == NULL) {
        perror("get_dir:opendir");
        if (pcopy) free(pcopy);
        return res;
    }

    dcopy = strdup(path);
    fname = basename(dcopy);

    while ((entry = readdir(dirp)) != NULL) {
        if ((entry->d_type != DT_DIR) &&
            (strstr(entry->d_name, fname))) {
            strncpy(fres, entry->d_name, len);
            res = 0;
            break;
        }
    }

    closedir(dirp);
    if (pcopy) free(pcopy);
    if (dcopy) free(dcopy);
    return res;
}
*/

// modify the loader string in the .interp section of appview
// return -1 on error
static int
set_loader(unsigned char *buf)
{
    int i;
    int found = 0;
    int name = 0;
    Elf64_Ehdr *elf;
    Elf64_Phdr *phead;

    if (!buf) return -1;

    elf = (Elf64_Ehdr *)buf;
    phead  = (Elf64_Phdr *)&buf[elf->e_phoff];

    for (i = 0; i < elf->e_phnum; i++) {
        if (phead[i].p_type == PT_INTERP) {
            char *exld = (char *)&buf[phead[i].p_offset];
            DIR *dirp;
            struct dirent *entry;
            char dir[PATH_MAX];
            size_t dir_len;

            if (strstr(exld, "ld-musl") != NULL) {
                return 0;
            }

            snprintf(dir, sizeof(dir), "/lib/");
            if ((dirp = opendir(dir)) == NULL) {
                perror("set_loader:opendir");
                break;
            }

            while ((entry = readdir(dirp)) != NULL) {
                if ((entry->d_type != DT_DIR) &&
                    (strstr(entry->d_name, "ld-musl"))) {
                    strncat(dir, entry->d_name, strlen(entry->d_name) + 1);
                    name = 1;
                    break;
                }
            }

            closedir(dirp);
            dir_len = strlen(dir);
            if (name && (strlen(exld) >= dir_len)) {
                if (g_debug) printf("%s:%d buf ld.so: %s to %s\n", __FUNCTION__, __LINE__, exld, dir);
                strncpy(exld, dir, dir_len + 1);
                found = 1;
                break;
            }
        }
    }

    if (!found) {
        fprintf(stderr, "WARNING: can't locate or set the loader string in %s\n", buf);
        return -1;
    }

    return 0;
}

static char *
get_loader(const unsigned char *buf) {
    int i;
    char *ldso = NULL;
    Elf64_Ehdr *elf;
    Elf64_Phdr *phead;

    if (!buf) return NULL;

    elf = (Elf64_Ehdr *)buf;
    phead  = (Elf64_Phdr *)&buf[elf->e_phoff];

    for (i = 0; i < elf->e_phnum; i++) {
        if (phead[i].p_type == PT_INTERP) {
            char * exld = (char *)&buf[phead[i].p_offset];
            if (g_debug) printf("%s:%d buf ld.so: %s\n", __FUNCTION__, __LINE__, exld);
            ldso = strdup(exld);
            break;
        }
    }

    return ldso;
}

static char *
getLoaderFile(const char *exe) {
    int fd;
    struct stat sbuf;
    unsigned char *buf = NULL;
    char *ldso = NULL;

    if (!exe) return NULL;

    if ((fd = open(exe, O_RDONLY)) == -1) {
        perror("getLoaderFile:open");
        return NULL;
    }

    if (fstat(fd, &sbuf) == -1) {
        perror("getLoaderFile:fstat");
        close(fd);
        return NULL;
    }

    buf = mmap(NULL, ROUND_UP(sbuf.st_size, sysconf(_SC_PAGESIZE)),
               PROT_READ, MAP_PRIVATE, fd, (off_t)NULL);
    if (buf == MAP_FAILED) {
        perror("getLoaderFile:mmap");
        close(fd);
        return NULL;
    }

    close(fd);

    ldso = get_loader(buf);

    munmap(buf, sbuf.st_size);
    return ldso;
}

#if 0
// modify the loader string in the .interp section of appview
static int
setLoaderFile(const char *exe)
{
    int fd;
    struct stat sbuf;
    unsigned char *buf;

    if (!exe) return -1;

    if ((fd = open(exe, O_RDONLY)) == -1) {
        perror("setLoaderFile:open");
        return -1;
    }

    if (fstat(fd, &sbuf) == -1) {
        perror("setLoaderFile:fstat");
        close(fd);
        return -1;
    }

    buf = mmap(NULL, ROUND_UP(sbuf.st_size, sysconf(_SC_PAGESIZE)),
               PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, (off_t)NULL);
    if (buf == MAP_FAILED) {
        perror("setLoaderFile:mmap");
        close(fd);
        return -1;
    }

    if (set_loader(buf) == -1) {
        if (close(fd) == -1) {
            munmap(buf, sbuf.st_size);
            return -1;
        }

        if ((fd = open(exe, O_RDWR)) == -1) {
            perror("setLoaderFile:open");
            munmap(buf, sbuf.st_size);
            return -1;
        }

        int rc = write(fd, buf, sbuf.st_size);
        if (rc < sbuf.st_size) {
            perror("setLoaderFile:write");
        }
    } else {
        fprintf(stderr, "WARNING: can't locate or set the loader string in %s\n", exe);
    }

    close(fd);
    munmap(buf, sbuf.st_size);
    return 0;
}
#endif

patch_status_t
patchLoader(unsigned char *appview, uid_t nsUid, gid_t nsGid)
{
    patch_status_t patch_res = PATCH_NO_OP;
    char *ldso_exe = NULL;
    char *ldso_appview = NULL;

    ldso_exe = getLoaderFile(EXE_TEST_FILE);
    if (ldso_exe && strstr(ldso_exe, LIBMUSL) != NULL) {

        // Avoid creating ld-musl-x86_64.so.1 -> /lib/ld-musl-x86_64.so.1
        if ((ldso_appview = get_loader(appview)) == NULL) {
            patch_res = PATCH_FAILED;
            goto out;
        }
        if (strstr(ldso_appview, "musl")) {
            goto out;
        }

        if (!set_loader(appview)) {
            patch_res = PATCH_SUCCESS;
        } else {
            patch_res = PATCH_FAILED;
        }
    }

out:
    if (ldso_appview) free(ldso_appview);
    if (ldso_exe) free(ldso_exe);
    return patch_res;
}

// modify NEEDED entries in libappview.so to avoid dependencies
static int
setLibraryFile(const char *libpath)
{
    int i, fd, found, name;
    struct stat sbuf;
    char *buf;
    Elf64_Ehdr *elf;
    Elf64_Shdr *sections;
    Elf64_Dyn *dyn;
    const char *section_strtab = NULL;
    const char *strtab = NULL;
    const char *sec_name = NULL;

    if (libpath == NULL)
        return -1;

    if ((fd = open(libpath, O_RDONLY)) == -1) {
        perror("setLibrary:open");
        return -1;
    }

    if (fstat(fd, &sbuf) == -1) {
        perror("setLibrary:fstat");
        close(fd);
        return -1;
    }

    buf = mmap(NULL, ROUND_UP(sbuf.st_size, sysconf(_SC_PAGESIZE)),
               PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, (off_t)NULL);
    if (buf == MAP_FAILED) {
        perror("setLibrary:mmap");
        close(fd);
        return -1;
    }

    // get the elf header, section table and string table
    elf = (Elf64_Ehdr *)buf;

    if (elf->e_ident[EI_MAG0] != ELFMAG0
        || elf->e_ident[EI_MAG1] != ELFMAG1
        || elf->e_ident[EI_MAG2] != ELFMAG2
        || elf->e_ident[EI_MAG3] != ELFMAG3
        || elf->e_ident[EI_VERSION] != EV_CURRENT) {
        fprintf(stderr, "ERROR:%s: is not valid ELF file", libpath);
        close(fd);
        munmap(buf, sbuf.st_size);
        return -1;
    }

    sections = (Elf64_Shdr *)((char *)buf + elf->e_shoff);
    section_strtab = (char *)buf + sections[elf->e_shstrndx].sh_offset;
    found = name = 0;

    // locate the .dynstr section
    for (i = 0; i < elf->e_shnum; i++) {
        sec_name = section_strtab + sections[i].sh_name;
        if (sections[i].sh_type == SHT_STRTAB && strcmp(sec_name, ".dynstr") == 0) {
            strtab = (const char *)(buf + sections[i].sh_offset);
        }
    }

    if (strtab == NULL) {
        fprintf(stderr, "ERROR:%s: did not locate the .dynstr from %s", __FUNCTION__, libpath);
        close(fd);
        munmap(buf, sbuf.st_size);
        return -1;
    }

    // locate the .dynamic section
    for (i = 0; i < elf->e_shnum; i++) {
        if (sections[i].sh_type == SHT_DYNAMIC) {
            for (dyn = (Elf64_Dyn *)((char *)buf + sections[i].sh_offset); dyn != NULL && dyn->d_tag != DT_NULL; dyn++) {
                if (dyn->d_tag == DT_NEEDED) {
                    char *depstr = (char *)(strtab + dyn->d_un.d_val);
                    if (depstr) {
                        if (strstr(depstr, "ld-linux")) {
                            char newdep[PATH_MAX];
                            size_t newdep_len;
                            snprintf(newdep, sizeof(newdep), MUSL_LD_NAME);
                            newdep_len = strlen(newdep);
                            if (strlen(depstr) >= newdep_len) {
                                strncpy(depstr, newdep, newdep_len + 1);
                                found = 1;
                                break;
                            }
                        } else if (strstr(depstr, "ld-musl")) {
                            // already patched
                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (found == 1) break;
    }

    if (found) {
        if (close(fd) == -1) {
            munmap(buf, sbuf.st_size);
            return -1;
        }

        if ((fd = open(libpath, O_RDWR)) == -1) {
            perror("setLibrary:open write");
            munmap(buf, sbuf.st_size);
            return -1;
        }

        int rc = write(fd, buf, sbuf.st_size);
        if (rc < sbuf.st_size) {
            perror("setLibrary:write");
        }
    }

    close(fd);
    munmap(buf, sbuf.st_size);
    return (found - 1);
}

patch_status_t
patchLibrary(const char *so_path, bool force)
{
    patch_status_t patch_res = PATCH_NO_OP;
      
    if (force || isMusl()) {
        if (!setLibraryFile(so_path)) {
            patch_res = PATCH_SUCCESS;
        } else {
            patch_res = PATCH_FAILED;
        }
    }

    return patch_res;
}

bool
isMusl(void)
{
    bool res = FALSE;
    char *ldso_exe = NULL;
      
    ldso_exe = getLoaderFile(EXE_TEST_FILE);
    if (ldso_exe && (strstr(ldso_exe, LIBMUSL) != NULL)) {
        res = TRUE;
    }

    if (ldso_exe) free(ldso_exe);
    return res;
}
