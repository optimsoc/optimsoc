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
 * This header file contains all definitions shared by the library and all of
 * its backends. It needs to be included by all backends.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <libglip.h>

#include <stddef.h>
#include <pthread.h>

#include <config.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

/*
 * Opaque struct containing the backend context, i.e. the state information
 * required by a backend. This struct needs to be declared inside the C file
 * of a backend implementation.
 */
struct glip_backend_ctx;

/**
 * GLIP backend interface
 *
 * All functions in this struct need to be implemented by all backends. In
 * addition, each backend needs to provide an init() function to set up this
 * struct of backend functions.
 */
struct glip_backend_if {
    /**
     * Open the connection to the target
     *
     * @param  ctx          the library context
     * @param  num_channels the number of channels to open
     * @return 0 if the call was successful, or an error code if something went
     *         wrong
     *
     * @see glip_open()
     */
    int (*open)(struct glip_ctx * /* ctx */, unsigned int /* num_channels */);

    /**
     * Close the connection to the target
     *
     * @param  ctx          the library context
     * @return 0 if the call was successful, or an error code if something went
     *         wrong
     *
     * @see glip_close()
     */
    int (*close)(struct glip_ctx * /* ctx */);

    /**
     * Send a reset signal to the target
     *
     * @param  ctx the library context
     * @return 0 if the call was successful, or an error code if something went
     *         wrong
     *
     * @see glip_logic_reset()
     */
    int (*logic_reset)(struct glip_ctx * /* ctx */);

    /**
     * Read from the target device FIFO on a given channel
     *
     * Requests to read \p size bytes of data from channel \p channel into the
     * variable \p data. The number of bytes actually read from the target is
     * stored in the \p size_read output argument.
     *
     * @param[in]  ctx        the library context
     * @param[in]  channel    the channel to read from
     * @param[in]  size       the number of bytes to read
     * @param[out] data       the data read from the target (preallocated)
     * @param[out] size_read  the number of bytes actually read from the target
     * @return     0 if the call was successful, or an error code if something
     *             went wrong
     *
     * @see glip_read()
     */
    int (*read)(struct glip_ctx* /* ctx */, uint32_t /* channel */,
                size_t /* size */, uint8_t* /* data */,
                size_t* /* size_read */);

    /**
     * Blocking read from the target device FIFO on a given channel
     *
     * Requests to read \p size bytes of data from channel \p channel into the
     * variable \p data. The number of bytes actually read from the target is
     * stored in the \p size_read output argument. The function returns if
     * either \p timeout milliseconds (ms) are exceeded or if \p size bytes
     * have been read.
     *
     * @param[in]  ctx        the library context
     * @param[in]  channel    the channel to read from
     * @param[in]  size       the number of bytes to read
     * @param[out] data       the data read from the target (preallocated)
     * @param[out] size_read  the number of bytes actually read from the target
     * @param[in]  timeout    the timeout [ms]
     * @return     0 if the call was successful, or an error code if something
     *             went wrong
     *
     * @see glip_read_b()
     */
    int (*read_b)(struct glip_ctx* /* ctx */, uint32_t /* channel */,
                  size_t /* size */, uint8_t* /* data */,
                  size_t* /* size_read */, unsigned int /* timeout */);

    /**
     * Write data to the target FIFO on a given channel
     *
     * Try to transfer transfer of \p size bytes of \p data to the target FIFO
     * of channel \p channel. The actual number of written bytes is returned as
     * \p size_written.
     *
     * This function behaves like you would expect from a FIFO with no latency
     * guarantees. This function is non-blocking. There are no write
     * acknowledgements; the data is scheduled to be transferred "soon", whereas
     * the definition of "soon" depends on the used backend. The backend is free
     * to do with the data whatever it wants, as long as the following
     * guarantees are obeyed:
     *
     * - \p size_written bytes of \p data will eventually reach the target
     *   device
     * - the data ordering is preserved; the first byte written will be the
     *   first byte read from the FIFO
     *
     * @param[in]  ctx          the library context
     * @param[in]  channel      the channel to write to
     * @param[in]  size         the number of bytes to write (length of data)
     * @param[in]  data         the data to write
     * @param[out] size_written the number of bytes actually written
     * @return     0 if the call was successful, or an error code if something
     *             went wrong
     *
     * @see glip_write()
     */
    int (*write)(struct glip_ctx* /* ctx */, uint32_t /* channel */,
                 size_t /* size */, uint8_t* /* data */,
                 size_t* /*size_written */);

    /**
     * Blocking write of data to the target FIFO on a given channel
     *
     * This function is similar to the non-blocking write, write(), but it
     * blocks until all @p size bytes are scheduled to be transferred, or until
     * the timeout of @p timeout milliseconds (ms) is exceeded.
     *
     * Note that "scheduled to be transferred" does only mean that the data is
     * safely buffered and will be transferred "soon", but *not* that it has
     * been received by the target device.
     *
     * @param[in]  ctx          the library context
     * @param[in]  channel      the channel to write to
     * @param[in]  size         the number of bytes to write (length of data)
     * @param[in]  data         the data to write
     * @param[out] size_written the number of bytes actually written
     * @param[in]  timeout      the amount of time in milliseconds (ms) to wait
     *                          before returning to the caller. Use the value 0
     *                          for an unlimited amount of time.
     * @return     0 if the call was successful, or an error code if something
     *             went wrong
     */
    int (*write_b)(struct glip_ctx* /* ctx */, uint32_t /* channel */,
                   size_t /* size */, uint8_t* /* data */,
                   size_t* /*size_written */, unsigned int /* timeout */);

    /**
     * @see glip_get_fifo_width()
     */
    unsigned int (*get_fifo_width)(struct glip_ctx* /* ctx */);

    /**
     * @see glip_set_fifo_width()
     */
    int (*set_fifo_width)(struct glip_ctx* /* ctx */,
                          unsigned int /* fifo_width_bytes */);

    /**
     * Get the number of supported channels
     *
     * Depending on the backend (and possibly the target device) a different
     * number of channels might be supported.
     *
     * @param ctx the library contexxt
     * @return the number of supported channels
     *
     * @see glip_get_channel_count()
     */
    unsigned int (*get_channel_count)(struct glip_ctx* /* ctx */);
};

/**
 * GLIP context
 *
 * This context is like the "this" variable of an object oriented programming
 * language. It contains all state information, thus eliminating the need for
 * a global state and allowing the library to be used for multiple backends
 * at the same time.
 *
 * This context struct is available to the backends as well as the top layer of
 * the library. All state information that is private to the backend should go
 * into the backend context (@p backend_ctx).
 */
struct glip_ctx {
    /**
     * logging function
     * @see glip_set_log_fn()
     */
    glip_log_fn log_fn;
    /**
     * logging priority
     * @see glip_set_log_priority()
     */
    int log_priority;

    /**
     * caller context
     * @see glip_set_caller_ctx()
     * @see glip_get_caller_ctx()
     */
    void *caller_ctx;

    /** the backend context (opaque outside of the backends) */
    struct glip_backend_ctx *backend_ctx;
    /** functions implementing the backend interface */
    struct glip_backend_if backend_functions;
    /** options passed to the backend */
    struct glip_option *backend_options;
    /** number of elements in @p backend_options */
    unsigned int num_backend_options;
    /**
     * are we connected to a target?
     * @see glip_is_connected()
     */
    bool connected;
    int backend_id;
};


int glip_option_get_uint32(struct glip_ctx *ctx, const char *option_name,
                           uint32_t *out);
int glip_option_get_uint16(struct glip_ctx *ctx, const char *option_name,
                           uint16_t *out);
int glip_option_get_uint8(struct glip_ctx *ctx, const char *option_name,
                          uint8_t *out);
int glip_option_get_char(struct glip_ctx *ctx, const char *option_name,
                         const char **out);


static inline void __attribute__((always_inline, format(printf, 2, 3)))
glip_log_null(struct glip_ctx *ctx, const char *format, ...) {}

#define glip_log_cond(ctx, prio, arg...) \
  do { \
    if (glip_get_log_priority(ctx) >= prio) \
      glip_log(ctx, prio, __FILE__, __LINE__, __FUNCTION__, ## arg); \
  } while (0)

#ifdef LOGGING
#  ifdef DEBUG
#    define dbg(ctx, arg...) glip_log_cond(ctx, LOG_DEBUG, ## arg)
#  else
#    define dbg(ctx, arg...) glip_log_null(ctx, ## arg)
#  endif
#  define info(ctx, arg...) glip_log_cond(ctx, LOG_INFO, ## arg)
#  define err(ctx, arg...) glip_log_cond(ctx, LOG_ERR, ## arg)
#else
#  define dbg(ctx, arg...) glip_log_null(ctx, ## arg)
#  define info(ctx, arg...) glip_log_null(ctx, ## arg)
#  define err(ctx, arg...) glip_log_null(ctx, ## arg)
#endif

void glip_log(struct glip_ctx *ctx,
              int priority, const char *file, int line, const char *fn,
              const char *format, ...)
              __attribute__((format(printf, 6, 7)));

#endif
