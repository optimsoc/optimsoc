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

#include <assert.h>
#include <osd/osd.h>
#include "osd-private.h"

static const struct osd_version osd_version_internal = {
    OSD_VERSION_MAJOR, OSD_VERSION_MINOR, OSD_VERSION_MICRO,
    OSD_VERSION_SUFFIX};

/**
 * Get the version of the library
 *
 * @return the library version information
 */
API_EXPORT
const struct osd_version* osd_version_get(void)
{
    return &osd_version_internal;
}

/**
 * Get the subnet for a debug interconnect address
 */
API_EXPORT
unsigned int osd_diaddr_subnet(unsigned int diaddr)
{
    return diaddr >> OSD_DIADDR_LOCAL_BITS;
}

/**
 * Get the local part of a debug interconnect address (i.e. without the subnet)
 *
 */
API_EXPORT
unsigned int osd_diaddr_localaddr(unsigned int diaddr)
{
    return diaddr & ((1 << OSD_DIADDR_LOCAL_BITS) - 1);
}

/**
 * Construct a debug interconnect address out of subnet and local address
 */
API_EXPORT
unsigned int osd_diaddr_build(unsigned int subnet, unsigned int local_diaddr)
{
    assert(subnet <= OSD_DIADDR_SUBNET_MAX);
    assert(local_diaddr <= OSD_DIADDR_LOCAL_MAX);

    return subnet << OSD_DIADDR_LOCAL_BITS | local_diaddr;
}
