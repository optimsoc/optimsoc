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

#ifndef OSD_GATEWAY_GLIP_H
#define OSD_GATEWAY_GLIP_H

#include <osd/module.h>
#include <osd/osd.h>
#include <osd/packet.h>

#include <czmq.h>
#include <libglip.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-gateway_glip Gateway to a GLIP-supported device
 * @ingroup libosd
 *
 * @{
 */

struct osd_gateway_glip_ctx;

/**
 * Create new osd_gateway_glip instance
 *
 * @param[out] ctx the osd_gateway_glip_ctx context to be created
 * @param[in] log_ctx the log context to be used. Set to NULL to disable logging
 * @param[in] host_controller_address ZeroMQ endpoint of the host controller
 * @param[in] device_subnet_addr Subnet address of the device
 * xxx
 * @return OSD_OK on success, any other value indicates an error
 *
 * @see osd_gateway_new()
 * @see osd_gateway_glip_free()
 */
osd_result osd_gateway_glip_new(struct osd_gateway_glip_ctx **ctx,
                                struct osd_log_ctx *log_ctx,
                                const char *host_controller_address,
                                uint16_t device_subnet_addr,
                                const char *glip_backend_name,
                                const struct glip_option *glip_backend_options,
                                size_t glip_backend_options_len);

/**
 * @copydoc osd_gateway_free()
 */
void osd_gateway_glip_free(struct osd_gateway_glip_ctx **ctx_p);

/**
 * @copydoc osd_gateway_connect()
 */
osd_result osd_gateway_glip_connect(struct osd_gateway_glip_ctx *ctx);

/**
 * @copydoc osd_gateway_disconnect()
 */
osd_result osd_gateway_glip_disconnect(struct osd_gateway_glip_ctx *ctx);

/**
 * @copydoc osd_is_connected()
 */
bool osd_gateway_glip_is_connected(struct osd_gateway_glip_ctx *ctx);

/**
 * @copydoc osd_gateway_get_transfer_stats();
 */
struct osd_gateway_transfer_stats*
osd_gateway_glip_get_transfer_stats(struct osd_gateway_glip_ctx *ctx);

/**@}*/ /* end of doxygen group libosd-gateway_glip */

#ifdef __cplusplus
}
#endif

#endif  // OSD_GATEWAY_GLIP_H
