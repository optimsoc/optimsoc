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

#include <osd/cl_scm.h>

#include <assert.h>
#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/packet.h>
#include "osd-private.h"

/** The SCM module is always at address 0 in a subnet */
#define OSD_DIADDR_LOCAL_SCM 0

static unsigned int get_scm_diaddr(unsigned int subnet_addr)
{
    return osd_diaddr_build(subnet_addr, OSD_DIADDR_LOCAL_SCM);
}

API_EXPORT
osd_result osd_cl_scm_cpus_start(struct osd_hostmod_ctx *hostmod_ctx,
                                 unsigned int subnet_addr)
{
    return osd_hostmod_reg_setbit(hostmod_ctx, OSD_REG_SCM_SYSRST_CPU_RST_BIT, 0,
                                  get_scm_diaddr(subnet_addr),
                                  OSD_REG_SCM_SYSRST, 16,
                                  OSD_HOSTMOD_BLOCKING);
}

API_EXPORT
osd_result osd_cl_scm_cpus_stop(struct osd_hostmod_ctx *hostmod_ctx,
                                unsigned int subnet_addr)
{
    return osd_hostmod_reg_setbit(hostmod_ctx, OSD_REG_SCM_SYSRST_CPU_RST_BIT, 1,
                                  get_scm_diaddr(subnet_addr),
                                  OSD_REG_SCM_SYSRST, 16,
                                  OSD_HOSTMOD_BLOCKING);
}

/**
 * Read the system information from the device, as stored in the SCM
 */
API_EXPORT
osd_result osd_cl_scm_get_subnetinfo(struct osd_hostmod_ctx *hostmod_ctx,
                                     unsigned int subnet_addr,
                                     struct osd_subnet_desc *subnet_desc)
{
    osd_result rv;

    uint16_t scm_diaddr = get_scm_diaddr(subnet_addr);
    struct osd_log_ctx* log_ctx = osd_hostmod_log_ctx(hostmod_ctx);

    rv = osd_hostmod_reg_read(hostmod_ctx, &subnet_desc->vendor_id, scm_diaddr,
                              OSD_REG_SCM_SYSTEM_VENDOR_ID, 16, 0);
    if (OSD_FAILED(rv)) {
        err(log_ctx, "Unable to read VENDOR_ID from SCM (rv=%d)", rv);
        return rv;
    }
    rv = osd_hostmod_reg_read(hostmod_ctx, &subnet_desc->device_id, scm_diaddr,
                                  OSD_REG_SCM_SYSTEM_DEVICE_ID, 16, 0);
    if (OSD_FAILED(rv)) {
        err(log_ctx, "Unable to read DEVICE_ID from SCM (rv=%d)", rv);
        return rv;
    }
    rv = osd_hostmod_reg_read(hostmod_ctx, &subnet_desc->max_pkt_len, scm_diaddr,
                                  OSD_REG_SCM_MAX_PKT_LEN, 16, 0);
    if (OSD_FAILED(rv)) {
        err(log_ctx, "Unable to read MAX_PKT_LEN from SCM (rv=%d)", rv);
        return rv;
    }

    dbg(log_ctx, "Got system information: VENDOR_ID = %u, DEVICE_ID = %u, "
        "MAX_PKT_LEN = %u\n\n",
        subnet_desc->vendor_id, subnet_desc->device_id,
        subnet_desc->max_pkt_len);

    return OSD_OK;
}
