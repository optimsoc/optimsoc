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

#ifndef OSD_REG_H
#define OSD_REG_H

#include <osd/osd.h>

/**
 * @defgroup libosd-reg Register definitions
 * @ingroup libosd
 *
 * @{
 */

// register maps
// base register map (common across all debug modules)
#define OSD_REG_BASE_MOD_VENDOR 0x0000     /* module type */
#define OSD_REG_BASE_MOD_TYPE 0x0001       /* module version */
#define OSD_REG_BASE_MOD_VERSION 0x0002    /* module vendor */
#define OSD_REG_BASE_MOD_CS 0x0003         /* control and status */
#define OSD_REG_BASE_MOD_CS_ACTIVE_BIT 0   /* activate/stall module */
#define OSD_REG_BASE_MOD_EVENT_DEST 0x0004 /* event destination */

/* unused with new format; keep as reference for other SHIFT/MASK use cases
  #define OSD_REG_BASE_MOD_EVENT_DEST_ADDR_SHIFT 0
  #define OSD_REG_BASE_MOD_EVENT_DEST_ADDR_MASK  ((1 << 10) - 1)
*/

// SCM register map
#define OSD_REG_SCM_SYSTEM_VENDOR_ID 0x0200
#define OSD_REG_SCM_SYSTEM_DEVICE_ID 0x0201
#define OSD_REG_SCM_NUM_MOD 0x0202
#define OSD_REG_SCM_MAX_PKT_LEN 0x0203
#define OSD_REG_SCM_SYSRST 0x0204
#define OSD_REG_SCM_SYSRST_SYS_RST_BIT 0
#define OSD_REG_SCM_SYSRST_CPU_RST_BIT 1

// MAM register map
#define OSD_REG_MAM_AW 0x0200
#define OSD_REG_MAM_DW 0x0201
#define OSD_REG_MAM_REGIONS 0x0202
#define OSD_REG_MAM_REGION_BASEADDR(region, word) (0x0280 + (0x10 * region) + (word))
#define OSD_REG_MAM_REGION_MEMSIZE(region, word) (0x0284 + (0x10 * region) + (word))

// STM register map
#define OSD_REG_STM_VALWIDTH 0x0200

// CTM register map
#define OSD_REG_CTM_ADDR_WIDTH 0x0200
#define OSD_REG_CTM_DATA_WIDTH 0x0200

/**@}*/ /* end of doxygen group libosd-reg */

#endif  // OSD_REG_H
