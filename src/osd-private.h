/* Copyright (c) 2016 by the author(s)
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
 */

#ifndef _OSD_PRIVATE_H_
#define _OSD_PRIVATE_H_

#include <libglip.h>

#include <pthread.h>
#include <stdlib.h>

#include "include/opensocdebug.h"

#define OSD_EXPORT __attribute__ ((visibility("default")))

struct module_handler;

struct osd_module_info {
    uint16_t addr;
    uint16_t type;
    uint16_t version;

    union {
        struct osd_memory_descriptor *memory;
        struct osd_stm_descriptor *stm;
        struct osd_ctm_descriptor *ctm;
    } descriptor;
};

struct osd_system_info {
    uint16_t identifier;
    uint16_t max_pkt_len;
    uint16_t num_modules;
    uint16_t num_memories;
    struct osd_module_info modules[];
};

struct osd_context_standalone {
    struct glip_ctx *glip_ctx;
    pthread_t receiver_thread;
};

struct osd_context_daemon {
    char* host;
    int port;
    int socket;
    pthread_t receiver_thread;
};

struct osd_mode_functions {
    int (*connect)(struct osd_context *);
    int (*send)(struct osd_context *, uint16_t *);
    int (*claim)(struct osd_context *ctx, uint16_t);
};

struct osd_context {
    enum osd_mode mode;
    union {
        struct osd_context_standalone *standalone;
        struct osd_context_daemon *daemon;
    } ctx;
    struct osd_mode_functions functions;

    struct {
        pthread_mutex_t lock;
        pthread_cond_t cond_complete;
        size_t size;
        uint16_t resp_packet[10];
    } reg_access;

    struct {
        pthread_mutex_t lock;
        pthread_cond_t cond_complete;
        size_t size;
        size_t count;
        uint8_t *data;
    } mem_access;

    struct osd_system_info *system_info;

    struct module_handler **module_handlers;

    uint8_t debug_packets;
};

int osd_connect_standalone(struct osd_context *ctx);
int osd_connect_daemon(struct osd_context *ctx);

int osd_send_packet_standalone(struct osd_context *ctx, uint16_t *data);
int osd_send_packet_daemon(struct osd_context *ctx, uint16_t *data);

void osd_handle_packet(struct osd_context *ctx, uint16_t *packet);

int osd_system_enumerate(struct osd_context *ctx);

static const uint8_t REG_READ16 = 0x0;
static const uint8_t REG_READ32 = 0x1;
static const uint8_t REG_READ64 = 0x2;
static const uint8_t REG_READ128 = 0x3;
static const uint8_t REG_WRITE16 = 0x4;
static const uint8_t REG_WRITE32 = 0x5;
static const uint8_t REG_WRITE64 = 0x6;
static const uint8_t REG_WRITE128 = 0x7;

static const uint16_t modules_max_id = 4;
struct module_types {
    const char *name;
};
extern const struct module_types module_lookup[6];

struct module_callback {
    osd_incoming_handler call;
    void *arg;
};

struct module_handler {
    struct module_callback packet_handler;
    size_t num_trace_handlers;
    struct module_callback trace_handlers[];
};

void control_init(struct osd_context *ctx);
int claim_standalone(struct osd_context *ctx, uint16_t id);
int claim_daemon(struct osd_context *ctx, uint16_t id);

#endif
