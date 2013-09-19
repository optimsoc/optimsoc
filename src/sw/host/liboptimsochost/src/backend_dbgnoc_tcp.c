/*
 * This file is part of liboptimsochost.
 *
 * liboptimsochost is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * liboptimsochost is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with liboptimsochost. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner <philipp.wagner@tum.de>
 *    Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "backend_dbgnoc_tcp.h"
#include "backend_dbgnoc.h"

/**
 * Opaque object anonymous to liboptimsochost and defined here.
 * Stores the connection context.
 */
struct ob_dbgnoc_connection_ctx {
    struct optimsoc_log_ctx *log_ctx;
    char *hostname;
    int port;
    int socketfd;
    int ctrlsocketfd;
};

int ob_dbgnoc_tcp_new(struct ob_dbgnoc_connection_ctx **ctx,
                      struct ob_dbgnoc_connection_interface *calls,
                      struct optimsoc_log_ctx *log_ctx,
                      int num_options,
                      struct optimsoc_backend_option options[])
{
    struct ob_dbgnoc_connection_ctx *c;
    c = calloc(1, sizeof(struct ob_dbgnoc_connection_ctx));

    // Set defaults
    c->hostname = "localhost";
    c->port = 23000;

    // Search options for non-default options
    int i;
    for (i = 0; i < num_options; i++) {
        if (strcmp(options[i].name, "host") == 0) {
            int vlen = strlen(options[i].value);
            c->hostname = calloc(1, vlen);
            strncpy(c->hostname, options[i].value, vlen);
        } else if (strcmp(options[i].name, "port") == 0) {
            c->port = atoi(options[i].value);
        }
    }

    // initially invalid
    c->socketfd = -1;
    c->ctrlsocketfd = -1;

    c->log_ctx = log_ctx;

    calls->read_fn = &ob_dbgnoc_tcp_read;
    calls->write_fn = &ob_dbgnoc_tcp_write;
    calls->free = &ob_dbgnoc_tcp_free;
    calls->connect = &ob_dbgnoc_tcp_connect;
    calls->disconnect = &ob_dbgnoc_tcp_disconnect;
    calls->connected = &ob_dbgnoc_tcp_connected;
    calls->reset = &ob_dbgnoc_tcp_reset;

    *ctx = c;

    return 0;
}

int ob_dbgnoc_tcp_free(struct ob_dbgnoc_connection_ctx *ctx)
{
    free(ctx);
    ctx = 0;
    return 0;
}

int ob_dbgnoc_tcp_connect(struct ob_dbgnoc_connection_ctx *ctx)
{
    int rv;
    struct hostent *server;
    struct sockaddr_in serv_addr;

    ctx->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->socketfd < 0) {
        err(ctx->log_ctx, "Error opening data socket\n");
        return -1;
    }

    /* Open the second socket in the system for control messages */
    ctx->ctrlsocketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->ctrlsocketfd < 0) {
        err(ctx->log_ctx, "Error opening control socket\n");
        return -1;
    }

    server = gethostbyname(ctx->hostname);
    if (server == NULL) {
        err(ctx->log_ctx, "Name lookup for host %s failed.\n", ctx->hostname);
        return -1;
    }

    memset((void*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(ctx->port);

    /* connect to system */
    rv = connect(ctx->socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (rv < 0) {
        err(ctx->log_ctx, "Cannot connect to system\n");
        return -1;
    }

    /* connect to control port */
    serv_addr.sin_port = htons(ctx->port+1);
    rv = connect(ctx->ctrlsocketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (rv < 0) {
        err(ctx->log_ctx, "Cannot connect to system (control)\n");
        return -1;
    }

    return 0;
}

int ob_dbgnoc_tcp_connected(struct ob_dbgnoc_connection_ctx *ctx)
{
    return (ctx->socketfd >= 0);
}

int ob_dbgnoc_tcp_disconnect(struct ob_dbgnoc_connection_ctx *ctx)
{
    close(ctx->socketfd);
    ctx->socketfd = -1;
    close(ctx->ctrlsocketfd);
    ctx->ctrlsocketfd = -1;

    return 0;
}

/**
 * Reset the whole system
 *
 * \param ctx backend context
 */
int ob_dbgnoc_tcp_reset(struct ob_dbgnoc_connection_ctx *ctx)
{
    info(ctx->log_ctx, "reset is not implemented for tcp backend\n");
    return 0;
}

int ob_dbgnoc_tcp_write(struct ob_dbgnoc_connection_ctx *ctx, uint16_t *buffer,
                        int len)
{
    assert(ctx->socketfd >= 0);

    return write(ctx->socketfd, buffer, len*2);
}

int ob_dbgnoc_tcp_read(struct ob_dbgnoc_connection_ctx *ctx, uint16_t *buffer,
                       int len)
{
    assert(ctx->socketfd >= 0);

    int rv = read(ctx->socketfd, buffer, len*2);
    if (rv > 0) {
        /* go from char to uint16_t */
        rv = rv / 2;
    }
    return rv;
}

