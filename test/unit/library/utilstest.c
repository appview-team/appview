#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"
#include "appviewstdlib.h"
#include "fn.h"
#include "test.h"

static void
testEndWithNegative(void **state)
{
    bool res = endsWith(NULL, NULL);
    assert_false(res);
    res = endsWith("hello", NULL);
    assert_false(res);
    res = endsWith(NULL, "hello");
    assert_false(res);
    res = endsWith("hello", "hell");
    assert_false(res);
    res = endsWith("hello", "hello1");
    assert_false(res);
}

static void
testEndWithPositive(void **state)
{
    bool res = endsWith("hello", "hello");
    assert_true(res);
    res = endsWith("hello", "o");
    assert_true(res);
    res = endsWith("hello", "llo");
    assert_true(res);
}

static void
testSetUUID(void **state)
{
    char uuid[37];
    setUUID(uuid);

    assert_int_equal(appview_strlen(uuid), 36);

    // UUID version is 4
    assert_int_equal(uuid[14] - '0', 4);

    assert_true(uuid[8] == '-');
    assert_true(uuid[13] == '-');
    assert_true(uuid[18] == '-');
    assert_true(uuid[23] == '-');

    // UUID is unique across calls
    char uuid_2[37];
    setUUID(uuid_2);

    assert_true(appview_strcmp(uuid, uuid_2));
}

static void
testSetMachineID(void **state)
{
    char mach_id[33];
    setMachineID(mach_id);

    assert_int_equal(appview_strlen(mach_id), 32);

    char mach_id_2[33];
    setMachineID(mach_id_2);

    // Machine ID is consistent across calls
    assert_true(!appview_strcmp(mach_id, mach_id_2));
}

static void
testSigSafeUtoa(void **state) {
    int len = 0;
    char buf[32] = {0};

    sigSafeUtoa(0, buf, 10, &len);
    assert_string_equal(buf, "0");
    assert_int_equal(len, 1);
    appview_memset(buf, 0, sizeof(buf));
    sigSafeUtoa(1234, buf, 10, &len);
    assert_string_equal(buf, "1234");
    assert_int_equal(len, 4);
    appview_memset(buf, 0, sizeof(buf));
    sigSafeUtoa(567, buf, 10, &len);
    assert_string_equal(buf, "567");
    assert_int_equal(len, 3);
    appview_memset(buf, 0, sizeof(buf));
    sigSafeUtoa(10, buf, 16, &len);
    assert_string_equal(buf, "a");
    assert_int_equal(len, 1);
}

static void
testGetEnvNonExisting(void **state) {
    char *env = fullGetEnv("foo");
    assert_null(env);
    int res = setenv("foo", "bar", 1);
    assert_int_equal(res, 0);
    env = fullGetEnv("foo");
    assert_string_equal(env, "bar");
    res = unsetenv("foo");
    assert_int_equal(res, 0);
    env = fullGetEnv("foo");
    assert_null(env);
}

int
main(int argc, char* argv[])
{
    printf("running %s\n", argv[0]);

    initFn();

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(testEndWithPositive),
        cmocka_unit_test(testEndWithNegative),
        cmocka_unit_test(testSetUUID),
        cmocka_unit_test(testSetMachineID),
        cmocka_unit_test(testSigSafeUtoa),
        cmocka_unit_test(testGetEnvNonExisting),
    };
    return cmocka_run_group_tests(tests, groupSetup, groupTeardown);
}
