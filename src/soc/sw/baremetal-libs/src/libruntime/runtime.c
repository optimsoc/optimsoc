/* Copyright (c) 2012-2015 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Stefan Rösch <roe.stefan@gmail.com>
 */

#include "scheduler.h"
#include "vmm.h"
#include "thread.h"
#include "context.h"
#include <assert.h>
#include <or1k-support.h>
#include <optimsoc-baremetal.h>

#include <stdio.h>
#include "syscalls.h"

// This is placed in the BSS section and core 0 will initialize it
// to 0 during the reset routine.
volatile uint8_t _optimsoc_boot_barrier;

void optimsoc_runtime_boot(void) {
    printf("Boot runtime system\n");

    // Activate proper handling of exceptions in section trace
    OPTIMSOC_TRACE(0x23, 0);

    if (or1k_coreid() == 0) {
        printf("Initializing OpTiMSoC platform..\n");
        optimsoc_init(0);

        printf("Initialize DMA..\n");
        dma_init();

        _optimsoc_context_init();

        printf("Bringing virtual memory up..\n");
        _optimsoc_vmm_init();

        printf("Initialize system call interface..\n");
        _optimsoc_syscalls_init();

        printf("Initialize scheduler..\n");
        _optimsoc_scheduler_init();

        printf("Core 0 boot finished.\n");

        printf("Start the other cores.\n");
        _optimsoc_boot_barrier = 1;
    } else {
        printf("Wait to be woken up.\n");
        while (_optimsoc_boot_barrier == 0) { }
        printf("Core %d woken up.\n", optimsoc_get_relcoreid());
    }

    _optimsoc_scheduler_start();

    return;
}

