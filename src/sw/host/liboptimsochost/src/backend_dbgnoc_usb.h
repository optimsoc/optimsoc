/*
 * This file is part of liboptimsochost.
 *
 * liboptimsochost is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * liboptimsochost is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with liboptimsochost. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner <philipp.wagner@tum.de>
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
