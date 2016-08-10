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

#include <opensocdebug.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include "terminal.h"

static int nxt_term_id = 0;

void *terminal_thread(void *arg);

void *terminal_thread(void *arg) {
    struct terminal *term = (struct terminal*) arg;
    uint16_t packet[4];
    packet[0] = 3;
    packet[1] = term->mod_id;
    packet[2] = OSD_EVENT_PACKET << 14;

    while (1) {
        int rv = read(term->socket, &packet[3], 1);
        assert(rv == 1);
        osd_send_packet(term->ctx, packet);
    }

    return 0;
}

int terminal_open(struct osd_context *ctx, uint16_t mod_id, struct terminal **term) {
    struct terminal *t = malloc(sizeof(struct terminal));
    t->ctx = ctx;
    t->mod_id = mod_id;

    *term = t;

    char name[128];
    snprintf(name, 128, "/tmp/osd-%d-term-%d", getpid(), nxt_term_id++);

    t->path = strdup(name);

    if ((t->socket_listen = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_un local, remote;

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, t->path);
    unlink(local.sun_path);
    int len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(t->socket_listen, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(t->socket_listen, 5) == -1) {
        perror("listen");
        exit(1);
    }

    if ((t->child = fork()) == 0) {
        char command[256];
        snprintf(command, 256, "xterm -title test -e bash -l -c 'osd_term %s'", name);
        int rv = system(command);
        exit(rv);
    }

    unsigned int s = sizeof(remote);
    if ((t->socket = accept(t->socket_listen, (struct sockaddr *)&remote, &s)) == -1) {
        perror("accept");
        exit(1);
    }

    pthread_create(&t->tx_thread, 0, terminal_thread, t);

    return 0;
}

void terminal_ingress(struct osd_context *ctx, void* mod_arg,
                      uint16_t *packet) {
    struct terminal *term = (struct terminal *) mod_arg;

    if (packet[0] == 3) {
        uint8_t c = packet[3] & 0xff;
        int rv = write(term->socket, &c, 1);
        (void) rv;
    }
}
