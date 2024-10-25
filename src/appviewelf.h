#ifndef __APPVIEWELF_H__
#define __APPVIEWELF_H__

#ifdef __linux__

#include <elf.h>
#include <link.h>

typedef struct {
    const char *symbol;
    void *func;
    void *gfn;
} got_list_t;

typedef struct {
    char *cmd;
    char *buf;
    int len;
    unsigned char *text_addr;
    uint64_t       text_len;
} elf_buf_t;

void freeElf(char *, size_t);
elf_buf_t * getElf(char *);
int doGotcha(struct link_map *, got_list_t *, Elf64_Sym *, char *,
             Elf64_Rela *, size_t, Elf64_Rela *, size_t, bool);
int getElfEntries(struct link_map *, Elf64_Sym **, char **,
                  Elf64_Rela **, size_t *, Elf64_Rela **, size_t *);
Elf64_Shdr* getElfSection(char *, const char *);
void * getSymbol(const char *, char *);
void * getDynSymbol(const char *, char *);
bool is_static(char *);
bool is_go(char *);
bool is_musl(char *);

#endif // __linux__

#endif // __APPVIEWELF_H__
