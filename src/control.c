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

#include "osd-private.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "tools/daemon/daemon-packets.h"

void control_init(struct osd_context *ctx) {
    size_t sz = sizeof(struct module_handler*);
    ctx->module_handlers = calloc(ctx->system_info->num_modules, sz);

    sz = sizeof(struct module_handler);
    for (int i = 0; i < ctx->system_info->num_modules + 1; i++) {
        ctx->module_handlers[i] = calloc(1, sz);
    }
}

OSD_EXPORT
int osd_module_claim(struct osd_context *ctx, uint16_t id) {
    return ctx->functions.claim(ctx, id);
}

int claim_standalone(struct osd_context *ctx, uint16_t id) {
    return 0;
}

int claim_daemon(struct osd_context *ctx, uint16_t id) {
    uint16_t packet[4];
    packet[0] = 3;
    packet[1] = 0xffff;
    packet[2] = OSD_DP_CLAIM;
    packet[3] = id;

    send(ctx->ctx.daemon->socket, packet, 8, 0);

    return 0;
}

OSD_EXPORT
int osd_module_register_handler(struct osd_context *ctx, uint16_t id,
                                enum osd_event_type type, void *arg,
                                osd_incoming_handler handler) {
    struct module_callback *cb;

    if (type == OSD_EVENT_PACKET) {
        cb = &ctx->module_handlers[id]->packet_handler;
    } else {
        cb = &ctx->module_handlers[id]->packet_handler;
    }

    cb->call = handler;
    cb->arg = arg;

    return 0;
}

OSD_EXPORT
int osd_reset_system(struct osd_context *ctx, int halt_cores) {
    uint16_t scm;

    if (osd_get_scm(ctx, &scm) != OSD_SUCCESS) {
        return OSD_E_GENERIC;
    }

    uint16_t addr = osd_modid2addr(ctx, scm);

    osd_reg_write16(ctx, addr, 0x203, 0x3);

    if (halt_cores) {
        osd_reg_write16(ctx, addr, 0x203, 0x2);
    } else {
        osd_reg_write16(ctx, addr, 0x203, 0x0);
    }

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_start_cores(struct osd_context *ctx) {
    uint16_t scm;

    if (osd_get_scm(ctx, &scm) != OSD_SUCCESS) {
        return OSD_E_GENERIC;
    }

    uint16_t addr = osd_modid2addr(ctx, scm);

    osd_reg_write16(ctx, addr, 0x203, 0x0);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_module_stall(struct osd_context *ctx, uint16_t id) {
    osd_reg_write16(ctx, osd_modid2addr(ctx, id), OSD_REG_CS, OSD_CS_STALL);
    return 0;
}

OSD_EXPORT
int osd_module_unstall(struct osd_context *ctx, uint16_t id) {
    osd_reg_write16(ctx, osd_modid2addr(ctx, id), OSD_REG_CS, OSD_CS_UNSTALL);
    return 0;
}

