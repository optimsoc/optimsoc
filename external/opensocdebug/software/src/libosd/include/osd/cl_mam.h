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

#ifndef OSD_CL_MAM_H
#define OSD_CL_MAM_H

#include <osd/module.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/hostmod.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-cl_mam MAM client
 * @ingroup libosd
 *
 * @{
 */

/**
 * Information about a memory region
 */
struct osd_mem_desc_region {
    uint64_t baseaddr;
    uint64_t memsize;
};

/**
 * Information about a memory attached to a MAM module
 */
struct osd_mem_desc {
    unsigned int di_addr; //!< DI address of the memory
    uint16_t data_width_bit; //!< Data width in bit
    uint16_t addr_width_bit; //!< Address width in bit
    uint8_t num_regions; //!< Number of accessible memory regions
    struct osd_mem_desc_region regions[8]; //!< Memory region information
};

/**
 * Obtain information about the memory connected to a MAM module
 *
 * @param hostmod_ctx the host module handling the communication
 * @param mam_di_addr DI address of the MAM module to get describe
 * @param[out] mem_desc pre-allocated memory descriptor for the result
 * @return OSD_OK on success, any other value indicates an error
 */
osd_result osd_cl_mam_get_mem_desc(struct osd_hostmod_ctx *hostmod_ctx,
                                   unsigned int mam_di_addr,
                                   struct osd_mem_desc *mem_desc);

/**
 * Write data to the memory attached to a Memory Access Module (MAM)
 *
 * The data does *not* need to be word-aligned.
 * Writes across memory region boundaries are not allowed.
 * This function blocks until the write is acknowledged by the memory.
 *
 * @param mem_desc descriptor of the target memory
 * @param hostmod_ctx the host module handling the communication
 * @param data the data to be written
 * @param nbyte the number of bytes to write
 * @param start_addr first byte address to write data to. All subsequent words
 *                   are written to consecutive addresses.
 * @return OSD_OK if the write was successful
 *         any other value indicates an error
 *
 * @see osd_cl_mam_read()
 */
osd_result osd_cl_mam_write(const struct osd_mem_desc *mem_desc,
                            struct osd_hostmod_ctx *hostmod_ctx,
                            const void *data, size_t nbyte,
                            uint64_t start_addr);

/**
 * Read data from a memory attached to a Memory Access Module (MAM)
 *
 * The data does *not* need to be word-aligned.
 * Reads across memory region boundaries are not allowed.
 * This function blocks until the write is acknowledged by the memory.
 *
 * @param mem_desc descriptor of the target memory
 * @param hostmod_ctx the host module handling the communication
 * @param data the returned read data. Must be preallocated and large enough for
 *             nbyte bytes of data.
 * @param nbyte the number of bytes to read
 * @param start_addr first byte address to read from
 * @return OSD_OK if the read was successful
 *         any other value indicates an error
 *
 * @see osd_cl_mam_write()
 */
osd_result osd_cl_mam_read(const struct osd_mem_desc *mem_desc,
                           struct osd_hostmod_ctx *hostmod_ctx,
                           void *data, size_t nbyte, uint64_t start_addr);

/**@}*/ /* end of doxygen group libosd-cl_mam */

#ifdef __cplusplus
}
#endif

#endif  // OSD_CL_MAM_H

