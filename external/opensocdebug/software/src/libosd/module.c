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

#include <osd/module.h>
#include <osd/osd.h>
#include "osd-private.h"

#define LIST_ENTRY(type_id, shortname, longname) [type_id] = #shortname,
static const char* OSD_MODULE_TYPE_STD_shortnames[] = {OSD_MODULE_TYPE_STD_LIST};
#undef LIST_ENTRY

#define LIST_ENTRY(type_id, shortname, longname) [type_id] = longname,
static const char* OSD_MODULE_TYPE_STD_longnames[] = {OSD_MODULE_TYPE_STD_LIST};
#undef LIST_ENTRY

API_EXPORT
const char* osd_module_get_type_short_name(unsigned int vendor_id,
                                           unsigned int type_id)
{
    if (vendor_id != OSD_MODULE_VENDOR_OSD) {
        return "UNKNOWN";
    }

    if (type_id >= sizeof(OSD_MODULE_TYPE_STD_shortnames)) {
        return "UNKNOWN";
    }
    return OSD_MODULE_TYPE_STD_shortnames[type_id];
}

API_EXPORT
const char* osd_module_get_type_long_name(unsigned int vendor_id,
                                          unsigned int type_id)
{
    if (vendor_id != OSD_MODULE_VENDOR_OSD) {
        return "UNKNOWN";
    }
    if (type_id >= sizeof(OSD_MODULE_TYPE_STD_longnames)) {
        return "UNKNOWN";
    }
    return OSD_MODULE_TYPE_STD_longnames[type_id];
}
