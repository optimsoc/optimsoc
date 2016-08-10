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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

static void* receiver_thread_function(void* arg);

int osd_send_packet_daemon(struct osd_context *ctx, uint16_t *packet) {
    OSD_PRINT_PACKET_LOCATION(ctx, packet);

    send(ctx->ctx.daemon->socket, packet, (packet[0]+1)*2, 0);

    return OSD_SUCCESS;
}

int osd_connect_daemon(struct osd_context *ctx) {
    struct osd_context_daemon *c = ctx->ctx.daemon;
    struct hostent *server;

    struct sockaddr_in addr;

    c->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (c->socket < 0) {
        return OSD_E_GENERIC;
    }

    server = gethostbyname(c->host);
    if (server == NULL) {
        return OSD_E_GENERIC;
    }

    /* build the server's Internet address */
    bzero((char *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&addr.sin_addr.s_addr, server->h_length);
    addr.sin_port = htons(c->port);

    /* connect: create a connection with the server */
    if (connect(c->socket, &addr, sizeof(addr)) < 0) {
      return OSD_E_GENERIC;
    }

    pthread_create(&c->receiver_thread, 0,
                   receiver_thread_function, ctx);

    return 0;
}

static void* receiver_thread_function(void* arg) {
    struct osd_context *ctx = (struct osd_context*) arg;
    struct osd_context_daemon *dctx = ctx->ctx.daemon;

    uint16_t packet[64];
    size_t size;

    int rv;

    while (1) {
        rv = recv(dctx->socket, packet, 2, MSG_WAITALL);
        assert(rv == 2);

        size = *((uint16_t*) &packet[0]);

        rv = recv(dctx->socket, &packet[1], size*2, MSG_WAITALL);
        assert(rv == (int) size*2);

        OSD_PRINT_PACKET_LOCATION(ctx, packet);

        osd_handle_packet(ctx, packet);
    }
}
