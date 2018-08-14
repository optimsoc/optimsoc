/* Copyright 2017 The Open SoC Debug Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <osd/gateway.h>
#include <osd/gateway_glip.h>
#include "osd-private.h"

#include <assert.h>
#include <byteswap.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

/**
 * GLIP gateway context
 */
struct osd_gateway_glip_ctx {
    /** Logging context */
    struct osd_log_ctx *log_ctx;

    /** GLIP context object */
    struct glip_ctx *glip_ctx;

    /** OSD gateway context object */
    struct osd_gateway_ctx *gw_ctx;
};

/**
 * Log handler for GLIP
 */
static void glip_log_handler(struct glip_ctx *ctx, int priority,
                             const char *file, int line, const char *fn,
                             const char *format, va_list args)
{
    int bytes_printed;
    struct osd_log_ctx *osd_log_ctx = glip_get_caller_ctx(ctx);
    assert(osd_log_ctx);

    char *log_str;
    bytes_printed = vasprintf(&log_str, format, args);
    assert(bytes_printed != -1 && log_str);

    // GLIP adds a newline at the end of the log record, which breaks our log
    // formatting. Remove that.
    if (log_str[bytes_printed - 1] == '\n') {
        log_str[bytes_printed - 1] = '\0';
    }

    osd_log(osd_log_ctx, priority, file, line, fn, "glip: %s", log_str);

    free(log_str);
}

/**
 * Read data from the device
 *
 * @param buf a preallocated buffer for the read data
 * @param size_words number of uint16_t words to read from the device
 * @param flags currently unused, set to 0
 *
 * @return the number of uint16_t words read
 * @return -ENOTCONN if the connection was closed during the read
 * @return any other negative value indicates an error
 */
static ssize_t device_read(struct glip_ctx *glip_ctx, uint16_t *buf,
                           size_t size_words, int flags)
{
    int rv;
    ssize_t words_read;
    size_t bytes_read;

    uint16_t *buf_be;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    buf_be = malloc(size_words * sizeof(uint16_t));
    if (!buf_be) {
        return -1;
    }
#else
    buf_be = buf;
#endif

    rv = glip_read_b(glip_ctx, 0, size_words * sizeof(uint16_t),
                     (uint8_t *)buf_be, &bytes_read,
                     0 /* timeout [ms]; 0 == never */);
    if (rv == -ENOTCONN || rv == -ECANCELED) {
        words_read = -ENOTCONN;
        goto free_return;
    } else if (rv != 0) {
        words_read = -1;
        goto free_return;
    }
    words_read = bytes_read / sizeof(uint16_t);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    for (ssize_t w = 0; w < words_read; w++) {
        buf[w] = bswap_16(buf_be[w]);
    }
#endif

free_return: ;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    free(buf_be);
#endif
    return words_read;
}

/**
 * Write to the device
 *
 * @param buf data to write
 * @param size_words size of @p buf in uint16_t words
 * @param flags currently unused, set to 0
 *
 * @return the number of uint16_t words written, if successful
 * @return -ENOTCONN if the device is not connected
 * @return any other negative value indicates an error
 */
static ssize_t device_write(struct glip_ctx *glip_ctx, const uint16_t *buf,
                            size_t size_words, int flags)
{
    size_t bytes_written;
    int rv;

    // GLIP and OSD are big endian, |buf| is in native endianness
    uint16_t *buf_be;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    buf_be = malloc(size_words * sizeof(uint16_t));
    if (!buf_be) {
        return -1;
    }

    for (size_t w = 0; w < size_words; w++) {
        buf_be[w] = bswap_16(buf[w]);
    }
#else
    buf_be = buf;
#endif

    rv = glip_write_b(glip_ctx, 0, size_words * sizeof(uint16_t),
                      (uint8_t *)buf_be, &bytes_written,
                      0 /* timeout [ms]; 0 == never */);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    free(buf_be);
#endif

    if (rv == -ENOTCONN || rv == -ECANCELED) {
        return -ENOTCONN;
    } else if (rv != 0) {
        return -1;
    }

    size_t words_written = bytes_written / sizeof(uint16_t);
    return words_written;
}

/**
 * Initialize GLIP for device communication
 */
static struct glip_ctx* init_glip(struct osd_log_ctx *log_ctx,
                                  const char* glip_backend_name,
                                  const struct glip_option *glip_backend_options,
                                  size_t glip_backend_options_len)
{
    int rv;

    struct glip_ctx *glip_ctx;

    dbg(log_ctx, "Creating GLIP device context for backend %s",
        glip_backend_name);
    rv = glip_new(&glip_ctx, glip_backend_name, glip_backend_options,
                  glip_backend_options_len, &glip_log_handler);

    if (rv < 0) {
        err(log_ctx, "Unable to create new GLIP context (%d).", rv);
        return NULL;
    }

    glip_set_caller_ctx(glip_ctx, log_ctx);
    glip_set_log_priority(glip_ctx, osd_log_get_priority(log_ctx));

    dbg(log_ctx, "Creating GLIP device context created.");

    return glip_ctx;
}

static osd_result packet_read_from_device(struct osd_packet **pkg, void *cb_arg)
{
    osd_result rv;
    ssize_t s_rv;

    struct osd_gateway_glip_ctx *gw_ctx = cb_arg;
    assert(gw_ctx);

    // read packet size, which is transmitted as first word in a DTD
    uint16_t pkg_size_words;
    s_rv = device_read(gw_ctx->glip_ctx, &pkg_size_words, 1, 0);
    if (s_rv == -ENOTCONN) {
        return OSD_ERROR_NOT_CONNECTED;
    } else if (s_rv != 1) {
        err(gw_ctx->log_ctx, "Unable to read packet length from device (%zd).",
            s_rv);
        return OSD_ERROR_FAILURE;
    }

    rv = osd_packet_new(pkg, pkg_size_words);
    assert(OSD_SUCCEEDED(rv));

    // read packet data
    s_rv = device_read(gw_ctx->glip_ctx, (*pkg)->data_raw, pkg_size_words, 0);
    if (s_rv == -ENOTCONN) {
        return OSD_ERROR_NOT_CONNECTED;
    } else if (s_rv != pkg_size_words) {
        err(gw_ctx->log_ctx, "Unable to read packet data from device (%zd).",
            s_rv);
        return OSD_ERROR_FAILURE;
    }

#ifdef DEBUG
    osd_packet_log(*pkg, gw_ctx->log_ctx,
                   "GLIP gateway: Read packet from device.");
#endif

    return OSD_OK;
}

static osd_result packet_write_to_device(const struct osd_packet *pkg,
                                         void *cb_arg)
{
    ssize_t s_rv;

    struct osd_gateway_glip_ctx *gw_ctx = cb_arg;
    assert(gw_ctx);

#ifdef DEBUG
    osd_packet_log(pkg, gw_ctx->log_ctx,
                   "GLIP gateway: Writing packet to device.");
#endif

    uint16_t *pkg_dtd = (uint16_t *)pkg;
    size_t pkg_dtd_size_words = 1 /* len */ + pkg->data_size_words;

    s_rv = device_write(gw_ctx->glip_ctx, pkg_dtd, pkg_dtd_size_words, 0);
    if (s_rv == -ENOTCONN) {
        return OSD_ERROR_NOT_CONNECTED;
    } else if (s_rv < 0) {
        err(gw_ctx->log_ctx, "Device write failed (%zu)", s_rv);
        return OSD_ERROR_FAILURE;
    } else if ((size_t)s_rv != pkg_dtd_size_words) {
        err(gw_ctx->log_ctx,
            "Short write: requested device write of %zu bytes, wrote %zu bytes",
            pkg_dtd_size_words, s_rv);
        return OSD_ERROR_FAILURE;
    }
    return OSD_OK;
}

osd_result osd_gateway_glip_new(struct osd_gateway_glip_ctx **ctx,
                                struct osd_log_ctx *log_ctx,
                                const char *host_controller_address,
                                uint16_t device_subnet_addr,
                                const char *glip_backend_name,
                                const struct glip_option *glip_backend_options,
                                size_t glip_backend_options_len)
{
    osd_result rv;

    struct osd_gateway_glip_ctx *c =
        calloc(1, sizeof(struct osd_gateway_glip_ctx));
    assert(c);

    c->log_ctx = log_ctx;

    c->glip_ctx = init_glip(log_ctx, glip_backend_name, glip_backend_options,
                            glip_backend_options_len);
    if (!c->glip_ctx) {
        err(log_ctx, "Unable to initialize GLIP");
        return OSD_ERROR_FAILURE;
    }

    dbg(log_ctx, "Creating gateway context.");
    rv = osd_gateway_new(&c->gw_ctx, log_ctx, host_controller_address,
                         device_subnet_addr, packet_read_from_device,
                         packet_write_to_device, (void *)c);
    if (OSD_FAILED(rv)) {
        return rv;
    }
    assert(c->gw_ctx);

    *ctx = c;

    return OSD_OK;
}

osd_result osd_gateway_glip_connect(struct osd_gateway_glip_ctx *ctx)
{
    int glip_rv;
    osd_result rv;

    dbg(ctx->log_ctx, "Connecting to device through GLIP");
    glip_rv = glip_open(ctx->glip_ctx, 1);
    if (glip_rv < 0) {
        err(ctx->log_ctx, "Unable to open connection to device (%d)", glip_rv);
        return OSD_ERROR_CONNECTION_FAILED;
    }

    // Warn about (possibly) wrong channel width
    unsigned int glip_fifo_width = glip_get_fifo_width(ctx->glip_ctx);
    if (glip_fifo_width != 0 && glip_fifo_width != 2) {
        info(ctx->log_ctx, "FIFO width of GLIP channel must be 16 bit, "
             "not %d bit. Continuing anyway.", glip_fifo_width * 8);
    }

    dbg(ctx->log_ctx, "Connected to device.");

    // connect to host controller
    dbg(ctx->log_ctx, "Connecting to host controller");
    rv = osd_gateway_connect(ctx->gw_ctx);
    if (OSD_FAILED(rv)) {
        err(ctx->log_ctx, "Unable to connect to host controller (%d).", rv);
        return rv;
    }
    dbg(ctx->log_ctx, "Connected to host controller");

    return OSD_OK;
}

osd_result osd_gateway_glip_disconnect(struct osd_gateway_glip_ctx *ctx)
{
    int glip_rv;
    osd_result rv;

    // disconnect from device
    glip_rv = glip_close(ctx->glip_ctx);
    if (glip_rv != 0) {
        err(ctx->log_ctx, "Unable to close device connection. (%d)", glip_rv);
    }

    // disconnect gateway from host controller and from device
    rv = osd_gateway_disconnect(ctx->gw_ctx);
    if (OSD_FAILED(rv)) {
        err(ctx->log_ctx, "Unable to disconnect from host controller (%d)", rv);
        return rv;
    }

    return OSD_OK;
}

void osd_gateway_glip_free(struct osd_gateway_glip_ctx **ctx_p)
{
    assert(ctx_p);
    struct osd_gateway_glip_ctx *ctx = *ctx_p;
    if (!ctx) {
        return;
    }

    osd_gateway_free(&ctx->gw_ctx);
    glip_free(ctx->glip_ctx);

    free(ctx);
    ctx_p = NULL;
}

bool osd_gateway_glip_is_connected(struct osd_gateway_glip_ctx *ctx)
{
    return osd_gateway_is_connected(ctx->gw_ctx) &&
           glip_is_connected(ctx->glip_ctx);
}

struct osd_gateway_transfer_stats*
osd_gateway_glip_get_transfer_stats(struct osd_gateway_glip_ctx *ctx)
{
    return osd_gateway_get_transfer_stats(ctx->gw_ctx);
}
