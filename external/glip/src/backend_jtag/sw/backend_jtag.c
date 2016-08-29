/* Copyright (c) 2014-2015 by the author(s)
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
 * JTAG GLIP backend.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Alexandra Weber <sandra.eli.weber@tum.de>
 *   Jan Alexander Wessel <jan.wessel@tum.de>
 */

#include "backend_jtag.h"
#include "glip-protected.h"
#include "cbuf.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Size of the buffer storing the read and write data before/after the
 * transmission. This buffer helps to ensure a steady stream of data from and
 * to the send/receive logic, even if small amounts of data are sent by the user
 * in glip_read()/glip_write()
 */
#define BUFFER_SIZE 64*1024 /* bytes */

/**
 * Default target configuration file for OpenOCD
 */
#define OPENOCD_CONF_TARGET_DEFAULT "glip.cfg"

/**
 * Timeout after which we assume starting openocd failed [ms]
 */
#define OPENOCD_STARTUP_TIMEOUT 10000 /* ms */

/**
 * Maximum payload size (number of data words) in a JTAG transfer
 *
 * This value is a trade-off between latency and performance and set
 * empirically for good performance.
 */
#define MAX_PAYLOAD_WORDS 15000

/**
 * GLIP backend context for the JTAG backend
 */
struct glip_backend_ctx {
    /** communication thread */
    pthread_t thread;

    /** write circular buffer */
    struct cbuf *write_buf;

    /** read circular buffer */
    struct cbuf *read_buf;

    /** address of the OpenOCD socket */
    int sockadrr;

    /**
     * OpenOCD process id, if OpenOCD was started by this backend.
     * NULL otherwise.
     */
    pid_t oocd_pid;

    /**
     * target word width in bytes. 0 represents an invalid value
     *
     * This value is read from the target at the begin of the transmission.
     * In the hardware, it's configured in the WORD_WIDTH parameter passed to
     * glip_jtag_toplevel. (Note that the hardware parameter is expressed in
     * bit, while this variable is in bytes.)
     */
    unsigned int target_word_width;

    /**
     * target buffer size in words. 0 represents an invalid value
     *
     * This value is read from the target at the begin of the transmission.
     * In the hardware, it's configured in the NUM_WORDS parameter passed to
     * glip_jtag_toplevel. (Note that the configuration is done log2(size), i.e.
     * the width of the size.)
     */
    unsigned int target_buffer_size;

    /**
     * Mutex to control access to the OpenOCD TCP socket
     */
    pthread_mutex_t oocd_access_mutex;
};

/* Control messages */
#define CTRL_MSG_LOGIC_RESET 0x0001

/**
 * Initialize the backend (constructor)
 *
 * @param[in]  ctx the library context
 *
 * @return 0 if initializing the backend was successful
 * @return any other value indicates failure
 *
 * @see glip_new()
 */
int gb_jtag_new(struct glip_ctx *ctx)
{
    struct glip_backend_ctx *c = calloc(1, sizeof(struct glip_backend_ctx));
    if (!c) {
        return -1;
    }

    int rv;

    /* setup vtable */
    ctx->backend_functions.open = gb_jtag_open;
    ctx->backend_functions.close = gb_jtag_close;
    ctx->backend_functions.logic_reset = gb_jtag_logic_reset;
    ctx->backend_functions.read = gb_jtag_read;
    ctx->backend_functions.read_b = gb_jtag_read_b;
    ctx->backend_functions.write = gb_jtag_write;
    ctx->backend_functions.write_b = gb_jtag_write_b;
    ctx->backend_functions.get_fifo_width = gb_jtag_get_fifo_width;
    ctx->backend_functions.get_channel_count = gb_jtag_get_channel_count;

    ctx->backend_ctx = c;

    /* initialize write circular buffer */
    rv = cbuf_init(&ctx->backend_ctx->write_buf, BUFFER_SIZE);
    if (rv < 0) {
        err(ctx, "Unable to setup write buffer: %d\n", rv);
        return -1;
    }

    /* initialize read circular buffer */
    rv = cbuf_init(&ctx->backend_ctx->read_buf, BUFFER_SIZE);
    if (rv < 0) {
        err(ctx, "Unable to setup read buffer: %d\n", rv);
        return -1;
    }
    ctx->backend_ctx->oocd_pid = 0;

    ctx->backend_ctx->target_word_width = 0;
    ctx->backend_ctx->target_buffer_size = 0;

    pthread_mutex_init(&ctx->backend_ctx->oocd_access_mutex, NULL);

    return 0;
}

/**
 * Open a target connection
 *
 * @param[in]  ctx the library context
 * @param[in]  num_channels the number of channels. Must be 1 for this backend.
 *
 * @return 0 if opening the connection was successful
 * @return any other value indicates failure
 *
 * @see glip_open()
 */
int gb_jtag_open(struct glip_ctx *ctx, unsigned int num_channels)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;

    int rv;
    uint8_t coding;
    if (glip_option_get_uint8(ctx, "coding", &coding) != 0) {
        coding = 32;
    }
    if (num_channels != 1) {
        err(ctx, "Channel number must be 1!\n");
        return -1;
    }

    unsigned int portno;
    const char *hostname;

    /* get hostname and port from options */
    if (glip_option_get_char(ctx, "hostname", &hostname) != 0) {
        dbg(ctx, "No 'hostname' option set, using default localhost.\n");
        hostname = "localhost";
    }
    if (glip_option_get_uint32(ctx, "port", &portno) != 0) {
        dbg(ctx, "No port number set, using default port 6666\n");
        portno = 6666;
    }

    /* connect to OpenOCD */
    rv = gl_util_connect_to_host(ctx, hostname, portno, &bctx->sockadrr);
    if (rv != 0) {
        if (strcmp(hostname, "localhost")) {
            err(ctx, "Unable to connect to OpenOCD on %s:%d\n", hostname, portno);
            return -1;
        }

        /* try to start OpenOCD and connect again */
        rv = start_openocd(ctx);
        if (rv != 0) {
            err(ctx, "Starting OpenOCD failed.\n");
            return -1;
        }

        /*
         * it takes some time for the socket to become available;
         * try a couple times before giving up.
         */
        for (unsigned int retry = 0; retry < 10; retry++) {
            rv = gl_util_connect_to_host(ctx, hostname, portno, &bctx->sockadrr);
            if (rv == 0) {
                break;
            }
            usleep(100*1000); /* 100 ms */
        }
        if (rv != 0) {
            err(ctx, "Unable to connect to OpenOCD on port %d after starting "
                "it on this PC.\n", portno);
            return -1;
        }
    }
    dbg(ctx, "Connected to OpenOCD.\n");

    /* start backend thread */
    pthread_create(&bctx->thread, 0, poll_thread, (void*) ctx);

    return 0;
}

/**
 * Close connection to the target
 *
 * @param[in]  ctx the library context
 *
 * @return 0 if closing the connection was successful
 * @return any other value indicates failure
 *
 * @see glip_close()
 */
int gb_jtag_close(struct glip_ctx *ctx)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;

    /* shut down backend thread, close socket */
    void *status;
    pthread_cancel(bctx->thread);
    pthread_join(bctx->thread, &status);
    close(bctx->sockadrr);

    /* shut down OpenOCD if started by backend */
    close_openocd(ctx);

    return 0;
}

/**
 * Reset the logic on the target
 *
 * @see glip_logic_reset()
 */
int gb_jtag_logic_reset(struct glip_ctx *ctx)
{
    static const uint16_t req[3] = { 0xFFFF, CTRL_MSG_LOGIC_RESET, 0x0000 };
    uint16_t resp[3];

    int rv = jtag_drscan(ctx, (uint8_t*)req, (uint8_t*)resp, sizeof(req));
    if (rv != 0) {
        return rv;
    }

    return 0;
}

/**
 * Read from the target device
 *
 * @param[in]  ctx the library context
 * @param[in]  channel the descriptor of the channel
 * @param[in]  size how much data is supposed to be read
 * @param[out] data the read data
 * @param[out] size_read how much data has been read
 *
 * @return 0 if reading was successful
 * @return any other value indicates failure
 *
 * @see glip_read()
 */
int gb_jtag_read(struct glip_ctx *ctx, uint32_t channel, size_t size,
                 uint8_t *data, size_t *size_read)
{
    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the jtag backend");
        return -1;
    }

    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    size_t fill_level = cbuf_fill_level(bctx->read_buf);
    size_t size_read_req = min(fill_level, size);

    int rv = cbuf_read(bctx->read_buf, data, size_read_req);
    if (rv < 0) {
        err(ctx, "Unable to get data from read buffer, rv = %d\n", rv);
        return -1;
    }

    *size_read = size_read_req;

    return 0;
}

/**
 * Blocking read from the device
 *
 * @param[in]  ctx the library context
 * @param[in]  channel the descriptor of the channel
 * @param[in]  size how much data is supposed to be read
 * @param[out] data the read data
 * @param[out] size_read how much data has been read
 * @param[in]  timeout the maximum duration the read operation can take
 *
 * @return 0 if reading was successful
 * @return -ETIMEDOUT if the read timeout was hit
 * @return any other value indicates failure
 *
 * @see glip_read_b()
 */
int gb_jtag_read_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                   uint8_t *data, size_t *size_read,
                   unsigned int timeout)
{
    int rv;
    struct glip_backend_ctx *bctx = ctx->backend_ctx;
    struct timespec ts;

    if (size > BUFFER_SIZE) {
        /*
         * This is not a problem for non-blocking reads, but blocking reads will
         * block forever in this case as the maximum amount of data ever
         * available is limited by the buffer size.
         */
        err(ctx, "The read size cannot be larger than %u bytes.", BUFFER_SIZE);
        return -1;
    }

    if (timeout != 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        timespec_add_ns(&ts, timeout * 1000 * 1000);
    }

    /*
     * Wait until sufficient data is available to be read.
     */
    if (timeout != 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        timespec_add_ns(&ts, timeout * 1000 * 1000);
    }

    size_t level = cbuf_fill_level(bctx->read_buf);

    while (level < size) {
        if (timeout == 0) {
            rv = cbuf_wait_for_level_change(bctx->read_buf, level);
        } else {
            rv = cbuf_timedwait_for_level_change(bctx->read_buf, level, &ts);
        }

        if (rv != 0) {
            break;
        }

        level = cbuf_fill_level(bctx->read_buf);
    }

    /*
     * We read whatever data is available, and assume a timeout if the available
     * amount of data does not match the requested amount.
     */
    *size_read = 0;
    rv = gb_jtag_read(ctx, channel, size, data, size_read);
    if (rv == 0 && size != *size_read) {
        return -ETIMEDOUT;
    }
    return rv;
}

/**
 * Write to the target
 *
 * @param[in]  ctx the library context
 * @param[in]  channel the descriptor of the channel
 * @param[in]  size how much data is supposed to be written
 * @param[in]  data that is supposed to be written
 * @param[out] size_written how much data has been written
 *
 * @return 0 if writing was successful
 * @return any other value indicates failure
 *
 * @see glip_write()
 */
int gb_jtag_write(struct glip_ctx *ctx, uint32_t channel, size_t size,
                 uint8_t *data, size_t *size_written)
{
    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the jtag backend");
        return -1;
    }

    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    unsigned int buf_size_free = cbuf_free_level(bctx->write_buf);
    *size_written = (size > buf_size_free ? buf_size_free : size);

    cbuf_write(bctx->write_buf, data, *size_written);

    return 0;
}

/**
 * Blocking write to the target
 *
 * @param[in]  ctx the library context
 * @param[in]  channel the descriptor of the channel
 * @param[in]  size how much data is supposed to be written
 * @param[in]  data that is supposed to be written
 * @param[out] size_written how much data has been written
 * @param[in]  timeout the maximum duration the write operation can take
 *
 * @return 0 if writing was successful
 * @return -ETIMEDOUT if the read timeout was hit
 * @return any other value indicates failure
 *
 * @see glip_write_b()
 */
int gb_jtag_write_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                   uint8_t *data, size_t *size_written, unsigned int timeout)
{
    if (channel != 0) {
        err(ctx, "Only channel 0 is supported by the jtag backend");
        return -1;
    }

    struct glip_backend_ctx* bctx = ctx->backend_ctx;
    struct timespec ts;

    if (timeout != 0) {
        clock_gettime(CLOCK_REALTIME, &ts);
        timespec_add_ns(&ts, timeout * 1000 * 1000);
    }

    size_t size_done = 0;
    while (1) {
        size_t size_done_tmp = 0;
        gb_jtag_write(ctx, channel, size - size_done, &data[size_done],
                            &size_done_tmp);
        size_done += size_done_tmp;

        if (size_done == size) {
            break;
        }

        if (cbuf_free_level(bctx->write_buf) == 0) {
            if (timeout == 0) {
                cbuf_wait_for_level_change(bctx->write_buf, 0);
            } else {
                cbuf_timedwait_for_level_change(bctx->write_buf, 0, &ts);
            }
        }
    }

    *size_written = size_done;
    if (size != *size_written) {
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
unsigned int gb_jtag_get_channel_count(struct glip_ctx *ctx)
{
    return 1;
}

/**
 * Get the width of the FIFO
 *
 * @param  ctx the library context
 * @return the word width on the target, in bytes
 *
 * @see glip_get_fifo_width()
 */
unsigned int gb_jtag_get_fifo_width(struct glip_ctx *ctx)
{
    return ctx->backend_ctx->target_word_width;
}

/**
 * Discover the configuration of the hardware target
 *
 * The discovered values are written to the appropriate fields in the ctx
 * struct.
 *
 * @param ctx the library context
 * @return 0 on success
 * @return any other value indicates an error
 *
 * @private
 */
int discover_targetconf(struct glip_ctx *ctx)
{
    static const uint16_t req[3] = { 0xFFFF, 0x0000, 0x0000 };
    uint16_t resp[3];

    int rv = jtag_drscan(ctx, (uint8_t*)req, (uint8_t*)resp, sizeof(req));
    if (rv != 0) {
        return rv;
    }

    assert(resp[1] != 0);
    assert(resp[1] % 8 == 0); /* word width needs to be full bytes */
    ctx->backend_ctx->target_word_width = resp[1] / 8;
    ctx->backend_ctx->target_buffer_size = 1 << resp[2];

    dbg(ctx, "Target configuration: word width = %u bytes, buffer size = %d\n",
        ctx->backend_ctx->target_word_width,
        ctx->backend_ctx->target_buffer_size);

    return 0;
}

/**
 * Send JTAG data shift command to OpenOCD and process its result
 *
 * We talk to OpenOCD using the Tcl RPC Server (see
 * http://openocd.org/doc/html/Tcl-Scripting-API.html). This text-based protocol
 * wraps JTAG commands into Tcl commands, the communication is handled over TCP
 * sockets.
 *
 * @param[in]  ctx the library context
 * @param[in]  req request data
 * @param[out] resp response data
 * @param[in]  size size of (equal sized) request and response in bytes
 * @return 0 on success
 * @return any other value indicates an error
 *
 * @private
 */
int jtag_drscan(struct glip_ctx *ctx, const uint8_t* req, uint8_t* resp,
                const size_t size)
{
    int rv = 0;

    pthread_mutex_lock(&ctx->backend_ctx->oocd_access_mutex);

    rv = jtag_drscan_write(ctx, req, size);
    if (rv) {
        err(ctx, "Writing request to OpenOCD socket failed. rv = %d\n", rv);
        goto unlock_ret;
    }

    rv = jtag_drscan_read(ctx, resp, size);
    if (rv) {
        err(ctx, "Reading response from OpenOCD socket failed. rv = %d\n", rv);
        goto unlock_ret;
    }

unlock_ret:
    pthread_mutex_unlock(&ctx->backend_ctx->oocd_access_mutex);

    return rv;
}

/**
 * Read the response to a JTAG drscan request from OpenOCD
 *
 * This function blocks until the full response has been received.
 *
 * @param[in]  ctx the library context
 * @param[out] resp the response
 * @param[in]  size the expected number of bytes
 * @return 0 on success
 * @return any other value indicates an error
 *
 * @private
 */
int jtag_drscan_read(struct glip_ctx *ctx, uint8_t* resp, const size_t size)
{
    static const size_t buf_initial_size = 2048;
    int rv = 0;

    int oocd_sfd = ctx->backend_ctx->sockadrr;

    unsigned int resp_pos = 0;

    char* buf;
    unsigned int buf_pos = 0;
    size_t buf_size = buf_initial_size * sizeof(char);
    buf = malloc(buf_size);
    assert(buf);

    unsigned int field_start = 0;
    unsigned int field_end = 0;
    while (1) {
        /* grow buffer if needed */
        if (buf_pos >= buf_size) {
            buf_size = grow_buf(buf_size);
            buf = realloc(buf, buf_size);
        }

        /* get new data */
        errno = 0;
        ssize_t n = read(oocd_sfd, buf + buf_pos, buf_size - buf_pos);
        if (n == 0) {
            err(ctx, "No valid data available for reading; connection broken?\n");
            rv = -1;
            goto free_return;
        }
        if (n == -1 && errno != EAGAIN) {
            err(ctx, "Error reading from socket: %s (%d)\n",
                strerror(errno), errno);
            rv = -1;
            goto free_return;
        }

        /* extract complete fields */
        for (int i = 0; i < n; i++) {
            if (buf[buf_pos + i] == ' ' || buf[buf_pos + i] == '\x1a') {
                /* reached end of field, copy to output */
                field_end = buf_pos + i;
                int field_size_chars = field_end - field_start;

                assert(resp_pos + field_size_chars / 2 <= size);

                hexstring_to_binarray_le(&buf[field_start],
                                         field_size_chars,
                                         &resp[resp_pos]);
                resp_pos += field_size_chars / 2;

                field_start = field_end + 1;
            }

            if (buf[buf_pos + i] == '\x1a') {
                rv = 0;
                goto free_return;
            }
        }

        /* remove complete fields from buffer */
        memmove(buf, &buf[field_end], buf_size - field_end);
        buf_pos = buf_pos + n - field_end;
        field_start = field_start - field_end;
    }

free_return:
    free(buf);
    return rv;
}

/**
 * Convert a hexadecimal character string into an array of bytes, using
 * little-endian byte ordering.
 *
 * @param in
 * @param in_size size of the input string. The output will be in_size/2
 * @param out
 * @return
 */
int hexstring_to_binarray_le(char *in, size_t in_size, uint8_t *out)
{
    assert(in_size % 2 == 0);
    unsigned int out_pos = 0;
    for (int c = in_size - 2; c >= 0; c -= 2) {
        out[out_pos] = hexstring_to_byte(&in[c]);
        out_pos++;
    }
    return 0;
}

/**
 * Convert a hexadecimal character string to a single byte
 *
 * @param in hexadecimal string (2 characters)
 * @return output value
 */
uint8_t hexstring_to_byte(char *in)
{
    uint8_t retval;
    char tmp[3];
    tmp[0] = in[0];
    tmp[1] = in[1];
    tmp[2] = '\0';

    errno = 0;
    retval = strtol(tmp, NULL, 16);
    assert(errno == 0);

    return retval;
}

/**
 * Write JTAG drscan request to OpenOCD socket
 *
 * @param ctx the library context
 * @param req the request
 * @param size the size of the request (in bytes)
 * @return 0 on success
 * @return -EOVERFLOW if the command is to long to be sent to OpenOCD
 * @return any other value indicates an error
 *
 * @private
 */
int jtag_drscan_write(struct glip_ctx *ctx, const uint8_t *req,
                      const size_t size)
{
    static const char OOCD_CMDSTART[] = "drscan or1k.fifo ";
    static const char OOCD_CMDEND[] = "\x1a"; /* CTRL-Z == EOF */

    static const size_t OOCD_CMD_MAXLEN = 1*1024*1024;

    /* this is TCL_MAX_LINE in tcl_server.c */
    static const size_t OOCD_MAX_LINESIZE = 4096*1024;

    /*
     * maximum size of a field sent to OpenOCD in bytes. Up to 4 bytes should
     * be supported by any OpenOCD adapter.
     *
     * "For portability, never pass fields which are more than 32 bits long.
     *  Many OpenOCD implementations do not support 64-bit (or larger) integer
     *  values."
     * -- http://openocd.org/doc/html/JTAG-Commands.html
     */
    static const size_t oocd_max_field_size = 4;

    /*
     * Make |buf| large enough to hold a full field in OpenOCD string notation
     * of oocd_max_field_size bytes.
     *
     * The field is formatted as
     *   <SIZE IN BITES> 0x<BYTES IN HEX>
     * This leads to the following formula to calculate the buffer size:
     *   floor(log10(oocd_max_field_size * 8)) + 4 + oocd_max_field_size * 2
     */
    char buf[1024];

    int oocd_sfd = ctx->backend_ctx->sockadrr;

    struct cmdbuf *cmdbuf = cmdbuf_new(OOCD_CMD_MAXLEN);
    assert(cmdbuf);

    cmdbuf_append(cmdbuf, OOCD_CMDSTART, strlen(OOCD_CMDSTART));

    size_t size_sent = 0;
    while (size_sent < size) {
        int field_size = min(size - size_sent, oocd_max_field_size);
        /* start of field: "[field size in bit] 0x" */
        int n = snprintf(buf, sizeof(buf), "%u 0x", field_size * 8);
        assert(n >= 0);
        assert((unsigned int)n < sizeof(buf)); /* oocd_max_field_size is too large */
        cmdbuf_append(cmdbuf, buf, n);

        /* build a little-endian hex character string out of the byte seq. */
        for (int i = 0; i < field_size; i++) {
            n = snprintf(buf, sizeof(buf), "%02x",
                         req[size_sent + (field_size - 1) - i]);
            assert(n == 2);
            cmdbuf_append(cmdbuf, buf, n);
        }
        cmdbuf_append(cmdbuf, " ", 1);

        size_sent += field_size;
    }

    cmdbuf_append(cmdbuf, OOCD_CMDEND, strlen(OOCD_CMDEND));

    /* send full command line to OpenOCD */
    if (cmdbuf->pos > OOCD_MAX_LINESIZE) {
        err(ctx, "TCL command line to large for OpenOCD over TCP "
            "(trying to send %zu chars, max. %zu allowed). Dropped command!\n",
            cmdbuf->pos, OOCD_MAX_LINESIZE);
        return -EOVERFLOW;
    }
    write_b(oocd_sfd, cmdbuf->buf, cmdbuf->pos);

    free(cmdbuf);

    return 0;
}

struct cmdbuf* cmdbuf_new(const size_t buf_size)
{
    struct cmdbuf *b;
    b = malloc(sizeof(struct cmdbuf) + buf_size * sizeof(char));
    if (b == NULL) {
        return NULL;
    }
    b->buf_size = buf_size;
    b->pos = 0;
    return b;
}

void cmdbuf_append(struct cmdbuf *buf, const char *data, const size_t data_size)
{
    assert(buf->pos + data_size <= buf->buf_size);
    memcpy(&buf->buf[buf->pos], data, data_size);
    buf->pos += data_size;
}

void* poll_thread(void *arg)
{
    struct glip_ctx* ctx = (struct glip_ctx*) arg;
    int rv;

    /* discover target configuration */
    discover_targetconf(ctx);

    /*
     * assume 2 byte words and 4 words header/tail; the buffer grows
     * if necessary.
     */
    size_t buf_size = (MAX_PAYLOAD_WORDS + 4) * 2;
    uint8_t *buf_tx = malloc(buf_size);
    uint8_t *buf_rx = malloc(buf_size);

    ssize_t transfer_size;
    wsize_t wr_words, rd_words;
    wsize_t wanted_words;
    wsize_t acked_words, read_words;

    wsize_t min_wanted_words = ctx->backend_ctx->target_buffer_size;
    wanted_words = min_wanted_words;
    while (1) {
        calc_payload_len(ctx, wanted_words, &wr_words, &rd_words);
        do {
            transfer_size = build_jtag_req(ctx, wr_words, rd_words, buf_tx, buf_size);
            if (transfer_size == -ENOMEM) {
                buf_size = grow_buf(buf_size);
                buf_tx = realloc(buf_tx, buf_size);
                buf_rx = realloc(buf_rx, buf_size);
            }
        } while (transfer_size == -ENOMEM);
        if (transfer_size < 0) {
            err(ctx, "Unable to build JTAG request (%zu)\n", transfer_size);
            goto free_return;
        }

        rv = jtag_drscan(ctx, buf_tx, buf_rx, transfer_size);
        if (rv != 0) {
            err(ctx, "Unable to send JTAG command to OpenOCD (%d)\n", rv);
            goto free_return;
        }

        parse_jtag_resp(ctx, buf_rx, transfer_size, &acked_words, &read_words);

        /* adjust payload length according to receiver */
        if (acked_words > 0.9 * wr_words || read_words >=0.9 * rd_words) {
            wanted_words *= 1.5;
        } else if (acked_words <= 0.5 * wr_words && read_words <= 0.5 * rd_words) {
            wanted_words *= 0.3;
        }
        /* don't allow 0xFF..FF inside the WR_REQ_SIZE field */
        wanted_words = min(wanted_words,
                           (1 << (ctx->backend_ctx->target_word_width * 8)) - 2);

        /* ensure min_wanted_words <= wanted_words <= MAX_PAYLOAD_WORDS */
        wanted_words = max(min_wanted_words, wanted_words);
        wanted_words = min(wanted_words, MAX_PAYLOAD_WORDS);
    }

free_return:
    free(buf_tx);
    free(buf_rx);

    return NULL;
}

/**
 * Grow buffer size
 *
 * Growth strategy:
 * - up to 32k double buffer size
 * - then grow linearly by 32k each time
 *
 * @private
 */
size_t grow_buf(size_t current_size)
{
    if (current_size < 32 * 1024) {
        return current_size * 2;
    }
    return current_size + 32 * 1024;
}

/**
 * Calculate the number of payload (data) words in the transfer
 *
 * The transfer size is constrained by the available spaces in the read and
 * write buffers on the host, and by the minimum and maximum number of words
 * to be transmitted.
 *
 * @param[in]  ctx the library context
 * @param[in]  wanted_words
 * @param[in]  max_words
 * @param[out] write_words
 * @param[out] read_words
 * @return the resulting number of words in the transfer
 *
 * @private
 */
wsize_t calc_payload_len(struct glip_ctx *ctx,
                         wsize_t wanted_words,
                         wsize_t *wr_words, wsize_t *rd_words)
{
    wsize_t avail_wr, avail_rd;
    wsize_t payload_len;

    avail_wr = to_words(ctx, cbuf_fill_level(ctx->backend_ctx->write_buf));
    avail_rd = to_words(ctx, cbuf_free_level(ctx->backend_ctx->read_buf));

    payload_len = min(wanted_words, max(avail_wr, avail_rd));

    *wr_words = min(avail_wr, payload_len);
    *rd_words = min(avail_rd, payload_len);

    return payload_len;
}

wsize_t to_words(struct glip_ctx *ctx, size_t bytes)
{
    return bytes / ctx->backend_ctx->target_word_width;
}

size_t to_bytes(struct glip_ctx *ctx, wsize_t words)
{
    return words * ctx->backend_ctx->target_word_width;
}

void copy_to_word(struct glip_ctx *ctx, uint32_t value, uint8_t *dest)
{
    size_t word_width = ctx->backend_ctx->target_word_width;

    switch (word_width) {
    case 4:
        dest[3] = value >> 24;
    case 3:
        dest[2] = value >> 16;
    case 2:
        dest[1] = value >> 8;
    case 1:
        dest[0] = value;
        break;
    default:
        memset(dest, 0, word_width);
        dest[word_width - sizeof(uint32_t)] = value;
    }
}

void copy_from_word(struct glip_ctx *ctx, uint8_t *from, uint32_t *out)
{
    size_t word_width = ctx->backend_ctx->target_word_width;

    size_t cp = min(word_width, sizeof(uint32_t));
    *out = 0;
    memcpy(out, from + (word_width - cp), cp);
}

/**
 *
 * @param ctx
 * @param wr_words
 * @param rd_words
 * @param req
 * @param req_size
 * @return the number valid bytes in @p req, i.e. the number of bytes to be
 *         transferred (> 0)
 * @return -ENOMEM @p req is too small. Reallocate and try again.
 * @return any other negative value indicates an error
 */
int build_jtag_req(struct glip_ctx *ctx, wsize_t wr_words, wsize_t rd_words,
                   uint8_t* req, size_t req_size)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;
    int rv;
    unsigned int word_width = bctx->target_word_width;

    size_t wr_bytes = to_bytes(ctx, wr_words);

    wsize_t payload_len_words = max(wr_words, rd_words);

    wsize_t transfer_len_words = payload_len_words + 4;
    size_t transfer_len_bytes = to_bytes(ctx, transfer_len_words);

    if (transfer_len_bytes > req_size) {
        return -ENOMEM;
    }

    memset(req, 0, transfer_len_bytes);

    /* head */
    copy_to_word(ctx, wr_words, req);
    copy_to_word(ctx, rd_words, req + word_width);

    /* TX data */
    uint8_t *write_buf_data;
    rv = cbuf_peek(bctx->write_buf, &write_buf_data, wr_bytes);
    if (rv < 0) {
        err(ctx, "cbuf_peek() returned error %d\n", rv);
        return -1;
    }
    memcpy(&req[word_width * 2], write_buf_data, wr_bytes);

    /* tail */
    /* it's empty (already set to 0x0000 above) */

    return transfer_len_bytes;
}

void parse_jtag_resp(struct glip_ctx *ctx, uint8_t *resp, size_t resp_size,
                     wsize_t *acked_words, wsize_t *read_words)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;
    unsigned int word_width = bctx->target_word_width;

    copy_from_word(ctx, &resp[resp_size - (word_width * 2)], acked_words);
    copy_from_word(ctx, &resp[resp_size - word_width], read_words);

    cbuf_discard(bctx->write_buf, to_bytes(ctx, *acked_words));
    cbuf_write(bctx->read_buf, &resp[word_width * 2],
               to_bytes(ctx, *read_words));
}

/**
 * Start the OpenOCD process with the configuration for GLIP
 *
 * Run the "openocd" binary (needs to be in the path) and pass it our
 * configuration files. The process output is then monitored for the string
 * "tap/device found", indicating successful startup. If this string does not
 * appear in the output within OPENOCD_STARTUP_TIMEOUT milliseconds, we assume
 * an error has occurred, close the connection and return a negative return
 * code.
 *
 * @param ctx the library context
 * @return 0 on success
 * @return any other return value indicates an error
 *
 * @see close_openocd()
 *
 * @private
 */
int start_openocd(struct glip_ctx *ctx)
{
    const char *oocd_conf_interface;
    const char *oocd_conf_target;

    /*
     * Get the used OpenOCD config files from options.
     *
     * Relative paths are resolved by OpenOCD using its default search path. To
     * this search path we add the directory $PKGDATADIR/backend_jtag/openocd.
     * $PKGDATADIR is set at build time and depends on the installation $PREFIX.
     * Usually, this results in PKGDATADIR=/usr/local/share/glip
     */
    if (glip_option_get_char(ctx, "oocd_conf_interface", &oocd_conf_interface) != 0) {
        err(ctx, "Option 'oocd_conf_interface' not set. Have a look at the \n"
            "'interface' directory of your OpenOCD installation and pass\n"
            "the name of the configuration file for your JTAG adapter like\n"
            "'interface/ftdi/olimex-arm-usb-tiny-h.cfg'.\n");
        return -1;
    }
    if (glip_option_get_char(ctx, "oocd_conf_target", &oocd_conf_target) != 0) {
        dbg(ctx, "Option 'oocd_conf_target' not set. Using default '%s'.\n",
            OPENOCD_CONF_TARGET_DEFAULT);
        oocd_conf_target = OPENOCD_CONF_TARGET_DEFAULT;
    }


    /* assemble search path */
    char oocd_search_path[1024];
    int rv = snprintf(oocd_search_path, sizeof(oocd_search_path),
                      "%s/backend_jtag/openocd", PKGDATADIR);
    assert(rv < (int)sizeof(oocd_search_path));

    /* run OpenOCD */
    char* oocd_args[8];
    oocd_args[0] = "openocd";
    oocd_args[1] = "-s";
    oocd_args[2] = oocd_search_path;
    oocd_args[3] = "-f";
    oocd_args[4] = (char*)oocd_conf_interface;
    oocd_args[5] = "-f";
    oocd_args[6] = (char*)oocd_conf_target;
    oocd_args[7] = NULL;

    pid_t oocd_pid;
    int oocd_stdoutfd;
    oocd_pid = gl_util_popen("openocd", oocd_args, NULL, &oocd_stdoutfd);
    if (oocd_pid < 0) {
        err(ctx, "Unable to start OpenOCD.");
        return -1;
    }
    ctx->backend_ctx->oocd_pid = oocd_pid;

    /* make pipe file descriptor nonblocking */
    rv = gl_util_fd_nonblock(ctx, oocd_stdoutfd);
    if (rv) {
        return -1;
    }

    /* wait for initialization to complete */
    static const char search_string[] = "tap/device found";
    int search_string_found = 0;
    int f = 0;

    struct timespec ts_now, ts_start;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    while (1) {
        /* check if we have run into a timeout */
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        int tspent = (ts_now.tv_sec * 1000 + ts_now.tv_nsec / 1000 / 1000) -
                     (ts_start.tv_sec * 1000 + ts_start.tv_nsec / 1000 / 1000);
        if (tspent >= OPENOCD_STARTUP_TIMEOUT) {
            err(ctx, "The timeout of %d ms was hit (1).\n",
                OPENOCD_STARTUP_TIMEOUT);
            break;
        }

        /* wait for new data from OpenOCD */
        struct timespec max_wait_ts;
        max_wait_ts.tv_sec = (OPENOCD_STARTUP_TIMEOUT - tspent) / 1000;
        max_wait_ts.tv_nsec = ((OPENOCD_STARTUP_TIMEOUT - tspent) -
                               (max_wait_ts.tv_sec * 1000)) * 1000;

        fd_set fset;
        FD_ZERO(&fset);
        FD_SET(oocd_stdoutfd, &fset);
        int nfds = pselect(oocd_stdoutfd + 1, &fset, NULL, NULL, &max_wait_ts, NULL);
        if (nfds == 0) {
            /* timeout hit */
            err(ctx, "The timeout of %d ms was hit (2).\n",
                OPENOCD_STARTUP_TIMEOUT);
            break;
        }
        if (nfds < 0) {
            err(ctx, "An error occurred while waiting for OpenOCD output. "
                "nfds = %d\n", nfds);
            break;
        }

        /* we have data to read */
        char lbuf[4096];
        unsigned int lbuf_pos = 0;
        char c;
        while (read(oocd_stdoutfd, &c, 1) == 1 && c != EOF) {
            /* dump output line-wise for debugging */
            lbuf[lbuf_pos++] = c;
            if (lbuf_pos == 4096 || c == '\n') {
                dbg(ctx, "OOCD: %.*s", lbuf_pos, lbuf);
                lbuf_pos = 0;
            }

            if (c == search_string[f]) {
                f++;

                /* sizeof(search_string) - 1 == strlen(search_string) (\0!) */
                if (f == sizeof(search_string) - 1) {
                    search_string_found = 1;
                    break;
                }
            } else {
                /* only partial match found, start from the beginning */
                f = 0;
            }
        }
        /* dump remaining output */
        if (lbuf_pos != 0) {
            dbg(ctx, "OOCD: %.*s\n", lbuf_pos, lbuf);
        }

        /* done! */
        if (search_string_found) {
            break;
        }
    }

    /*
     * Close the OpenOCD stdout stream.
     * It's not read any more and leaving it open will overflow the buffer and
     * stop all processing, since write() in openocd will block until the
     * stream becomes available again
     */
    close(oocd_stdoutfd);

    if (!search_string_found) {
        err(ctx, "Starting OpenOCD failed.\n"
            "The string '%s', indicating successful startup, did not appear "
            "in the OpenOCD output.\n"
            "Did you load the bitstream to the FPGA and connect all cabled?\n",
            search_string);

        close_openocd(ctx);
        return -1;
    }

    dbg(ctx, "OpenOCD successfully started\n");
    return 0;
}

/**
 * Close the OpenOCD process
 *
 * This function blocks until the openocd process exits properly.
 *
 * @param ctx the library context
 * @return 0 in case of success
 */
int close_openocd(struct glip_ctx *ctx)
{
    if (ctx->backend_ctx->oocd_pid) {
        /* try to send exit command over Tcl interface */
        if (ctx->backend_ctx->sockadrr) {
            uint8_t exit_cmd[] = "exit\x1a";
            write_b(ctx->backend_ctx->sockadrr, exit_cmd, sizeof(exit_cmd));
        }

        gl_util_pclose(ctx->backend_ctx->oocd_pid);
    }
    return 0;
}

/**
 * Blocking version of the write() POSIX function
 *
 * This function wraps the POSIX write() function. It behaves identically, but
 * repeats the write() call until all @p nbyte bytes are written.
 */
ssize_t write_b(int fildes, const void *buf, size_t nbyte)
{
    ssize_t rv;
    size_t nbyte_written = 0;
    do {
        errno = 0;
        rv = write(fildes, (char*)buf + nbyte_written, nbyte - nbyte_written);
        if (rv == -1 && errno != EAGAIN) {
            return -1;
        }
        nbyte_written += rv;
    } while (nbyte_written < nbyte);

    return nbyte_written;
}
