/* Copyright 2017 The Open SoC Debug Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define TEST_SUITE_NAME "check_hostctrl"

#include "testutil.h"

#include <czmq.h>
#include <osd/hostctrl.h>
#include <osd/osd.h>
#include <osd/packet.h>

struct osd_hostctrl_ctx *hostctrl_ctx;
struct osd_log_ctx *log_ctx;

/**
 * Test fixture: setup (called before each tests)
 */
void setup(void)
{
    osd_result rv;
    hostctrl_ctx = NULL;
    rv = osd_hostctrl_new(&hostctrl_ctx, log_ctx, "inproc://testing");
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(hostctrl_ctx, NULL);

    ck_assert_int_eq(osd_hostctrl_is_running(hostctrl_ctx), 0);

    rv = osd_hostctrl_start(hostctrl_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert_int_eq(osd_hostctrl_is_running(hostctrl_ctx), 1);
}

/**
 * Test fixture: setup (called after each test)
 */
void teardown(void)
{
    osd_result rv;
    ck_assert_int_eq(osd_hostctrl_is_running(hostctrl_ctx), 1);

    rv = osd_hostctrl_stop(hostctrl_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert_int_eq(osd_hostctrl_is_running(hostctrl_ctx), 0);

    osd_hostctrl_free(&hostctrl_ctx);
    ck_assert_ptr_eq(hostctrl_ctx, NULL);
}

START_TEST(test_init_base)
{
    setup();
    teardown();
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_init, *tc_core;

    s = suite_create(TEST_SUITE_NAME);

    // Initialization
    // As the setup and teardown functions are pretty heavy, we check them
    // here independently and use them as test fixtures after this test
    // succeeds.
    tc_init = tcase_create("Init");
    tcase_add_test(tc_init, test_init_base);
    suite_add_tcase(s, tc_init);

    return s;
}
