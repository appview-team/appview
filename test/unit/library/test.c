#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "appviewstdlib.h"
#include "dbg.h"
#include "test.h"

/*
* Following part of code is required to use memory sanitizers during unit tests
* To correct instrument code we redirect allocations from our internal
* library to allocator from standard library.
* The instrumentation is enabled by default on x86_64.
*
* make FSAN=true libtest
*
* Allows to instrumentation on aarch64.
* 
* See details in:
* https://github.com/google/sanitizers/wiki/AddressSanitizerIncompatiblity
*
* The memory leak instrumentation is done by "-fsanitize=address"
*/

/*
* In GCC sanitize address is defined when -fsanitize=address is used
* In Clang the code below is recommended way to check this feature
*/
#if defined(__has_feature)
# if __has_feature(address_sanitizer)
#  define __SANITIZE_ADDRESS__ 1
# endif
#endif

#ifdef __SANITIZE_ADDRESS__
#include <stdlib.h>
void * __real_appviewlibc_malloc(size_t);
void * __wrap_appviewlibc_malloc(size_t size)
{
    return malloc(size);
}

void __real_appviewlibc_free(void *);
void __wrap_appviewlibc_free(void * ptr)
{
    return free(ptr);
}

void * __real_appviewlibc_calloc(size_t, size_t);
void * __wrap_appviewlibc_calloc(size_t nelem, size_t size)
{
    return calloc(nelem, size);
}

void * __real_appviewlibc_realloc(void *, size_t);
void * __wrap_appviewlibc_realloc(void * ptr, size_t size)
{
    return realloc(ptr, size);
}
#endif


int
groupSetup(void** state)
{
    dbgInit();
    return 0;
}

int
groupTeardown(void** state)
{
    // as a policy, we're saying all tests that call groupSetup and
    // groupTeardown should be aware of things that would cause dbg
    // failures, and cleanup after themselves (call dbgInit()) before
    // execution is complete.
    unsigned long long failures = dbgCountAllLines();
    if (failures) {
        dbgDumpAll(stdout);
    }
    dbgDestroy();
    return failures;
}

void
dbgHasNoUnexpectedFailures(void** state)
{
    unsigned long long failures = dbgCountAllLines();
    if (failures) {
        dbgDumpAll(stdout);
    }
    assert_false(failures);
}

void
dbgDumpAllToBuffer(char* buf, int size)
{
    FILE* f = appview_fmemopen(buf, size, "a+");
    assert_non_null(f);
    dbgDumpAll(f);
    if (appview_ftell(f) >= size) {
        fail_msg("size of %d was inadequate for dbgDumpAllToBuffer, "
                 "%ld was needed", size, appview_ftell(f));
    }
    if (appview_fclose(f)) fail_msg("Couldn't close fmemopen'd file");
}

int
writeFile(const char* path, const char* text)
{
    FILE* f = appview_fopen(path, "w");
    if (!f)
        fail_msg("Couldn't open file");

    if (!appview_fwrite(text, appview_strlen(text), 1, f))
        fail_msg("Couldn't write file");

    if (appview_fclose(f))
        fail_msg("Couldn't close file");

    return 0;
}

int
deleteFile(const char* path)
{
    return appview_unlink(path);
}

long
fileEndPosition(const char* path)
{
    FILE* f;
    if ((f = appview_fopen(path, "r"))) {
        appview_fseek(f, 0, SEEK_END);
        long pos = appview_ftell(f);
        appview_fclose(f);
        return pos;
    }
    return -1;
}
