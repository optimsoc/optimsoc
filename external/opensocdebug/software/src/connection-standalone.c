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
#include <libglip.h>

#include <assert.h>
#include <stdio.h>

static void* receiver_thread_function(void* arg);

int osd_send_packet_standalone(struct osd_context *ctx, uint16_t *packet) {
    struct glip_ctx *gctx = ctx->ctx.standalone->glip_ctx;

    OSD_PRINT_PACKET_LOCATION(ctx, packet);
    size_t actual;
    glip_write_b(gctx, 0, (packet[0]+1)*2, (void*) packet, &actual, 0);

    return OSD_SUCCESS;
}

int osd_connect_standalone(struct osd_context *ctx) {
    struct glip_ctx *gctx = ctx->ctx.standalone->glip_ctx;

    glip_open(gctx, 1);

    pthread_create(&ctx->ctx.standalone->receiver_thread, 0,
                   receiver_thread_function, ctx);

    return 0;
}

static void* receiver_thread_function(void* arg) {
    struct osd_context *ctx = (struct osd_context*) arg;
    struct glip_ctx *gctx = ctx->ctx.standalone->glip_ctx;

    uint16_t packet[64];
    size_t size, actual;

    int rv;

    while (1) {
        rv = glip_read_b(gctx, 0, 2, (void*) packet, &actual, 0);
        assert(rv == 0);

        size = *((uint16_t*) &packet[0]);

        rv = glip_read_b(gctx, 0, size*2, (void*) &packet[1], &actual, 0);
        assert(rv == 0);

        OSD_PRINT_PACKET_LOCATION(ctx, packet);

        osd_handle_packet(ctx, packet);
    }
}
