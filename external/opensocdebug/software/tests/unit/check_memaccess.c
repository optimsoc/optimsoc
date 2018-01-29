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

#define TEST_SUITE_NAME "check_memaccess"

#include "testutil.h"

#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/memaccess.h>

#include "mock_host_controller.h"

struct osd_memaccess_ctx *memaccess_ctx;
struct osd_log_ctx* log_ctx;

const unsigned int target_subnet_addr = 0;
unsigned int mock_hostmod_diaddr;
unsigned int mock_scm_diaddr;

/**
 * Setup everything related to osd_hostmod
 */
void setup_hostmod(void)
{
    osd_result rv;

    log_ctx = testutil_get_log_ctx();

    // initialize module context
    rv = osd_memaccess_new(&memaccess_ctx, log_ctx, "inproc://testing");
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(memaccess_ctx, NULL);

    // connect
    mock_host_controller_expect_diaddr_req(mock_hostmod_diaddr);

    rv = osd_memaccess_connect(memaccess_ctx);
    ck_assert_int_eq(rv, OSD_OK);
}

void teardown_hostmod(void)
{
    osd_result rv;
    rv = osd_memaccess_disconnect(memaccess_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    osd_memaccess_free(&memaccess_ctx);
    ck_assert_ptr_eq(memaccess_ctx, NULL);
}

/**
 * Test fixture: setup (called before each tests)
 */
void setup(void)
{
    mock_hostmod_diaddr = osd_diaddr_build(1, 1);
    mock_scm_diaddr = osd_diaddr_build(target_subnet_addr, 0);

    mock_host_controller_setup();
    setup_hostmod();
}

/**
 * Test fixture: setup (called after each test)
 */
void teardown(void)
{
    mock_host_controller_wait_for_event_tx();
    teardown_hostmod();
    mock_host_controller_teardown();
}

START_TEST(test_init_base)
{
    setup();
    teardown();
}
END_TEST

START_TEST(test_core_cpus_start)
{
    osd_result rv;
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr,
                                         mock_scm_diaddr,
                                         OSD_REG_SCM_SYSRST,
                                         0x0002);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr,
                                          mock_scm_diaddr,
                                          OSD_REG_SCM_SYSRST,
                                          0x0000);
    rv = osd_memaccess_cpus_start(memaccess_ctx, target_subnet_addr);
    ck_assert(OSD_SUCCEEDED(rv));
}
END_TEST

START_TEST(test_core_cpus_stop)
{
    osd_result rv;
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr,
                                         mock_scm_diaddr,
                                         OSD_REG_SCM_SYSRST,
                                         0x0000);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr,
                                          mock_scm_diaddr,
                                          OSD_REG_SCM_SYSRST,
                                          0x0002);
    rv = osd_memaccess_cpus_stop(memaccess_ctx, target_subnet_addr);
    ck_assert(OSD_SUCCEEDED(rv));
}
END_TEST

START_TEST(test_core_find_memories)
{
    osd_result rv;

    const unsigned int mam_diaddr = 1;

    // Step 1: Get number of modules in the subnet by reading NUM_MODS
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, 0,
                                         OSD_REG_SCM_NUM_MOD, 3);

    // Step 2: Enumerate all debug modules
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr, 0,
                                             OSD_MODULE_VENDOR_OSD,
                                             OSD_MODULE_TYPE_STD_SCM, 0);
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr,
                                             mam_diaddr,
                                             OSD_MODULE_VENDOR_OSD,
                                             OSD_MODULE_TYPE_STD_MAM, 0);
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr, 2,
                                             OSD_MODULE_VENDOR_OSD,
                                             OSD_MODULE_TYPE_STD_STM, 0);

    // Step 3: Query information from the MAM module
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_AW, 16);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_DW, 32);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGIONS, 1);

    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGION_BASEADDR(0, 0),
                                         0xcdef);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGION_BASEADDR(0, 1),
                                         0x89ab);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGION_BASEADDR(0, 2),
                                         0x4567);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGION_BASEADDR(0, 3),
                                         0x0123);

    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGION_MEMSIZE(0, 0),
                                         0xdead);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGION_MEMSIZE(0, 1),
                                         0x89ab);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGION_MEMSIZE(0, 2),
                                         0x4567);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mam_diaddr,
                                         OSD_REG_MAM_REGION_MEMSIZE(0, 3),
                                         0x0123);


    struct osd_mem_desc *memories;
    size_t memories_len;
    rv = osd_memaccess_find_memories(memaccess_ctx, target_subnet_addr,
                                     &memories, &memories_len);
    ck_assert(OSD_SUCCEEDED(rv));
    ck_assert_uint_eq(memories_len, 1);
    ck_assert_uint_eq(memories[0].di_addr, 1);
    ck_assert_uint_eq(memories[0].addr_width_bit, 16);
    ck_assert_uint_eq(memories[0].data_width_bit, 32);

    free(memories);
}
END_TEST

Suite * suite(void)
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

    // Core functionality
    tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_core_cpus_start);
    tcase_add_test(tc_core, test_core_cpus_stop);
    tcase_add_test(tc_core, test_core_find_memories);
    suite_add_tcase(s, tc_core);

    return s;
}
