/* Copyright 2018 The Open SoC Debug Project
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

#define TEST_SUITE_NAME "check_cl_dem_uart"

#include <osd/cl_dem_uart.h>
#include <osd/osd.h>

#include "mock_hostmod.h"
#include "testutil.h"

// DI address of the DEM-UART module to be tested; chosen arbitrarily
const unsigned int dem_uart_diaddr = 16;

/**
 * Test fixture: setup (called before each test)
 */
void setup(void) { mock_hostmod_setup(); }
/**
 * Test fixture: setup (called after each test)
 */
void teardown(void) { mock_hostmod_teardown(); }

START_TEST(test_get_desc)
{
    osd_result rv;

    mock_hostmod_expect_mod_describe(dem_uart_diaddr, OSD_MODULE_VENDOR_OSD,
                                     OSD_MODULE_TYPE_STD_DEM_UART, 0);

    struct osd_dem_uart_desc dem_uart_desc;
    rv = osd_cl_dem_uart_get_desc(mock_hostmod_get_ctx(), dem_uart_diaddr,
                                  &dem_uart_desc);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_uint_eq(dem_uart_desc.di_addr, dem_uart_diaddr);
}
END_TEST

START_TEST(test_get_desc_wrong_module)
{
    osd_result rv;

    mock_hostmod_expect_mod_describe(dem_uart_diaddr, OSD_MODULE_VENDOR_OSD,
                                     OSD_MODULE_TYPE_STD_STM, 0);

    struct osd_dem_uart_desc dem_uart_desc;
    rv = osd_cl_dem_uart_get_desc(mock_hostmod_get_ctx(), dem_uart_diaddr,
                                  &dem_uart_desc);
    ck_assert_int_eq(rv, OSD_ERROR_WRONG_MODULE);
}
END_TEST

static void dem_uart_handler(void *arg, const char *str, size_t len)
{
    ck_assert(arg);
    ck_assert(str && len > 0);

    char *exp_char = (char *)arg;

    ck_assert_uint_eq(1, len);
    ck_assert_uint_eq(exp_char[0], str[0]);
}

START_TEST(test_receive_event)
{
    osd_result rv;

    const char TEST_CHAR = 0x42;

    struct osd_dem_uart_event_handler ev_handler;
    ev_handler.cb_fn = dem_uart_handler;

    struct osd_packet *pkg;
    osd_packet_new(&pkg, osd_packet_sizeconv_payload2data(1));
    osd_packet_set_header(pkg, MOCK_HOSTMOD_DIADDR, dem_uart_diaddr,
                          OSD_PACKET_TYPE_EVENT, EV_LAST);

    pkg->data.payload[0] = TEST_CHAR;

    ev_handler.cb_arg = (void *)&TEST_CHAR;

    rv = osd_cl_dem_uart_receive_event((void *)&ev_handler, pkg);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

START_TEST(test_send_string)
{
    osd_result rv;

    const char *TEST_STR = "123Test";

    struct osd_dem_uart_desc desc;
    desc.di_addr = dem_uart_diaddr;

    struct osd_packet *exp_packet;

    for (int i = 0; i < strlen(TEST_STR); i++) {
        osd_packet_new(&exp_packet, osd_packet_sizeconv_payload2data(1));
        osd_packet_set_header(exp_packet, dem_uart_diaddr, MOCK_HOSTMOD_DIADDR,
                              OSD_PACKET_TYPE_EVENT, EV_LAST);
        exp_packet->data.payload[0] = TEST_STR[i] & 0xFF;

        mock_hostmod_expect_event_send(exp_packet, OSD_OK);
    }

    rv = osd_cl_dem_uart_send_string(mock_hostmod_get_ctx(), &desc,
                                     TEST_STR, strlen(TEST_STR));
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_util, *tc_event_rxtx;

    s = suite_create(TEST_SUITE_NAME);

    tc_util = tcase_create("Utility Functionality");
    tcase_add_checked_fixture(tc_util, setup, teardown);
    tcase_add_test(tc_util, test_get_desc);
    tcase_add_test(tc_util, test_get_desc_wrong_module);
    suite_add_tcase(s, tc_util);

    tc_event_rxtx = tcase_create("Core RX/TX Functionality");
    tcase_add_checked_fixture(tc_event_rxtx, setup, teardown);
    tcase_add_test(tc_event_rxtx, test_receive_event);
    tcase_add_test(tc_event_rxtx, test_send_string);
    suite_add_tcase(s, tc_event_rxtx);

    return s;
}
