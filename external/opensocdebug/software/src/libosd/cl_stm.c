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

#include <osd/cl_stm.h>

#include <assert.h>
#include <osd/osd.h>
#include <osd/reg.h>
#include <osd/packet.h>
#include "osd-private.h"

static struct osd_stm_event* build_stm_event(const struct osd_stm_desc *stm_desc,
                                             const struct osd_packet *pkg)
{
    struct osd_stm_event *ev = calloc(1, sizeof(struct osd_stm_event));

    if (osd_packet_get_type_sub(pkg) == EV_OVERFLOW) {
        assert(osd_packet_sizeconv_payload2data(1) == pkg->data_size_words
                   && "STM Protocol violation detected.");

        ev->overflow = pkg->data.payload[0];
        return ev;
    }

    size_t exp_payload_len_bit =
        32 // timestamp
        + 16 // id
        + stm_desc->value_width_bit; // value
    size_t exp_payload_len = INT_DIV_CEIL(exp_payload_len_bit, 16);
    assert(osd_packet_sizeconv_payload2data(exp_payload_len)
           == pkg->data_size_words
           && "STM Protocol violation detected.");

    uint32_t timestamp = (pkg->data.payload[1] << 16) | pkg->data.payload[0];
    uint16_t id = pkg->data.payload[2];
    uint64_t value = 0;
    unsigned int valw_words = stm_desc->value_width_bit / 16;
    for (unsigned int i = 0; i < valw_words; i++) {
        value |= (uint64_t)pkg->data.payload[3 + i] << (i * 16);
    }

    ev->timestamp = timestamp;
    ev->id = id;
    ev->value = value;
    ev->overflow = 0;

    return ev;
}

API_EXPORT
osd_result osd_cl_stm_handle_event(void *arg, struct osd_packet *pkg)
{
    assert(arg &&
           "You need to give an event_handler_arg of type "
           "struct osd_stm_event_handler in osd_hostmod_new()");
    assert(pkg);

    struct osd_stm_event_handler *handler = arg;

    struct osd_stm_event *ev = build_stm_event(handler->stm_desc, pkg);
    osd_packet_free(&pkg);

    handler->cb_fn(handler->cb_arg, handler->stm_desc, ev);

    free(ev);

    return OSD_OK;
}

static bool is_stm_module(struct osd_hostmod_ctx *hostmod_ctx,
                          unsigned int stm_di_addr)
{
    osd_result rv;

    struct osd_module_desc desc;

    rv = osd_hostmod_mod_describe(hostmod_ctx, stm_di_addr, &desc);
    if (OSD_FAILED(rv)) {
        return false;
    }

    if (desc.vendor != OSD_MODULE_VENDOR_OSD ||
        desc.type != OSD_MODULE_TYPE_STD_STM || desc.version != 0) {
        return false;
    }

    return true;
}

API_EXPORT
osd_result osd_cl_stm_get_desc(struct osd_hostmod_ctx *hostmod_ctx,
                               unsigned int stm_di_addr,
                               struct osd_stm_desc *stm_desc)
{
    assert(hostmod_ctx);
    assert(stm_desc);

    osd_result rv;

    stm_desc->di_addr = stm_di_addr;

    if (!is_stm_module(hostmod_ctx, stm_di_addr)) {
        return OSD_ERROR_WRONG_MODULE;
    }

    uint16_t regvalue;
    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, stm_di_addr,
                              OSD_REG_STM_VALWIDTH, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    assert((regvalue == 16 || regvalue == 32 || regvalue == 64)
           && "Spec violation: VALWIDTH register has an invalid value.");
    stm_desc->value_width_bit = regvalue;

    return OSD_OK;
}

API_EXPORT
bool osd_cl_stm_is_print_event(const struct osd_stm_event *ev)
{
    return ev->overflow == 0 && ev->id == 4;
}

API_EXPORT
osd_result osd_cl_stm_print_buf_new(struct osd_cl_stm_print_buf **print_buf_p)
{
    struct osd_cl_stm_print_buf *pb =
            calloc(1, sizeof(struct osd_cl_stm_print_buf));
    *print_buf_p = pb;
    return OSD_OK;
}

API_EXPORT
void osd_cl_stm_print_buf_free(struct osd_cl_stm_print_buf **print_buf_p)
{
    assert(print_buf_p);
    struct osd_cl_stm_print_buf *print_buf = *print_buf_p;
    assert(print_buf);

    free(print_buf->buf);
    free(print_buf);
    *print_buf_p = NULL;
}

API_EXPORT
osd_result osd_cl_stm_add_to_print_buf(const struct osd_stm_event *ev,
                                       struct osd_cl_stm_print_buf *buf,
                                       bool *should_flush)
{
    size_t new_len_str = buf->len_str + 1;

    if (buf->len_buf < new_len_str + 1) {
        buf->len_buf += 4 * 1024; // increase buffer size by 4 KByte
        buf->buf = realloc(buf->buf, buf->len_buf);
    }

    assert(ev->value <= UINT8_MAX);
    char c = (uint8_t)ev->value;
    buf->buf[new_len_str - 1] = c;
    buf->buf[new_len_str] = '\0';
    buf->len_str = new_len_str;

    *should_flush = (c == '\n');

    return OSD_OK;
}
