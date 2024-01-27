#define _GNU_SOURCE
#include "backoff.h"
#include "appviewstdlib.h"
#include "test.h"


static void
backoffCreateReturnsValidPtr(void **state)
{
    backoff_t *backoff;
    backoff = backoffCreate();
    assert_non_null(backoff);

    backoffDestroy(&backoff);
}

static void
backoffDestroyNullDoesntCrash(void **state)
{
    backoffDestroy(NULL);
    backoff_t *backoff = NULL;
    backoffDestroy(&backoff);
}

static void
backoffAlgoAllowsConnectReturnsTrueIfNull(void **state)
{
    assert_true(backoffAlgoAllowsConnect(NULL));
}

static void
backoffAlgoAllowsConnectFirstTime(void **state)
{
    backoff_t *backoff = backoffCreate();
    assert_non_null(backoff);

    assert_true(backoffAlgoAllowsConnect(backoff));

    backoffDestroy(&backoff);
}


unsigned values[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256 };
const unsigned num_values = sizeof(values) / sizeof(values[0]);

static void
backoffAlgoAllowsConnectIsInRange(void **state)
{
    backoff_t *backoff = backoffCreate();

    // test values are in range while the window is expanding
    int i;
    for (i=0; i<num_values; i++) {
        unsigned lower_bound_ms = values[i] * 1000;
        // as much as 1000 ms of jitter is possible
        unsigned upper_bound_ms = lower_bound_ms + 1000;

        int ms = 0;
        while (!backoffAlgoAllowsConnect(backoff)) ms++;

        //printf("ms = %d, lower_bound_ms = %d, upper_bound_ms = %d\n",
        //       ms, lower_bound_ms, upper_bound_ms);
        assert_in_range(ms, lower_bound_ms, upper_bound_ms);
    }

    // test that the values show that the window stops expanding
    for (i=0; i<10; i++) {
        unsigned lower_bound_ms = 256 * 1000;
        unsigned upper_bound_ms = lower_bound_ms + 1000;

        int ms = 0;
        while (!backoffAlgoAllowsConnect(backoff)) ms++;

        //printf("ms = %d, lower_bound_ms = %d, upper_bound_ms = %d\n",
        //       ms, lower_bound_ms, upper_bound_ms);
        assert_in_range(ms, lower_bound_ms, upper_bound_ms);
    }

    backoffDestroy(&backoff);
}

static void
backoffResetNullDoesntCrash(void **state)
{
    backoffReset(NULL);
}

static void
backoffResetReinitializesState(void **state)
{
    backoff_t *backoff = backoffCreate();

    // iterate through half of the expanding windows.
    int i;
    for (i=0; i<num_values/2; i++) {
        unsigned lower_bound_ms = values[i] * 1000;
        unsigned upper_bound_ms = lower_bound_ms + 1000;

        int ms = 0;
        while (!backoffAlgoAllowsConnect(backoff)) ms++;

        //printf("ms = %d, lower_bound_ms = %d, upper_bound_ms = %d\n",
        //       ms, lower_bound_ms, upper_bound_ms);
        assert_in_range(ms, lower_bound_ms, upper_bound_ms);
    }

    // Call reset
    backoffReset(backoff);

    // repeat the above starting at values[0] again.
    // This shows backoffReset() had the desired effect.
    for (i=0; i<num_values; i++) {
        unsigned lower_bound_ms = values[i] * 1000;
        unsigned upper_bound_ms = lower_bound_ms + 1000;

        int ms = 0;
        while (!backoffAlgoAllowsConnect(backoff)) ms++;

        //printf("ms = %d, lower_bound_ms = %d, upper_bound_ms = %d\n",
        //       ms, lower_bound_ms, upper_bound_ms);
        assert_in_range(ms, lower_bound_ms, upper_bound_ms);
    }

    backoffDestroy(&backoff);
}


int
main(int argc, char* argv[])
{
    printf("running %s\n", argv[0]);

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(backoffCreateReturnsValidPtr),
        cmocka_unit_test(backoffDestroyNullDoesntCrash),
        cmocka_unit_test(backoffAlgoAllowsConnectReturnsTrueIfNull),
        cmocka_unit_test(backoffAlgoAllowsConnectFirstTime),
        cmocka_unit_test(backoffAlgoAllowsConnectIsInRange),
        cmocka_unit_test(backoffResetNullDoesntCrash),
        cmocka_unit_test(backoffResetReinitializesState),
        cmocka_unit_test(dbgHasNoUnexpectedFailures),
    };
    return cmocka_run_group_tests(tests, groupSetup, groupTeardown);
}

