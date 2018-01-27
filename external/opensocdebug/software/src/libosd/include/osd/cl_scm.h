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

#ifndef OSD_CL_SCM_H
#define OSD_CL_SCM_H

#include <osd/module.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/hostmod.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-cl_scm SCM client
 * @ingroup libosd
 *
 * @{
 */

struct osd_subnet_desc {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t max_pkt_len;
};

/**
 * Start (un-halt) all CPUs in the SCM subnet
 *
 * @see osd_cl_scm_cpus_stop()
 */
osd_result osd_cl_scm_cpus_start(struct osd_hostmod_ctx *hostmod_ctx,
                                 unsigned int subnet_addr);

/**
 * Stop (halt) all CPUs in the SCM subnet
 */
osd_result osd_cl_scm_cpus_stop(struct osd_hostmod_ctx *hostmod_ctx,
                                unsigned int subnet_addr);

/**
 * Get a description of a given subnet from the SCM
 */
osd_result osd_cl_scm_get_subnetinfo(struct osd_hostmod_ctx *hostmod_ctx,
                                     unsigned int subnet_addr,
                                     struct osd_subnet_desc *subnet_desc);

/**@}*/ /* end of doxygen group libosd-cl_scm */

#ifdef __cplusplus
}
#endif

#endif  // OSD_CL_SCM_H

