/*
 * This file is part of liboptimsochost.
 *
 * liboptimsochost is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * liboptimsochost is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with liboptimsochost. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * liboptimsochost is a host-side library for talking with the OpTiMSoC system.
 *
 * For connecting with the system, the USB interface of the Debug NoC is used.
 *
 * This file is the private header file, i.e. it is not installed and not
 * part of the public API. All functions declared here will not be exported
 * from the library and cannot be called by external users.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 */

#ifndef _LIBOPTIMSOC_PRIVATE_H_
#define _LIBOPTIMSOC_PRIVATE_H_

#include <stdbool.h>
#include <pthread.h>

#include <optimsochost/liboptimsochost.h>

#define OPTIMSOC_EXPORT __attribute__ ((visibility("default")))

#ifdef DEBUG
//#  define DEBUG_DUMP_DATA
#endif

struct optimsoc_sysinfo {
    /** unique system identifier */
    uint16_t sysid;
    /** number of debug modules in addition to the TCM */
    uint16_t dbg_module_count;
    /** debug modules */
    struct optimsoc_dbg_module *dbg_modules;
    /** configuration of the ITM modules */
    struct optimsoc_itm_config **itm_config;
    /** configuration of the MAM modules */
    struct optimsoc_mam_config **mam_config;
};

void optimsoc_sysinfo_free(struct optimsoc_sysinfo* sysinfo);

#endif /* _LIBOPTIMSOC_PRIVATE_H_ */
