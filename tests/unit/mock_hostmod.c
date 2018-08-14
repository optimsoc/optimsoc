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

#include "mock_hostmod.h"

#include <check.h>
#include <czmq.h>
#include <osd/osd.h>
#include <osd/reg.h>

#define MOCK_HOSTMOD_FLAGS_NOCHECK -1

// Development aid: dump all packet passed to the osd_hostmod_event_send()
// function into a file to be used as reference file later on.
// Copy the resulting file DUMP_EVENT_SEND_FILE into the test directory as
// golden reference after you have validated its contents.
//#define DUMP_EVENT_SEND
#define DUMP_EVENT_SEND_FILE "/tmp/pkgdump"

zlist_t *mock_exp_read_list;
zlist_t *mock_exp_write_list;
zlist_t *mock_exp_event_tx_list;
zlist_t *mock_exp_event_rx_list;
FILE *mock_exp_event_tx_fd;

struct mock_osd_hostmod_ctx *mock_hostmod_ctx;

struct mock_osd_hostmod_ctx {
    bool is_connected;
    struct osd_log_ctx *log_ctx;
    uint16_t diaddr;
    struct worker_ctx *ioworker_ctx;
};

#ifdef DUMP_EVENT_SEND
FILE* event_send_dump_fd;
#endif

void mock_hostmod_setup(void)
{
    mock_exp_read_list = zlist_new();
    mock_exp_write_list = zlist_new();
    mock_exp_event_tx_list = zlist_new();
    mock_exp_event_rx_list = zlist_new();
    mock_exp_event_tx_fd = NULL;

    mock_hostmod_ctx = calloc(1, sizeof(struct mock_osd_hostmod_ctx));
    mock_hostmod_ctx->is_connected = true;

#ifdef DUMP_EVENT_SEND
    event_send_dump_fd = fopen(DUMP_EVENT_SEND_FILE, "w");
#endif
}

void mock_hostmod_teardown(void)
{
    free(mock_hostmod_ctx);

    ck_assert_uint_eq(zlist_size(mock_exp_read_list), 0);
    ck_assert_uint_eq(zlist_size(mock_exp_write_list), 0);
    ck_assert_uint_eq(zlist_size(mock_exp_event_rx_list), 0);

    zlist_destroy(&mock_exp_read_list);
    zlist_destroy(&mock_exp_write_list);
    zlist_destroy(&mock_exp_event_rx_list);

#ifndef DUMP_EVENT_SEND
    ck_assert_uint_eq(zlist_size(mock_exp_event_tx_list), 0);
    zlist_destroy(&mock_exp_event_tx_list);

    if (mock_exp_event_tx_fd) {
        uint8_t byte;
        ck_assert_msg(0 == fread(&byte, 1, 1, mock_exp_event_tx_fd)
                      && feof(mock_exp_event_tx_fd),
                      "Not all packets have been read from the event dump "
                      "file.");
        fclose(mock_exp_event_tx_fd);
    }
#else
    fclose(event_send_dump_fd);
#endif
}

void mock_hostmod_expect_reg_read_raw(struct mock_hostmod_regaccess *exp)
{
    int rv = zlist_append(mock_exp_read_list, exp);
    ck_assert_int_eq(rv, 0);
}

void mock_hostmod_expect_reg_write_raw(struct mock_hostmod_regaccess *exp)
{
    int rv = zlist_append(mock_exp_write_list, exp);
    ck_assert_int_eq(rv, 0);
}

void mock_hostmod_expect_reg_read16(uint16_t reg_val, uint16_t diaddr,
                                    uint16_t reg_addr, osd_result retval)
{
    struct mock_hostmod_regaccess *exp;

    exp = calloc(1, sizeof(struct mock_hostmod_regaccess));
    ck_assert(exp);

    exp->flags = MOCK_HOSTMOD_FLAGS_NOCHECK;
    exp->reg_size_bit = 16;

    exp->diaddr = diaddr;
    exp->reg_addr = reg_addr;
    exp->reg_val = (uint64_t)reg_val;
    exp->retval = retval;
    mock_hostmod_expect_reg_read_raw(exp);
}

void mock_hostmod_expect_reg_read32(uint32_t reg_val, uint16_t diaddr,
                                    uint16_t reg_addr, osd_result retval)
{
    struct mock_hostmod_regaccess *exp;

    exp = calloc(1, sizeof(struct mock_hostmod_regaccess));
    ck_assert(exp);

    exp->flags = MOCK_HOSTMOD_FLAGS_NOCHECK;
    exp->reg_size_bit = 32;

    exp->diaddr = diaddr;
    exp->reg_addr = reg_addr;
    exp->reg_val = (uint64_t)reg_val;
    exp->retval = retval;
    mock_hostmod_expect_reg_read_raw(exp);
}

void mock_hostmod_expect_reg_read64(uint64_t reg_val, uint16_t diaddr,
                                    uint16_t reg_addr, osd_result retval)
{
    struct mock_hostmod_regaccess *exp;

    exp = calloc(1, sizeof(struct mock_hostmod_regaccess));
    ck_assert(exp);

    exp->flags = MOCK_HOSTMOD_FLAGS_NOCHECK;
    exp->reg_size_bit = 64;

    exp->diaddr = diaddr;
    exp->reg_addr = reg_addr;
    exp->reg_val = (uint64_t)reg_val;
    exp->retval = retval;
    mock_hostmod_expect_reg_read_raw(exp);
}

void mock_hostmod_expect_mod_describe(uint16_t diaddr, uint16_t vendor,
                                      uint16_t type, uint16_t version)
{
    mock_hostmod_expect_reg_read16(vendor, diaddr, OSD_REG_BASE_MOD_VENDOR,
                                   OSD_OK);
    mock_hostmod_expect_reg_read16(type, diaddr, OSD_REG_BASE_MOD_TYPE,
                                   OSD_OK);
    mock_hostmod_expect_reg_read16(version, diaddr, OSD_REG_BASE_MOD_VERSION,
                                   OSD_OK);
}

void mock_hostmod_expect_reg_write16(uint16_t reg_val, uint16_t diaddr,
                                     uint16_t reg_addr, osd_result retval)
{
    struct mock_hostmod_regaccess *exp;

    exp = calloc(1, sizeof(struct mock_hostmod_regaccess));
    ck_assert(exp);

    exp->flags = MOCK_HOSTMOD_FLAGS_NOCHECK;
    exp->reg_size_bit = 16;

    exp->diaddr = diaddr;
    exp->reg_addr = reg_addr;
    exp->reg_val = (uint64_t)reg_val;
    exp->retval = retval;
    mock_hostmod_expect_reg_write_raw(exp);
}

void mock_hostmod_expect_reg_write32(uint32_t reg_val, uint16_t diaddr,
                                     uint16_t reg_addr, osd_result retval)
{
    struct mock_hostmod_regaccess *exp;

    exp = calloc(1, sizeof(struct mock_hostmod_regaccess));
    ck_assert(exp);

    exp->flags = MOCK_HOSTMOD_FLAGS_NOCHECK;
    exp->reg_size_bit = 32;

    exp->diaddr = diaddr;
    exp->reg_addr = reg_addr;
    exp->reg_val = (uint64_t)reg_val;
    exp->retval = retval;
    mock_hostmod_expect_reg_write_raw(exp);
}

void mock_hostmod_expect_reg_write64(uint64_t reg_val, uint16_t diaddr,
                                     uint16_t reg_addr, osd_result retval)
{
    struct mock_hostmod_regaccess *exp;

    exp = calloc(1, sizeof(struct mock_hostmod_regaccess));
    ck_assert(exp);

    exp->flags = MOCK_HOSTMOD_FLAGS_NOCHECK;
    exp->reg_size_bit = 64;

    exp->diaddr = diaddr;
    exp->reg_addr = reg_addr;
    exp->reg_val = (uint64_t)reg_val;
    exp->retval = retval;
    mock_hostmod_expect_reg_write_raw(exp);
}

void mock_hostmod_expect_event_send(struct osd_packet *event_pkg,
                                    osd_result retval)
{
    struct mock_hostmod_event *exp;

    exp = calloc(1, sizeof(struct mock_hostmod_event));
    ck_assert(exp);

    exp->pkg = event_pkg;
    exp->retval = retval;
    int rv = zlist_append(mock_exp_event_tx_list, exp);
    ck_assert_int_eq(rv, 0);
}

void mock_hostmod_expect_event_send_fromfile(const char* path)
{
    mock_exp_event_tx_fd = fopen(path, "r");
    ck_assert_msg(mock_exp_event_tx_fd,
                  "Unable to open packet dump file %s.", path);
}

void mock_hostmod_expect_event_receive(struct osd_packet *event_pkg,
                                       osd_result retval)
{
    struct mock_hostmod_event *exp;

    exp = calloc(1, sizeof(struct mock_hostmod_event));
    ck_assert(exp);

    exp->pkg = event_pkg;
    exp->retval = retval;
    int rv = zlist_append(mock_exp_event_rx_list, exp);
    ck_assert_int_eq(rv, 0);
}

struct osd_hostmod_ctx *mock_hostmod_get_ctx()
{
    return (struct osd_hostmod_ctx *)mock_hostmod_ctx;
}

osd_result osd_hostmod_reg_read(struct osd_hostmod_ctx *ctx, void *reg_val,
                                uint16_t diaddr, uint16_t reg_addr,
                                int reg_size_bit, int flags)
{
    struct mock_hostmod_regaccess *exp;
    exp = zlist_pop(mock_exp_read_list);
    ck_assert(exp);

    // check parameters
    ck_assert_int_eq(exp->diaddr, diaddr);
    ck_assert_int_eq(exp->reg_addr, reg_addr);
    ck_assert_int_eq(exp->reg_size_bit, reg_size_bit);
    if (exp->flags != MOCK_HOSTMOD_FLAGS_NOCHECK) {
        ck_assert_int_eq(exp->flags, flags);
    }

    // prepare return data
    memcpy(reg_val, &exp->reg_val, reg_size_bit / 8);
    osd_result retval = exp->retval;
    free(exp);

    return retval;
}

osd_result osd_hostmod_reg_write(struct osd_hostmod_ctx *ctx,
                                 const void *reg_val, uint16_t diaddr,
                                 uint16_t reg_addr, int reg_size_bit,
                                 int flags)
{
    struct mock_hostmod_regaccess *exp;
    exp = zlist_pop(mock_exp_write_list);
    ck_assert(exp);

    // check parameters
    ck_assert_int_eq(exp->diaddr, diaddr);
    ck_assert_int_eq(exp->reg_addr, reg_addr);
    ck_assert_int_eq(exp->reg_size_bit, reg_size_bit);
    if (exp->flags != MOCK_HOSTMOD_FLAGS_NOCHECK) {
        ck_assert_int_eq(exp->flags, flags);
    }

    // XXX: Extend for 128-bit wide register size.
    assert(reg_size_bit == 16 || reg_size_bit == 32 || reg_size_bit == 64);  
    
    if (reg_size_bit == 16) {
        uint64_t written_reg_val = *(uint16_t*)reg_val;
        ck_assert_uint_eq(written_reg_val, exp->reg_val);
    } else if (reg_size_bit == 32) {
        uint64_t written_reg_val = *(uint32_t*)reg_val;
        ck_assert_uint_eq(written_reg_val, exp->reg_val);
    } else {
        uint64_t written_reg_val = *(uint64_t*)reg_val;
        ck_assert_uint_eq(written_reg_val, exp->reg_val);
    }

    osd_result retval = exp->retval;
    free(exp);

    return retval;
}

uint16_t osd_hostmod_get_diaddr(struct osd_hostmod_ctx *ctx)
{
    return MOCK_HOSTMOD_DIADDR;
}

osd_result osd_hostmod_event_send(struct osd_hostmod_ctx *ctx,
                                  const struct osd_packet* event_pkg)
{
#ifdef DUMP_EVENT_SEND
    osd_packet_dump(event_pkg, stdout);
    fflush(stdout);

    ck_assert(osd_packet_fwrite(event_pkg, event_send_dump_fd));
    fflush(event_send_dump_fd);
    return OSD_OK;
#endif

    struct osd_packet *exp_event_pkg;
    osd_result exp_retval;

    if (mock_exp_event_tx_fd) {
        exp_event_pkg = osd_packet_fread(mock_exp_event_tx_fd);
        ck_assert_msg(exp_event_pkg, "Test called osd_hostmod_event_send() but "
                      "no expected event could be read from the packet dump "
                      "file.");
        exp_retval = OSD_OK;

        osd_packet_dump(exp_event_pkg, stdout);
        fflush(stdout);
    } else {
        struct mock_hostmod_event *exp;
        exp = zlist_pop(mock_exp_event_tx_list);
        ck_assert_msg(exp, "Test called osd_hostmod_event_send() but no "
                      "expected event was queued.");

        exp_event_pkg = exp->pkg;
        exp_retval = exp->retval;
        ck_assert(exp_event_pkg);
        free(exp);
    }

    bool is_equal = osd_packet_equal(exp_event_pkg, event_pkg);
    if (!is_equal) {
        printf("Sent packet:\n");
        osd_packet_dump(event_pkg, stdout);

        printf("Expected packet:\n");
        osd_packet_dump(exp_event_pkg, stdout);
        fflush(stdout);
    }
    ck_assert(is_equal);

    free(exp_event_pkg);

    return exp_retval;
}

osd_result osd_hostmod_event_receive(struct osd_hostmod_ctx *ctx,
                                     struct osd_packet **event_pkg,
                                     int flags)
{
    struct mock_hostmod_event *exp;
    exp = zlist_pop(mock_exp_event_rx_list);
    ck_assert_msg(exp, "Test called osd_hostmod_event_receive() but no event "
                  "was queued.");

    struct osd_packet *exp_event_pkg = exp->pkg;
    ck_assert(exp_event_pkg);

    *event_pkg = exp_event_pkg;
    osd_result exp_retval = exp->retval;
    free(exp);

    return exp_retval;
}
