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

#define TEST_SUITE_NAME "check_systracelogger"

#include "testutil.h"

#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/systracelogger.h>

#include "mock_host_controller.h"

struct osd_systracelogger_ctx *systracelogger_ctx;
struct osd_log_ctx* log_ctx;

const unsigned int target_subnet_addr = 0;
unsigned int mock_hostmod_diaddr;
unsigned int mock_stm_diaddr;

/**
 * Setup everything related to osd_hostmod
 */
void setup_hostmod(void)
{
    osd_result rv;

    log_ctx = testutil_get_log_ctx();

    // initialize module context
    rv = osd_systracelogger_new(&systracelogger_ctx, log_ctx,
                                "inproc://testing", mock_stm_diaddr);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(systracelogger_ctx, NULL);

    // connect
    mock_host_controller_expect_diaddr_req(mock_hostmod_diaddr);

    rv = osd_systracelogger_connect(systracelogger_ctx);
    ck_assert_int_eq(rv, OSD_OK);
}

void teardown_hostmod(void)
{
    osd_result rv;
    rv = osd_systracelogger_disconnect(systracelogger_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    osd_systracelogger_free(&systracelogger_ctx);
    ck_assert_ptr_eq(systracelogger_ctx, NULL);
}

/**
 * Test fixture: setup (called before each tests)
 */
void setup(void)
{
    mock_hostmod_diaddr = osd_diaddr_build(1, 1);
    mock_stm_diaddr = osd_diaddr_build(target_subnet_addr, 5);

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

static void logger_start(void)
{
    osd_result rv;
    uint16_t old_reg_val, new_exp_reg_val;

    // describe
    mock_host_controller_expect_mod_describe(mock_hostmod_diaddr,
                                             mock_stm_diaddr,
                                             OSD_MODULE_VENDOR_OSD,
                                             OSD_MODULE_TYPE_STD_STM, 0);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr,
                                         mock_stm_diaddr,
                                         OSD_REG_STM_VALWIDTH, 32);

    // set event dest
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, mock_stm_diaddr,
                                          OSD_REG_BASE_MOD_EVENT_DEST,
                                          mock_hostmod_diaddr);

    // activate event sending
    old_reg_val = 0;
    new_exp_reg_val = 1;
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mock_stm_diaddr,
                                         OSD_REG_BASE_MOD_CS, old_reg_val);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, mock_stm_diaddr,
                                          OSD_REG_BASE_MOD_CS,
                                          new_exp_reg_val);

    rv = osd_systracelogger_start(systracelogger_ctx);
    ck_assert_int_eq(rv, OSD_OK);
}

START_TEST(test_core_start)
{
    logger_start();
}
END_TEST

static void logger_stop(void)
{
    osd_result rv;
    uint16_t old_reg_val, new_exp_reg_val;

    // deactivate event sending
    old_reg_val = 1;
    new_exp_reg_val = 0;
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mock_stm_diaddr,
                                         OSD_REG_BASE_MOD_CS, old_reg_val);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, mock_stm_diaddr,
                                          OSD_REG_BASE_MOD_CS,
                                          new_exp_reg_val);

    rv = osd_systracelogger_stop(systracelogger_ctx);
    ck_assert_int_eq(rv, OSD_OK);
}

START_TEST(test_core_stop)
{
    logger_stop();
}
END_TEST

static void assert_files_eq(const char *file1, const char *file2)
{
    FILE* fp1 = fopen(file1, "r");
    ck_assert_ptr_ne(fp1, NULL);
    FILE* fp2 = fopen(file2, "r");
    ck_assert_ptr_ne(fp2, NULL);

    int ch1 = getc(fp1);
    int ch2 = getc(fp2);

    unsigned int i = 0;
    while ((ch1 != EOF) && (ch2 != EOF) && (ch1 == ch2)) {
        ch1 = getc(fp1);
        ch2 = getc(fp2);
        i++;
    }

    ck_assert_msg(ch1 == ch2, "Files differ at character %d: %c != %c",
                  i, ch1, ch2);

    fclose(fp1);
    fclose(fp2);
}

START_TEST(test_core_record_trace)
{
    osd_result rv;
    int irv;

    // setup systracelogger log files
    // sysprint log file
    char sysprint_filename[] = "/tmp/osd-sysprint-log-XXXXXX";
    int fd_sysprint = mkstemp(sysprint_filename);
    ck_assert_int_ne(fd_sysprint, -1);
    FILE* fp_sysprint = fdopen(fd_sysprint, "w");
    ck_assert_ptr_ne(fp_sysprint, NULL);

    rv = osd_systracelogger_set_sysprint_log(systracelogger_ctx, fp_sysprint);
    ck_assert_int_eq(rv, OSD_OK);

    // event log file
    char event_filename[] = "/tmp/osd-event-log-XXXXXX";
    int fd_event = mkstemp(event_filename);
    ck_assert_int_ne(fd_event, -1);
    FILE * fp_event = fdopen(fd_event, "w");
    ck_assert_ptr_ne(fp_event, NULL);

    rv = osd_systracelogger_set_event_log(systracelogger_ctx, fp_event);
    ck_assert_int_eq(rv, OSD_OK);

    printf("sysprint_filename: %s, event_filename: %s\n",
           sysprint_filename, event_filename);

    // start listening to STM events
    logger_start();

    // Queue a couple events to be sent by the STM
    struct osd_packet *pkg;
    rv = osd_packet_new(&pkg, osd_packet_sizeconv_payload2data(5));
    ck_assert_int_eq(rv, OSD_OK);
    rv = osd_packet_set_header(pkg, 1, 2, OSD_PACKET_TYPE_EVENT, 0);
    ck_assert_int_eq(rv, OSD_OK);
    pkg->data.payload[0] = 0xdead; // timestamp (LSB)
    pkg->data.payload[1] = 0xbeef; // timestamp (MSB)
    pkg->data.payload[2] = 4; // id; 4 == sysprint
    pkg->data.payload[3] = 0; // value (LSB)
    pkg->data.payload[4] = 0; // value (LSB)

    pkg->data.payload[2] = 4;
    pkg->data.payload[3] = 'H';
    mock_host_controller_queue_data_packet(pkg);

    pkg->data.payload[2] = 4;
    pkg->data.payload[3] = 'e';
    mock_host_controller_queue_data_packet(pkg);

    pkg->data.payload[2] = 4;
    pkg->data.payload[3] = 'l';
    mock_host_controller_queue_data_packet(pkg);

    pkg->data.payload[2] = 8;
    pkg->data.payload[3] = 0xdead;
    mock_host_controller_queue_data_packet(pkg);

    pkg->data.payload[2] = 4;
    pkg->data.payload[3] = 'l';
    mock_host_controller_queue_data_packet(pkg);

    pkg->data.payload[2] = 4;
    pkg->data.payload[3] = 'o';
    mock_host_controller_queue_data_packet(pkg);

    pkg->data.payload[2] = 4;
    pkg->data.payload[3] = '\n';
    mock_host_controller_queue_data_packet(pkg);

    osd_packet_free(&pkg);

    // wait until all events are consumed
    mock_host_controller_wait_for_event_tx();

    // tell STM to stop sending events
    logger_stop();

    // now check if the written files match our expectations
    fclose(fp_event);
    fclose(fp_sysprint);

    assert_files_eq("check_systracelogger_record_trace.sysprint.txt",
                    sysprint_filename);
    assert_files_eq("check_systracelogger_record_trace.events.txt",
                    event_filename);

    irv = unlink(sysprint_filename);
    ck_assert_int_eq(irv, 0);
    irv = unlink(event_filename);
    ck_assert_int_eq(irv, 0);
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
    tcase_add_test(tc_core, test_core_start);
    tcase_add_test(tc_core, test_core_stop);
    tcase_add_test(tc_core, test_core_record_trace);
    suite_add_tcase(s, tc_core);

    return s;
}
