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

#define TEST_SUITE_NAME "check_hostmod"

#include "mock_host_controller.h"
#include "testutil.h"

#include <czmq.h>
#include <osd/hostmod.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/reg.h>

struct osd_hostmod_ctx *hostmod_ctx;
struct osd_log_ctx *log_ctx;

const unsigned int mock_hostmod_diaddr = 7;

/**
 * Setup everything related to osd_hostmod
 */
void setup_hostmod(void)
{
    osd_result rv;

    log_ctx = testutil_get_log_ctx();

    // initialize hostmod context
    rv = osd_hostmod_new(&hostmod_ctx, log_ctx, "inproc://testing", NULL, NULL);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(hostmod_ctx, NULL);

    ck_assert_int_eq(osd_hostmod_is_connected(hostmod_ctx), 0);

    // connect
    mock_host_controller_expect_diaddr_req(mock_hostmod_diaddr);

    rv = osd_hostmod_connect(hostmod_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert_int_eq(osd_hostmod_is_connected(hostmod_ctx), 1);

    ck_assert_uint_eq(osd_hostmod_get_diaddr(hostmod_ctx), mock_hostmod_diaddr);
}

void teardown_hostmod(void)
{
    osd_result rv;

    ck_assert_int_eq(osd_hostmod_is_connected(hostmod_ctx), 1);

    rv = osd_hostmod_disconnect(hostmod_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert_int_eq(osd_hostmod_is_connected(hostmod_ctx), 0);

    osd_hostmod_free(&hostmod_ctx);
    ck_assert_ptr_eq(hostmod_ctx, NULL);
}

/**
 * Test fixture: setup (called before each tests)
 */
void setup(void)
{
    mock_host_controller_setup();
    setup_hostmod();
}

/**
 * Test fixture: setup (called after each test)
 */
void teardown(void)
{
    teardown_hostmod();
    mock_host_controller_teardown();
}

START_TEST(test_init_base)
{
    setup();
    teardown();
}
END_TEST

/**
 * Test how hostmod copes with the host controller not being reachable
 */
START_TEST(test_init_hostctrl_unreachable)
{
    osd_result rv;

    // log context
    rv = osd_log_new(&log_ctx, LOG_DEBUG, osd_log_handler);
    ck_assert_int_eq(rv, OSD_OK);

    // initialize hostmod context
    rv = osd_hostmod_new(&hostmod_ctx, log_ctx, "inproc://testing", NULL, NULL);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(hostmod_ctx, NULL);

    ck_assert_int_eq(osd_hostmod_is_connected(hostmod_ctx), 0);

    // try to connect
    rv = osd_hostmod_connect(hostmod_ctx);
    ck_assert_int_eq(rv, OSD_ERROR_CONNECTION_FAILED);

    ck_assert_int_eq(osd_hostmod_is_connected(hostmod_ctx), 0);

    osd_hostmod_free(&hostmod_ctx);
}
END_TEST

START_TEST(test_core_read_register)
{
    osd_result rv;

    uint16_t reg_read_result;

    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, 1, 0x0000,
                                         0x0001);

    rv = osd_hostmod_reg_read(hostmod_ctx, &reg_read_result, 1, 0x0000, 16, 0);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_uint_eq(reg_read_result, 0x0001);
}
END_TEST

START_TEST(test_core_write_register)
{
    osd_result rv;

    uint16_t reg_val = 0xdead;

    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, 1, 0x0000,
                                          reg_val);

    rv = osd_hostmod_reg_write(hostmod_ctx, &reg_val, 1, 0x0000, 16, 0);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

START_TEST(test_core_reg_setbit)
{
    osd_result rv;
    uint16_t old_reg_val, new_exp_reg_val;


    // set bit to zero
    old_reg_val = 0xdead;
    new_exp_reg_val = 0xdea5;
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, 1,
                                         4, old_reg_val);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, 1, 4,
                                          new_exp_reg_val);

    rv = osd_hostmod_reg_setbit(hostmod_ctx, 3, 0, 1, 4, 16, 0);
    ck_assert_int_eq(rv, OSD_OK);


    // set bit to one
    old_reg_val = 0xdea5;
    new_exp_reg_val = 0xdead;
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, 1,
                                         4, old_reg_val);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, 1, 4,
                                          new_exp_reg_val);

    rv = osd_hostmod_reg_setbit(hostmod_ctx, 3, 1, 1, 4, 16, 0);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

/**
 * Test timeout handling if a debug module doesn't respond to a register read
 * request.
 */
START_TEST(test_core_read_register_timeout)
{
    osd_result rv;
    uint16_t reg_read_result;

    mock_host_controller_expect_reg_read_noresp(mock_hostmod_diaddr, 1, 0x0000);

    rv = osd_hostmod_reg_read(hostmod_ctx, &reg_read_result, 1, 0x0000, 16, 0);
    ck_assert_int_eq(rv, OSD_ERROR_TIMEDOUT);
}
END_TEST

START_TEST(test_core_event_send)
{
    osd_result rv;

    struct osd_packet *event_pkg;
    osd_packet_new(&event_pkg, osd_packet_sizeconv_payload2data(1));
    osd_packet_set_header(event_pkg, mock_hostmod_diaddr, 1,
                          OSD_PACKET_TYPE_EVENT, 0);
    event_pkg->data.payload[0] = 0x0000;

    mock_host_controller_expect_data_req(event_pkg, NULL);

    osd_hostmod_event_send(hostmod_ctx, event_pkg);
    ck_assert_int_eq(rv, OSD_OK);

    osd_packet_free(&event_pkg);
}
END_TEST

START_TEST(test_core_event_receive)
{
    osd_result rv;

    struct osd_packet *event_pkg;
    osd_packet_new(&event_pkg, osd_packet_sizeconv_payload2data(1));
    osd_packet_set_header(event_pkg, 1, mock_hostmod_diaddr,
                          OSD_PACKET_TYPE_EVENT, 0);
    event_pkg->data.payload[0] = 0x0000;

    mock_host_controller_queue_data_packet(event_pkg);

    struct osd_packet *rcv_event_pkg;
    osd_hostmod_event_receive(hostmod_ctx, &rcv_event_pkg, 0);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert(osd_packet_equal(event_pkg, rcv_event_pkg));

    osd_packet_free(&event_pkg);
    osd_packet_free(&rcv_event_pkg);
}
END_TEST

START_TEST(test_layer2_mod_describe)
{
    osd_result rv;

    uint16_t mod_vendor, mod_type, mod_version;
    mod_vendor = 0xbeef;
    mod_type = 0xdead;
    mod_version = 0xaddf;

    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr, 1,
                                          mod_vendor, mod_type, mod_version);

    struct osd_module_desc desc;
    rv = osd_hostmod_mod_describe(hostmod_ctx, 1, &desc);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert_uint_eq(desc.addr, 1);
    ck_assert_uint_eq(desc.vendor, mod_vendor);
    ck_assert_uint_eq(desc.type, mod_type);
    ck_assert_uint_eq(desc.version, mod_version);
}
END_TEST

START_TEST(test_layer2_mod_event_active)
{
    osd_result rv;
    uint16_t old_reg_val, new_exp_reg_val;

    old_reg_val = 0;
    new_exp_reg_val = 1;
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, 1,
                                         OSD_REG_BASE_MOD_CS, old_reg_val);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, 1,
                                          OSD_REG_BASE_MOD_CS,
                                          new_exp_reg_val);

    rv = osd_hostmod_mod_set_event_active(hostmod_ctx, 1, true, 0);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

START_TEST(test_layer2_mod_event_dest)
{
    osd_result rv;
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, 1,
                                          OSD_REG_BASE_MOD_EVENT_DEST,
                                          mock_hostmod_diaddr);

    rv = osd_hostmod_mod_set_event_dest(hostmod_ctx, 1, 0);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

/**
 * Test the debug module enumeration handled by osd_hostmod_get_modules()
 */
START_TEST(test_layer2_get_modules)
{
    osd_result rv;
    struct osd_module_desc *modules;
    size_t modules_len;

    const unsigned int target_subnet = 0;
    unsigned int scm_diaddr = osd_diaddr_build(0, 0); // SCM is always at x.0

    // Step 1: Get number of modules in the subnet by reading NUM_MODS
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, scm_diaddr,
                                         OSD_REG_SCM_NUM_MOD, 3);

    // Step 2: Enumerate all debug modules
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr, scm_diaddr,
                                          OSD_MODULE_VENDOR_OSD,
                                          OSD_MODULE_TYPE_STD_SCM, 0);
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr,
                                          scm_diaddr + 1,
                                          OSD_MODULE_VENDOR_OSD,
                                          OSD_MODULE_TYPE_STD_MAM, 0);
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr,
                                          scm_diaddr + 2,
                                          OSD_MODULE_VENDOR_OSD,
                                          OSD_MODULE_TYPE_STD_STM, 0);

    rv = osd_hostmod_get_modules(hostmod_ctx, 0, &modules, &modules_len);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert(modules);
    ck_assert_uint_eq(modules_len, 3);

    free(modules);
}
END_TEST

/**
 * Test the debug module enumeration handled by osd_hostmod_get_modules()
 */
START_TEST(test_layer2_get_modules_partial)
{
    osd_result rv;
    struct osd_module_desc *modules;
    size_t modules_len;

    const unsigned int target_subnet = 0;
    unsigned int scm_diaddr = osd_diaddr_build(0, 0); // SCM is always at x.0

    // Step 1: Get number of modules in the subnet by reading NUM_MODS
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, scm_diaddr,
                                         OSD_REG_SCM_NUM_MOD, 3);

    // Step 2: Enumerate all debug modules
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr,
                                             scm_diaddr,
                                             OSD_MODULE_VENDOR_OSD,
                                             OSD_MODULE_TYPE_STD_SCM, 0);

    // time out when reading the VENDOR register from module at address x.1
    mock_host_controller_expect_reg_read_noresp(mock_hostmod_diaddr,
                                                scm_diaddr + 1,
                                                OSD_REG_BASE_MOD_VENDOR);

    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr,
                                             scm_diaddr + 2,
                                             OSD_MODULE_VENDOR_OSD,
                                             OSD_MODULE_TYPE_STD_STM, 0);

    rv = osd_hostmod_get_modules(hostmod_ctx, 0, &modules, &modules_len);
    ck_assert_int_eq(rv, OSD_ERROR_PARTIAL_RESULT);
    ck_assert(modules);
    ck_assert_uint_eq(modules_len, 3);

    ck_assert_uint_eq(modules[0].type, OSD_MODULE_TYPE_STD_SCM);
    ck_assert_uint_eq(modules[1].type, OSD_MODULE_TYPE_STD_UNKNOWN);
    ck_assert_uint_eq(modules[2].type, OSD_MODULE_TYPE_STD_STM);

    free(modules);

}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_init, *tc_core, *tc_layer2;

    s = suite_create(TEST_SUITE_NAME);

    // Initialization
    // As the setup and teardown functions are pretty heavy, we check them
    // here independently and use them as test fixtures after this test
    // succeeds.
    tc_init = tcase_create("Init");
    tcase_add_test(tc_init, test_init_base);
    tcase_add_test(tc_init, test_init_hostctrl_unreachable);
    suite_add_tcase(s, tc_init);

    // Core functionality
    tc_core = tcase_create("Core");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_core_read_register);
    tcase_add_test(tc_core, test_core_read_register_timeout);
    tcase_add_test(tc_core, test_core_write_register);
    tcase_add_test(tc_core, test_core_reg_setbit);

    tcase_add_test(tc_core, test_core_event_send);
    tcase_add_test(tc_core, test_core_event_receive);
    suite_add_tcase(s, tc_core);

    // Higher-layer functionality
    tc_layer2 = tcase_create("Layer2");
    tcase_add_checked_fixture(tc_layer2, setup, teardown);
    tcase_add_test(tc_layer2, test_layer2_mod_describe);
    tcase_add_test(tc_layer2, test_layer2_mod_event_active);
    tcase_add_test(tc_layer2, test_layer2_mod_event_dest);
    tcase_add_test(tc_layer2, test_layer2_get_modules);
    tcase_add_test(tc_layer2, test_layer2_get_modules_partial);
    suite_add_tcase(s, tc_layer2);

    return s;
}
