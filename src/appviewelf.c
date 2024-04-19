#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "dbg.h"
#include "appviewstdlib.h"
#include "os.h"
#include "fn.h"
#include "appviewelf.h"
#include "utils.h"

/*******************************************************************************
 * Consider updating src/loader/loaderutils.c if you make changes to this file *
 *******************************************************************************/

void
freeElf(char *buf, size_t len)
{
    if (!buf) return;

    if (appview_munmap(buf, len) == -1) {
        appviewLogError("freeElf: munmap failed");
    }
}

static void
setTextSizeAndLenFromElf(elf_buf_t *ebuf)
{
    int i;
    Elf64_Ehdr *ehdr;
    Elf64_Shdr *sections;
    const char *sec_name;
    const char *section_strtab = NULL;
    ehdr = (Elf64_Ehdr *)ebuf->buf;
    sections = (Elf64_Shdr *)((char *)ebuf->buf + ehdr->e_shoff);
    section_strtab = (char *)ebuf->buf + sections[ehdr->e_shstrndx].sh_offset;

    for (i = 0; i < ehdr->e_shnum; i++) {
        sec_name = section_strtab + sections[i].sh_name;

        if (!appview_strcmp(sec_name, ".text")) {
            ebuf->text_addr = (unsigned char *)sections[i].sh_addr;
            ebuf->text_len = sections[i].sh_size;
            appviewLog(CFG_LOG_DEBUG, "%s:%d %s addr %p - %p\n", __FUNCTION__, __LINE__,
                        sec_name, ebuf->text_addr, ebuf->text_addr + ebuf->text_len);
        }
    }
}

static bool
app_type(char *buf, const uint32_t sh_type, const char *sh_name)
{
    int i = 0;
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)buf;
    Elf64_Shdr *sections;
    const char *section_strtab = NULL;
    const char *sec_name = NULL;

    sections = (Elf64_Shdr *)(buf + ehdr->e_shoff);
    section_strtab = buf + sections[ehdr->e_shstrndx].sh_offset;

    for (i = 0; i < ehdr->e_shnum; i++) {
        sec_name = section_strtab + sections[i].sh_name;
        //printf("section %s type = %d \n", sec_name, sections[i].sh_type);
        if (sections[i].sh_type == sh_type && appview_strcmp(sec_name, sh_name) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

Elf64_Shdr*
getElfSection(char *buf, const char *sh_name)
{
    int i = 0;
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)buf;
    Elf64_Shdr *sections;
    const char *section_strtab = NULL;
    const char *sec_name = NULL;

    sections = (Elf64_Shdr *)(buf + ehdr->e_shoff);
    section_strtab = buf + sections[ehdr->e_shstrndx].sh_offset;

    for (i = 0; i < ehdr->e_shnum; i++) {
        sec_name = section_strtab + sections[i].sh_name;
        if (appview_strcmp(sec_name, sh_name) == 0) {
            return &sections[i];
        }
    }
    return NULL;
}

elf_buf_t *
getElf(char *path)
{
    int fd = -1;
    elf_buf_t *ebuf = NULL;
    Elf64_Ehdr *elf;
    struct stat sbuf;
    int get_elf_successful = FALSE;


    if ((ebuf = appview_calloc(1, sizeof(elf_buf_t))) == NULL) {
        appviewLogError("getElf: memory alloc failed");
        goto out;
    }

    if (((fd = appview_open(path, O_RDONLY)) == -1) && ((fd = osFindFd(appview_getpid(), path)) == -1)) {
        appviewLogError("getElf: open failed");
        goto out;
    }

    if (appview_fstat(fd, &sbuf) == -1) {
        appviewLogError("fd:%d getElf: fstat failed", fd);
        goto out;
    }

    char * mmap_rv = appview_mmap(NULL, ROUND_UP(sbuf.st_size, appview_sysconf(_SC_PAGESIZE)),
                          PROT_READ, MAP_PRIVATE, fd, (off_t)NULL);
    if (mmap_rv == MAP_FAILED) {
        appviewLogError("fd:%d getElf: appview_mmap failed", fd);
        goto out;
    }

    ebuf->cmd = path;
    ebuf->buf = mmap_rv;
    ebuf->len = sbuf.st_size;

    elf = (Elf64_Ehdr *)ebuf->buf;
    if((elf->e_ident[EI_MAG0] != 0x7f) ||
       appview_strncmp((char *)&elf->e_ident[EI_MAG1], "ELF", 3) ||
       (elf->e_ident[EI_CLASS] != ELFCLASS64) ||
       (elf->e_ident[EI_DATA] != ELFDATA2LSB)) {
        appviewLogError("fd:%d %s:%d ERROR: %s is not a viable ELF file\n",
                    fd, __FUNCTION__, __LINE__, path);
        goto out;
    }

    if ((elf->e_type != ET_EXEC) && (elf->e_type != ET_DYN)) {
        appviewLogError("fd:%d %s:%d %s with type %d is not an executable\n",
                    fd, __FUNCTION__, __LINE__, path, elf->e_type);
        goto out;
    }

    setTextSizeAndLenFromElf(ebuf);

    get_elf_successful = TRUE;

out:
    if (fd != -1) appview_close(fd);
    if (!get_elf_successful && ebuf) {
        freeElf(ebuf->buf, ebuf->len);
        appview_free(ebuf);
        ebuf = NULL;
    }
    return ebuf;
}

/*
 * Find the GOT ptr as defined in RELA/DYNSYM (.rela.dyn) for symbol & hook it.
 * .rela.dyn section:
 * The address of relocation entries associated solely with the PLT.
 * The relocation table's entries have a one-to-one correspondence with the PLT.
 */
int
doGotcha(struct link_map *lm, got_list_t *hook, Elf64_Rela *rel, Elf64_Sym *sym, char *str, int rsz, bool attach)
{
    int i, match = -1;
    uint64_t prev;

    for (i = 0; i < rsz / sizeof(Elf64_Rela); i++) {
        /*
         * Index into the dynamic symbol table (not the 'symbol' table)
         * with the index from the current relocation entry and get the
         * sym tab offset (st_name). The index is calculated with the
         * ELF64_R_SYM macro, which shifts the elf value >> 32. The dyn sym
         * tab offset is added to the start of the string table (str) to get
         * the string name. Compare the str entry with a symbol in the list.
         *
         * Note, it would be nice to check the array bounds before indexing the
         * sym[] table. However, DT_SYMENT gives the byte size of a single entry.
         * According to the ELF spec there is no size/number of entries for the
         * symbol table at the program header table level. This is not needed at
         * runtime as the symbol lookup always go through the hash table; ELF64_R_SYM.
         *
         * Locating and dereferencing the GOT can be confusing, for reference:
         * What symbol is defined in this GOT entry
         * .rel.plt -> r.info -> .dynsym -> st_name -> .dynstr -> read\0
         *
         * If this is a symbol we want to interpose, then:
         * .rel.plt -> r.offset + load address -> GOT entry for read
         */
        if (!appview_strcmp(sym[ELF64_R_SYM(rel[i].r_info)].st_name + str, hook->symbol)) {
            uint64_t *gaddr = (uint64_t *)(rel[i].r_offset + lm->l_addr);
            int page_size = appview_getpagesize();
            size_t saddr = ROUND_DOWN((size_t)gaddr, page_size);
            int prot = osGetPageProt((uint64_t)gaddr);

            if (prot != -1) {
                if ((prot & PROT_WRITE) == 0) {
                    // allow for write permission it write permission are not set
                    if (osMemPermAllow((void *)saddr, 16, prot, PROT_WRITE) == FALSE) {
                        appviewLog(CFG_LOG_DEBUG, "doGotcha: osMemPermAllow add write protection flag failed");
                        return -1;
                    }
                }
            } else {
                /*
                 * We don't have a valid protection setting for the GOT page.
                 * It's "almost assuredly" safe to set perms to RD | WR as this is
                 * a GOT page; we know what settings are expected. However, it
                 * may be safest to just bail out at this point.
                 */
                return -1;
            }

            /*
             * The offset from the matching relocation entry defines the GOT
             * entry associated with 'symbol'. ELF docs describe that this
             * is an offset and not a virtual address. Take the load address
             * of the shared module as defined in the link map's l_addr + offset.
             * as in: rel[i].r_offset + lm->l_addr
             */
            prev = *gaddr;
            if (attach == TRUE) {
                // been here before, don't update the GOT entry
                if ((void *)*gaddr == hook->func) return -1;
                *gaddr = (uint64_t)hook->func;
            } else {
                // handle a detach operation
                *gaddr = *(uint64_t *)hook->gfn;
            }

            appviewLog(CFG_LOG_DEBUG, "%s:%d sym=%s offset 0x%lx GOT entry %p saddr 0x%lx, prev=0x%lx, curr=%p",
                        __FUNCTION__, __LINE__, hook->symbol, rel[i].r_offset, gaddr, saddr, prev, hook->func);

            if ((prot & PROT_WRITE) == 0) {
                // if we didn't mod above leave prot settings as is
                if (osMemPermRestore((void *)saddr, 16, prot) == FALSE) {
                    appviewLog(CFG_LOG_DEBUG, "doGotcha: osMemPermRestore remove write memory protection flags failed");
                    return -1;
                }
            }
            match = 0;
            break;
        }
    }

    return match;
}

// Locate the needed elf entries from a given link map.
int
getElfEntries(struct link_map *lm, Elf64_Rela **rel, Elf64_Sym **sym, char **str, int *rsz)
{
    Elf64_Dyn *dyn = NULL;
    char *got = NULL; // TODO; got is not needed, debug, remove
    bool jmprel = FALSE, pltrelsz = FALSE;

    appviewLog(CFG_LOG_DEBUG, "%s:%d name: %s", __FUNCTION__, __LINE__, lm->l_name);
    for (dyn = lm->l_ld; dyn->d_tag != DT_NULL; dyn++) {
        if (dyn->d_tag == DT_SYMTAB) {
            // Note: using osGetPageProt() to determine if the addr is present in the
            // process address space. We don't need the prot value.
            if (osGetPageProt((uint64_t)dyn->d_un.d_ptr) != -1) {
                *sym = (Elf64_Sym *)((char *)(dyn->d_un.d_ptr));
            } else {
                *sym = (Elf64_Sym *)((char *)(dyn->d_un.d_ptr + lm->l_addr));
            }
        } else if (dyn->d_tag == DT_STRTAB) {
            if (osGetPageProt((uint64_t)dyn->d_un.d_ptr) != -1) {
                *str = (char *)(dyn->d_un.d_ptr);
            } else {
                *str = (char *)(dyn->d_un.d_ptr + lm->l_addr);
            }
        } else if (dyn->d_tag == DT_JMPREL) {
            if (osGetPageProt((uint64_t)dyn->d_un.d_ptr) != -1) {
                *rel = (Elf64_Rela *)((char *)(dyn->d_un.d_ptr));
            } else {
                *rel = (Elf64_Rela *)((char *)(dyn->d_un.d_ptr + lm->l_addr));
            }
            jmprel = TRUE;
        } else if (dyn->d_tag == DT_PLTRELSZ) {
            *rsz = dyn->d_un.d_val;
            pltrelsz = TRUE;
        } else if (dyn->d_tag == DT_PLTGOT) {
            if (osGetPageProt((uint64_t)dyn->d_un.d_ptr) != -1) {
                got = (char *)(dyn->d_un.d_ptr);
            } else {
                got = (char *)(dyn->d_un.d_ptr + lm->l_addr);
            }
        } else if ((dyn->d_tag == DT_RELA) && (jmprel == FALSE)) {
            *rel = (Elf64_Rela *)((char *)(dyn->d_un.d_ptr + lm->l_addr));
        } else if ((dyn->d_tag == DT_RELASZ) && (pltrelsz == FALSE)) {
            *rsz = dyn->d_un.d_val;
        }
    }

    appviewLog(CFG_LOG_DEBUG, "%s:%d name: %s dyn %p sym %p rel %p str %p rsz %d got %p laddr 0x%lx\n",
                __FUNCTION__, __LINE__, lm->l_name, dyn, *sym, *rel, *str, *rsz, got, lm->l_addr);

    if (*sym == NULL || *rel == NULL || (*rsz < sizeof(Elf64_Rela))) {
        return -1;
    }

    return 0;
}

void *
getDynSymbol(const char *buf, char *sname)
{
    int i, nsyms = 0;
    Elf64_Addr symaddr = 0;
    Elf64_Ehdr *ehdr;
    Elf64_Shdr *sections;
    Elf64_Sym *symtab = NULL;
    const char *section_strtab = NULL;
    const char *strtab = NULL;
    const char *sec_name = NULL;

    if (!buf || !sname) return NULL;

    ehdr = (Elf64_Ehdr *)buf;
    sections = (Elf64_Shdr *)((char *)buf + ehdr->e_shoff);
    section_strtab = (char *)buf + sections[ehdr->e_shstrndx].sh_offset;

    for (i = 0; i < ehdr->e_shnum; i++) {
        sec_name = section_strtab + sections[i].sh_name;

        if (sections[i].sh_type == SHT_DYNSYM) {
            symtab = (Elf64_Sym *)((char *)buf + sections[i].sh_offset);
            nsyms = sections[i].sh_size / sections[i].sh_entsize;
        } else if (sections[i].sh_type == SHT_STRTAB && appview_strcmp(sec_name, ".dynstr") == 0) {
            strtab = (const char *)(buf + sections[i].sh_offset);
        }

        if ((strtab != NULL) && (symtab != NULL)) break;
        appviewLogDebug("section %s type = %d flags = 0x%lx addr = 0x%lx-0x%lx, size = 0x%lx off = 0x%lx\n",
                      sec_name,
                      sections[i].sh_type,
                      sections[i].sh_flags,
                      sections[i].sh_addr,
                      sections[i].sh_addr + sections[i].sh_size,
                      sections[i].sh_size,
                      sections[i].sh_offset);
    }

    for (i=0; i < nsyms; i++) {
        if (appview_strcmp(sname, strtab + symtab[i].st_name) == 0) {
            symaddr = symtab[i].st_value;
            sysprint("symbol found %s = 0x%08lx\n", strtab + symtab[i].st_name, symtab[i].st_value);
            break;
        }
    }

    return (void *)symaddr;
}

void *
getSymbol(const char *buf, char *sname)
{
    int i, nsyms = 0;
    Elf64_Addr symaddr = 0;
    Elf64_Ehdr *ehdr;
    Elf64_Shdr *sections;
    Elf64_Sym *symtab = NULL;
    const char *section_strtab = NULL;
    const char *strtab = NULL;
    const char *sec_name = NULL;

    if (!buf || !sname) return NULL;

    ehdr = (Elf64_Ehdr *)buf;
    sections = (Elf64_Shdr *)((char *)buf + ehdr->e_shoff);
    section_strtab = (char *)buf + sections[ehdr->e_shstrndx].sh_offset;

    for (i = 0; i < ehdr->e_shnum; i++) {
        sec_name = section_strtab + sections[i].sh_name;

        if (sections[i].sh_type == SHT_SYMTAB) {
            symtab = (Elf64_Sym *)((char *)buf + sections[i].sh_offset);
            nsyms = sections[i].sh_size / sections[i].sh_entsize;
        } else if (sections[i].sh_type == SHT_STRTAB && appview_strcmp(sec_name, ".strtab") == 0) {
            strtab = (const char *)(buf + sections[i].sh_offset);
        }

        if ((strtab != NULL) && (symtab != NULL)) break;
        /*printf("section %s type = %d flags = 0x%lx addr = 0x%lx-0x%lx, size = 0x%lx off = 0x%lx\n",
               sec_name,
               sections[i].sh_type,
               sections[i].sh_flags,
               sections[i].sh_addr,
               sections[i].sh_addr + sections[i].sh_size,
               sections[i].sh_size,
               sections[i].sh_offset);*/
    }

    for (i=0; i < nsyms; i++) {
        if (appview_strcmp(sname, strtab + symtab[i].st_name) == 0) {
            symaddr = symtab[i].st_value;
            appviewLog(CFG_LOG_TRACE, "symbol found %s = 0x%08lx\n", strtab + symtab[i].st_name, symtab[i].st_value);
            break;
        }
    }

    return (void *)symaddr;
}

bool
is_static(char *buf)
{
    int i;
    Elf64_Ehdr *elf = (Elf64_Ehdr *)buf;
    Elf64_Phdr *phead = (Elf64_Phdr *)&buf[elf->e_phoff];

    for (i = 0; i < elf->e_phnum; i++) {
        if ((phead[i].p_type == PT_DYNAMIC) || (phead[i].p_type == PT_INTERP)) {
            return FALSE;
        }
    }

    return TRUE;
}

bool
is_go(char *buf)
{
    if (buf && (app_type(buf, SHT_PROGBITS, ".gosymtab") ||
                app_type(buf, SHT_PROGBITS, ".gopclntab") ||
                app_type(buf, SHT_NOTE, ".note.go.buildid"))) {
        return TRUE;
    }

    return FALSE;
}

bool
is_musl(char *buf)
{
    int i;
    char *ldso = NULL;
    Elf64_Ehdr *elf = (Elf64_Ehdr *)buf;
    Elf64_Phdr *phead = (Elf64_Phdr *)&buf[elf->e_phoff];

    for (i = 0; i < elf->e_phnum; i++) {
        if ((phead[i].p_type == PT_INTERP)) {
            char *exld = (char *)&buf[phead[i].p_offset];

            ldso = appview_strdup(exld);
            if (ldso) {
                if (appview_strstr(ldso, "musl") != NULL) {
                    appview_free(ldso);
                    return TRUE;
                }
                appview_free(ldso);
            } else {
                DBG(NULL); // not expected
            }
            break;
        }
    }

    return FALSE;
}

