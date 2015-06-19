/* Copyright (c) 2015 by the author(s)
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
 * =================================================================
 *
 * Utility functions
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include "include/optimsoc-baremetal.h"

uint32_t optimsoc_get_numct(void) {
    return REG32(OPTIMSOC_NA_CT_NUM);
}

int optimsoc_get_ctrank(void) {
    return optimsoc_get_tilerank(optimsoc_get_tileid());
}


int optimsoc_get_tilerank(unsigned int tile) {
    uint16_t *ctlist = (uint16_t*) OPTIMSOC_NA_CT_LIST;
    for (int i = 0; i < optimsoc_get_numct(); i++) {
        if (REG16(&ctlist[i]) == tile) {
            return i;
        }
    }
    return -1;
}

int optimsoc_get_ranktile(unsigned int rank) {
    uint16_t *ctlist = (uint16_t*) OPTIMSOC_NA_CT_LIST;
    return ctlist[rank];
}

void optimsoc_init(optimsoc_conf *config) {

}

uint32_t optimsoc_mainmem_size() {
    return REG32(OPTIMSOC_NA_GMEM_SIZE);
}

uint32_t optimsoc_mainmem_tile() {
    return REG32(OPTIMSOC_NA_GMEM_TILE);
}

uint32_t optimsoc_noc_maxpacketsize(void) {
    return 32;
}

void optimsoc_trace_definesection(int id, char* name) {
    OPTIMSOC_TRACE(0x20,id);
    while (*name!=0) {
        OPTIMSOC_TRACE(0x21,*name);
        name = name + 1;
    }
}

void optimsoc_trace_defineglobalsection(int id, char* name) {

}

void optimsoc_trace_section(int id) {
    OPTIMSOC_TRACE(0x22,id);
}

void optimsoc_trace_kernelsection(void) {
    OPTIMSOC_TRACE(0x23,0);
}

void optimsoc_mutex_init(optimsoc_mutex_t *mutex) {
    *mutex = 0;
}

void optimsoc_mutex_lock(optimsoc_mutex_t *mutex) {
    while (or1k_sync_tsl(mutex) != 0) {}
}

void optimsoc_mutex_unlock(optimsoc_mutex_t *mutex) {
    *mutex = 0;
}

uint32_t optimsoc_get_seed(void) {
    return REG32(OPTIMSOC_NA_SEED);
}
