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

#ifndef OSD_CORETRACELOGGER_H
#define OSD_CORETRACELOGGER_H

#include <osd/osd.h>
#include <osd/hostmod.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-coretracelogger Core Trace Logger
 * @ingroup libosd
 *
 * @{
 */

struct osd_coretracelogger_ctx;

/**
 * Create a new context object
 */
osd_result osd_coretracelogger_new(struct osd_coretracelogger_ctx **ctx,
                                     struct osd_log_ctx *log_ctx,
                                     const char *host_controller_address,
                                     uint16_t ctm_di_addr);

/**
 * @copydoc osd_hostmod_connect()
 */
osd_result osd_coretracelogger_connect(struct osd_coretracelogger_ctx *ctx);

/**
 * @copydoc osd_hostmod_disconnect()
 */
osd_result osd_coretracelogger_disconnect(struct osd_coretracelogger_ctx *ctx);

/**
 * @copydoc osd_hostmod_is_connected()
 */
bool osd_coretracelogger_is_connected(struct osd_coretracelogger_ctx *ctx);

/**
 * Free the context object
 */
void osd_coretracelogger_free(struct osd_coretracelogger_ctx **ctx_p);

/**
 * Start collecting system logs
 *
 * Instruct the STM module to start sending traces to us.
 */
osd_result osd_coretracelogger_start(struct osd_coretracelogger_ctx *ctx);

/**
 * Stop collecting system logs
 */
osd_result osd_coretracelogger_stop(struct osd_coretracelogger_ctx *ctx);

/**
 * Set a file to write all log output to
 *
 * @param ctx context object
 * @param fp a file pointer to write the logs to
 * @return OSD_OK if successful, any other value indicates an error
 */
osd_result osd_coretracelogger_set_log(struct osd_coretracelogger_ctx *ctx,
                                       FILE *fp);

/**
 * Set the path to the ELF file used to decode the core trace events
 *
 * To disable ELF parsing, set elf_filename to NULL.
 *
 * @param ctx context object
 * @param elf_filename path to the ELF file. Set to NULL to disable ELF parsing.
 * @return OSD_OK when reading the ELF file succeeded
 *         any other value indicates an error
 */
osd_result osd_coretracelogger_set_elf(struct osd_coretracelogger_ctx *ctx,
                                       const char* elf_filename);


/**@}*/ /* end of doxygen group libosd-coretracelogger */

#ifdef __cplusplus
}
#endif

#endif  // OSD_CORETRACELOGGER_H
