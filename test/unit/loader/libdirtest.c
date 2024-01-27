#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libdir.h"
#include "libver.h"
#include "test.h"
#include "appviewtypes.h"

#define TEST_BASE_DIR "/tmp/appview-test/"
#define TEST_INSTALL_BASE "/tmp/appview-test/install"
#define TEST_INSTALL_BASE_NOT_REACHABLE "/root/base/appview"
#define TEST_TMP_BASE "/tmp/appview-test/tmp"
#define TEST_TMP_BASE_NOT_REACHALBE "/root/tmp/appview"

static int
rm_callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    return (remove(fpath) < 0) ? -1 : 0;
}

static int
rm_recursive(char *path) {
    return nftw(path, rm_callback, 64, FTW_DEPTH | FTW_PHYS);
}

static int
teardownlibdirTest(void **state) {
     rm_recursive(TEST_BASE_DIR);
     return 0;
}

/*
 * Define the extern offset for integration test compilation 
 * See details in libdir.c
 */
unsigned char _binary_libappview_so_start;
unsigned char _binary_libappview_so_end;

static void
CreateDirIfMissingWrongPathNull(void **state) {
    mkdir_status_t res = libdirCreateDirIfMissing(NULL, 0777, geteuid(), getegid());
    assert_int_equal(res, MKDIR_STATUS_ERR_NOT_ABS_DIR);
}

static void
CreateDirIfMissingWrongPathCurrentDir(void **state) {
    mkdir_status_t res = libdirCreateDirIfMissing(".", 0777, geteuid(), getegid());
    assert_int_equal(res, MKDIR_STATUS_ERR_NOT_ABS_DIR);
}

static void
CreateDirIfMissingWrongPathFile(void **state) {
    int res;
    char buf[PATH_MAX] = {0};
    const char *fileName = "unitTestLibVerFile";

    int fd = open(fileName, O_RDWR|O_CREAT, 0777);
    assert_int_not_equal(fd, -1);
    assert_non_null(getcwd(buf, PATH_MAX));
    strcat(buf, "/");
    strcat(buf, fileName);
    mkdir_status_t status = libdirCreateDirIfMissing(buf, 0777, geteuid(), getegid());
    assert_int_equal(status, MKDIR_STATUS_ERR_NOT_ABS_DIR);
    res = unlink(fileName);
    assert_int_equal(res, 0);
}

static void
CreateDirIfMissingPermissionIssue(void **state) {
    mkdir_status_t res = libdirCreateDirIfMissing("/root/loremIpsumFile/", 0777, geteuid(), getegid());
    assert_int_equal(res, MKDIR_STATUS_ERR_OTHER);
}

static void
CreateDirIfMissingAlreadyExists(void **state) {
    int res;
    char buf[PATH_MAX] = {0};
    const char *dirName = "unitTestLibVerDir";

    assert_non_null(getcwd(buf, PATH_MAX));
    strcat(buf, "/");
    strcat(buf, dirName);
    res = mkdir(buf, 0755);
    assert_int_equal(res, 0);
    mkdir_status_t status = libdirCreateDirIfMissing(dirName, 0777, geteuid(), getegid());
    assert_int_equal(status, MKDIR_STATUS_ERR_NOT_ABS_DIR);
    status = libdirCreateDirIfMissing(buf, 0777, geteuid(), getegid());
    assert_int_equal(status, MKDIR_STATUS_EXISTS);
    res = rm_recursive(buf);
    assert_int_equal(res, 0);
}

static void
CreateDirIfMissingAlreadyExistsPermIssue(void **state) {
    mkdir_status_t status = libdirCreateDirIfMissing("/root", 0777, geteuid(), getegid());
    assert_int_equal(status, MKDIR_STATUS_ERR_PERM_ISSUE);
}

static void
CreateDirIfMissingSuccessCreated(void **state) {
    int res;
    struct stat dirStat = {0};
    char buf[PATH_MAX] = {0};
    const char *dirName = "unitTestLibVerDir";

    assert_non_null(getcwd(buf, PATH_MAX));
    strcat(buf, "/");
    strcat(buf, dirName);
    res = stat(buf, &dirStat);
    assert_int_not_equal(res, 0);
    mkdir_status_t status = libdirCreateDirIfMissing(buf, 0777, geteuid(), getegid());
    assert_int_equal(status, MKDIR_STATUS_CREATED);
    res = stat(buf, &dirStat);
    assert_int_equal(res, 0);
    assert_int_equal(S_ISDIR(dirStat.st_mode), 1);
    assert_int_not_equal(dirStat.st_mode & S_IRUSR, 0);
    assert_int_not_equal(dirStat.st_mode & S_IWUSR, 0);
    assert_int_not_equal(dirStat.st_mode & S_IXUSR, 0);
    assert_int_not_equal(dirStat.st_mode & S_IRGRP, 0);
    assert_int_not_equal(dirStat.st_mode & S_IWGRP, 0);
    assert_int_not_equal(dirStat.st_mode & S_IXGRP, 0);
    assert_int_not_equal(dirStat.st_mode & S_IROTH, 0);
    assert_int_not_equal(dirStat.st_mode & S_IWOTH, 0);
    assert_int_not_equal(dirStat.st_mode & S_IXOTH, 0);
    res = rm_recursive(buf);
    assert_int_equal(res, 0);
     status = libdirCreateDirIfMissing(buf, 0755, geteuid(), getegid());
    assert_int_equal(status, MKDIR_STATUS_CREATED);
    res = stat(buf, &dirStat);
    assert_int_equal(res, 0);
    assert_int_equal(S_ISDIR(dirStat.st_mode), 1);
    assert_int_not_equal(dirStat.st_mode & S_IRUSR, 0);
    assert_int_not_equal(dirStat.st_mode & S_IWUSR, 0);
    assert_int_not_equal(dirStat.st_mode & S_IXUSR, 0);
    assert_int_not_equal(dirStat.st_mode & S_IRGRP, 0);
    assert_int_equal(dirStat.st_mode & S_IWGRP, 0);
    assert_int_not_equal(dirStat.st_mode & S_IXGRP, 0);
    assert_int_not_equal(dirStat.st_mode & S_IROTH, 0);
    assert_int_equal(dirStat.st_mode & S_IWOTH, 0);
    assert_int_not_equal(dirStat.st_mode & S_IXOTH, 0);
    res = rm_recursive(buf);
    assert_int_equal(res, 0);
}

static void
SetLibraryBaseInvalid(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE, "dev");
    int res = libdirSetLibraryBase("/does_not_exist");
    assert_int_equal(res, -1);
}

static void
SetLibrarySuccessDev(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE, "dev");
    // Create dummy file
    mkdir_status_t mkres = libdirCreateDirIfMissing("/tmp/appview-test/success/dev", 0777, geteuid(), getegid());
    assert_in_range(mkres, MKDIR_STATUS_CREATED, MKDIR_STATUS_EXISTS);
    FILE *fp = fopen("/tmp/appview-test/success/dev/libappview.so", "w");
    assert_non_null(fp);
    fclose(fp);
    int res = libdirSetLibraryBase("/tmp/appview-test/success");
    assert_int_equal(res, 0);
}

static void
SetLibraryBaseNull(void **state) {
    libdirInitTest(NULL, NULL, NULL);
    int res = libdirSetLibraryBase("");
    assert_int_equal(res, -1);
}

#if 0
static void
ExtractNewFileDev(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE, "dev");
    const char *normVer = libverNormalizedVersion("dev");
    char expected_location[PATH_MAX] = {0};
    struct stat dirStat = {0};

    // TEST_TMP_BASE will be used
    int res = libdirExtract(geteuid(), getegid());
    assert_int_equal(res, 0);
    snprintf(expected_location, PATH_MAX, "%s/%s/%s", TEST_TMP_BASE, normVer, "libappview.so");
    res = stat(expected_location, &dirStat);
    assert_int_equal(res, 0);
    memset(expected_location, 0, PATH_MAX);
}

static void
ExtractNewFileDevAlternative(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE_NOT_REACHALBE, "dev");
    const char *normVer = libverNormalizedVersion("dev");
    char expected_location[PATH_MAX] = {0};
    struct stat dirStat = {0};

    // Extract will fail because second path is not accessbile
    int res = libdirExtract(geteuid(), getegid());
    assert_int_not_equal(res, 0);
    snprintf(expected_location, PATH_MAX, "%s/%s/%s", TEST_TMP_BASE, normVer, "libappview.so");
    res = stat(expected_location, &dirStat);
    assert_int_not_equal(res, 0);
    memset(expected_location, 0, PATH_MAX);
}

static void
ExtractNewFileOfficial(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE, "v1.1.0");
    const char *normVer = libverNormalizedVersion("v1.1.0");
    char expected_location[PATH_MAX] = {0};
    struct stat dirStat = {0};

    // TEST_INSTALL_BASE will be used
    int res = libdirExtract(geteuid(), getegid());
    assert_int_equal(res, 0);
    snprintf(expected_location, PATH_MAX, "%s/%s/%s", TEST_INSTALL_BASE, normVer, "libappview.so");
    res = stat(expected_location, &dirStat);
    assert_int_equal(res, 0);
    remove(expected_location);
    memset(expected_location, 0, PATH_MAX);
}

static void
ExtractNewFileOfficialAlternative(void **state) {
    libdirInitTest(TEST_INSTALL_BASE_NOT_REACHABLE, TEST_TMP_BASE, "v1.1.0");
    const char *normVer = libverNormalizedVersion("v1.1.0");
    char expected_location[PATH_MAX] = {0};
    struct stat dirStat = {0};

    // TEST_TMP_BASE will be used
    int res = libdirExtract(geteuid(), getegid());
    assert_int_equal(res, 0);
    snprintf(expected_location, PATH_MAX, "%s/%s/%s", TEST_TMP_BASE, normVer, "libappview.so");
    res = stat(expected_location, &dirStat);
    assert_int_equal(res, 0);
    remove(expected_location);
    memset(expected_location, 0, PATH_MAX);
}

static void
ExtractFileExistsOfficial(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE, "v1.1.0");
    const char *normVer = libverNormalizedVersion("v1.1.0");
    char expected_location[PATH_MAX] = {0};
    struct stat firstStat = {0};
    struct stat secondStat = {0};

    int res = libdirExtract(geteuid(), getegid());
    assert_int_equal(res, 0);
    snprintf(expected_location, PATH_MAX, "%s/%s/%s", TEST_INSTALL_BASE, normVer, "libappview.so");
    res = stat(expected_location, &firstStat);
    assert_int_equal(res, 0);

    res = libdirExtract(geteuid(), getegid());
    assert_int_equal(res, 0);
    snprintf(expected_location, PATH_MAX, "%s/%s/%s", TEST_INSTALL_BASE, normVer, "libappview.so");
    res = stat(expected_location, &secondStat);
    assert_int_equal(res, 0);
    assert_int_equal(firstStat.st_ctim.tv_sec, secondStat.st_ctim.tv_sec);
    assert_int_equal(firstStat.st_ctim.tv_nsec, secondStat.st_ctim.tv_nsec);
}
#endif

static void
GetPathDev(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE, "dev");
    const char *normVer = libverNormalizedVersion("dev");
    char expected_location[PATH_MAX] = {0};
    // Create dummy directory
    snprintf(expected_location, PATH_MAX, "%s/%s/", TEST_TMP_BASE, normVer);
    mkdir_status_t mkres = libdirCreateDirIfMissing(expected_location, 0777, geteuid(), getegid());
    assert_in_range(mkres, MKDIR_STATUS_CREATED, MKDIR_STATUS_EXISTS);
    // Create dummy file
    memset(expected_location, 0, PATH_MAX);
    snprintf(expected_location, PATH_MAX, "%s/%s/%s", TEST_TMP_BASE, normVer, "libappview.so");
    FILE *fp = fopen(expected_location, "w");
    assert_non_null(fp);
    fclose(fp);

    const char *library_path = libdirGetPath(LIBRARY_FILE);
    int res = strcmp(expected_location, library_path);
    assert_int_equal(res, 0);
    remove(expected_location);
}

static void
GetPathOfficial(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE, "v1.1.0");
    const char *normVer = libverNormalizedVersion("v1.1.0");
    char expected_location[PATH_MAX] = {0};
    // Create dummy directory
    snprintf(expected_location, PATH_MAX, "%s/%s/", TEST_INSTALL_BASE, normVer);
    mkdir_status_t mkres = libdirCreateDirIfMissing(expected_location, 0777, geteuid(), getegid());
    assert_in_range(mkres, MKDIR_STATUS_CREATED, MKDIR_STATUS_EXISTS);
    // Create dummy file
    memset(expected_location, 0, PATH_MAX);
    snprintf(expected_location, PATH_MAX, "%s/%s/%s", TEST_INSTALL_BASE, normVer, "libappview.so");
    FILE *fp = fopen(expected_location, "w");
    assert_non_null(fp);
    fclose(fp);

    const char *library_path = libdirGetPath(LIBRARY_FILE);
    int res = strcmp(expected_location, library_path);
    assert_int_equal(res, 0);
    remove(expected_location);
}

static void
GetPathNoFile(void **state) {
    libdirInitTest(TEST_INSTALL_BASE, TEST_TMP_BASE, "v1.1.0");

    const char *library_path = libdirGetPath(LIBRARY_FILE);
    assert_int_equal(library_path, NULL);
}

int
main(int argc, char* argv[]) {
    printf("running %s\n", argv[0]);

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(CreateDirIfMissingWrongPathNull),
        cmocka_unit_test(CreateDirIfMissingWrongPathFile),
        cmocka_unit_test(CreateDirIfMissingWrongPathCurrentDir),
        cmocka_unit_test(CreateDirIfMissingPermissionIssue),
        cmocka_unit_test(CreateDirIfMissingAlreadyExists),
        cmocka_unit_test(CreateDirIfMissingAlreadyExistsPermIssue),
        cmocka_unit_test(CreateDirIfMissingSuccessCreated),
        cmocka_unit_test_teardown(SetLibraryBaseInvalid, teardownlibdirTest),
        cmocka_unit_test_teardown(SetLibraryBaseNull, teardownlibdirTest),
        cmocka_unit_test_teardown(SetLibrarySuccessDev, teardownlibdirTest),
    //    cmocka_unit_test_teardown(ExtractNewFileDev, teardownlibdirTest),
    //    cmocka_unit_test_teardown(ExtractNewFileDevAlternative, teardownlibdirTest),
    //    cmocka_unit_test_teardown(ExtractNewFileOfficial, teardownlibdirTest),
    //    cmocka_unit_test_teardown(ExtractNewFileOfficialAlternative, teardownlibdirTest),
    //    cmocka_unit_test_teardown(ExtractFileExistsOfficial, teardownlibdirTest),
        cmocka_unit_test_teardown(GetPathDev, teardownlibdirTest),  
    //    cmocka_unit_test_teardown(GetPathOfficial, teardownlibdirTest),
        cmocka_unit_test_teardown(GetPathNoFile, teardownlibdirTest),
    };
    return cmocka_run_group_tests(tests, groupSetup, groupTeardown);
}
