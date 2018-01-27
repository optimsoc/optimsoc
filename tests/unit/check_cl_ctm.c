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

#define TEST_SUITE_NAME "check_cl_ctm"

#include "mock_hostmod.h"
#include "testutil.h"

#include <osd/cl_ctm.h>
#include <osd/osd.h>
#include <osd/reg.h>

struct osd_hostmod_ctx *hostmod_ctx;
struct osd_log_ctx *log_ctx;

// DI address of the CTM module to be tested; chosen arbitrarily
const unsigned int ctm_diaddr = 15;

/**
 * Test fixture: setup (is_called before each tests)
 */
void setup(void) { mock_hostmod_setup(); }

/**
 * Test fixture: setup (is_called after each test)
 */
void teardown(void) { mock_hostmod_teardown(); }

START_TEST(test_get_desc)
{
    osd_result rv;

    mock_hostmod_expect_mod_describe(ctm_diaddr, OSD_MODULE_VENDOR_OSD,
                                     OSD_MODULE_TYPE_STD_CTM, 0);

    mock_hostmod_expect_reg_read16(32, ctm_diaddr, OSD_REG_CTM_ADDR_WIDTH,
                                   OSD_OK);
    mock_hostmod_expect_reg_read16(64, ctm_diaddr, OSD_REG_CTM_DATA_WIDTH,
                                   OSD_OK);

    struct osd_ctm_desc ctm_desc;
    rv = osd_cl_ctm_get_desc(mock_hostmod_get_ctx(), ctm_diaddr, &ctm_desc);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_uint_eq(ctm_desc.addr_width_bit, 32);
    ck_assert_uint_eq(ctm_desc.data_width_bit, 64);
    ck_assert_uint_eq(ctm_desc.di_addr, ctm_diaddr);
}
END_TEST

START_TEST(test_get_desc_wrong_module)
{
    osd_result rv;

    mock_hostmod_expect_mod_describe(ctm_diaddr, OSD_MODULE_VENDOR_OSD,
                                     OSD_MODULE_TYPE_STD_MAM, 0);

    struct osd_ctm_desc ctm_desc;
    rv = osd_cl_ctm_get_desc(mock_hostmod_get_ctx(), ctm_diaddr, &ctm_desc);
    ck_assert_int_eq(rv, OSD_ERROR_WRONG_MODULE);
}
END_TEST

static void event_handler(void *arg, const struct osd_ctm_desc *ctm_desc,
                          const struct osd_ctm_event *event)
{
    ck_assert(arg);
    ck_assert(ctm_desc);
    ck_assert(event);

    // in this test we pass the expected event as is_callback argument
    struct osd_ctm_event *exp_event = arg;

    ck_assert_uint_eq(exp_event->overflow, event->overflow);
    ck_assert_uint_eq(exp_event->timestamp, event->timestamp);
    ck_assert_uint_eq(exp_event->npc, event->npc);
    ck_assert_uint_eq(exp_event->pc, event->pc);
    ck_assert_uint_eq(exp_event->mode, event->mode);
    ck_assert_uint_eq(exp_event->is_ret, event->is_ret);
    ck_assert_uint_eq(exp_event->is_call, event->is_call);
    ck_assert_uint_eq(exp_event->is_modechange, event->is_modechange);
}

START_TEST(test_handle_event)
{
    osd_result rv;

    struct osd_ctm_desc ctm_desc;
    ctm_desc.di_addr = 2;
    ctm_desc.addr_width_bit = 32;
    ctm_desc.data_width_bit = 32;

    struct osd_ctm_event_handler ev_handler;
    ev_handler.cb_fn = event_handler;
    ev_handler.ctm_desc = &ctm_desc;

    struct osd_packet *pkg_trace;
    osd_packet_new(&pkg_trace, osd_packet_sizeconv_payload2data(7));
    rv = osd_packet_set_header(pkg_trace, 1, 2, OSD_PACKET_TYPE_EVENT, 0);
    ck_assert_int_eq(rv, OSD_OK);
    pkg_trace->data.payload[0] = 0xdead; // timestamp (LSB)
    pkg_trace->data.payload[1] = 0xbeef; // timestamp (MSB)
    pkg_trace->data.payload[2] = 0xad00; // npc (LSB)
    pkg_trace->data.payload[3] = 0xdeaf; // npc (MSB)
    pkg_trace->data.payload[4] = 0x0100; // pc (LSB)
    pkg_trace->data.payload[5] = 0x4567; // pc (MSB)
    pkg_trace->data.payload[6] = 0x15; // mode=01, ret=1, call=0, modechange=1

    struct osd_ctm_event exp_event;
    exp_event.overflow = 0;
    exp_event.timestamp = 0xbeefdead;
    exp_event.npc = 0xdeafad00;
    exp_event.pc = 0x45670100;
    exp_event.mode = 1;
    exp_event.is_ret = 1;
    exp_event.is_call = 0;
    exp_event.is_modechange = 1;
    ev_handler.cb_arg = (void*)&exp_event;

    rv = osd_cl_ctm_handle_event((void*)&ev_handler, pkg_trace);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

START_TEST(test_handle_event_overflow)
{
    osd_result rv;

    struct osd_ctm_desc ctm_desc;
    ctm_desc.di_addr = 2;
    ctm_desc.addr_width_bit = 32;
    ctm_desc.data_width_bit = 32;

    struct osd_ctm_event_handler ev_handler;
    ev_handler.cb_fn = event_handler;
    ev_handler.ctm_desc = &ctm_desc;

    struct osd_packet *pkg_trace;
    osd_packet_new(&pkg_trace, osd_packet_sizeconv_payload2data(1));
    rv = osd_packet_set_header(pkg_trace, 1, 2, OSD_PACKET_TYPE_EVENT, 5);
    ck_assert_int_eq(rv, OSD_OK);
    pkg_trace->data.payload[0] = 25; // overflowed events

    struct osd_ctm_event exp_event = { 0 };
    exp_event.overflow = 25;
    ev_handler.cb_arg = (void*)&exp_event;

    rv = osd_cl_ctm_handle_event((void*)&ev_handler, pkg_trace);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create(TEST_SUITE_NAME);

    tc_core = tcase_create("Core Functionality");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_get_desc);
    tcase_add_test(tc_core, test_get_desc_wrong_module);
    tcase_add_test(tc_core, test_handle_event);
    tcase_add_test(tc_core, test_handle_event_overflow);
    suite_add_tcase(s, tc_core);

    return s;
}
