/* Copyright 2017-2018 The Open SoC Debug Project
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

#define TEST_SUITE_NAME "check_cl_scm"

#include "mock_hostmod.h"
#include "testutil.h"

#include <osd/cl_scm.h>
#include <osd/osd.h>
#include <osd/reg.h>

struct osd_hostmod_ctx *hostmod_ctx;
struct osd_log_ctx *log_ctx;

// Target subnet during test
const unsigned int subnet_addr = 0;

/**
 * Test fixture: setup (called before each tests)
 */
void setup(void) { mock_hostmod_setup(); }

/**
 * Test fixture: setup (called after each test)
 */
void teardown(void) { mock_hostmod_teardown(); }

START_TEST(test_cpus_start)
{
    osd_result rv;

    uint16_t old_reg_val = 0xde0F;
    uint16_t new_reg_val = 0xde0D;

    mock_hostmod_expect_reg_read16(old_reg_val,
                                   osd_diaddr_build(subnet_addr, 0),
                                   OSD_REG_SCM_SYSRST,
                                   OSD_OK);
    mock_hostmod_expect_reg_write16(new_reg_val,
                                    osd_diaddr_build(subnet_addr, 0),
                                    OSD_REG_SCM_SYSRST,
                                    OSD_OK);

    rv = osd_cl_scm_cpus_start(mock_hostmod_get_ctx(), subnet_addr);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

START_TEST(test_cpus_stop)
{
    osd_result rv;

    uint16_t old_reg_val = 0xde0D;
    uint16_t new_reg_val = 0xde0F;

    mock_hostmod_expect_reg_read16(old_reg_val,
                                   osd_diaddr_build(subnet_addr, 0),
                                   OSD_REG_SCM_SYSRST,
                                   OSD_OK);
    mock_hostmod_expect_reg_write16(new_reg_val,
                                    osd_diaddr_build(subnet_addr, 0),
                                    OSD_REG_SCM_SYSRST,
                                    OSD_OK);

    rv = osd_cl_scm_cpus_stop(mock_hostmod_get_ctx(), subnet_addr);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

START_TEST(test_get_subnetinfo)
{
    osd_result rv;

    mock_hostmod_expect_reg_read16(0x42,
                                   osd_diaddr_build(subnet_addr, 0),
                                   OSD_REG_SCM_SYSTEM_VENDOR_ID,
                                   OSD_OK);
    mock_hostmod_expect_reg_read16(0xabcd,
                                   osd_diaddr_build(subnet_addr, 0),
                                   OSD_REG_SCM_SYSTEM_DEVICE_ID,
                                   OSD_OK);
    mock_hostmod_expect_reg_read16(8,
                                   osd_diaddr_build(subnet_addr, 0),
                                   OSD_REG_SCM_MAX_PKT_LEN,
                                   OSD_OK);

    struct osd_subnet_desc subnet_desc;
    rv = osd_cl_scm_get_subnetinfo(mock_hostmod_get_ctx(), subnet_addr,
                                   &subnet_desc);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_uint_eq(subnet_desc.vendor_id, 0x42);
    ck_assert_uint_eq(subnet_desc.device_id, 0xabcd);
    ck_assert_uint_eq(subnet_desc.max_pkt_len, 8);
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create(TEST_SUITE_NAME);

    tc_core = tcase_create("Core Functionality");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_cpus_start);
    tcase_add_test(tc_core, test_cpus_stop);
    tcase_add_test(tc_core, test_get_subnetinfo);
    suite_add_tcase(s, tc_core);

    return s;
}
