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

#ifndef OSD_CL_STM_H
#define OSD_CL_STM_H

#include <osd/module.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/hostmod.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-cl_stm STM client
 * @ingroup libosd
 *
 * @{
 */

/**
 * Information a System Trace Module
 */
struct osd_stm_desc {
    unsigned int di_addr; //!< DI address of the memory
    uint16_t value_width_bit; //<! Size of the value in a trace event in bit
};

/**
 * A single event emitted by the STM module
 *
 * In case of an overflow in the system an overflow even is generated,
 * containing the number of missed events. If the overflow value is set to a
 * non-zero value, id and value are invalid.
 */
struct osd_stm_event {
    uint32_t timestamp; //!< timestamp
    uint16_t id; //!< event identifier
    uint64_t value; //!< traced value
    uint16_t overflow; //!< Number of lost packets due to overflow.
};

typedef void (*osd_cl_stm_handler_fn)(
    void * /* arg */, const struct osd_stm_desc * /* stm_desc */,
    const struct osd_stm_event * /* event */);

struct osd_stm_event_handler {
    const struct osd_stm_desc *stm_desc;
    osd_cl_stm_handler_fn cb_fn;
    void *cb_arg;
};

struct osd_cl_stm_print_buf {
    char* buf; //!< data buffer
    size_t len_buf; //!< allocated size of |buf|

    /**
     * length of the string inside |buf|, excluding the zero termination
     */
    size_t len_str;
};

/**
 * Populate the STM descriptor with data from the debug module
 *
 * @param hostmod_ctx the host module handling the communication
 * @param stm_di_addr DI address of the STM module to get describe
 * @param[out] stm_desc pre-allocated memory descriptor for the result
 * @return OSD_OK on success
 *         OSD_ERROR_WRONG_MODULE if the module at stm_di_addr is not a STM
 *         any other value indicates an error
 */
osd_result osd_cl_stm_get_desc(struct osd_hostmod_ctx *hostmod_ctx,
                               unsigned int stm_di_addr,
                               struct osd_stm_desc *stm_desc);

/**
 * Event handler to process STM event, to be passed to a hostmod instance
 */
osd_result osd_cl_stm_handle_event(void *arg, struct osd_packet *pkg);

/**
 * Is the given STM event a sysprint event?
 */
bool osd_cl_stm_is_print_event(const struct osd_stm_event *ev);

/**
 * Allocate memory for a new osd_cl_stm_print_buf struct
 */
osd_result osd_cl_stm_print_buf_new(struct osd_cl_stm_print_buf **print_buf_p);

/**
 * Free a osd_cl_stm_print_buf struct
 */
void osd_cl_stm_print_buf_free(struct osd_cl_stm_print_buf **print_buf_p);

/**
 * Add a STM event to the print buffer
 */
osd_result osd_cl_stm_add_to_print_buf(const struct osd_stm_event *ev,
                                       struct osd_cl_stm_print_buf *buf,
                                       bool *should_flush);

/**@}*/ /* end of doxygen group libosd-cl_stm */

#ifdef __cplusplus
}
#endif

#endif  // OSD_CL_STM_H
