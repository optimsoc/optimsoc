/* Copyright 2017 The Open SoC Debug Project
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

#ifndef OSD_MODULE_H
#define OSD_MODULE_H

#include <stdint.h>

/**
 * @defgroup libosd-module Module definitions
 * @ingroup libosd
 *
 * @{
 */

/**
 * List of all registered vendor IDs
 *
 * Keep this list in sync with
 * http://opensocdebug.readthedocs.io/en/latest/05_idregistry/index.html
 */
#define OSD_MODULE_VENDOR_LIST                            \
    LIST_ENTRY(0x0000, UNKNOWN, "UNKNOWN")                \
    LIST_ENTRY(0x0001, OSD, "The Open SoC Debug Project") \
    LIST_ENTRY(0x0002, OPTIMSOC, "The OpTiMSoC Project")  \
    LIST_ENTRY(0x0003, LOWRISC, "LowRISC")

/**
 * List of module types as defined in the Open SoC Debug Specification
 *
 * All modules in this list have a vendor id 0x0001
 */
#define OSD_MODULE_TYPE_STD_LIST                                 \
    LIST_ENTRY(0x0000, UNKNOWN, "UNKNOWN")                       \
    LIST_ENTRY(0x0001, SCM, "Subnet Control Module")             \
    LIST_ENTRY(0x0002, DEM_UART, "Device Emulation Module UART") \
    LIST_ENTRY(0x0003, MAM, "Memory Access Module")              \
    LIST_ENTRY(0x0004, STM, "System Trace Module")               \
    LIST_ENTRY(0x0005, CTM, "Core Trace Module")

/**
 * Vendor identifiers
 */
enum osd_module_vendor {
#define LIST_ENTRY(vendor_id, shortname, longname) \
    OSD_MODULE_VENDOR_##shortname = vendor_id,  //!< longname
    OSD_MODULE_VENDOR_LIST
#undef LIST_ENTRY
};

/**
 * Module type identifiers for the standard-defined modules (vendor id 0x0001)
 */
enum osd_module_type_std {
#define LIST_ENTRY(type_id, shortname, longname) \
    OSD_MODULE_TYPE_STD_##shortname = type_id,  //!< longname
    OSD_MODULE_TYPE_STD_LIST
#undef LIST_ENTRY
};

/**
 * A single module instance in the Open SoC Debug system
 */
struct osd_module_desc {
    uint16_t addr;     //!< Module address
    uint16_t vendor;   //!< Module version
    uint16_t type;     //!< Module type
    uint16_t version;  //!< Module version
};

/**
 * Get the short name for a debug module
 *
 * This returns the short type name for all well-known debug modules, and
 * "UNKNOWN" otherwise.
 *
 * @param vendor_id the vendor ID of the debug module
 * @param type_id the type ID of the debug module
 * @return the module name acronym (e.g. "CTM") or "UNKNOWN"
 */
const char* osd_module_get_type_short_name(unsigned int vendor_id,
                                           unsigned int type_id);

/**
 * Get the long name for a debug module
 *
 * This returns the long type name for all well-known debug modules, and
 * "UNKNOWN" otherwise.
 *
 * @param vendor_id the vendor ID of the debug module
 * @param type_id the type ID of the debug module
 * @return the full module name (e.g. "Core Trace Module")
 */
const char* osd_module_get_type_long_name(unsigned int vendor_id,
                                          unsigned int type_id);

/**@}*/ /* end of doxygen group libosd-module */

#endif  // OSD_MODULE_H
