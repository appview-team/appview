#define _GNU_SOURCE
#define _XOPEN_SOURCE 500

#include <ftw.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "setup.h"
#include "test.h"

/*
 * Define the extern offset for integration test compilation 
 * See details in libdir.c
 */
unsigned char _binary_libappview_so_start;
unsigned char _binary_libappview_so_end;

static int
create_file(const char *file_path, const char* file_contents) {
    FILE *f;

    f = fopen(file_path, "w");
    if (!f) {
        return -1;
    }

    for (int i = 0; i < strlen(file_contents); i++) {
        if (putc(file_contents[i], f) == EOF) {
            fclose(f);
            return -1;
        }
    }
    fclose(f);

    return 0;
}

/*
 * Assertions:
 * Function does not return an error
 * Function returns 0 (no changes made)
 * File size does not change
 */
static void
removeAppViewCfgFile0Changes(void **state) {
    int res;
    struct stat st;
    int orig_file_size;
    char *file_path = "/tmp/example_cfg_file";
    const char *file_contents = "Contents of line 1\nContents of line 2\nContents of line 3\nContents of line 4";
    
    if (create_file(file_path, file_contents)) {
        fail_msg("Couldn't create file %s", file_path);
    }

    stat(file_path, &st);
    orig_file_size = st.st_size;

    res = removeAppViewCfgFile(file_path);
    assert_int_equal(res, 0);

    stat(file_path, &st);
    assert_int_equal(orig_file_size, st.st_size);

    if (remove(file_path)) {
        fail_msg("Couldn't remove file %s", file_path);
    }
}

/*
 * Assertions:
 * Function does not return an error
 * Function returns 1 (1 change made)
 * File size decreases by correct amount
 * "/libappview.so" cannot be found in the file
 */
static void
removeAppViewCfgFile1Change(void **state) {
    int res;
    struct stat st;
    int orig_file_size;
    char *file_path = "/tmp/example_cfg_file";
    const char *file_contents = "LD_PRELOAD=/usr/lib/appview/libappview.so\nContents of line 2\nContents of line 3\nContents of line 4";

    if (create_file(file_path, file_contents)) {
        fail_msg("Couldn't create file %s", file_path);
    }

    stat(file_path, &st);
    orig_file_size = st.st_size;

    res = removeAppViewCfgFile(file_path);
    assert_int_equal(res, 1);

    // Check for presence of "/libappview.so"
    res = isCfgFileConfigured(file_path);
    assert_int_equal(res, 0);

    stat(file_path, &st);
    assert_int_equal(orig_file_size - 42, st.st_size);

    if (remove(file_path)) {
        fail_msg("Couldn't remove file %s", file_path);
    }
}

/*
 * Assertions:
 * Function does not return an error
 * Function returns 2 (2 changes made)
 * File size decreases by correct amount
 * "/libappview.so" cannot be found in the file
 */
static void
removeAppViewCfgFile2Changes(void **state) {
    int res;
    struct stat st;
    int orig_file_size;
    char *file_path = "/tmp/example_cfg_file";
    const char *file_contents = "LD_PRELOAD=/usr/lib/appview/libappview.so\nContents of line 2\n\nContents of line 3\nLD_PRELOAD=/usr/lib/appview/libappview.so\n";

    if (create_file(file_path, file_contents)) {
        fail_msg("Couldn't create file %s", file_path);
    }

    stat(file_path, &st);
    orig_file_size = st.st_size;

    res = removeAppViewCfgFile(file_path);
    assert_int_equal(res, 2);

    // Check for presence of "/libappview.so"
    res = isCfgFileConfigured(file_path);
    assert_int_equal(res, 0);

    stat(file_path, &st);
    assert_int_equal(orig_file_size - 84, st.st_size);

    if (remove(file_path)) {
        fail_msg("Couldn't remove file %s", file_path);
    }
}

int
main(int argc, char* argv[]) {
    printf("running %s\n", argv[0]);

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(removeAppViewCfgFile0Changes),
        cmocka_unit_test(removeAppViewCfgFile1Change),
        cmocka_unit_test(removeAppViewCfgFile2Changes),
    };
    return cmocka_run_group_tests(tests, groupSetup, groupTeardown);
}
