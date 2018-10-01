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

#define TEST_SUITE_NAME "check_terminal"

#include <osd/cl_dem_uart.h>
#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/terminal.h>

#include "mock_host_controller.h"
#include "testutil.h"

const unsigned int target_subnet_addr = 0;
unsigned int mock_hostmod_diaddr;
unsigned int mock_dem_uart_diaddr;

// Context of the terminal we are testing.
struct osd_log_ctx *log_ctx;
struct osd_terminal_ctx *terminal_ctx;

void setup_hostmod()
{
    osd_result rv;

    log_ctx = testutil_get_log_ctx();

    rv = osd_terminal_new(&terminal_ctx, log_ctx, "inproc://testing",
                          mock_dem_uart_diaddr);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(terminal_ctx, NULL);

    // connect
    mock_host_controller_expect_diaddr_req(mock_hostmod_diaddr);

    rv = osd_terminal_connect(terminal_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    // describe
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr,
                                             mock_dem_uart_diaddr,
                                             OSD_MODULE_VENDOR_OSD,
                                             OSD_MODULE_TYPE_STD_DEM_UART, 0);

    // set event dest
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr,
                                          mock_dem_uart_diaddr,
                                          OSD_REG_BASE_MOD_EVENT_DEST,
                                          mock_hostmod_diaddr);

    // activate module
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr,
                                         mock_dem_uart_diaddr,
                                         OSD_REG_BASE_MOD_CS, 0);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr,
                                          mock_dem_uart_diaddr,
                                          OSD_REG_BASE_MOD_CS, 1);

    rv = osd_terminal_start(terminal_ctx);
    ck_assert_int_eq(rv, OSD_OK);
}

void teardown_hostmod()
{
    osd_result rv;

    // deactivate event sending
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr,
                                         mock_dem_uart_diaddr,
                                         OSD_REG_BASE_MOD_CS, 1);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr,
                                          mock_dem_uart_diaddr,
                                          OSD_REG_BASE_MOD_CS, 0);

    rv = osd_terminal_stop(terminal_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    rv = osd_terminal_disconnect(terminal_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    osd_terminal_free(&terminal_ctx);
    ck_assert_ptr_eq(terminal_ctx, NULL);

    osd_log_free(&log_ctx);
}

/**
 * Test fixture: setup (called before each test)
 */
void setup(void)
{
    mock_hostmod_diaddr = osd_diaddr_build(1, 1);
    mock_dem_uart_diaddr = osd_diaddr_build(target_subnet_addr, 15);

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

START_TEST(test_basic_init)
{
    setup();
    teardown();
}
END_TEST

START_TEST(test_read_from_target)
{
    osd_result rv;

    struct osd_packet *packet;
    osd_packet_new(&packet, osd_packet_sizeconv_payload2data(1));
    osd_packet_set_header(packet, mock_hostmod_diaddr, mock_dem_uart_diaddr,
                          OSD_PACKET_TYPE_EVENT, EV_LAST);

    // Check the entire value range of a byte
    for (int c = 0; c < 256; c++) {
        packet->data.payload[0] = c;
        rv = mock_host_controller_queue_data_packet(packet);
        ck_assert_int_eq(rv, OSD_OK);
    }

    mock_host_controller_wait_for_event_tx();
    osd_packet_free(&packet);

    int pts = open(osd_terminal_get_pts_path(terminal_ctx), O_RDONLY | O_NOCTTY);
    ck_assert(pts > 0);

    unsigned char c;
    for (int ret, exp = 0; exp < 256; exp++) {
        ret = read(pts, &c, 1);
        ck_assert(ret == 1);

        ck_assert_int_eq(c, exp);
    }

    close(pts);
}
END_TEST

START_TEST(test_write_to_target)
{
    osd_result rv;

    struct osd_packet *packet;
    osd_packet_new(&packet, osd_packet_sizeconv_payload2data(1));
    osd_packet_set_header(packet, mock_dem_uart_diaddr, mock_hostmod_diaddr,
                          OSD_PACKET_TYPE_EVENT, EV_LAST);

    unsigned char buf[256];
    for (int c = 0; c < 256; c++) {
        packet->data.payload[0] = c;
        mock_host_controller_expect_data_req(packet, NULL);

        buf[c] = c;
    }

    int pts = open(osd_terminal_get_pts_path(terminal_ctx), O_WRONLY | O_NOCTTY);
    ck_assert(pts > 0);

    for (int written = 0, ret; written < sizeof(buf); /*NOP*/) {
        ret = write(pts, buf + written, sizeof(buf) - written);
        ck_assert(ret > 0);

        written += ret;
    }

    mock_host_controller_wait_for_requests();

    close(pts);
    osd_packet_free(&packet);
}
END_TEST

START_TEST(test_file_status)
{
    struct stat pts_stat;

    const char* pts_path = osd_terminal_get_pts_path(terminal_ctx);
    ck_assert(pts_path);

    int rv = stat(pts_path, &pts_stat);
    ck_assert(rv == 0);
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_init, *tc_core;

    s = suite_create(TEST_SUITE_NAME);

    tc_init = tcase_create("Basic Initialization");
    tcase_add_test(tc_init, test_basic_init);
    suite_add_tcase(s, tc_init);

    tc_core = tcase_create("Core Functionality");
    tcase_add_checked_fixture(tc_core, setup, teardown);
    tcase_add_test(tc_core, test_file_status);
    tcase_add_test(tc_core, test_read_from_target);
    tcase_add_test(tc_core, test_write_to_target);
    suite_add_tcase(s, tc_core);

    return s;
}
