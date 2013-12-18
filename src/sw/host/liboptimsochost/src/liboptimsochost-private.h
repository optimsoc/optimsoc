/* Copyright (c) 2012-2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
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
