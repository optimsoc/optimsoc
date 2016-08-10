/* Copyright (c) 2014 by the author(s)
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
 * libglip is a flexible FIFO-based communication library between an FPGA and
 * a PC.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include "backend_tcp.h"
#include "glip-protected.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @defgroup backend_tcp-sw TCP libglip backend
 * @ingroup backend_tcp
 *
 * @section backend_tcp-sw-configoptions Configuration Options
 * Use "tcp" as backend name with glip_new(). The following options are
 * available:
 *
 * <table>
 *   <caption><code>tcp</code> backend options</caption>
 *   <tr>
 *     <td><code>hostname</code></td>
 *     <td>the host to connect to, defaults to <code>localhost</code>.
 *     Both IPv4 and IPv6 addresses are supported, as well as FQDNs
 *     (such as <code>example.com</code>)</td>
 *   </tr>
 *   <tr>
 *     <td><code>port</code></td>
 *     <td>the port to connect to, defaults to <code>23000</code></td>
 *   </tr>
 * </table>
 */

/**
 * GLIP backend context for the TCP backend
 */
struct glip_backend_ctx {
    /** socket fd of the data channel */
    int data_sfd;

    /** epoll fd for the data channel */
    int data_efd;
    /** epoll event structure for the data channel */
    struct epoll_event data_ev;

    /** socket fd of the control channel */
    int ctrl_sfd;
};

/**
 * Default hostname for the TCP connection
 */
static const char* DEFAULT_HOSTNAME = "localhost";
/**
 * Default port for the TCP connection
 */
static const unsigned int DEFAULT_PORT_DATA = 23000;

// control messages
static const unsigned int CTRL_MSG_LOGIC_RESET = 0x0001;

/**
 * Initialize the backend (constructor)
 *
 * @see glip_new()
 */
int gb_tcp_new(struct glip_ctx *ctx)
{
    struct glip_backend_ctx *c = calloc(1, sizeof(struct glip_backend_ctx));

    c->data_sfd = -1;
    c->ctrl_sfd = -1;
    c->data_efd = -1;

    /* setup vtable */
    ctx->backend_functions.open = gb_tcp_open;
    ctx->backend_functions.close = gb_tcp_close;
    ctx->backend_functions.logic_reset = gb_tcp_logic_reset;
    ctx->backend_functions.read = gb_tcp_read;
    ctx->backend_functions.read_b = gb_tcp_read_b;
    ctx->backend_functions.write = gb_tcp_write;
    ctx->backend_functions.write_b = gb_tcp_write_b;
    ctx->backend_functions.get_fifo_width = gb_tcp_get_fifo_width;
    ctx->backend_functions.get_channel_count = gb_tcp_get_channel_count;

    ctx->backend_ctx = c;

    return 0;
}

/**
 * Open a target connection
 *
 * @see glip_open()
 */
int gb_tcp_open(struct glip_ctx *ctx, unsigned int num_channels)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;

    int rv;
    const char* hostname;
    unsigned int port_data;
    unsigned int port_ctrl;

    if (bctx->data_sfd >= 0 || bctx->ctrl_sfd >= 0) {
        err(ctx, "Already connected, disconnect first!\n");
        return -1;
    }

    /* get hostname and port from options */
    if (glip_option_get_char(ctx, "hostname", &hostname) != 0) {
        dbg(ctx, "No 'hostname' option set, using default %s.\n",
            DEFAULT_HOSTNAME);
        hostname = DEFAULT_HOSTNAME;
    }
    if (glip_option_get_uint32(ctx, "port", &port_data) != 0) {
        dbg(ctx, "No 'port' option set, using default %u\n",
            DEFAULT_PORT_DATA);
        port_data = DEFAULT_PORT_DATA;
    }
    port_ctrl = port_data + 1;

    /* connect to data channel */
    bctx->data_sfd = 0;
    rv = gl_util_connect_to_host(ctx, hostname, port_data, &bctx->data_sfd);
    if (rv != 0) {
        return -1;
    }
    rv = gl_util_fd_nonblock(ctx, bctx->data_sfd);
    if (rv != 0) {
        return -1;
    }

    /* setup polling (for blocking I/O on data channel) */
    bctx->data_efd = epoll_create1(0);
    if (bctx->data_efd == -1) {
        err(ctx, "Unable to create epoll fd for the data channel: %s\n",
            strerror(errno));
        return -1;
    }
    bctx->data_ev.data.fd = bctx->data_sfd;
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLET;
    rv = epoll_ctl(bctx->data_efd, EPOLL_CTL_ADD, bctx->data_sfd, &ev);
    if (rv != 0) {
        return -1;
    }

    /* connect to control channel */
    bctx->ctrl_sfd = 0;
    rv = gl_util_connect_to_host(ctx, hostname, port_ctrl, &bctx->ctrl_sfd);
    if (rv != 0) {
        return -1;
    }
    rv = gl_util_fd_nonblock(ctx, bctx->ctrl_sfd);
    if (rv != 0) {
        return -1;
    }

    return 0;
}

/**
 * Close connection to the target
 *
 * @see glip_close()
 */
int gb_tcp_close(struct glip_ctx *ctx)
{
    if (ctx->backend_ctx->data_efd >= 0) {
        close(ctx->backend_ctx->data_efd);
    }

    if (ctx->backend_ctx->data_sfd >= 0) {
        close(ctx->backend_ctx->data_sfd);
        ctx->backend_ctx->data_sfd = -1;
    }
    if (ctx->backend_ctx->ctrl_sfd >= 0) {
        close(ctx->backend_ctx->ctrl_sfd);
        ctx->backend_ctx->ctrl_sfd = -1;
    }

    return 0;
}

/**
 * Reset the logic on the target
 *
 * @see glip_logic_reset()
 */
int gb_tcp_logic_reset(struct glip_ctx *ctx)
{
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    uint16_t buf[1];
    buf[0] = CTRL_MSG_LOGIC_RESET;

    ssize_t wsize = write(bctx->ctrl_sfd, buf, sizeof(buf));
    if (wsize == -1 || wsize != sizeof(buf)) {
        err(ctx, "Unable to write data to control channel: %s\n",
            strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * Read from the target device
 *
 * @see glip_read()
 */
int gb_tcp_read(struct glip_ctx *ctx, uint32_t channel, size_t size,
                uint8_t *data, size_t *size_read)
{
    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the tcp backend");
        return -1;
    }

    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    ssize_t rsize = read(bctx->data_sfd, data, size);
    if (rsize == -1) {
        *size_read = 0;
        if (errno == EAGAIN) {
            return 0;
        } else {
            return -1;
        }
    }

    *size_read = rsize;
    return 0;
}

/**
 * Blocking read from the target
 *
 * @see glip_read_b()
 */
int gb_tcp_read_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                  uint8_t *data, size_t *size_read, unsigned int timeout)
{
    int rv;
    size_t size_read_tmp = 0;
    size_t sr;

    struct epoll_event events[1];

    struct timespec ts_start, ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    do {
        size_t size_remaining = size - size_read_tmp;
        rv = gb_tcp_read(ctx, channel, size_remaining, &data[size_read_tmp], &sr);
        if (rv != 0) {
            err(ctx, "TCP read error!\n");
            return -1;
        }
        size_read_tmp += sr;

        if ((sr != size_remaining) && (timeout > 0)) {
            /* we didn't get as much data as we requested - wait for new data! */
            do {
                /* calculate remaining wait time */
                clock_gettime(CLOCK_MONOTONIC, &ts_now);
                int tspent = (ts_now.tv_sec * 1000 + ts_now.tv_nsec / 1000 / 1000) -
                             (ts_start.tv_sec * 1000 + ts_start.tv_nsec / 1000 / 1000);
                if (tspent >= (int)timeout) {
                    /* we've hit the timeout already; return what we got */
                    goto return_read_data;
                }

                int nfds = epoll_wait(ctx->backend_ctx->data_efd, events,
                                      sizeof(events), timeout - tspent);
                if (nfds == -1) {
                    err(ctx, "epoll_wait() failed: %s\n", strerror(errno));
                    return -1;
                }
                if (nfds == 0) {
                    /* we've hit the timeout */
                    goto return_read_data;
                }

                assert(nfds == 1);

                if ((events[0].events & EPOLLRDHUP) ||
                    (events[0].events & EPOLLERR) ||
                    (events[0].events & EPOLLHUP)) {
                    /* an error happened; did the connection die? */

                    err(ctx, "An error happened while waiting for the "
                        "TCP I/O. Returning whatever data we have.\n");
                    /*
                     * XXX: is this error handling ok, or should we inform the
                     * API user?
                     */
                    goto return_read_data;
                }
            } while (!((events[0].events & EPOLLIN) ||
                       (events[0].events & EPOLLPRI)));
        }
    } while (size > size_read_tmp);

return_read_data:
    *size_read = size_read_tmp;
    if (size_read_tmp != size) {
        return -ETIMEDOUT;
    }
    return 0;
}

/**
 * Write to the target
 *
 * @see glip_write()
 */
int gb_tcp_write(struct glip_ctx *ctx, uint32_t channel, size_t size,
                 uint8_t *data, size_t *size_written)
{
    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the tcp backend");
        return -1;
    }

    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    ssize_t wsize = write(bctx->data_sfd, data, size);
    if (wsize == -1) {
        *size_written = 0;
        if (errno == EAGAIN) {
            return 0;
        } else {
            return -1;
        }
    }

    *size_written = wsize;
    return 0;
}

/**
 * Blocking write to the target
 *
 * @see glip_write_b()
 */
int gb_tcp_write_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                   uint8_t *data, size_t *size_written, unsigned int timeout)
{
    int rv;
    size_t size_written_tmp = 0;
    size_t sw;

    struct epoll_event events[1];

    struct timespec ts_start, ts_now;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    do {
        size_t size_remaining = size - size_written_tmp;
        rv = gb_tcp_write(ctx, channel, size_remaining,
                          &data[size_written_tmp], &sw);
        if (rv != 0) {
            err(ctx, "TCP write error!\n");
            return -1;
        }
        size_written_tmp += sw;

        if ((sw != size_remaining) && (timeout > 0)) {
            /* more data needs to be written */
            do {
                /* calculate remaining wait time */
                clock_gettime(CLOCK_MONOTONIC, &ts_now);
                int tspent = (ts_now.tv_sec * 1000 + ts_now.tv_nsec / 1000 / 1000) -
                             (ts_start.tv_sec * 1000 + ts_start.tv_nsec / 1000 / 1000);
                if (tspent >= (int)timeout) {
                    /* we've hit the timeout already */
                    goto return_written_data;
                }

                int nfds = epoll_wait(ctx->backend_ctx->data_efd, events,
                                      sizeof(events), timeout - tspent);
                if (nfds == -1) {
                    err(ctx, "epoll_wait() failed: %s\n", strerror(errno));
                    return -1;
                }
                if (nfds == 0) {
                    /* we've hit the timeout */
                    goto return_written_data;
                }

                assert(nfds == 1);

                if ((events[0].events & EPOLLRDHUP) ||
                    (events[0].events & EPOLLERR) ||
                    (events[0].events & EPOLLHUP)) {
                    /* an error happened; did the connection die? */

                    err(ctx, "An error happened while waiting for the "
                        "TCP I/O when writing.\n");
                    /*
                     * XXX: is this error handling ok, or should we inform the
                     * API user?
                     */
                    goto return_written_data;
                }
            } while (!(events[0].events & EPOLLOUT));
        }
    } while (size_written_tmp < size);

return_written_data:
    *size_written = size_written_tmp;
    if (size_written_tmp != size) {
        return -ETIMEDOUT;
    }
    return 0;
}


/**
 * Get the number of supported channels by this backend
 *
 * @param  ctx the library context
 * @return always 1
 *
 * @see glip_get_channel_count()
 */
unsigned int gb_tcp_get_channel_count(struct glip_ctx *ctx)
{
    return 1;
}

/**
 * Get the width of the FIFO
 *
 * @param  ctx the library context
 * @return always 2 bytes, i.e. 16 bit
 *
 * @see glip_get_fifo_width()
 */
unsigned int gb_tcp_get_fifo_width(struct glip_ctx *ctx)
{
    return 2;
}
