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

#ifndef OSD_MEMACCESS_H
#define OSD_MEMACCESS_H

#include <osd/osd.h>
#include <osd/hostmod.h>
#include <osd/cl_mam.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-memaccess Memory Access Utility
 * @ingroup libosd
 *
 * @{
 */

struct osd_memaccess_ctx;

/**
 * Create a new context object
 */
osd_result osd_memaccess_new(struct osd_memaccess_ctx **ctx,
                             struct osd_log_ctx *log_ctx,
                             const char *host_controller_address);

/**
 * @copydoc osd_hostmod_connect()
 */
osd_result osd_memaccess_connect(struct osd_memaccess_ctx *ctx);

/**
 * @copydoc osd_hostmod_disconnect()
 */
osd_result osd_memaccess_disconnect(struct osd_memaccess_ctx *ctx);

/**
 * @copydoc osd_hostmod_is_connected()
 */
bool osd_memaccess_is_connected(struct osd_memaccess_ctx *ctx);

/**
 * Free the context object
 */
void osd_memaccess_free(struct osd_memaccess_ctx **ctx_p);

/**
 * (Re-)Start all CPUs in the subnet
 *
 * @param ctx the context object
 * @param subnet_addr the subnet to start all CPUs in
 * @return OSD_OK on success, any other value indicates an error
 *
 * @see osd_memaccess_cpus_stop()
 */
osd_result osd_memaccess_cpus_start(struct osd_memaccess_ctx *ctx,
                                    unsigned int subnet_addr);

/**
 * Stop all CPUs in the given subnet to avoid interfering with the memory access
 *
 * @param ctx the context object
 * @param subnet_addr the subnet to stop all CPUs in
 * @return OSD_OK on success, any other value indicates an error
 *
 * @see osd_memaccess_cpus_start()
 */
osd_result osd_memaccess_cpus_stop(struct osd_memaccess_ctx *ctx,
                                   unsigned int subnet_addr);

/**
 * Get all memories in a subnet
 *
 * @param ctx the context object
 * @param subnet_addr the subnet to find memories in
 * @param[out] the memories found in the subnet
 * @param[out] the number of entries in the @p memories array
 * @return OSD_OK if the operation was successful
 *         OSD_ERROR_PARTIAL_RESULT not all memories could be enumerated
 *         any other value indicates an erorr
 */
osd_result osd_memaccess_find_memories(struct osd_memaccess_ctx *ctx,
                                       unsigned int subnet_addr,
                                       struct osd_mem_desc **memories,
                                       size_t *num_memories);

/**
 * Load an ELF file into a memory
 *
 * @param ctx the context object
 * @param mem_desc the memory to load the data into
 * @param elf_file_path file system path to the ELF file to be loaded
 * @param verify verify the write operation by reading the file back and
 *               and compare the data.
 * @param OSD_OK if successful, any other value indicates an error
 */
osd_result osd_memaccess_loadelf(struct osd_memaccess_ctx *ctx,
                                 const struct osd_mem_desc* mem_desc,
                                 const char* elf_file_path, bool verify);

/**@}*/ /* end of doxygen group libosd-memaccess */

#ifdef __cplusplus
}
#endif

#endif  // OSD_MEMACCESS_H
