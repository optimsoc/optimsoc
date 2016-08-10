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

OSD_EXPORT
int osd_reg_access(struct osd_context *ctx, uint16_t* packet) {

    pthread_mutex_lock(&ctx->reg_access.lock);

    osd_send_packet(ctx, packet);

    pthread_cond_wait(&ctx->reg_access.cond_complete,
                      &ctx->reg_access.lock);

    memcpy(packet, ctx->reg_access.resp_packet, (ctx->reg_access.size+1)*2);
    pthread_mutex_unlock(&ctx->reg_access.lock);

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_reg_read16(struct osd_context *ctx, uint16_t mod,
                   uint16_t addr, uint16_t *value) {

    uint16_t packet[4];
    size_t size = 3;

    packet[0] = size;
    packet[1] = mod & 0x3ff;
    packet[2] = (REG_READ16 << 10);
    packet[3] = addr;

    osd_reg_access(ctx, packet);
    assert(packet[0] == 3);

    *value = packet[3];

    return OSD_SUCCESS;
}

OSD_EXPORT
int osd_reg_write16(struct osd_context *ctx, uint16_t mod,
                    uint16_t addr, uint16_t value) {
    uint16_t packet[5];
    size_t size = 4;

    packet[0] = size;
    packet[1] = mod & 0x3ff;
    packet[2] = (REG_WRITE16 << 10);
    packet[3] = addr;
    packet[4] = value;

    osd_reg_access(ctx, packet);

    return OSD_SUCCESS;
}
