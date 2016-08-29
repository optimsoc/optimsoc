/* Copyright (c) 2016 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

#include "cli.h"

#include <assert.h>

static int fill_rand_bytes(uint8_t* data, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++) {
        data[i] = rand() % 256;
    }
}

static int memory_test(struct osd_context *ctx, uint16_t mod,
                       uint8_t region) {
    uint64_t addr;
    uint8_t *wdata, *rdata;
    size_t size;
    size_t blocksize;

    struct osd_memory_descriptor *desc;

    wdata = malloc(1024*1024);
    rdata = malloc(1024*1024);

    osd_get_memory_descriptor(ctx, mod, &desc);
    assert(desc);

    // Bytes per block
    blocksize = desc->data_width / 8;

    // Perform one aligned write of one word
    addr = desc->regions[region].base_addr;
    size = blocksize;
    for (size_t i = 0; i < size; i++) wdata[i] = i & 0xff;

    osd_memory_write(ctx, mod, addr, wdata, size);

    // Write the next ten blocks
    addr = desc->regions[region].base_addr + blocksize;
    size = blocksize * 10;
    for (size_t i = 0; i < size; i++) wdata[i] = i & 0xff;

    osd_memory_write(ctx, mod, addr, wdata, size);

    // Read back the first block
    addr = desc->regions[region].base_addr;
    size = blocksize;

    osd_memory_read(ctx, mod, addr, rdata, size);

    for (size_t i = 0; i < size; i++) {
        if (wdata[i] != rdata[i]) {
            printf("Test 0 failed\n");
            return -1;
        }
    }

    printf("Test 0 passed\n");

    // Read back the next ten blocks
    addr = desc->regions[region].base_addr + blocksize;
    size = blocksize * 10;

    osd_memory_read(ctx, mod, addr, rdata, size);

    for (size_t i = 0; i < size; i++) {
        if (wdata[i] != rdata[i]) {
            printf("Test 1 failed\n");
            return -1;
        }
    }

    printf("Test 1 passed\n");

    // Verify reading the hole memory
    unsigned int chunk;
    unsigned int chunk_size = 32*1024 / blocksize; /* read 32 kByte per chunk */
    unsigned int chunk_count = desc->regions[region].size / blocksize / chunk_size;
    for (chunk = 0; chunk < chunk_count; chunk++) {
        uint64_t chunk_addr_start = desc->regions[region].base_addr + chunk * chunk_size * blocksize;
        uint64_t chunk_addr_end = chunk_addr_start + (chunk_size * blocksize) - 1;
        printf("Verifying chunk %d of %d from 0x%lx to 0x%lx ...", chunk,
               chunk_count, chunk_addr_start, chunk_addr_end);
        fflush(stdout);

        // get reproducible random numbers to write and read back
        srand(chunk);
        fill_rand_bytes(wdata, chunk_size * blocksize);

        printf(" write ... ");
        fflush(stdout);
        osd_memory_write(ctx, mod, chunk_addr_start, wdata, chunk_size * blocksize);
        printf(" read ... ");
        fflush(stdout);
        osd_memory_read(ctx, mod, chunk_addr_start, rdata, chunk_size * blocksize);

        for (size_t i = 0; i < chunk_size; i++) {
            if (wdata[i] != rdata[i]) {
                printf(" failed at address 0x%lx: wrote 0x%x, read 0x%x\n",
                       chunk_addr_start + i, wdata[i], rdata[i]);
                return -1;
            }
        }
        printf(" passed\n");
    }

//    Byte-wise writing not implemented in MAM yet
//
//    // Test 2: Check writing single bytes
//
//    // First write three blocks
//    size = blocksize * 3;
//
//    for (size_t i = 0; i < size; i++) {
//        wdata[i] = (0x7c - i) % 0xff;
//    }
//
//    addr = 0;
//    osd_memory_write(ctx, mod, addr, wdata, size);
//
//    // Now write each a single byte and check it
//    // Write into the second block
//    addr = blocksize;
//    for (size_t b = 0; b < blocksize; b++) {
//        uint8_t w = 0xd9 - b;
//        osd_memory_write(ctx, mod, addr + b, &w, 1);
//
//        wdata[addr + b] = w;
//
//        osd_memory_read(ctx, mod, 0, rdata, size);
//
//        for (size_t i = 0; i < size; i++) {
//            if (wdata[i] != rdata[i]) {
//                printf("Test 2 failed in iteration %zu\n", b);
//                return -1;
//            }
//        }
//    }
//
//    printf("Test 2 passed\n");

    return 0;
}

int memory_tests(struct osd_context *ctx) {
    // Get list of memories
    uint16_t *memories;
    size_t num_memories;
    int success = 1;

    int rv = osd_get_memories(ctx, &memories, &num_memories);
    if (rv != OSD_SUCCESS) {
        printf("Unable to obtain a list of memories in the system.\n");
        return 0;
    }

    for (size_t m = 0; m < num_memories; m++) {
        printf("Test memory connected to MAM with ID %d\n", memories[m]);
        if (memory_test(ctx, memories[m], 0) != 0) {
            printf("Failed\n");
            success = 0;
        } else {
            printf("Passed\n");
        }
    }

    free(memories);

    return success;
}
