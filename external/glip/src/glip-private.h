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
 * This file is the private header for glip.c, it contains all non-backend
 * specific, non-public definitions.
 * Do *not* include this file in any of the backends!
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef __GLIP_PRIVATE_H__
#define __GLIP_PRIVATE_H__

#include <libglip.h>
#include "glip-protected.h"
#include <config.h>

#include <stddef.h>
#include <syslog.h>

/*
 * all supported backends
 *
 * NOTE BACKEND AUTHORS:
 *   If you add a new backend to GLIP include the header here!
 */
#ifdef BACKEND_UART_ENABLED
#include "backend_uart/sw/backend_uart.h"
#endif
#ifdef BACKEND_CYPRESSFX2_ENABLED
#include "backend_cypressfx2/sw/backend_cypressfx2.h"
#endif
#ifdef BACKEND_CYPRESSFX3_ENABLED
#include "backend_cypressfx3/sw/backend_cypressfx3.h"
#endif
#ifdef BACKEND_TCP_ENABLED
#include "backend_tcp/sw/backend_tcp.h"
#endif
#ifdef BACKEND_JTAG_ENABLED
#include "backend_jtag/sw/backend_jtag.h"
#endif


/**
 * Macro used by the configure script to enable a backend in the glip_backends
 * struct.
 */
#define ENABLED_BACKEND(NAME)  { #NAME, gb_ ## NAME ##_new, gb_ ## NAME ##_free }
/**
 * Macro used by the configure script to disable a backend
 */
#define DISABLED_BACKEND(NAME) { #NAME, NULL, NULL }

/**
 * API_EXPORT marks all exported functions which are part of the public API.
 * All other functions will be hidden and not exported.
 */
#define API_EXPORT __attribute__ ((visibility("default")))

/**
 * Information about a single backend in the glip_backends struct.
 */
struct glip_backend {
    const char *name; /**< unique name of the backend */
    int (*new)(struct glip_ctx* /* ctx */); /**< constructor */
    void (*free)(struct glip_ctx* /* ctx */); /**< destructor */
};

/**
 * An array of all available backends and their init function
 *
 * The BACKEND_* macros are resolved to either ENABLE_BACKEND or DISABLE_BACKEND
 * in the config.h file by the configure script. Essentially, an enabled
 * backend has a non-zero new() function pointer.
 *
 * NOTE BACKEND AUTHORS:
 *   If you add a new backend to GLIP add it here too!
 */
static struct glip_backend glip_backends[] = {
    BACKEND_UART,
    BACKEND_CYPRESSFX2,
    BACKEND_CYPRESSFX3,
    BACKEND_TCP,
    BACKEND_JTAG
};

#endif /* __GLIP_PRIVATE_H__ */
