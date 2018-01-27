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

#define TEST_SUITE_NAME "check_packet"

#include "testutil.h"

#include <osd/osd.h>
#include <osd/packet.h>

START_TEST(test_packet_header_extractparts)
{
    osd_result rv;
    struct osd_packet *pkg;
    rv = osd_packet_new(&pkg, osd_packet_sizeconv_payload2data(0));
    ck_assert_int_eq(rv, OSD_OK);

    pkg->data.dest = 0xa5ab;
    pkg->data.src = 0x1234;
    pkg->data.flags = 0x5557;
    ck_assert_int_eq(osd_packet_get_dest(pkg), 0xa5ab);
    ck_assert_int_eq(osd_packet_get_src(pkg), 0x1234);
    ck_assert_int_eq(osd_packet_get_type(pkg), 0x1);
    ck_assert_int_eq(osd_packet_get_type_sub(pkg), 0x5);

    osd_packet_free(&pkg);
    ck_assert_ptr_eq(pkg, NULL);
}
END_TEST

START_TEST(test_packet_header_set)
{
    osd_result rv;
    struct osd_packet *pkg;
    rv = osd_packet_new(&pkg, osd_packet_sizeconv_payload2data(0));
    ck_assert_int_eq(rv, OSD_OK);

    osd_packet_set_header(pkg, 0x1ab, 0x157, OSD_PACKET_TYPE_EVENT, 0x5);

    ck_assert_int_eq(pkg->data.dest, 0x1ab);
    ck_assert_int_eq(pkg->data.src, 0x157);
    ck_assert_int_eq(pkg->data.flags, 0x9400);

    osd_packet_free(&pkg);
    ck_assert_ptr_eq(pkg, NULL);
}
END_TEST

START_TEST(test_packet_equal)
{
    osd_result rv;
    struct osd_packet *pkg;
    rv = osd_packet_new(&pkg, osd_packet_sizeconv_payload2data(2));
    ck_assert_int_eq(rv, OSD_OK);

    osd_packet_set_header(pkg, 0x1ab, 0x157, OSD_PACKET_TYPE_EVENT, 0x5);
    pkg->data.payload[0] = 0xdead;
    pkg->data.payload[1] = 0xbeef;

    // NULL
    ck_assert(!osd_packet_equal(pkg, NULL));

    // identical pointers
    ck_assert(osd_packet_equal(pkg, pkg));

    // same contents
    struct osd_packet *pkg2 = malloc(osd_packet_sizeof(pkg));
    memcpy(pkg2, pkg, osd_packet_sizeof(pkg));
    ck_assert(osd_packet_equal(pkg, pkg2));

    // different contents
    pkg2->data.payload[0] = 0xabcd;
    ck_assert(!osd_packet_equal(pkg, pkg2));

    osd_packet_free(&pkg);
    ck_assert_ptr_eq(pkg, NULL);
    osd_packet_free(&pkg2);
    ck_assert_ptr_eq(pkg2, NULL);
}
END_TEST

START_TEST(test_packet_tostring)
{
    osd_result rv;
    struct osd_packet *pkg;
    rv = osd_packet_new(&pkg, osd_packet_sizeconv_payload2data(2));
    ck_assert_int_eq(rv, OSD_OK);

    osd_packet_set_header(pkg, 0x1ab, 0x157, OSD_PACKET_TYPE_EVENT, 0x5);
    pkg->data.payload[0] = 0xdead;
    pkg->data.payload[1] = 0xbeef;

    char *exp_str = "Packet of 5 data words:\n"
        "DEST = 427, SRC = 343, TYPE = 2 (OSD_PACKET_TYPE_EVENT), TYPE_SUB = 5\n"
        "Packet data (including header):\n"
        "  0x01ab\n"
        "  0x0157\n"
        "  0x9400\n"
        "  0xdead\n"
        "  0xbeef\n";

    char *str = NULL;
    osd_packet_to_string(pkg, &str);

    ck_assert_msg(strcmp(str, exp_str) == 0,
                  "Got string:\n%s\nExpected string:\n%s", str, exp_str);

    free(str);
    free(pkg);
}
END_TEST

START_TEST(test_packet_sizeconv)
{
    unsigned int hdr_words = 3;
    ck_assert_int_eq(osd_packet_sizeconv_payload2data(2), hdr_words + 2);
    ck_assert_int_eq(osd_packet_sizeconv_data2payload(7), 7 - hdr_words);
}
END_TEST

Suite *suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create(TEST_SUITE_NAME);

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_packet_sizeconv);
    tcase_add_test(tc_core, test_packet_header_set);
    tcase_add_test(tc_core, test_packet_header_extractparts);
    tcase_add_test(tc_core, test_packet_equal);
    tcase_add_test(tc_core, test_packet_tostring);
    suite_add_tcase(s, tc_core);

    return s;
}
