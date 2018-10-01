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

#include <osd/cl_dem_uart.h>

#include <assert.h>
#include <osd/osd.h>
#include <stdio.h>
#include <unistd.h>
#include "osd-private.h"

static bool is_dem_uart_module(struct osd_hostmod_ctx *hostmod_ctx,
                               uint16_t dem_uart_di_addr)
{
    osd_result rv;

    struct osd_module_desc desc;

    rv = osd_hostmod_mod_describe(hostmod_ctx, dem_uart_di_addr, &desc);
    if (OSD_FAILED(rv)) {
        return false;
    }

    if (desc.vendor != OSD_MODULE_VENDOR_OSD ||
        desc.type != OSD_MODULE_TYPE_STD_DEM_UART || desc.version != 0) {
        return false;
    }

    return true;
}

API_EXPORT
osd_result osd_cl_dem_uart_get_desc(struct osd_hostmod_ctx *hostmod_ctx,
                                    uint16_t dem_uart_di_addr,
                                    struct osd_dem_uart_desc *dem_uart_desc)
{
    assert(hostmod_ctx);
    assert(dem_uart_desc);

    if (!is_dem_uart_module(hostmod_ctx, dem_uart_di_addr)) {
        return OSD_ERROR_WRONG_MODULE;
    }

    dem_uart_desc->di_addr = dem_uart_di_addr;

    return OSD_OK;
}

API_EXPORT
osd_result osd_cl_dem_uart_receive_event(void *handler, struct osd_packet *pkg)
{
    assert(handler &&
           "You need to give an event_handler_arg of type "
           "struct osd_dem_uart_event_handler in osd_hostmod_new()");
    assert(pkg);

    struct osd_dem_uart_event_handler *dem_uart_event_handler = handler;

    char c = pkg->data.payload[0] & 0xFF;

    osd_packet_free(&pkg);

    dem_uart_event_handler->cb_fn(dem_uart_event_handler->cb_arg, &c, 1);

    return OSD_OK;
}

API_EXPORT
osd_result osd_cl_dem_uart_send_string(struct osd_hostmod_ctx *hostmod_ctx,
                                       struct osd_dem_uart_desc *dem_uart_desc,
                                       const char *str, size_t len)
{
    osd_result rv;

    assert(str && len > 0);

    struct osd_packet *packet;
    osd_packet_new(&packet, osd_packet_sizeconv_payload2data(1));

    osd_packet_set_header(packet, dem_uart_desc->di_addr,
                          osd_hostmod_get_diaddr(hostmod_ctx),
                          OSD_PACKET_TYPE_EVENT, EV_LAST);

    for (size_t i = 0; i < len; i++) {
        packet->data.payload[0] = str[i] & 0xFF;

        rv = osd_hostmod_event_send(hostmod_ctx, packet);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    osd_packet_free(&packet);

    return OSD_OK;
}
