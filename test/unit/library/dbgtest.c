#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "dbg.h"

#include "appviewstdlib.h"
#include "test.h"

static void
dbgInitSetsGlobal(void** state)
{
    assert_null(g_dbg);
    dbgInit();
    assert_non_null(g_dbg);
    dbgInit();
    assert_non_null(g_dbg);
    assert_int_equal(dbgCountAllLines(), 0);
}

static void
dbgDestroyClearsGlobal(void** state)
{
    assert_non_null(g_dbg);
    dbgDestroy();
    assert_null(g_dbg);
    dbgDestroy();
    assert_null(g_dbg);
    assert_int_equal(dbgCountAllLines(), 0);
}

static void
dbgAddLineSetsGlobalIfNotSet(void** state)
{
    assert_null(g_dbg);
    dbgInit();
    assert_non_null(g_dbg);
    dbgAddLine("file:7", "%d", 123);
    assert_int_equal(1, dbgCountAllLines());
}

static void
dbgMacroIdentifiesFileAndLine(void** state)
{
    dbgInit();
    DBG(NULL);                        // test/unit/library/dbgtest.c:46
    DBG("blah");                      // test/unit/library/dbgtest.c:47
    DBG("%s", "something");           // test/unit/library/dbgtest.c:48
    DBG("%d%s", 314159, "something"); // test/unit/library/dbgtest.c:49

    char buf[4096] = {0};
    dbgDumpAllToBuffer(buf, sizeof(buf));

    assert_non_null(appview_strstr(buf, "test/unit/library/dbgtest.c:46"));
    assert_non_null(appview_strstr(buf, "test/unit/library/dbgtest.c:47"));
    assert_non_null(appview_strstr(buf, "test/unit/library/dbgtest.c:48"));
    assert_non_null(appview_strstr(buf, "test/unit/library/dbgtest.c:49"));
    assert_int_equal(4, dbgCountAllLines());
}

static void
dbgAddLineHasCorrectCount(void** state)
{
    dbgInit();
    dbgAddLine("key1", NULL);
    int i;
    for (i=0; i<5; i++) {
        dbgAddLine("key2", NULL);
    }

    char buf[4096] = {0};
    dbgDumpAllToBuffer(buf, sizeof(buf));

    assert_non_null(appview_strstr(buf, "1: key1 "));
    assert_non_null(appview_strstr(buf, "5: key2 "));
    assert_int_equal(2, dbgCountAllLines());
    assert_int_equal(2, dbgCountMatchingLines("key"));
    assert_int_equal(1, dbgCountMatchingLines("key1"));
    assert_int_equal(0, dbgCountMatchingLines("test"));
}

static void
dbgAddLineCapturesTimeErrnoAndStr(void** state)
{
    dbgInit();
    appview_errno = EINVAL;
    dbgAddLine("key1", "str1");
    appview_errno = EEXIST;
    assert_int_equal(appview_errno, EEXIST);
    dbgAddLine("key2", "%s", "str2");

    char buf[4096] = {0};
    dbgDumpAllToBuffer(buf, sizeof(buf));

    unsigned long long count;
    char key[64] = {0};
    char time[64] = {0};
    int err = 0;
    char err_str[64] = {0};
    char str[64] = {0};
    int rv;

    char* key1_line = appview_strstr (buf, "1: key1");
    rv = appview_sscanf(key1_line, "%llu: %64s %64s %d(%64[^)]) %64s\n",
                    &count, key, time, &err, err_str, str);
    assert_int_equal(rv, 6);
    assert_int_equal(count, 1);
    assert_string_equal(key, "key1");
    assert_int_equal(err, EINVAL);
    assert_string_equal(err_str, "Invalid argument");
    assert_string_equal(str, "str1");

    char* key2_line = appview_strstr (buf, "1: key2");
    rv = appview_sscanf(key2_line, "%llu: %64s %64s %d(%64[^)]) %64s\n",
                    &count, key, time, &err, err_str, str);
    assert_int_equal(rv, 6);
    assert_int_equal(count, 1);
    assert_string_equal(key, "key2");
    assert_int_equal(err, EEXIST);
    assert_string_equal(err_str, "File exists");
    assert_string_equal(str, "str2");
}

static void
dbgAddLineCapturesFirstAndLastInstance(void** state)
{
    dbgInit();
    appview_errno = 1;
    dbgAddLine("key1", "str1");
    appview_errno = 2;
    dbgAddLine("key1", "str2");
    appview_errno = 3;
    dbgAddLine("key1", "str3");

    char buf[4096] = {0};
    dbgDumpAllToBuffer(buf, sizeof(buf));

    assert_non_null(appview_strstr(buf, "str1"));
    assert_null(appview_strstr(buf, "str2"));
    assert_non_null(appview_strstr(buf, "str3"));
}

static void
dbgAddLineTestReallocWorks(void** state)
{
    dbgInit();
    int i;
    char* key = appview_calloc(1, 128*8); // Create an array big enough for all
    for (i=0; i<128; i++) {
        assert_true(appview_snprintf(&key[i*8], 8, "key%d", i) > 0);
        dbgAddLine(&key[i*8], NULL);
    }
    assert_int_equal(dbgCountAllLines(), 128);
    dbgDestroy();
    appview_free(key);
}

static void
dbgDumpAllOutputsVersionAndTime(void** state)
{
    char buf[4096] = {0};
    dbgDumpAllToBuffer(buf, sizeof(buf));

    char version[128] = {0};
    char date[128] = {0};
    int rv = appview_sscanf(buf, "AppView Version: %128s   Dump From: %128s\n", version, date);

    assert_int_equal(rv, 2);
    assert_string_equal(version, APPVIEW_VER);
    assert_string_not_equal(date, "");

}


int
main(int argc, char* argv[])
{
    printf("running %s\n", argv[0]);
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(dbgInitSetsGlobal),
        cmocka_unit_test(dbgDestroyClearsGlobal),
        cmocka_unit_test(dbgAddLineSetsGlobalIfNotSet),
        cmocka_unit_test(dbgMacroIdentifiesFileAndLine),
        cmocka_unit_test(dbgAddLineHasCorrectCount),
        cmocka_unit_test(dbgAddLineCapturesTimeErrnoAndStr),
        cmocka_unit_test(dbgAddLineCapturesFirstAndLastInstance),
        cmocka_unit_test(dbgAddLineTestReallocWorks),
        cmocka_unit_test(dbgDumpAllOutputsVersionAndTime),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
