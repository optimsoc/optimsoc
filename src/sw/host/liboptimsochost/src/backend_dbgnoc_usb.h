/* Copyright (c) 2012-2013 by the author(s)
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
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#ifndef _BACKEND_DBGNOC_USB_H_
#define _BACKEND_DBGNOC_USB_H_

#include <inttypes.h>

#include "log.h"
#include "backends.h"

#include "backend_dbgnoc.h"

struct ob_dbgnoc_connection_ctx;

int ob_dbgnoc_usb_new(struct ob_dbgnoc_connection_ctx **ctx,
                      struct ob_dbgnoc_connection_interface *calls,
                      struct optimsoc_log_ctx *log_ctx,
                      int num_options,
                      struct optimsoc_backend_option options[]);
int ob_dbgnoc_usb_free(struct ob_dbgnoc_connection_ctx *ctx);
int ob_dbgnoc_usb_connect(struct ob_dbgnoc_connection_ctx *ctx);
int ob_dbgnoc_usb_connected(struct ob_dbgnoc_connection_ctx *ctx);
int ob_dbgnoc_usb_disconnect(struct ob_dbgnoc_connection_ctx *ctx);
int ob_dbgnoc_usb_reset(struct ob_dbgnoc_connection_ctx *ctx);
int ob_dbgnoc_usb_write(struct ob_dbgnoc_connection_ctx *ctx, uint16_t *buffer,
                        int len);
int ob_dbgnoc_usb_read(struct ob_dbgnoc_connection_ctx *ctx, uint16_t *buffer,
                       int len);


int bulk_transfer_size_valid(struct ob_dbgnoc_connection_ctx *ctx, int num_samples);

#endif /* _BACKEND_DBGNOC_USB_H_ */
