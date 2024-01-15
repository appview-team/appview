#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oci.h"
#include "cJSON.h"
#include "test.h"
#include "appviewstdlib.h"

static char dirPath[PATH_MAX];

static const char *testTypeJson[] = {
    "EmptyJson",
    "ProcessOnlyJson",
    "ProcessEnvPresentLDPreload",
    "ProcessEnvEmptyLDPreload",
    "MissingHooks",
    "IncompleteHooks",
    "CompleteHooks"
};

static int
testDirPath(char *path, const char *argv0) {
    char buf[PATH_MAX];
    if (argv0[0] == '/') {
        appview_strcpy(buf, argv0);
    } else {
        if (appview_getcwd(buf, PATH_MAX) == NULL) {
            appview_perror("getcwd error");
            return -1;
        }
        appview_strcat(buf, "/");
        appview_strcat(buf, argv0);
    }

    if (appview_realpath(buf, path) == NULL) {
        appview_perror("appview_realpath error");
        return -1;
    }

    /*
    * Retrieve the test directory path.
    * From:
    * /<dir>/appview/test/linux/cfgutilsrulestest
    * To:
    * /<dir>/appview/test/
    */
    for (int i= 0; i < 2; ++i) {
        path = appview_dirname(path);
        if (path == NULL) {
            appview_perror("appview_dirname error");
            return -1;
        }
    }
    return 0;
}

static bool
verifyModifiedCfg(int id, const char *cmpStr, const char *outPath) {
    int fdOut = appview_open(outPath, O_RDONLY);
    if (fdOut == -1) {
        assert_non_null(NULL);
        return FALSE;
    }

    struct stat stOut;
    if (appview_fstat(fdOut, &stOut) == -1) {
        assert_non_null(NULL);
        appview_close(fdOut);
        return FALSE;
    }

    void *fdOutMap = appview_mmap(NULL, stOut.st_size, PROT_READ, MAP_PRIVATE, fdOut, 0);
    if (fdOutMap == MAP_FAILED) {
        assert_non_null(NULL);
        appview_close(fdOut);
        return FALSE;
    }
    appview_close(fdOut);

    cJSON* parseOut = cJSON_Parse(fdOutMap);
    char* outBuf = cJSON_PrintUnformatted(parseOut);
    cJSON_Delete(parseOut);

    // assert_int_equal(appview_strlen(outBuf), appview_strlen(cmpStr));
    if (appview_strcmp(cmpStr, outBuf) != 0 ){
        appview_fprintf(appview_stderr, cmpStr);
        appview_fprintf(appview_stderr, outBuf);
        assert_non_null(NULL);
        appview_munmap(fdOutMap, stOut.st_size);
        return FALSE;
    }

    appview_free(outBuf);
    appview_munmap(fdOutMap, stOut.st_size);

    return TRUE;
}

static bool
rewriteOpenContainersConfigTest(int id, const char* unixSocketPath) {

    char inPath [PATH_MAX] = {0};
    appview_snprintf(inPath, PATH_MAX, "%s/data/oci/oci%din.json", dirPath, id);
    const char *appviewWithVersion = "/usr/lib/appview/1.2.3/appview";

    void *cfgMem = ociReadCfgIntoMem(inPath);

    char *modifMem = ociModifyCfg(cfgMem, appviewWithVersion, unixSocketPath);

    char outPath [PATH_MAX] = {0};

    if (unixSocketPath) {
        appview_snprintf(outPath, PATH_MAX, "%s/data/oci/oci%doutfull.json", dirPath, id);
    } else {
        appview_snprintf(outPath, PATH_MAX, "%s/data/oci/oci%doutpartial.json", dirPath, id);
    }

    bool res = verifyModifiedCfg(id, modifMem, outPath);

    appview_free(cfgMem);
    appview_free(modifMem);

    return res;
}

static void
ocitest_with_unix_path(void **state) {
    for (int i = 0; i < ARRAY_SIZE(testTypeJson); ++i) {
        bool res = rewriteOpenContainersConfigTest(i, "/var/run/appview/appview.sock");
        if (res != TRUE) {
            appview_fprintf(appview_stderr, "Error with test: id=%d name=%s\n", i, testTypeJson[i]);
        }
        assert_int_equal(res, TRUE);
    }
}

static void
ocitest_without_unix_path(void **state) {
    for (int i = 0; i < ARRAY_SIZE(testTypeJson); ++i) {
        bool res = rewriteOpenContainersConfigTest(i, NULL);
        if (res != TRUE) {
            appview_fprintf(appview_stderr, "Error with test: id=%d name=%s\n", i, testTypeJson[i]);
        }
        assert_int_equal(res, TRUE);
    }
}

int
main(int argc, char* argv[])
{
    appview_printf("running %s\n", argv[0]);
    if (testDirPath(dirPath, argv[0])) {
        return EXIT_FAILURE;
    }

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(ocitest_with_unix_path),
        cmocka_unit_test(ocitest_without_unix_path),
        cmocka_unit_test(dbgHasNoUnexpectedFailures),
    };
    return cmocka_run_group_tests(tests, groupSetup, groupTeardown);
}
