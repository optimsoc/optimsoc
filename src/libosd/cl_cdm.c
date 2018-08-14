/* Copyright 2018 The Open SoC Debug Project
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

#include <osd/cl_cdm.h>

#include <assert.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/reg.h>
#include "osd-private.h"

static struct osd_cdm_event *build_cdm_event(
    const struct osd_cdm_desc *cdm_desc, const struct osd_packet *pkg)
{
    struct osd_cdm_event *ev = calloc(1, sizeof(struct osd_cdm_event));

    size_t exp_payload_len = 1;  // stall
    assert(osd_packet_sizeconv_payload2data(exp_payload_len) ==
               pkg->data_size_words &&
           "CDM Protocol violation detected.");

    bool stall = pkg->data.payload[0];

    ev->stall = stall;
    return ev;
}

API_EXPORT
osd_result osd_cl_cdm_handle_event(void *arg, struct osd_packet *pkg)
{
    assert(arg &&
           "You need to give an event_handler_arg of type "
           "struct osd_cdm_event_handler in osd_hostmod_new()");
    assert(pkg);

    struct osd_cdm_event_handler *handler = arg;

    struct osd_cdm_event *ev = build_cdm_event(handler->cdm_desc, pkg);
    osd_packet_free(&pkg);

    handler->cb_fn(handler->cb_arg, handler->cdm_desc, ev);
    free(ev);

    return OSD_OK;
}

static bool is_cdm_module(struct osd_hostmod_ctx *hostmod_ctx,
                          unsigned int cdm_di_addr)
{
    osd_result rv;

    struct osd_module_desc desc;

    rv = osd_hostmod_mod_describe(hostmod_ctx, cdm_di_addr, &desc);
    if (OSD_FAILED(rv)) {
        return false;
    }

    if (desc.vendor != OSD_MODULE_VENDOR_OSD ||
        desc.type != OSD_MODULE_TYPE_STD_CDM || desc.version != 0) {
        return false;
    }

    return true;
}

API_EXPORT
osd_result osd_cl_cdm_get_desc(struct osd_hostmod_ctx *hostmod_ctx,
                               unsigned int cdm_di_addr,
                               struct osd_cdm_desc *cdm_desc)
{
    assert(hostmod_ctx);
    assert(cdm_desc);

    osd_result rv;

    cdm_desc->di_addr = cdm_di_addr;

    if (!is_cdm_module(hostmod_ctx, cdm_di_addr)) {
        return OSD_ERROR_WRONG_MODULE;
    }

    uint16_t regvalue;
    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, cdm_di_addr,
                              OSD_REG_CDM_CORE_CTRL, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    cdm_desc->core_ctrl = regvalue;

    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, cdm_di_addr,
                              OSD_REG_CDM_CORE_REG_UPPER, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    cdm_desc->core_reg_upper = regvalue;

    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, cdm_di_addr,
                              OSD_REG_CDM_CORE_DATA_WIDTH, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    cdm_desc->core_data_width = regvalue;

    return OSD_OK;
}

API_EXPORT
osd_result cl_cdm_cpureg_read(struct osd_hostmod_ctx *hostmod_ctx,
                              struct osd_cdm_desc *cdm_desc, void *reg_val,
                              uint16_t reg_addr, int flags)
{
    assert(hostmod_ctx);
    assert(cdm_desc);

    osd_result rv;

    uint16_t cdm_di_addr = cdm_desc->di_addr;
    uint16_t reg_addr_upper = reg_addr >> 15;
    uint16_t core_upper = cdm_desc->core_reg_upper;

    if (core_upper != reg_addr_upper) {
        rv = osd_hostmod_reg_write(hostmod_ctx, &reg_addr_upper, cdm_di_addr,
                                   OSD_REG_CDM_CORE_REG_UPPER, 16, 0);
        if (OSD_FAILED(rv)) return rv;
        cdm_desc->core_reg_upper = reg_addr_upper;
    }

    uint16_t cdm_reg_addr = 0x8000 + (reg_addr & 0x7fff);
    uint16_t core_dw = cdm_desc->core_data_width;
    assert(core_dw != 128
           && "128 bit wide register accesses are currently not supported.");

    rv = osd_hostmod_reg_read(hostmod_ctx, reg_val, cdm_di_addr, cdm_reg_addr,
                              core_dw, flags);
    if (OSD_FAILED(rv)) return rv;

    return OSD_OK;
}

API_EXPORT
osd_result cl_cdm_cpureg_write(struct osd_hostmod_ctx *hostmod_ctx,
                               struct osd_cdm_desc *cdm_desc,
                               const void *reg_val, uint16_t reg_addr,
                               int flags)
{
    assert(hostmod_ctx);
    assert(cdm_desc);

    osd_result rv;

    uint16_t cdm_di_addr = cdm_desc->di_addr;
    uint16_t reg_addr_upper = reg_addr >> 15;
    uint16_t core_upper = cdm_desc->core_reg_upper;

    if (core_upper != reg_addr_upper) {
        rv = osd_hostmod_reg_write(hostmod_ctx, &reg_addr_upper, cdm_di_addr,
                                   OSD_REG_CDM_CORE_REG_UPPER, 16, 0);
        if (OSD_FAILED(rv)) return rv;
        cdm_desc->core_reg_upper = reg_addr_upper;
    }

    uint16_t cdm_reg_addr = 0x8000 + (reg_addr & 0x7fff);
    uint16_t core_dw = cdm_desc->core_data_width;
    assert(core_dw != 128
           && "128 bit wide register accesses are currently not supported.");

    rv = osd_hostmod_reg_write(hostmod_ctx, reg_val, cdm_di_addr, cdm_reg_addr,
                               core_dw, flags);
    if (OSD_FAILED(rv)) return rv;

    return OSD_OK;
}
