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

#define TEST_SUITE_NAME "check_util"

#include "testutil.h"

#include <osd/osd.h>

START_TEST(test_util)
{
    const struct osd_version *v = osd_version_get();

    ck_assert_int_eq(v->major, OSD_VERSION_MAJOR);
    ck_assert_int_eq(v->minor, OSD_VERSION_MINOR);
    ck_assert_int_eq(v->micro, OSD_VERSION_MICRO);
    ck_assert_str_eq(v->suffix, OSD_VERSION_SUFFIX);
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create(TEST_SUITE_NAME);

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_util);
    suite_add_tcase(s, tc_core);

    return s;
}
