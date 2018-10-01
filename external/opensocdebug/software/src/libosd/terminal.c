/* Copyright 2018 The Open SoC Debug Project
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

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include <osd/cl_dem_uart.h>
#include <osd/osd.h>
#include <osd/terminal.h>
#include "osd-private.h"

/**
 * Information about an active terminal
 */
struct osd_terminal_ctx {
    struct osd_hostmod_ctx *hostmod_ctx;
    struct osd_log_ctx *log_ctx;
    struct osd_dem_uart_desc *dem_uart_desc;
    struct osd_dem_uart_event_handler dem_uart_event_handler;

    /** Thread responsible for transmitting data to the DEM-UART */
    pthread_t tx_thread;

    /** Absolute path of the created device file */
    char *pts_path;

    /** FD used for all the communication, it is thread-safe */
    int masterfd;

    /** DI-address of the DEM-UART module we should connect to */
    uint16_t dem_uart_di_addr;

    /** Status of this terminal, read-only for the tx_thread */
    volatile bool running;
};

static void handle_rx_data(void *arg, const char *str, size_t len)
{
    struct osd_terminal_ctx *ctx = arg;

    size_t written = 0;
    int ret = 0;
    while (written < len) {
        ret = write(ctx->masterfd, str + written, len - written);

        if (ret == -1) {
            err(ctx->log_ctx, "Failed to write() to device file: %s",
                strerror(errno));
            return;
        }

        written += ret;
    }
}

static void *terminal_tx_thread(void *arg)
{
    struct osd_terminal_ctx *ctx = arg;
    osd_result rv;

    struct pollfd fds;
    fds.fd = ctx->masterfd;
    fds.events = POLLIN;

    int ret;
    char buf[256];

    while (ctx->running) {
        // We use a timeout here so that ctx->running is always checked
        ret = poll(&fds, 1, 2000);
        switch (ret) {
            case -1:
                err(ctx->log_ctx, "Failed to poll() masterfd: %s",
                    strerror(errno));
                return NULL;
            case 0:
                continue;
            // The default is to run the code below
        }

        ret = read(ctx->masterfd, buf, sizeof(buf));

        if (ret == -1) {
            err(ctx->log_ctx, "Failed to read() from masterfd: %s",
                strerror(errno));
            continue;
        }

        rv = osd_cl_dem_uart_send_string(ctx->hostmod_ctx, ctx->dem_uart_desc,
                                         buf, (size_t) ret);
        if (OSD_FAILED(rv)) {
            err(ctx->log_ctx, "Failed to send string to DEM-UART!");
        }
    }

    return NULL;
}

API_EXPORT
osd_result osd_terminal_new(struct osd_terminal_ctx **ctx,
                            struct osd_log_ctx *log_ctx,
                            const char *host_controller_address,
                            uint16_t dem_uart_di_addr)
{
    osd_result rv;

    struct osd_terminal_ctx *c = calloc(1, sizeof(struct osd_terminal_ctx));
    assert(c);

    c->log_ctx = log_ctx;
    c->dem_uart_di_addr = dem_uart_di_addr;
    c->dem_uart_event_handler.cb_arg = (void *)c;
    c->dem_uart_event_handler.cb_fn = handle_rx_data;

    rv = osd_hostmod_new(&(c->hostmod_ctx), log_ctx, host_controller_address,
                         osd_cl_dem_uart_receive_event,
                         &(c->dem_uart_event_handler));
    if (OSD_FAILED(rv)) {
        return rv;
    }

    c->dem_uart_desc = calloc(1, sizeof(struct osd_dem_uart_desc));
    assert(c->dem_uart_desc);

    *ctx = c;

    return OSD_OK;
}

API_EXPORT
void osd_terminal_free(struct osd_terminal_ctx **ctx_p)
{
    assert(ctx_p);
    struct osd_terminal_ctx *ctx = *ctx_p;
    if (!ctx) {
        return;
    }

    osd_hostmod_free(&(ctx->hostmod_ctx));

    free(ctx->dem_uart_desc);
    free(ctx->pts_path);

    free(ctx);
    *ctx_p = NULL;
}

API_EXPORT
osd_result osd_terminal_connect(struct osd_terminal_ctx *ctx)
{
    return osd_hostmod_connect(ctx->hostmod_ctx);
}

API_EXPORT
osd_result osd_terminal_disconnect(struct osd_terminal_ctx *ctx)
{
    if (ctx->running) {
        err(ctx->log_ctx,
            "Attempt to disconnect terminal that is still running!");
    }
    return osd_hostmod_disconnect(ctx->hostmod_ctx);
}

API_EXPORT
bool osd_terminal_is_connected(struct osd_terminal_ctx *ctx)
{
    return osd_hostmod_is_connected(ctx->hostmod_ctx);
}

API_EXPORT
osd_result osd_terminal_start(struct osd_terminal_ctx *ctx)
{
    struct termios termios;
    osd_result rv;

    osd_cl_dem_uart_get_desc(ctx->hostmod_ctx, ctx->dem_uart_di_addr,
                             ctx->dem_uart_desc);

    rv = osd_hostmod_mod_set_event_dest(ctx->hostmod_ctx,
                                        ctx->dem_uart_desc->di_addr, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    ctx->masterfd = posix_openpt(O_RDWR | O_NOCTTY);
    if (ctx->masterfd < 0) {
        err(ctx->log_ctx, "Failed to open new pseudo-terminal: %s",
            strerror(errno));
        return OSD_ERROR_FAILURE;
    }

    if (grantpt(ctx->masterfd) != 0) {
        err(ctx->log_ctx, "granpt() failed: %s", strerror(errno));
        rv = OSD_ERROR_FAILURE;
        goto error_return;
    }

    if (tcgetattr(ctx->masterfd, &termios) != 0) {
        err(ctx->log_ctx, "Failed to get current termios: %s", strerror(errno));
        rv = OSD_ERROR_FAILURE;
        goto error_return;
    }

    // Don't echo every char that's written
    termios.c_lflag &= ~ECHO;

    // Disable canonical mode, this means no '\n' is needed.
    termios.c_lflag &= ~(ICANON);

    // Disable Control-signals (Ctrl-C, etc..) being sent to this process
    termios.c_lflag &= ~(ISIG);

    // Disable Nl-CR swapping for IO
    termios.c_iflag &= ~(ICRNL);
    termios.c_iflag &= ~(INLCR);
    termios.c_oflag &= ~(OCRNL);
    termios.c_oflag &= ~(ONLCR);

    // Disable Software Flow Control
    termios.c_iflag &= ~(IXON);
    termios.c_iflag &= ~(IXOFF);

    if (tcsetattr(ctx->masterfd, TCSANOW, &termios) != 0) {
        err(ctx->log_ctx, "Failed to set new termios: %s", strerror(errno));
        rv = OSD_ERROR_FAILURE;
        goto error_return;
    }

    if (unlockpt(ctx->masterfd) != 0) {
        err(ctx->log_ctx, "Failed to open new pseudo-terminal: %s",
            strerror(errno));
        rv = OSD_ERROR_FAILURE;
        goto error_return;
    }

    rv = osd_hostmod_mod_set_event_active(ctx->hostmod_ctx,
                                          ctx->dem_uart_desc->di_addr, true, 0);
    if (OSD_FAILED(rv)) {
        goto error_return;
    }

    ctx->running = true;
    int ret = pthread_create(&(ctx->tx_thread), NULL, terminal_tx_thread, ctx);
    if (ret) {
        err(ctx->log_ctx, "Failed to create new pthread: %s", strerror(ret));
        rv = OSD_ERROR_FAILURE;
        goto error_return;
    }

    ctx->pts_path = strdup(ptsname(ctx->masterfd));
    assert(ctx->pts_path);

    info(ctx->log_ctx, "DEM-UART pseudo-terminal available at %s\n",
         ctx->pts_path);

    rv = OSD_OK;
error_return:
    if (OSD_FAILED(rv)) {
        close(ctx->masterfd);
    }

    return rv;
}

API_EXPORT
osd_result osd_terminal_stop(struct osd_terminal_ctx *ctx)
{
    osd_result rv;

    rv = osd_hostmod_mod_set_event_active(
        ctx->hostmod_ctx, ctx->dem_uart_desc->di_addr, false, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    ctx->running = false;

    if (pthread_join(ctx->tx_thread, NULL)) {
        err(ctx->log_ctx, "Unable to join terminal_tx_thread");
    }

    // Closing the master FD also removes the corresponding /dev/pts/ node
    close(ctx->masterfd);

    return OSD_OK;
}

API_EXPORT
bool osd_terminal_is_started(struct osd_terminal_ctx *ctx)
{
    return ctx->running;
}

API_EXPORT
const char *osd_terminal_get_pts_path(struct osd_terminal_ctx *ctx)
{
    return ctx->pts_path;
}
