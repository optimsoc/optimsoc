/*
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Driver for the compare and swap functionality.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include <spr-defs.h>
#include <or1k-support.h>

extern uint32_t optimsoc_critical_begin(void);
extern void optimsoc_critical_end(uint32_t);

uint32_t cas(void* address, uint32_t compare, uint32_t value) {
    uint32_t restore;

    restore = optimsoc_critical_begin();

    unsigned int result;

    REG32(0x7ffffffc) = (uint32_t) address;
    REG32(0x7ffffffc) = compare;
    REG32(0x7ffffffc) = value;
    result = REG32(0x7ffffffc);

    optimsoc_critical_end(restore);

    return result;
}
