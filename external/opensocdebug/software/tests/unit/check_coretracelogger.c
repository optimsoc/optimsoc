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

#define TEST_SUITE_NAME "check_coretracelogger"

#include "testutil.h"

#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/coretracelogger.h>

#include "mock_host_controller.h"

struct osd_coretracelogger_ctx *coretracelogger_ctx;
struct osd_log_ctx* log_ctx;

const unsigned int target_subnet_addr = 0;
unsigned int mock_hostmod_diaddr;
unsigned int mock_ctm_diaddr;

/**
 * Setup everything related to osd_hostmod
 */
void setup_hostmod(void)
{
    osd_result rv;

    log_ctx = testutil_get_log_ctx();

    // initialize module context
    rv = osd_coretracelogger_new(&coretracelogger_ctx, log_ctx,
                                "inproc://testing", mock_ctm_diaddr);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_ptr_ne(coretracelogger_ctx, NULL);

    // connect
    mock_host_controller_expect_diaddr_req(mock_hostmod_diaddr);

    rv = osd_coretracelogger_connect(coretracelogger_ctx);
    ck_assert_int_eq(rv, OSD_OK);
}

void teardown_hostmod(void)
{
    osd_result rv;
    rv = osd_coretracelogger_disconnect(coretracelogger_ctx);
    ck_assert_int_eq(rv, OSD_OK);

    osd_coretracelogger_free(&coretracelogger_ctx);
    ck_assert_ptr_eq(coretracelogger_ctx, NULL);
}

/**
 * Test fixture: setup (called before each tests)
 */
void setup(void)
{
    mock_hostmod_diaddr = osd_diaddr_build(1, 1);
    mock_ctm_diaddr = osd_diaddr_build(target_subnet_addr, 5);

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
                                             mock_ctm_diaddr,
                                             OSD_MODULE_VENDOR_OSD,
                                             OSD_MODULE_TYPE_STD_CTM, 0);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr,
                                         mock_ctm_diaddr,
                                         OSD_REG_CTM_ADDR_WIDTH, 32);
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr,
                                         mock_ctm_diaddr,
                                         OSD_REG_CTM_DATA_WIDTH, 32);

    // set event dest
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, mock_ctm_diaddr,
                                          OSD_REG_BASE_MOD_EVENT_DEST,
                                          mock_hostmod_diaddr);

    // activate event sending
    old_reg_val = 0;
    new_exp_reg_val = 1;
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mock_ctm_diaddr,
                                         OSD_REG_BASE_MOD_CS, old_reg_val);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, mock_ctm_diaddr,
                                          OSD_REG_BASE_MOD_CS,
                                          new_exp_reg_val);

    rv = osd_coretracelogger_start(coretracelogger_ctx);
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
    mock_host_controller_expect_reg_read(mock_hostmod_diaddr, mock_ctm_diaddr,
                                         OSD_REG_BASE_MOD_CS, old_reg_val);
    mock_host_controller_expect_reg_write(mock_hostmod_diaddr, mock_ctm_diaddr,
                                          OSD_REG_BASE_MOD_CS,
                                          new_exp_reg_val);

    rv = osd_coretracelogger_stop(coretracelogger_ctx);
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

    // log file
    char log_filename[] = "/tmp/osd-coretrace-log-XXXXXX";
    int fd_log = mkstemp(log_filename);
    ck_assert_int_ne(fd_log, -1);
    FILE* fp_log = fdopen(fd_log, "w");
    ck_assert_ptr_ne(fp_log, NULL);

    rv = osd_coretracelogger_set_log(coretracelogger_ctx, fp_log);
    ck_assert_int_eq(rv, OSD_OK);

    printf("log_filename: %s\n", log_filename);

    // start listening to CTM events
    logger_start();

    // Queue a couple events to be sent by the CTM
    struct osd_packet *pkg;
    rv = osd_packet_new(&pkg, osd_packet_sizeconv_payload2data(7));
    ck_assert_int_eq(rv, OSD_OK);
    rv = osd_packet_set_header(pkg, 1, 2, OSD_PACKET_TYPE_EVENT, 0);
    ck_assert_int_eq(rv, OSD_OK);
    pkg->data.payload[0] = 0xdead; // timestamp (LSB)
    pkg->data.payload[1] = 0xbeef; // timestamp (MSB)
    pkg->data.payload[2] = 0xad00; // npc (LSB)
    pkg->data.payload[3] = 0xdeaf; // npc (MSB)
    pkg->data.payload[4] = 0x0100; // pc (LSB)
    pkg->data.payload[5] = 0x4567; // pc (MSB)
    pkg->data.payload[6] = 0x9; // mode=01, ret=0, call=1, modechange=0
    mock_host_controller_queue_data_packet(pkg);

    pkg->data.payload[0] = 0xdead; // timestamp (LSB)
    pkg->data.payload[1] = 0xdead; // timestamp (MSB)
    pkg->data.payload[2] = 0x1200; // npc (LSB)
    pkg->data.payload[3] = 0xdeaf; // npc (MSB)
    pkg->data.payload[4] = 0x0100; // pc (LSB)
    pkg->data.payload[5] = 0x4567; // pc (MSB)
    pkg->data.payload[6] = 0x05; // mode=01, ret=1, call=0, modechange=0
    mock_host_controller_queue_data_packet(pkg);

    pkg->data.payload[0] = 0xdead; // timestamp (LSB)
    pkg->data.payload[1] = 0xaddf; // timestamp (MSB)
    pkg->data.payload[2] = 0xaf00; // npc (LSB)
    pkg->data.payload[3] = 0xafaf; // npc (MSB)
    pkg->data.payload[4] = 0x0100; // pc (LSB)
    pkg->data.payload[5] = 0x4567; // pc (MSB)
    pkg->data.payload[6] = 0x10; // mode=00, ret=0, call=0, modechange=1
    mock_host_controller_queue_data_packet(pkg);

    osd_packet_free(&pkg);

    // wait until all events are consumed
    mock_host_controller_wait_for_event_tx();

    // tell CTM to stop sending events
    logger_stop();

    // now check if the written files match our expectations
    fclose(fp_log);

    assert_files_eq("check_coretracelogger_record_trace.txt",
                    log_filename);

    irv = unlink(log_filename);
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
