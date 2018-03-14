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

#include <osd/cl_ctm.h>

#include <assert.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/reg.h>
#include "osd-private.h"

static struct osd_ctm_event *build_ctm_event(
    const struct osd_ctm_desc *ctm_desc, const struct osd_packet *pkg)
{
    struct osd_ctm_event *ev = calloc(1, sizeof(struct osd_ctm_event));

    if (osd_packet_get_type_sub(pkg) == EV_OVERFLOW) {
        assert(osd_packet_sizeconv_payload2data(1) == pkg->data_size_words &&
               "STM Protocol violation detected.");

        ev->overflow = pkg->data.payload[0];
        return ev;
    }
    size_t exp_payload_len_bit = 32                          // timestamp
                                 + ctm_desc->addr_width_bit  // npc
                                 + ctm_desc->addr_width_bit  // pc
                                 + 2                         // mode
                                 + 1                         // ret
                                 + 1                         // call
                                 + 1;                        // modechange
    size_t exp_payload_len = INT_DIV_CEIL(exp_payload_len_bit, 16);
    assert(osd_packet_sizeconv_payload2data(exp_payload_len) ==
               pkg->data_size_words &&
           "CTM Protocol violation detected.");

    unsigned int aw_words = ctm_desc->addr_width_bit / 16;

    size_t w = 0;

    ev->overflow = 0;
    ev->timestamp = (pkg->data.payload[w + 1] << 16) | pkg->data.payload[w];
    w += 2;

    for (unsigned int i = 0; i < aw_words; i++) {
        ev->npc |= (uint64_t)pkg->data.payload[w + i] << (i * 16);
    }
    w += aw_words;

    for (unsigned int i = 0; i < aw_words; i++) {
        ev->pc |= (uint64_t)pkg->data.payload[w + i] << (i * 16);
    }
    w += aw_words;

    ev->mode = pkg->data.payload[w] & 0x3;
    ev->is_ret = pkg->data.payload[w] >> 2 & 0x1;
    ev->is_call = pkg->data.payload[w] >> 3 & 0x1;
    ev->is_modechange = pkg->data.payload[w] >> 4 & 0x1;

    return ev;
}

API_EXPORT
osd_result osd_cl_ctm_handle_event(void *arg, struct osd_packet *pkg)
{
    assert(arg &&
           "You need to give an event_handler_arg of type "
           "struct osd_ctm_event_handler in osd_hostmod_new()");
    assert(pkg);

    struct osd_ctm_event_handler *handler = arg;

    struct osd_ctm_event *ev = build_ctm_event(handler->ctm_desc, pkg);
    osd_packet_free(&pkg);

    handler->cb_fn(handler->cb_arg, handler->ctm_desc, ev);

    free(ev);

    return OSD_OK;
}

static bool is_ctm_module(struct osd_hostmod_ctx *hostmod_ctx,
                          unsigned int ctm_di_addr)
{
    osd_result rv;

    struct osd_module_desc desc;

    rv = osd_hostmod_mod_describe(hostmod_ctx, ctm_di_addr, &desc);
    if (OSD_FAILED(rv)) {
        return false;
    }

    if (desc.vendor != OSD_MODULE_VENDOR_OSD ||
        desc.type != OSD_MODULE_TYPE_STD_CTM || desc.version != 0) {
        return false;
    }

    return true;
}

API_EXPORT
osd_result osd_cl_ctm_get_desc(struct osd_hostmod_ctx *hostmod_ctx,
                               unsigned int ctm_di_addr,
                               struct osd_ctm_desc *ctm_desc)
{
    assert(hostmod_ctx);
    assert(ctm_desc);

    osd_result rv;

    ctm_desc->di_addr = ctm_di_addr;

    if (!is_ctm_module(hostmod_ctx, ctm_di_addr)) {
        return OSD_ERROR_WRONG_MODULE;
    }

    uint16_t regvalue;
    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, ctm_di_addr,
                              OSD_REG_CTM_ADDR_WIDTH, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    assert((regvalue == 16 || regvalue == 32 || regvalue == 64) &&
           "Spec violation: ADDR_WIDTH register has an invalid value.");
    ctm_desc->addr_width_bit = regvalue;

    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, ctm_di_addr,
                              OSD_REG_CTM_DATA_WIDTH, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    assert((regvalue == 16 || regvalue == 32 || regvalue == 64) &&
           "Spec violation: DATA_WIDTH register has an invalid value.");
    ctm_desc->data_width_bit = regvalue;

    return OSD_OK;
}
