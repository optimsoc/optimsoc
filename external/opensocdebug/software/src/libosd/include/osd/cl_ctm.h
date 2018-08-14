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

#ifndef OSD_CL_CTM_H
#define OSD_CL_CTM_H

#include <osd/module.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/hostmod.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-cl_ctm CTM client
 * @ingroup libosd
 *
 * @{
 */

/**
 * Information a Core Trace Module
 */
struct osd_ctm_desc {
    unsigned int di_addr; //!< DI address of the memory
    uint16_t addr_width_bit; //!< Width of an address in bit
    uint16_t data_width_bit; //!< Width of a data word in bit
};

/**
 * A single event emitted by the CTM module
 *
 * In case of an overflow in the system an overflow even is generated,
 * containing the number of missed events. If the overflow value is set to a
 * non-zero value, all other fields except for the timestamp are invalid.
 */
struct osd_ctm_event {
    uint16_t overflow; //!< number of overflowed packets
    uint32_t timestamp; //!< timestamp
    uint64_t npc; //!< npc
    uint64_t pc; //!< pc
    uint8_t mode; //!< privilege mode
    bool is_ret; //!< executed instruction is a function return
    bool is_call; //!< executed instruction is a function call
    bool is_modechange; //!< executed instruction changed the privilege mode
};

typedef void (*osd_cl_ctm_handler_fn)(
    void * /* arg */, const struct osd_ctm_desc * /* ctm_desc */,
    const struct osd_ctm_event * /* event */);

struct osd_ctm_event_handler {
    const struct osd_ctm_desc *ctm_desc;
    osd_cl_ctm_handler_fn cb_fn;
    void *cb_arg;
};

/**
 * Populate the CTM descriptor with data from the debug module
 *
 * @param hostmod_ctx the host module handling the communication
 * @param ctm_di_addr DI address of the CTM module to get describe
 * @param[out] ctm_desc pre-allocated memory descriptor for the result
 * @return OSD_OK on success
 *         OSD_ERROR_WRONG_MODULE if the module at ctm_di_addr is not a CTM
 *         any other value indicates an error
 */
osd_result osd_cl_ctm_get_desc(struct osd_hostmod_ctx *hostmod_ctx,
                               unsigned int ctm_di_addr,
                               struct osd_ctm_desc *ctm_desc);

/**
 * Event handler to process CTM event, to be passed to a hostmod instance
 */
osd_result osd_cl_ctm_handle_event(void *arg, struct osd_packet *pkg);

/**@}*/ /* end of doxygen group libosd-cl_ctm */

#ifdef __cplusplus
}
#endif

#endif  // OSD_CL_CTM_H
