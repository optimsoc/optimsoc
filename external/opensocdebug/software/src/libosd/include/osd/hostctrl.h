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

#ifndef OSD_HOSTCTRL_H
#define OSD_HOSTCTRL_H

#include <osd/osd.h>

#include <czmq.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-hostctrl Host Controller
 * @ingroup libosd
 *
 * @{
 */

struct osd_hostctrl_ctx;

/**
 * Create new host controller
 *
 * The host controller will listen to requests at @p router_addres
 *
 * @param ctx context object
 * @param log_ctx logging context
 * @param router_address ZeroMQ endpoint/URL the host controller will listen on
 * @return OSD_OK if initialization was successful,
 *         any other return code indicates an error
 */
osd_result osd_hostctrl_new(struct osd_hostctrl_ctx **ctx,
                            struct osd_log_ctx *log_ctx,
                            const char *router_address);

/**
 * Start host controller
 */
osd_result osd_hostctrl_start(struct osd_hostctrl_ctx *ctx);

/**
 * Stop host controller
 */
osd_result osd_hostctrl_stop(struct osd_hostctrl_ctx *ctx);

/**
 * Free the host controller context object (destructor)
 *
 * By calling this function all resources associated with the context object
 * are freed and the ctx_p itself is NULLed.
 *
 * @param ctx_p the host controller context object
 */
void osd_hostctrl_free(struct osd_hostctrl_ctx **ctx_p);

/**
 * Is the host controller running?
 *
 * @param ctx the context object
 * @return true if the host controller is running, false otherwise
 */
bool osd_hostctrl_is_running(struct osd_hostctrl_ctx *ctx);

/**@}*/ /* end of doxygen group libosd-hostctrl */

#ifdef __cplusplus
}
#endif

#endif  // OSD_HOSTCTRL_H
