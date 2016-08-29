/* Copyright (c) 2012-2016 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ============================================================================
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#ifndef _OPENSOCDEBUG_H_
#define _OPENSOCDEBUG_H_

#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque context object
 *
 * This object contains all state information. Create and initialize a new
 * object with opensocdebug_new() and delete it with opensocdebug_free().
 */
struct osd_context;

struct osd_mode_option {
    char* name;
    char* value;
};

enum osd_mode {
    OSD_MODE_STANDALONE,
    OSD_MODE_DAEMON
};

int osd_new(struct osd_context **ctx, enum osd_mode standalone,
            size_t num_mode_options, struct osd_mode_option *options);

int osd_connect(struct osd_context *ctx);

enum osd_module_types {
    OSD_MOD_HOST = 0,
    OSD_MOD_SCM = 1,
    OSD_MOD_DEM_UART = 2,
    OSD_MOD_MAM = 3,
    OSD_MOD_STM = 4,
    OSD_MOD_CTM = 5
};

struct osd_memory_descriptor {
    uint16_t data_width;
    uint16_t addr_width;
    uint8_t num_regions;
    struct {
        uint64_t base_addr;
        uint64_t size;
    } regions[];
};

struct osd_stm_descriptor {
    uint16_t xlen;
};

struct osd_ctm_descriptor {
  uint16_t addr_width;
  uint16_t data_width;
};

static const int OSD_SUCCESS = 0;
static const int OSD_E_GENERIC = 1;
static const int OSD_E_CANNOTENUMERATE = 2;

void osd_print_packet(uint16_t *packet);

#define OSD_PRINT_PACKET_LOCATION(ctx, packet)      \
        if (ctx->debug_packets) {                   \
            printf("%s:%d:\n", __FILE__, __LINE__); \
            osd_print_packet(packet);               \
        }

int osd_reset_system(struct osd_context *ctx, int halt_cores);
int osd_start_cores(struct osd_context *ctx);

int osd_send_packet(struct osd_context *ctx, uint16_t *packet);

int osd_reg_access(struct osd_context *ctx, uint16_t* packet);
int osd_reg_read16(struct osd_context *ctx, uint16_t mod,
                   uint16_t addr, uint16_t *value);
int osd_reg_write16(struct osd_context *ctx, uint16_t mod,
                    uint16_t addr, uint16_t value);

int osd_get_system_identifier(struct osd_context *ctx, uint16_t *id);
size_t osd_get_max_pkt_len(struct osd_context *ctx);
int osd_get_num_modules(struct osd_context *ctx, uint16_t *n);

int osd_get_module_name(struct osd_context *ctx, uint16_t addr,
                        char **name);
int osd_print_module_info(struct osd_context *ctx, uint16_t addr,
                          FILE *fd, int indent);

static const uint16_t OSD_REG_CS = 0x3;
static const uint16_t OSD_CS_STALL = 0x1 << 11 | 0x1;
static const uint16_t OSD_CS_UNSTALL = 0x1 << 11;

int osd_module_is_terminal(struct osd_context *ctx, uint16_t addr);

int osd_get_scm(struct osd_context *ctx, uint16_t *addr);
int osd_get_memories(struct osd_context *ctx,
                     uint16_t **memories, size_t *num);

int osd_get_memory_descriptor(struct osd_context *, uint16_t addr,
                              struct osd_memory_descriptor **desc);

int osd_module_claim(struct osd_context *ctx, uint16_t addr);

uint16_t osd_modid2addr(struct osd_context *ctx, uint16_t id);
uint16_t osd_addr2modid(struct osd_context *ctx, uint16_t addr);

enum osd_event_type {
    OSD_EVENT_PACKET = 1,
    OSD_EVENT_TRACE = 2
};

typedef void (*osd_incoming_handler)(struct osd_context *ctx,
        void* arg, uint16_t* packet);

int osd_module_register_handler(struct osd_context *ctx, uint16_t id,
                                enum osd_event_type type, void *arg,
                                osd_incoming_handler handler);

int osd_module_stall(struct osd_context *ctx, uint16_t id);
int osd_module_unstall(struct osd_context *ctx, uint16_t id);

int osd_memory_write(struct osd_context *ctx, uint16_t mod, uint64_t addr, uint8_t* data, size_t size);
int osd_memory_read(struct osd_context *ctx, uint16_t mod, uint64_t addr, uint8_t* data, size_t size);
int osd_memory_loadelf(struct osd_context *ctx, uint16_t mod, char *filename, int verify);

int osd_stm_log(struct osd_context *ctx, uint16_t modid, char *filename);

int osd_ctm_log(struct osd_context *ctx, uint16_t modid, char *filename, char* elffile);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
