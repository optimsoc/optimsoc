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

#ifndef OSD_CL_DEM_UART_H
#define OSD_CL_DEM_UART_H

#include <stdlib.h>

#include <osd/hostmod.h>
#include <osd/osd.h>
#include <osd/packet.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*osd_cl_dem_uart_handler_fn)(void * /*cb_arg*/, const char * /*str*/,
                                           size_t /*len*/);

struct osd_dem_uart_event_handler {
    osd_cl_dem_uart_handler_fn cb_fn;
    void *cb_arg;
};

struct osd_dem_uart_desc {
    uint16_t di_addr;
};

/**
 * Obtain information about a UART Device-Emulation-Module (DEM)
 *
 * @param hostmod_ctx the host module handling the communication
 * @param dem_uart_di_adr DI address of the UART-DEM to describe
 * @param dem_uart_desc a pre-allocated dem_uart descriptor for the result
 *
 * @return OSD_OK on success, any other value indicates an error
 */
osd_result osd_cl_dem_uart_get_desc(struct osd_hostmod_ctx *hostmod_ctx,
                                    uint16_t dem_uart_di_addr,
                                    struct osd_dem_uart_desc *dem_uart_desc);

/**
 * Handle a packet received from a UART Device-Emulation-Module (DEM)
 *
 * This function should be passed to osd_hostmod_new. It converts the
 * received osd_packet into a simple string and calls the specified
 * callback function.
 *
 * @param handler the event handler that should handle this event
 * @param pkg the osd_packet to be handled
 *
 * @return OSD_OK on success, any other value indicates an error
 */
osd_result osd_cl_dem_uart_receive_event(void *handler, struct osd_packet *pkg);

/**
 * Send the data to the UART Device-Emulation-Module (DEM)
 *
 * @param hostmod_ctx the context in which this event should be handled
 * @param dem_uart_desc a valid dem_uart_desc containing the di_addr
 * @param str pointer to the data to be sent
 * @param len number of bytes to be send starting at str
 *
 * @return OSD_OK on success, any other value indicates an error
 *
 * @see osd_cl_dem_uart_get_desc
 */
osd_result osd_cl_dem_uart_send_string(struct osd_hostmod_ctx *hostmod_ctx,
                                       struct osd_dem_uart_desc *dem_uart_desc,
                                       const char *str, size_t len);

#ifdef __cplusplus
}
#endif

#endif  // OSD_CL_DEM_UART_H
