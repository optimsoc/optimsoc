/* Copyright (c) 2015-2016 by the author(s)
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
 * a PC. This is the UART backend.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

#include "glip-protected.h"

#include "backend_uart.h"
#include "cbuf.h"
#include "util.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <asm/ioctls.h>

#include <sys/time.h>
#include <stdio.h>

/* Forward declarations of local helpers */
/**
 * Check if time is reached
 *
 * @private
 * @param start Start time
 * @param ms Milliseconds to check
 * @return -ETIMEDOUT if time reached, 0 otherwise
 */
static int check_timeout(struct timeval *start, unsigned long ms);

/**
 * Blocking read from terminal
 *
 * @private
 * @param fd File to read from
 * @param buffer Buffer to read to
 * @param size Size to read
 * @param size_read Actual read (only if timeout or error)
 * @param timeout Timeout to use, 0 for none
 * @return 0 if success, -ETIMEDOUT on timeout, -1 else
 */
static int read_blocking(int fd, uint8_t *buffer, size_t size,
                         size_t *size_read, unsigned int timeout);

/**
 * Blocking write to terminal
 *
 * @private
 * @param fd File to write to
 * @param buffer Buffer to write from
 * @param size Size to write
 * @param size_write Actual written (only if timeout or error)
 * @param timeout Timeout to use, 0 for none
 * @return 0 if success, -ETIMEDOUT on timeout, -1 else
 */
static int write_blocking(int fd, uint8_t *buffer, size_t size,
                          size_t *size_written, unsigned int timeout);

/**
 * Reset the user logic (ctrl_logic_rst)
 *
 * @private
 * Sets the user logic reset register
 *
 * @param ctx GLIP context
 * @param state Value (binary) to set the register to
 * @return Always returns 0
 */
static int reset_logic(struct glip_ctx *ctx, uint8_t state);

/**
 * Reset the communication logic (com_rst)
 *
 * @private
 * Sets the communication logic register
 *
 * @param ctx GLIP context
 * @param state Value (binary) to set the register to
 * @return Always returns 0
 */
static int reset_com(struct glip_ctx *ctx, uint8_t state);

/**
 * Update a received debt value
 *
 * @private
 * Debt is the credit the logic gives us. Extracts this new tranche
 * from the words on the line.
 *
 * @param ctx Context
 * @param first First received word of credit message
 * @param second Second received word of credit message
 */
static void update_debt(struct glip_backend_ctx* ctx, uint8_t first,
                        uint8_t second);

/**
 * Inform the target about the number of bytes we can receive
 *
 * @private
 *
 * @param ctx Context
 */
static int update_credit(struct glip_backend_ctx* ctx);

/**
 * Reset the backend
 *
 * @private
 * Resets the buffers, counters etc.
 *
 * @param ctx GLIP context
 * @return 0 on success, -1 otherwise
 */
static int reset(struct glip_ctx* ctx);

/**
 * Communication (POSIX) thread function
 *
 * @param ctx_void GLIP context
 * @return Ignored
 */
static void* thread_func(void *ctx_void);

/**
 * Parses an incoming buffer and filters credits
 *
 * Filter and process credits, write data otherwise
 *
 * @param ctx Context
 * @param buffer Buffer to parse
 * @param size Size of the buffer
 */
void parse_buffer(struct glip_backend_ctx *ctx, uint8_t *buffer,
                  size_t size);

/** Maximum tranche we can give in a message */
static const size_t UART_MAX_TRANCHE = 0x3fff;

/** Temporary buffer size */
static const size_t TMP_BUFFER_SIZE = 256;

/** Size in bytes of the read/write buffer */
static const size_t UART_BUF_SIZE = 32 * 1024; // bytes

/**
 * GLIP backend context for the UART backend
 */
struct glip_backend_ctx {
    char *device; /**< Device name */
    int fd; /**< Terminal file */
    uint32_t speed; /**< Baud rate */
    unsigned int fifo_width; /**< Width of the FIFO on the target in bytes */

    pthread_t thread; /**< Thread instance */

    struct cbuf *input_buffer; /**< Input buffer */
    struct cbuf *output_buffer; /**< Output buffer */

    size_t debt; /**< Current debt (what we can send) */
    size_t credit; /**< Current credit (what logic can send) */

    volatile int reset_request; /**< Request a logic reset */
    volatile int term_request; /**< Request a termination */
};

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
int gb_uart_new(struct glip_ctx *ctx)
{
    /* Allocate (zero-initialized) memory for our context */
    struct glip_backend_ctx *c = calloc(1, sizeof(struct glip_backend_ctx));
    if (!c) {
        return -1;
    }

    /* Register functions for this backend */
    ctx->backend_functions.open = gb_uart_open;
    ctx->backend_functions.close = gb_uart_close;
    ctx->backend_functions.logic_reset = gb_uart_logic_reset;
    ctx->backend_functions.read = gb_uart_read;
    ctx->backend_functions.read_b = gb_uart_read_b;
    ctx->backend_functions.write = gb_uart_write;
    ctx->backend_functions.write_b = gb_uart_write_b;
    ctx->backend_functions.get_fifo_width = gb_uart_get_fifo_width;
    ctx->backend_functions.set_fifo_width = gb_uart_set_fifo_width;
    ctx->backend_functions.get_channel_count = gb_uart_get_channel_count;

    /* Initially the FIFO width is unknown */
    c->fifo_width = 0;

    ctx->backend_ctx = c;

    /* Set the local buffer sizes and initialize */
    if (cbuf_init(&c->input_buffer, UART_BUF_SIZE) != 0) {
        return -1;
    }

    if (cbuf_init(&c->output_buffer, UART_BUF_SIZE) != 0) {
        return -1;
    }

    return 0;
}

/**
 * Destruct the backend
 *
 * @see glip_free()
 */
void gb_uart_free(struct glip_ctx *ctx)
{
    cbuf_free(ctx->backend_ctx->output_buffer);
    cbuf_free(ctx->backend_ctx->input_buffer);
    free(ctx->backend_ctx);
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
int gb_uart_open(struct glip_ctx *ctx, unsigned int num_channels)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;

    /* Initialize the session variables */
    bctx->term_request = 0;
    bctx->reset_request = 0;

    if (num_channels != 1) {
        err(ctx, "Channel number must be 1!\n");
        return -1;
    }

    /* Extract parameters */
    if (glip_option_get_char(ctx, "device", (const char**) &bctx->device) != 0) {
        bctx->device = "/dev/ttyUSB0";
    }

    if (glip_option_get_uint32(ctx, "speed", &bctx->speed) != 0) {
        bctx->speed = 115200;
    }
    dbg(ctx, "Connecting to device %s using %d baud\n", bctx->device, bctx->speed);

    /* Open the device, we use the "new" (2006) style of handling the serial
     * interface.
     */
    bctx->fd = open(bctx->device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (bctx->fd < 0) {
        err(ctx, "Cannot open device %s\n", bctx->device);
        return -1;
    }

    /* Get the attributes of the terminal to manipulate them */
    struct termios2 tty;
    if (ioctl(bctx->fd, TCGETS2, &tty) != 0) {
        err(ctx, "Cannot get device attributes\n");
        return -1;
    }

    /* We always set a custom baud rate */
    tty.c_cflag &= ~CBAUD;
    tty.c_cflag |= BOTHER;
    tty.c_ispeed = bctx->speed;
    tty.c_ospeed = bctx->speed;

    /* 8N1 */
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    /* Hardware flow control */
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag |= CRTSCTS;

    /* read doesn't block */
    tty.c_cc[VMIN]  = 0;
    /* 0.5 seconds read timeout */
    tty.c_cc[VTIME] = 5;

    /* Raw */
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* No input handling */
    tty.c_iflag = 0;
    /* No output handling */
    tty.c_oflag = 0;

    /* Write the changed attributes */
    if (ioctl(bctx->fd, TCSETS2, &tty) != 0) {
        err(ctx, "Cannot set attributes\n");
        return -1;
    }

    /* @todo
     * When the default or user-defined speed does not work, we try
     * to autodetect the line speed. We try from a given set of
     * speeds and take the first one that detects. Unfortunately, we
     * can still get false positives as the match pattern is 0xfe.
     * This pattern is not so easy to distinguish as it only has one
     *  low bit in the byte. */
    int autodetect = 0; /* We entered autodetect */
    /* Options to autodetect */
    int autodetect_candidates[] = { 4000000, 3000000, 2000000, 1000000,
                                    500000, 230400, 115200, 57600, 38400,
                                    19200, 9600, 0 };
    /* Current autodetect (from autodetect_candidates) */
    int *autodetect_try = 0;
    /* Indicate if we successfully connected */
    int success = 0;

    do {
        if (autodetect) {
            /* If we are in autodetecting, adopt speed */
            dbg(ctx, "Try speed: %d\n", *autodetect_try);

            if (ioctl(bctx->fd, TCGETS2, &tty) != 0) {
                err(ctx, "Cannot get device attributes\n");
                return -1;
            }

            bctx->speed = *autodetect_try;

            tty.c_ispeed = bctx->speed;
            tty.c_ospeed = bctx->speed;

            if (ioctl(bctx->fd, TCSETS2, &tty) != 0) {
                err(ctx, "Cannot set attributes\n");
                return -1;
            }
        }

        dbg(ctx, "Trying to establish connection ...\n");

        /* The ramp up sequence is */
        /* - Set com_rst to high (flush buffers) */
        int rv;
        rv = reset_com(ctx, 1);

        /* - Wait for one millisecond to be sure it is in there */
        usleep(1000);

        /* - Drain all remaining in intermediate buffers */
        do {
            uint8_t buffer[128];
            rv = read(bctx->fd, buffer, 128);
        } while (rv > 0);

        /* - De-assert reset */
        rv = reset_com(ctx, 0);

        /* - Read the debt
         *   We do this just to check the read and if we have detected
         *   the correct speed */
        {
            dbg(ctx, "Reading debt from target\n");
            uint8_t debt[2];
            size_t size_read;

            /* Do a blocking read with 1s timeout to settle */
            rv = read_blocking(bctx->fd, debt, 2, &size_read, 1000);
            if (rv == -ETIMEDOUT) {
                /* Timeout means we did not read */
                success = 0;
            } else if ((debt[0] != 0xfe) && (debt[1] != 0xfe)) {
                /* This is we neither had the credit message as first
                 * nor as second word. We check the second as we
                 * observed a leading garbage word on some platforms
                 * with fresh bitstreams */
                success = 0;
            } else {
                /* Else it seems we have the right speed */
                success = 1;
            }
        }

        if (!success) {
            /* If the ramp up was not successfull */
            if (!autodetect) {
                /* Activate autodetect if we were not in it */
                err(ctx, "Given speed %d did not work with device. "
                    "Try autodetect..\n", bctx->speed);
                autodetect = 1;
                autodetect_try = autodetect_candidates;
                continue;
            } else {
                /* Increment autodetect otherwise or abort if we
                 checked all */
                autodetect_try++;
                if (*autodetect_try == 0) {
                    err(ctx, "Could not autodetect baud rate.\n");
                    return -1;
                }
            }
        }
    } while (!success);
    dbg(ctx, "Device connection established.\n");

    /* Reset the communication logic, state and data buffers */
    reset(ctx);

    /* Start the communication thread. From here on all read and write
     * operations go to the circular buffers, and the thread handles
     * the device interface. */
    pthread_create(&bctx->thread, 0, thread_func, ctx);

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
int gb_uart_close(struct glip_ctx *ctx)
{
    void* rv;
    struct glip_backend_ctx *bctx = ctx->backend_ctx;

    /* Send termination request to thread */
    bctx->term_request = 1;

    pthread_cancel(bctx->thread);
    pthread_join(bctx->thread, &rv);

    close(bctx->fd);

    return 0;
}

/**
 * Reset the logic on the target
 * *
 * @see glip_logic_reset()
 */
int gb_uart_logic_reset(struct glip_ctx *ctx)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;

    assert(bctx->reset_request == 0);

    /* Trigger request in thread */
    bctx->reset_request = 1;

    /* Wait until completed */
    while (bctx->reset_request == 1) {}

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
int gb_uart_read(struct glip_ctx *ctx, uint32_t channel, size_t size,
                 uint8_t *data, size_t *size_read)
{
    if (channel > 0) {
        return -1;
    }
    return gb_util_cbuf_read(ctx->backend_ctx->input_buffer, size, data,
                             size_read);
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
int gb_uart_read_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                   uint8_t *data, size_t *size_read, unsigned int timeout)
{
    if (channel > 0) {
        return -1;
    }
    return gb_util_cbuf_read_b(ctx->backend_ctx->input_buffer, size, data,
                               size_read, timeout);
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
int gb_uart_write(struct glip_ctx *ctx, uint32_t channel, size_t size,
                  uint8_t *data, size_t *size_written)
{
    if (channel > 0) {
        return -1;
    }

    return gb_util_cbuf_write(ctx->backend_ctx->output_buffer, size, data,
                              size_written);
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
int gb_uart_write_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                    uint8_t *data, size_t *size_written, unsigned int timeout)
{
    if (channel > 0) {
        return -1;
    }

    return gb_util_cbuf_write_b(ctx->backend_ctx->output_buffer, size, data,
                                size_written, timeout);
}

/**
 * Get the number of supported channels by this backend
 *
 * @param  ctx the library context
 * @return always 1
 *
 * @see glip_get_channel_count()
 */
unsigned int gb_uart_get_channel_count(struct glip_ctx *ctx)
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
unsigned int gb_uart_get_fifo_width(struct glip_ctx *ctx)
{
    return ctx->backend_ctx->fifo_width;
}

/**
 * Set the width of the FIFO on the target side in bytes
 *
 * The UART backend has no way to auto-detect the WIDTH parameter as its used
 * on the glip_uart_toplevel on the target. API users therefore must set the
 * width manually.
 */
int gb_uart_set_fifo_width(struct glip_ctx *ctx, unsigned int fifo_width_bytes)
{
    if (fifo_width_bytes != 1 && fifo_width_bytes != 2) {
        err(ctx, "The UART backend supports only 1 and 2 byte wide FIFOs.\n");
        return -1;
    }
    ctx->backend_ctx->fifo_width = fifo_width_bytes;
    return 0;
}

static int reset_logic(struct glip_ctx *ctx, uint8_t state)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;
    uint8_t reset[2];
    size_t written;

    reset[0] = 0xfe;
    reset[1] = ((state & 0x1) << 1) | 0x81;

    dbg(ctx, "Setting ctrl_logic_rst to %d\n", state);
    return write_blocking(bctx->fd, reset, 2, &written, 0);
}

static int reset_com(struct glip_ctx *ctx, uint8_t state)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;
    uint8_t reset[2];
    size_t written;

    reset[0] = 0xfe;
    reset[1] = ((state & 0x1) << 1) | 0x85;

    dbg(ctx, "Setting com_rst to %d\n", state);
    return write_blocking(bctx->fd, reset, 2, &written, 0);
}

void parse_buffer(struct glip_backend_ctx *ctx, uint8_t *buffer,
                  size_t size)
{
    size_t actual, i;
    int rv;

    for (i = 0; i < size; i++) {
        if (buffer[i] == 0xfe) {
            /* Check if next item is already in buffer */
            if ((i + 1) < size) {
                if (buffer[i+1] == 0xfe) {
                    /* If this is the data word, write it */
                    assert(ctx->credit > 0);
                    rv = cbuf_write(ctx->input_buffer, &buffer[i], 1);
                    assert(rv == 0);
                    ctx->credit--;
                } else {
                    /* This is the first of the credit message
                     * Check if the next is also in buffer */
                    if ((i + 2) < size) {
                        update_debt(ctx, buffer[i+1], buffer[i+2]);
                        /* Increment the counter for the second extra word */
                        i++;
                    } else {
                        /* We have reached the end of the buffer, it is
                         * safe to use the begin of buffer for the credit */
                        rv = read_blocking(ctx->fd, buffer, 1, &actual, 0);
                        assert(rv == 0);
                        update_debt(ctx, buffer[i+1], buffer[0]);
                    }
                }
                /* Increment the counter for the first extra word */
                i++;
            } else {
                /* If the next item was not in buffer, read another one,
                 * we can now reuse the buffer */

                /* Make sure that the target has enough credits to send the
                 * second data item. */
                while (ctx->credit < 2) {
                    usleep(10);
                    update_credit(ctx);
                }

                rv = read_blocking(ctx->fd, buffer, 1, &actual, 0);
                assert(rv == 0);

                if (buffer[0] == 0xfe) {
                    /* That was the data word, write it */
                    assert(ctx->credit > 0);
                    rv = cbuf_write(ctx->input_buffer, &buffer[0], 1);
                    assert(rv == 0);
                    ctx->credit--;
                } else {
                    /* We received a credit, read the next */
                    rv = read_blocking(ctx->fd, &buffer[1], 1, &actual, 0);
                    assert(rv == 0);
                    update_debt(ctx, buffer[0], buffer[1]);
                }
            }
        } else {
            /* This is a data word */
            assert(ctx->credit > 0);
            rv = cbuf_write(ctx->input_buffer, &buffer[i], 1);
            assert(rv == 0);
            ctx->credit--;
        }
    }
}

static void* thread_func(void *ctx_void)
{
    struct glip_ctx *ctx = ctx_void;
    struct glip_backend_ctx* bctx = ctx->backend_ctx;

    uint8_t buffer[TMP_BUFFER_SIZE];
    size_t avail, actual;
    int rv;

    while (1) {
        /* Check for reset */
        if (bctx->reset_request == 1) {
            rv = reset_logic(ctx, 1);
            assert(rv == 0);

            rv = reset_logic(ctx, 0);
            assert(rv == 0);
            bctx->reset_request = 0;
        }

        if (bctx->term_request) {
            /* Termination requested from user */
            break;
        }

        /* Read */
        avail = cbuf_free_level(bctx->input_buffer);
        if (avail == 0) {
            /* We are only expecting a credit message now
             * Check if there is one */
            rv = read(bctx->fd, buffer, 1);
            if (rv == 1) {
                assert(buffer[0] == 0xfe);
                rv = read_blocking(bctx->fd, buffer, 2, &actual, 0);
                assert(rv == 0);
                assert(actual == 2);
                update_debt(bctx, buffer[0], buffer[1]);
            }
        } else {
            /* Read a chunk of data if the circular buffer can accept it */
            size_t size = min(avail, TMP_BUFFER_SIZE);
            rv = read(bctx->fd, buffer, size);
            if (rv > 0) {
                parse_buffer(bctx, buffer, rv);
            }
        }

        /* Write */
        if (bctx->debt > 0) {
            size_t size = min(bctx->debt, TMP_BUFFER_SIZE);
            avail = cbuf_fill_level(bctx->output_buffer);
            if (avail > 0) {
                /* If there is data to be transfered, take the smallest
                 * number of available words, the debt size and the
                 * size of the temporary buffer as transfer size */
                size = min(avail, size);

                /* Read the words */
                assert(cbuf_read(bctx->output_buffer, buffer, size) != -EINVAL);

                for (size_t i = 0; i < size; i++) {
                    /* Write each word */
                    rv = write_blocking(bctx->fd, &buffer[i], 1, &actual, 0);
                    assert(rv == 0);
                    assert(actual == 1);
                    if (buffer[i] == 0xfe) {
                        /* .. and repeat the marker word 0xfe */
                        rv = write_blocking(bctx->fd, &buffer[i], 1, &actual, 0);
                        assert(rv == 0);
                        assert(actual == 1);
                    }
                }
                bctx->debt -= size;
            }
        }

        /* Update credit if necessary */
        if (bctx->credit < UART_MAX_TRANCHE) {
            update_credit(bctx);
        }
    }

    return 0;
}

static int read_blocking(int fd, uint8_t *buffer, size_t size,
                         size_t *size_read, unsigned int timeout)
{
    struct timeval tval_start;
    int rv;

    *size_read = 0;

    if (timeout > 0) {
        gettimeofday(&tval_start, NULL);
    }

    do {
        /* Try to read as many as we still have left to read */
        rv = read(fd, &buffer[*size_read], size - *size_read);

        if (rv >= 0) {
            /* Update number of read */
            *size_read += rv;
        }

        if ((rv == -1) && (errno != EAGAIN)) {
            /* We tolerate EAGAIN errors, but cannot tolerate others */
            return -1;
        }

        if (timeout > 0) {
            /* Check if the timeout has occured */
            if (check_timeout(&tval_start, timeout)) {
                return -ETIMEDOUT;
            }
        }
    } while(*size_read != size);

    return 0;
}

static int write_blocking(int fd, uint8_t *buffer, size_t size,
                          size_t *size_written, unsigned int timeout)
{
    struct timeval tval_start;
    int rv;

    *size_written = 0;

    if (timeout > 0) {
        gettimeofday(&tval_start, NULL);
    }

    do {
        /* Write as many as possible from remaining */
        rv = write(fd, &buffer[*size_written], size - *size_written);

        if (rv >= 0) {
            /* Update number of actually written */
            *size_written += rv;
        }

        if ((rv == -1) && (errno != EAGAIN)) {
            /* Cannot tolerate errors other then EAGAIN */
            return -1;
        }

        if (timeout > 0) {
            /* Check for a timeout */
            if (check_timeout(&tval_start, timeout)) {
                return -ETIMEDOUT;
            }
        }
    } while (*size_written != size);

    return 0;
}

static int check_timeout(struct timeval *start, unsigned long ms)
{
    struct timeval tval_current, tval_diff;
    gettimeofday(&tval_current, NULL);

    /* Calculate the difference */
    tval_diff.tv_sec = tval_current.tv_sec - start->tv_sec;
    tval_diff.tv_usec = tval_current.tv_usec - start->tv_usec;

    /* Microsecond overflows increase the second */
    if (tval_current.tv_usec < start->tv_usec) {
        tval_diff.tv_sec += 1;
        tval_diff.tv_usec += 1000000;
    }

    /* Check if we reached the timeout */
    if ((tval_diff.tv_sec * 1000000 + tval_diff.tv_usec)
            > ((signed) ms * 1000)) {
        return -ETIMEDOUT;
    }

    return 0;
}

static void update_debt(struct glip_backend_ctx* ctx, uint8_t first,
                        uint8_t second)
{
    /* Assemble new debt tranche from the two control datagrams */
    ctx->debt += (((first >> 1) & 0x7f) << 8) | second;
}

static int update_credit(struct glip_backend_ctx* ctx)
{
    uint8_t credit[3];
    size_t written;
    size_t target_credit_add;

    /* calculate how many credits we can give to the target */
    target_credit_add = min(cbuf_free_level(ctx->input_buffer) - ctx->credit,
                            UART_MAX_TRANCHE);
    if (target_credit_add == 0) {
        return -1;
    }

    /* Assemble the message datagrams */
    credit[0] = 0xfe;
    credit[1] = 0x1 | ((target_credit_add >> 8) << 1);
    credit[2] = target_credit_add & 0xff;

    if (write_blocking(ctx->fd, credit, 3, &written, 0) != 0) {
        return -1;
    }

    /* update local representation of the credit counter */
    ctx->credit += target_credit_add;

    return 0;
}

static int reset(struct glip_ctx* ctx)
{
    struct glip_backend_ctx *bctx = ctx->backend_ctx;
    int rv;

    /* Assert reset */
    rv = reset_com(ctx, 1);
    if (rv != 0) {
        return -1;
    }

    /* Drain the interface */
    uint8_t buffer[16];
    do {
        rv = read(bctx->fd, buffer, 16);
    } while (rv > 0);

    /* De-assert reset */
    rv = reset_com(ctx, 0);
    if (rv != 0) {
        return -1;
    }

    /* Clear the buffers */
    rv = cbuf_discard(bctx->input_buffer, cbuf_fill_level(bctx->input_buffer));
    assert(rv == 0);

    rv = cbuf_discard(bctx->output_buffer, cbuf_fill_level(bctx->output_buffer));
    assert(rv == 0);

    /* Reset values to defaults */
    bctx->debt = 0;

    /* Send our initial credit tranche to the logic */
    update_credit(bctx);

    /* Read the debt from the logic */
    uint8_t debt[3];
    size_t read;
    rv = read_blocking(bctx->fd, debt, 3, &read, 0);
    assert(rv == 0);

    /* Strangely, the UART sometimes returns a trash byte on the first
     * access after the bitstream was loaded. Ignore this. */
    if (debt[0] != 0xfe) {
        debt[0] = debt[1];
        debt[1] = debt[2];

        rv = read_blocking(bctx->fd, &debt[2], 1, &read, 0);
        assert(rv == 0);
    }

    assert(debt[0] == 0xfe);

    update_debt(bctx, debt[1], debt[2]);

    return 0;
}
