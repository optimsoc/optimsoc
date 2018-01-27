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
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

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
    /** TCP communication thread */
    pthread_t tcp_com_thread;

    /** Shutdown request for tcp_com_thread */
    volatile bool tcp_com_thread_shutdown;

    /** Input buffer */
    struct cbuf *read_buf;
    /** Output buffer */
    struct cbuf *write_buf;
    /** semaphore notifying the TCP communication thread to fetch new data */
    sem_t tcp_com_notification_sem;

    /** socket fd of the data channel */
    int data_sfd;

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

/** Size of the read/write buffers */
static const unsigned int BUF_SIZE = 64 * 1024; // kB

static int tcp_read(struct glip_ctx *ctx)
{
    int rv;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    if (bctx->data_sfd < 0) {
        return -ENOTCONN;
    }

    size_t free_level = cbuf_free_level(bctx->read_buf);
    if (free_level == 0) {
        return 0;
    }
    uint8_t *buf;
    rv = cbuf_reserve(bctx->read_buf, &buf, free_level);
    assert(rv == 0);

    ssize_t rsize = read(bctx->data_sfd, buf, free_level);
    if (rsize == -1) {
        if (errno == EAGAIN) {
            return 0;
        } else if (errno == EBADF) {
            dbg(ctx, "TCP connection was closed during read.\n");
            return -ENOTCONN;
        } else {
            dbg(ctx, "TCP read() returned %zd (errno = %d)\n", rsize,
                errno);
            return -1;
        }
    }
    rv = cbuf_commit(bctx->read_buf, buf, rsize);
    assert(rv == 0);

    return 0;
}

static int tcp_write(struct glip_ctx *ctx)
{
    int rv;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    if (bctx->data_sfd < 0) {
        return -ENOTCONN;
    }

    size_t fill_level = cbuf_fill_level(bctx->write_buf);
    if (fill_level == 0) {
        return 0;
    }

    uint8_t *data;
    rv = cbuf_peek(bctx->write_buf, &data, fill_level);
    assert(rv == 0);

    ssize_t wsize = send(bctx->data_sfd, data, fill_level, MSG_NOSIGNAL);
    if (wsize == -1) {
        if (errno == EAGAIN) {
            return 0;
        } else if (errno == EBADF || errno == EPIPE) {
            return -ENOTCONN;
        } else {
            dbg(ctx, "TCP send() returned %zd (errno = %d)\n", wsize, errno);
            return -1;
        }
    }

    rv = cbuf_discard(bctx->write_buf, wsize);
    assert(rv == 0);

    return 0;
}

/**
 * Thread: Read/write from/to the TCP socket, store data in cbufs
 */
static void* tcp_com_thread(void* ctx_void)
{
    struct glip_ctx *ctx = ctx_void;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    int rv;
    struct timespec ts;

    while (1) {
        clock_gettime(CLOCK_REALTIME, &ts);
        timespec_add_ns(&ts, 100 * 1000); // 100 us
        sem_timedwait(&ctx->backend_ctx->tcp_com_notification_sem, &ts);

        /* handle shutdown request */
        if (bctx->tcp_com_thread_shutdown) {
            goto cleanup_return;
        }

        /* READ: Try to read as much data as cbuf can hold */
        rv = tcp_read(ctx);
        if (rv != 0) {
            goto cleanup_return;
        }

        /* WRITE: Try to write as much data as we have in the write cbuf */
        rv = tcp_write(ctx);
        if (rv != 0) {
            goto cleanup_return;
        }
    }

cleanup_return:
    dbg(ctx, "Shutting down tcp_com_thread\n");

    if (ctx->backend_ctx->data_sfd >= 0) {
        close(ctx->backend_ctx->data_sfd);
        ctx->backend_ctx->data_sfd = -1;
    }
    if (ctx->backend_ctx->ctrl_sfd >= 0) {
        close(ctx->backend_ctx->ctrl_sfd);
        ctx->backend_ctx->ctrl_sfd = -1;
    }

    bctx->tcp_com_thread_shutdown = false;
    return NULL;
}

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
    c->tcp_com_thread_shutdown = false;

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
 * Destruct the backend
 *
 * @see glip_free()
 */
void gb_tcp_free(struct glip_ctx *ctx)
{
    free(ctx->backend_ctx);
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

    /* initialize write circular buffer */
    rv = cbuf_init(&ctx->backend_ctx->write_buf, BUF_SIZE);
    if (rv < 0) {
        err(ctx, "Unable to setup write buffer: %d\n", rv);
        return -1;
    }

    /* initialize read circular buffer */
    rv = cbuf_init(&ctx->backend_ctx->read_buf, BUF_SIZE);
    if (rv < 0) {
        err(ctx, "Unable to setup read buffer: %d\n", rv);
        return -1;
    }

    sem_init(&ctx->backend_ctx->tcp_com_notification_sem, 0, 0);

    rv = pthread_create(&ctx->backend_ctx->tcp_com_thread, NULL,
                        tcp_com_thread, (void*)ctx);
    if (rv) {
        err(ctx, "Unable to create TCP communication thread: %d\n", rv);
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
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    /* clean-up TCP communication thread */
    ctx->backend_ctx->tcp_com_thread_shutdown = true;

    int sval;
    sem_getvalue(&bctx->tcp_com_notification_sem, &sval);
    if (sval == 0) {
        sem_post(&bctx->tcp_com_notification_sem);
    }
    pthread_join(ctx->backend_ctx->tcp_com_thread, NULL);

    /* tear down event notifications */
    sem_destroy(&bctx->tcp_com_notification_sem);

    /*
     * Tear down read/write buffer. This also unblocks all blocking read/write
     * functions and makes them return -ECANCELED
     */
    cbuf_free(ctx->backend_ctx->write_buf);
    cbuf_free(ctx->backend_ctx->read_buf);

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

    ssize_t wsize = send(bctx->ctrl_sfd, buf, sizeof(buf), MSG_NOSIGNAL);
    if (wsize == -1 || wsize != sizeof(buf)) {
        err(ctx, "Unable to write data to control channel: %s\n",
            strerror(errno));
        return -1;
    }
    return 0;
}

/**
 * Possibly trigger a refill/flush of the incoming and outgoing buffers
 */
static void trigger_com_refill_flush(struct glip_backend_ctx *bctx)
{
    if (cbuf_fill_level(bctx->write_buf) >= 4 * 1024 ||
        cbuf_free_level(bctx->read_buf) > 0) {
        int sval;
        sem_getvalue(&bctx->tcp_com_notification_sem, &sval);
        if (sval == 0) {
            sem_post(&bctx->tcp_com_notification_sem);
        }
    }
}
/**
 * Read from the target device
 *
 * @see glip_read()
 */
int gb_tcp_read(struct glip_ctx *ctx, uint32_t channel, size_t size,
                uint8_t *data, size_t *size_read)
{
    int rv;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the tcp backend");
        return -1;
    }

    rv = gb_util_cbuf_read(bctx->read_buf, size, data, size_read);
    if (rv != 0) {
        return rv;
    }

    trigger_com_refill_flush(bctx);

    return rv;
}

/**
 * Blocking read from the device
 *
 * @see glip_read_b()
 */
int gb_tcp_read_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                  uint8_t *data, size_t *size_read, unsigned int timeout)
{
    int rv;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the tcp backend");
        return -1;
    }

    rv = gb_util_cbuf_read_b(bctx->read_buf, size, data, size_read, timeout);
    if (rv != 0) {
        return rv;
    }

    trigger_com_refill_flush(bctx);

    return rv;
}

/**
 * Write to the target device
 *
 * @see glip_write()
 */
int gb_tcp_write(struct glip_ctx *ctx, uint32_t channel, size_t size,
                 uint8_t *data, size_t *size_written)
{
    int rv;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the tcp backend");
        return -1;
    }

    rv = gb_util_cbuf_write(bctx->write_buf, size, data, size_written);
    if (rv != 0) {
        return rv;
    }

    trigger_com_refill_flush(bctx);

    return 0;
}

/**
 * Blocking write to the target device
 *
 * @see glip_write_b()
 */
int gb_tcp_write_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                   uint8_t *data, size_t *size_written, unsigned int timeout)
{
    int rv;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the tcp backend");
        return -1;
    }

    rv = gb_util_cbuf_write_b(bctx->write_buf, size, data, size_written,
                              timeout);
    if (rv != 0) {
        return rv;
    }

    trigger_com_refill_flush(bctx);

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
