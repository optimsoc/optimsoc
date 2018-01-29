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

#define TEST_SUITE_NAME "check_cl_stm"

#include "mock_hostmod.h"
#include "testutil.h"

#include <osd/cl_stm.h>
#include <osd/osd.h>
#include <osd/reg.h>

struct osd_hostmod_ctx *hostmod_ctx;
struct osd_log_ctx *log_ctx;

// DI address of the STM module to be tested; chosen arbitrarily
const unsigned int stm_diaddr = 9;

/**
 * Test fixture: setup (called before each tests)
 */
void setup(void) { mock_hostmod_setup(); }

/**
 * Test fixture: setup (called after each test)
 */
void teardown(void) { mock_hostmod_teardown(); }

START_TEST(test_get_desc)
{
    osd_result rv;

    mock_hostmod_expect_mod_describe(stm_diaddr, OSD_MODULE_VENDOR_OSD,
                                     OSD_MODULE_TYPE_STD_STM, 0);

    mock_hostmod_expect_reg_read16(32, stm_diaddr, OSD_REG_STM_VALWIDTH,
                                   OSD_OK);

    struct osd_stm_desc stm_desc;
    rv = osd_cl_stm_get_desc(mock_hostmod_get_ctx(), stm_diaddr, &stm_desc);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_uint_eq(stm_desc.value_width_bit, 32);
    ck_assert_uint_eq(stm_desc.di_addr, stm_diaddr);
}
END_TEST

START_TEST(test_get_desc_wrong_module)
{
    osd_result rv;

    mock_hostmod_expect_mod_describe(stm_diaddr, OSD_MODULE_VENDOR_OSD,
                                     OSD_MODULE_TYPE_STD_CTM, 0);

    struct osd_stm_desc stm_desc;
    rv = osd_cl_stm_get_desc(mock_hostmod_get_ctx(), stm_diaddr, &stm_desc);
    ck_assert_int_eq(rv, OSD_ERROR_WRONG_MODULE);
}
END_TEST

START_TEST(test_add_to_print_buf)
{
    osd_result rv;

    struct osd_cl_stm_print_buf *print_buf;
    rv = osd_cl_stm_print_buf_new(&print_buf);
    ck_assert_int_eq(rv, OSD_OK);

    bool should_flush;

    struct osd_stm_event ev;
    ev.overflow = 0;
    ev.id = 4;
    ev.value = 'A';

    rv = osd_cl_stm_add_to_print_buf(&ev, print_buf, &should_flush);
    ck_assert_int_eq(rv, OSD_OK);

    ck_assert_uint_eq(print_buf->len_str, 1);
    ck_assert_str_eq(print_buf->buf, "A");
    ck_assert_int_eq(should_flush, false);

    osd_cl_stm_print_buf_free(&print_buf);
    ck_assert_ptr_eq(print_buf, NULL);
}
END_TEST

START_TEST(test_is_print_event)
{
    osd_result rv;

    struct osd_stm_event ev;

    ev.overflow = 0;
    ev.id = 4;
    ev.value = 'A';
    ck_assert_int_eq(osd_cl_stm_is_print_event(&ev), true);

    ev.overflow = 0;
    ev.id = 5;
    ev.value = 'A';
    ck_assert_int_eq(osd_cl_stm_is_print_event(&ev), false);

    ev.overflow = 27;
    ev.id = 4;
    ev.value = 'A';
    ck_assert_int_eq(osd_cl_stm_is_print_event(&ev), false);
}
END_TEST

static void event_handler(void *arg, const struct osd_stm_desc *stm_desc,
                          const struct osd_stm_event *event)
{
    ck_assert(arg);
    ck_assert(stm_desc);
    ck_assert(event);

    // in this test we pass the expected event as callback argument
    struct osd_stm_event *exp_event = arg;

    ck_assert_uint_eq(exp_event->overflow, event->overflow);
    ck_assert_uint_eq(exp_event->timestamp, event->timestamp);
    ck_assert_uint_eq(exp_event->id, event->id);
    ck_assert_uint_eq(exp_event->value, event->value);
}

START_TEST(test_handle_event)
{
    osd_result rv;

    struct osd_stm_desc stm_desc;
    stm_desc.di_addr = 2;
    stm_desc.value_width_bit = 32;

    struct osd_stm_event_handler ev_handler;
    ev_handler.cb_fn = event_handler;
    ev_handler.stm_desc = &stm_desc;

    struct osd_packet *pkg_trace;
    osd_packet_new(&pkg_trace, osd_packet_sizeconv_payload2data(5));
    rv = osd_packet_set_header(pkg_trace, 1, 2, OSD_PACKET_TYPE_EVENT, 0);
    ck_assert_int_eq(rv, OSD_OK);
    pkg_trace->data.payload[0] = 0xdead; // timestamp (LSB)
    pkg_trace->data.payload[1] = 0xbeef; // timestamp (MSB)
    pkg_trace->data.payload[2] = 4; // id
    pkg_trace->data.payload[3] = 0xaddf; // value (LSB)
    pkg_trace->data.payload[4] = 0xdeaf; // value (MSB)

    struct osd_stm_event exp_event;
    exp_event.overflow = 0;
    exp_event.timestamp = 0xbeefdead;
    exp_event.id = 4;
    exp_event.value = 0xdeafaddf;
    ev_handler.cb_arg = (void*)&exp_event;

    rv = osd_cl_stm_handle_event((void*)&ev_handler, pkg_trace);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

START_TEST(test_handle_event_overflow)
{
    osd_result rv;

    struct osd_stm_desc stm_desc;
    stm_desc.di_addr = 2;
    stm_desc.value_width_bit = 32;

    struct osd_stm_event_handler ev_handler;
    ev_handler.cb_fn = event_handler;
    ev_handler.stm_desc = &stm_desc;

    struct osd_packet *pkg_trace;
    osd_packet_new(&pkg_trace, osd_packet_sizeconv_payload2data(1));
    rv = osd_packet_set_header(pkg_trace, 1, 2, OSD_PACKET_TYPE_EVENT, 5);
    ck_assert_int_eq(rv, OSD_OK);
    pkg_trace->data.payload[0] = 25; // overflowed events

    struct osd_stm_event exp_event;
    exp_event.overflow = 25;
    exp_event.timestamp = 0;
    exp_event.id = 0;
    exp_event.value = 0;
    ev_handler.cb_arg = (void*)&exp_event;

    rv = osd_cl_stm_handle_event((void*)&ev_handler, pkg_trace);
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
    tcase_add_test(tc_core, test_add_to_print_buf);
    tcase_add_test(tc_core, test_is_print_event);
    tcase_add_test(tc_core, test_handle_event);
    tcase_add_test(tc_core, test_handle_event_overflow);
    suite_add_tcase(s, tc_core);

    return s;
}
