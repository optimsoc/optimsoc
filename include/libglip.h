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

#ifndef __LIBGLIP_H__
#define __LIBGLIP_H__

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque context object
 *
 * This object contains all state information.
 */
struct glip_ctx;

/**
 * Logging function template
 *
 * Implement a function with this signature and pass it to glip_set_log_fn()
 * if you want to implement custom logging.
 *
 * @ingroup log
 */
typedef void (*glip_log_fn)(struct glip_ctx *ctx,
                            int priority, const char *file,
                            int line, const char *fn,
                            const char *format, va_list args);

/**
 * A single option, expressed as a key/value pair
 *
 * @ingroup backend
 */
struct glip_option {
    /** option name */
    char *name;
    /** option value */
    char *value;
};

/* error codes */
#define GLIP_EUNKNOWNBACKEND 1 /**< unknown backend */
#define GLIP_ENOINIT         2 /**< not initialized */
#define GLIP_EFEATURE        3 /**< feature not supported */

/**
 * GLIP API version
 *
 * @ingroup utilities
 */
struct glip_version {
    /** major version */
    const uint16_t major;

    /** minor version */
    const uint16_t minor;

    /** micro version */
    const uint16_t micro;

    /**
     * suffix string, e.g. for release candidates ("-rc4") and development
     * versions ("-dev")
     */
    const char *suffix;
};

const struct glip_version * glip_get_version(void);

int glip_new(struct glip_ctx **ctx, const char* backend_name,
             const struct glip_option options[], size_t num_options,
             glip_log_fn log_fn);
int glip_free(struct glip_ctx *ctx);

int glip_open(struct glip_ctx *ctx, unsigned int num_channels);
int glip_close(struct glip_ctx *ctx);
bool glip_is_connected(struct glip_ctx *ctx);

int glip_logic_reset(struct glip_ctx *ctx);

int glip_write(struct glip_ctx *ctx, uint32_t channel, size_t size,
               uint8_t *data, size_t *size_written);
int glip_write_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                 uint8_t *data, size_t *size_written, unsigned int timeout);
int glip_read(struct glip_ctx *ctx, uint32_t channel, size_t size,
              uint8_t *data, size_t *size_read);
int glip_read_b(struct glip_ctx *ctx, uint32_t channel, size_t size,
                uint8_t *data, size_t *size_read, unsigned int timeout);

void glip_set_log_fn(struct glip_ctx *ctx, glip_log_fn log_fn);
int glip_get_log_priority(struct glip_ctx *ctx);
void glip_set_log_priority(struct glip_ctx *ctx, int priority);

unsigned int glip_get_fifo_width(struct glip_ctx *ctx);
int glip_set_fifo_width(struct glip_ctx *ctx, unsigned int fifo_width_bytes);
unsigned int glip_get_channel_count(struct glip_ctx *ctx);

int glip_get_backends(const char ***name, size_t *count);

void glip_set_caller_ctx(struct glip_ctx *ctx, void *caller_ctx);
void* glip_get_caller_ctx(struct glip_ctx *ctx);

int glip_parse_option_string(const char* str, struct glip_option* options[],
                             size_t *num_options);


#ifdef __cplusplus
}
#endif

#endif /* __LIBGLIP_H__ */
