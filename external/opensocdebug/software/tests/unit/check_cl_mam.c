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

#define TEST_SUITE_NAME "check_cl_mam"

#include "mock_hostmod.h"
#include "testutil.h"

#include <osd/cl_mam.h>
#include <osd/osd.h>
#include <osd/reg.h>

struct osd_hostmod_ctx *hostmod_ctx;
struct osd_log_ctx *log_ctx;

// DI address of the MAM module to be tested; chosen arbitrarily
const unsigned int mam_diaddr = 7;

// XXX: keep this in sync with osd-private.h until it can be determined
// dynamically.
#define OSD_MAX_PKG_LEN_WORDS 8

/**
 * Test fixture: setup (called before each tests)
 */
void setup(void) { mock_hostmod_setup(); }

/**
 * Test fixture: setup (called after each test)
 */
void teardown(void) { mock_hostmod_teardown(); }

START_TEST(test_get_mem_desc)
{
    osd_result rv;

    mock_hostmod_expect_reg_read16(16, mam_diaddr, OSD_REG_MAM_AW, OSD_OK);
    mock_hostmod_expect_reg_read16(32, mam_diaddr, OSD_REG_MAM_DW, OSD_OK);
    mock_hostmod_expect_reg_read16(1, mam_diaddr, OSD_REG_MAM_REGIONS, OSD_OK);

    mock_hostmod_expect_reg_read16(0xcdef, mam_diaddr,
                                   OSD_REG_MAM_REGION_BASEADDR(0, 0), OSD_OK);
    mock_hostmod_expect_reg_read16(0x89ab, mam_diaddr,
                                   OSD_REG_MAM_REGION_BASEADDR(0, 1), OSD_OK);
    mock_hostmod_expect_reg_read16(0x4567, mam_diaddr,
                                   OSD_REG_MAM_REGION_BASEADDR(0, 2), OSD_OK);
    mock_hostmod_expect_reg_read16(0x0123, mam_diaddr,
                                   OSD_REG_MAM_REGION_BASEADDR(0, 3), OSD_OK);

    mock_hostmod_expect_reg_read16(0xdead, mam_diaddr,
                                   OSD_REG_MAM_REGION_MEMSIZE(0, 0), OSD_OK);
    mock_hostmod_expect_reg_read16(0x89ab, mam_diaddr,
                                   OSD_REG_MAM_REGION_MEMSIZE(0, 1), OSD_OK);
    mock_hostmod_expect_reg_read16(0x4567, mam_diaddr,
                                   OSD_REG_MAM_REGION_MEMSIZE(0, 2), OSD_OK);
    mock_hostmod_expect_reg_read16(0x0123, mam_diaddr,
                                   OSD_REG_MAM_REGION_MEMSIZE(0, 3), OSD_OK);

    struct osd_mem_desc mem_desc;
    rv = osd_cl_mam_get_mem_desc(mock_hostmod_get_ctx(), mam_diaddr, &mem_desc);
    ck_assert_int_eq(rv, OSD_OK);
    ck_assert_uint_eq(mem_desc.di_addr, mam_diaddr);
    ck_assert_uint_eq(mem_desc.addr_width_bit, 16);
    ck_assert_uint_eq(mem_desc.data_width_bit, 32);
    ck_assert_uint_eq(mem_desc.num_regions, 1);
    ck_assert_uint_eq(mem_desc.regions[0].baseaddr, 0x0123456789abcdefULL);
    ck_assert_uint_eq(mem_desc.regions[0].memsize, 0x0123456789abdeadULL);
}
END_TEST

START_TEST(test_get_mem_desc_err)
{
    osd_result rv;
    struct osd_mem_desc mem_desc;

    mock_hostmod_expect_reg_read16(16, mam_diaddr, OSD_REG_MAM_AW,
                                   OSD_ERROR_NOT_CONNECTED);
    rv = osd_cl_mam_get_mem_desc(mock_hostmod_get_ctx(), mam_diaddr, &mem_desc);
    ck_assert_int_eq(rv, OSD_ERROR_NOT_CONNECTED);
}
END_TEST

struct osd_mem_desc get_simple_mem_desc(void)
{
    struct osd_mem_desc mem_desc = { 0 };
    mem_desc.di_addr = mam_diaddr;
    mem_desc.addr_width_bit = 32;
    mem_desc.data_width_bit = 32;
    mem_desc.num_regions = 1;
    mem_desc.regions[0].baseaddr = 0;
    mem_desc.regions[0].memsize = 1024 * 1024 * 1024; // 1 GB

    return mem_desc;
}

static void expect_sync_packet(void)
{
    struct osd_packet *sync_pkg;
    osd_packet_new(&sync_pkg, osd_packet_sizeconv_payload2data(0));
    osd_packet_set_header(sync_pkg, MOCK_HOSTMOD_DIADDR, mam_diaddr,
                          OSD_PACKET_TYPE_EVENT, 0);
    mock_hostmod_expect_event_receive(sync_pkg, OSD_OK);
}

/**
 * Do an aligned single-word write
 */
START_TEST(test_write_single)
{
    osd_result rv;
    struct osd_mem_desc mem_desc = get_simple_mem_desc();

    uint8_t testdata[3] = { 0xde, 0xad, 0xbe };
    uint64_t addr = 0x1224;

    struct osd_packet *pkg;
    osd_packet_new(&pkg, 8);
    osd_packet_set_header(pkg, mam_diaddr, MOCK_HOSTMOD_DIADDR,
                          OSD_PACKET_TYPE_EVENT, 0);
    pkg->data.payload[0] = 0xA007;
    pkg->data.payload[1] = 0x0000;
    pkg->data.payload[2] = 0x1224;
    pkg->data.payload[3] = 0xdead;
    pkg->data.payload[4] = 0xbe00;

    mock_hostmod_expect_event_send(pkg, OSD_OK);
    expect_sync_packet();

    rv = osd_cl_mam_write(&mem_desc, mock_hostmod_get_ctx(), testdata,
                          sizeof(testdata), addr);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

/**
 * Do a single-word unaligned write
 */
START_TEST(test_write_single_unaligned)
{
    osd_result rv;
    struct osd_mem_desc mem_desc = get_simple_mem_desc();

    uint8_t testdata[3] = { 0xde, 0xad, 0xbe };
    uint64_t addr = 0x1225;

    struct osd_packet *pkg;
    osd_packet_new(&pkg, 8);
    osd_packet_set_header(pkg, mam_diaddr, MOCK_HOSTMOD_DIADDR,
                          OSD_PACKET_TYPE_EVENT, 0);
    pkg->data.payload[0] = 0xA00e;
    pkg->data.payload[1] = 0x0000;
    pkg->data.payload[2] = 0x1224;
    pkg->data.payload[3] = 0x00de;
    pkg->data.payload[4] = 0xadbe;

    mock_hostmod_expect_event_send(pkg, OSD_OK);
    expect_sync_packet();

    rv = osd_cl_mam_write(&mem_desc, mock_hostmod_get_ctx(), testdata,
                          sizeof(testdata), addr);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

/**
 * Test a burst write
 *
 * The burst write consists of 2052 bytes, which results in 3 transfers. Two
 * of these transfers are using the full burst size of 256 words, and the last
 * transfer submits 1 word.
 */
START_TEST(test_write_burst)
{
    osd_result rv;
    struct osd_mem_desc mem_desc = get_simple_mem_desc();

    uint8_t testdata[2052];
    for (int i = 0; i < 2052; i++) {
        testdata[i] = i % 0xFF;
    }
    uint64_t addr = 0x1224;

    mock_hostmod_expect_event_send_fromfile("check_cl_mam_write_burst.pkgdump");
    expect_sync_packet();

    rv = osd_cl_mam_write(&mem_desc, mock_hostmod_get_ctx(),
                          testdata, 2052, addr);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

/**
 * Test an unaligned burst write
 *
 * The unaligned burst results in three write transfers: a single-word write
 * as prolog, then a burst transfer, and finally a concluding single-word
 * transfer.
 */
START_TEST(test_write_burst_unaligned)
{
    osd_result rv;
    struct osd_mem_desc mem_desc = get_simple_mem_desc();

    uint8_t testdata[2052];
    for (int i = 0; i < 2052; i++) {
        testdata[i] = i % 0xFF;
    }
    uint64_t addr = 0x1225;

    mock_hostmod_expect_event_send_fromfile("check_cl_mam_write_burst_unaligned.pkgdump");
    expect_sync_packet();

    rv = osd_cl_mam_write(&mem_desc, mock_hostmod_get_ctx(),
                          testdata, 2052, addr);
    ck_assert_int_eq(rv, OSD_OK);
}
END_TEST

/**
 * Do an aligned single-word read
 */
START_TEST(test_read_single)
{
    osd_result rv;
    struct osd_mem_desc mem_desc = get_simple_mem_desc();

    uint8_t exp_testdata[3] = { 0xde, 0xad, 0xbe };
    uint8_t rcv_testdata[3] = { 0x00 };
    uint64_t addr = 0x1224;

    // request packet
    struct osd_packet *req_pkg;
    osd_packet_new(&req_pkg, 6);
    osd_packet_set_header(req_pkg, mam_diaddr, MOCK_HOSTMOD_DIADDR,
                          OSD_PACKET_TYPE_EVENT, 0);
    req_pkg->data.payload[0] = 0x0007;
    req_pkg->data.payload[1] = 0x0000;
    req_pkg->data.payload[2] = 0x1224;

    // response packet
    struct osd_packet *resp_pkg;
    osd_packet_new(&resp_pkg, 5);
    osd_packet_set_header(req_pkg, mam_diaddr, MOCK_HOSTMOD_DIADDR,
                          OSD_PACKET_TYPE_EVENT, 0);
    resp_pkg->data.payload[0] = 0xdead;
    resp_pkg->data.payload[1] = 0xbe00;

    mock_hostmod_expect_event_send(req_pkg, OSD_OK);
    mock_hostmod_expect_event_receive(resp_pkg, OSD_OK);

    rv = osd_cl_mam_read(&mem_desc, mock_hostmod_get_ctx(), rcv_testdata,
                         sizeof(rcv_testdata), addr);
    ck_assert_int_eq(rv, OSD_OK);

    for (size_t i = 0; i < sizeof(rcv_testdata); i++) {
        ck_assert_uint_eq(exp_testdata[i],  rcv_testdata[i]);
    }
}
END_TEST

/**
 * Do an aligned single-word read
 */
START_TEST(test_read_single_unaligned)
{
    osd_result rv;
    struct osd_mem_desc mem_desc = get_simple_mem_desc();

    uint8_t exp_testdata[3] = { 0xde, 0xad, 0xbe };
    uint8_t rcv_testdata[3] = { 0x00 };
    uint64_t addr = 0x1225;

    // request packet
    struct osd_packet *req_pkg;
    osd_packet_new(&req_pkg, 6);
    osd_packet_set_header(req_pkg, mam_diaddr, MOCK_HOSTMOD_DIADDR,
                          OSD_PACKET_TYPE_EVENT, 0);
    req_pkg->data.payload[0] = 0x000e;
    req_pkg->data.payload[1] = 0x0000;
    req_pkg->data.payload[2] = 0x1225;

    // response packet
    struct osd_packet *resp_pkg;
    osd_packet_new(&resp_pkg, 5);
    osd_packet_set_header(req_pkg, mam_diaddr, MOCK_HOSTMOD_DIADDR,
                          OSD_PACKET_TYPE_EVENT, 0);
    resp_pkg->data.payload[0] = 0x00de;
    resp_pkg->data.payload[1] = 0xadbe;

    mock_hostmod_expect_event_send(req_pkg, OSD_OK);
    mock_hostmod_expect_event_receive(resp_pkg, OSD_OK);

    rv = osd_cl_mam_read(&mem_desc, mock_hostmod_get_ctx(), rcv_testdata,
                         sizeof(rcv_testdata), addr);
    ck_assert_int_eq(rv, OSD_OK);

    for (size_t i = 0; i < sizeof(rcv_testdata); i++) {
        ck_assert_uint_eq(exp_testdata[i],  rcv_testdata[i]);
    }
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_util, *tc_read, *tc_write;

    s = suite_create(TEST_SUITE_NAME);

    tc_util = tcase_create("Utility Functionality");
    tcase_add_checked_fixture(tc_util, setup, teardown);
    tcase_add_test(tc_util, test_get_mem_desc);
    tcase_add_test(tc_util, test_get_mem_desc_err);
    suite_add_tcase(s, tc_util);

    tc_write = tcase_create("Memory writes");
    tcase_add_checked_fixture(tc_write, setup, teardown);
    tcase_add_test(tc_write, test_write_single);
    tcase_add_test(tc_write, test_write_single_unaligned);
    tcase_add_test(tc_write, test_write_burst);
    tcase_add_test(tc_write, test_write_burst_unaligned);
    suite_add_tcase(s, tc_write);

    tc_read = tcase_create("Memory reads");
    tcase_add_checked_fixture(tc_read, setup, teardown);
    tcase_add_test(tc_read, test_read_single);
    tcase_add_test(tc_read, test_read_single_unaligned);
    suite_add_tcase(s, tc_read);

    return s;
}
