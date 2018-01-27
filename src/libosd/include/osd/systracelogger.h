/* Copyright 2017-2018 The Open SoC Debug Project
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

#ifndef OSD_SYSTRACELOGGER_H
#define OSD_SYSTRACELOGGER_H

#include <osd/osd.h>
#include <osd/hostmod.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-systracelogger System Trace Logger
 * @ingroup libosd
 *
 * @{
 */

struct osd_systracelogger_ctx;

/**
 * Create a new context object
 */
osd_result osd_systracelogger_new(struct osd_systracelogger_ctx **ctx,
                                  struct osd_log_ctx *log_ctx,
                                  const char *host_controller_address,
                                  uint16_t stm_di_addr);

/**
 * @copydoc osd_hostmod_connect()
 */
osd_result osd_systracelogger_connect(struct osd_systracelogger_ctx *ctx);

/**
 * @copydoc osd_hostmod_disconnect()
 */
osd_result osd_systracelogger_disconnect(struct osd_systracelogger_ctx *ctx);

/**
 * @copydoc osd_hostmod_is_connected()
 */
bool osd_systracelogger_is_connected(struct osd_systracelogger_ctx *ctx);

/**
 * Free the context object
 */
void osd_systracelogger_free(struct osd_systracelogger_ctx **ctx_p);

/**
 * Start collecting system logs
 *
 * Instruct the STM module to start sending traces to us.
 */
osd_result osd_systracelogger_start(struct osd_systracelogger_ctx *ctx);

/**
 * Stop collecting system logs
 */
osd_result osd_systracelogger_stop(struct osd_systracelogger_ctx *ctx);

/**
 * Set a file to write all sysprint output to
 */
osd_result osd_systracelogger_set_sysprint_log(
        struct osd_systracelogger_ctx *ctx, FILE *fp);

/**
 * Set a file to write all received STM events to
 */
osd_result osd_systracelogger_set_event_log(struct osd_systracelogger_ctx *ctx,
                                            FILE *fp);


/**@}*/ /* end of doxygen group libosd-systracelogger */

#ifdef __cplusplus
}
#endif

#endif  // OSD_SYSTRACELOGGER_H
