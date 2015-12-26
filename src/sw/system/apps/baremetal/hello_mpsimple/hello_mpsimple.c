/* Copyright (c) 2013-2015 by the author(s)
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
 * =============================================================================
 *
 * Simple hello world example.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */


#include <stdio.h> // For printf

#include <optimsoc-mp.h>
#include <or1k-support.h>
#include <optimsoc-baremetal.h>

#include <assert.h>

// In this example every rank except 0 sends a simple message to rank 0.
// Rank 0 will output a hello world for each sender and wait until all
// of them arrived.

// Used by rank 0 to count all received hello messages
unsigned int volatile hello_received = 0;

// This function is called by the driver on receival of a message
void recv(unsigned int *buffer,int len) {
    int source_tile, source_rank;

    assert(len==1); // This is always one flit

    // Extract tile id from flit
    source_tile = extract_bits(buffer[0],OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);

    // Calculate rank from tile
    source_rank = optimsoc_get_tilerank(source_tile);

    // Print hello for this
//    printf("Hello World from %d!\n",source_rank);

    // Count up received messages
    hello_received++;
}

// The main function
void main() {
	if (or1k_coreid() != 0)
		return;

    // Initialize optimsoc library
    optimsoc_init(0);
    optimsoc_mp_simple_init();

    // Add handler for received messages (of class 0)
    optimsoc_mp_simple_addhandler(0,&recv);
    or1k_interrupts_enable();

    // Determine tiles rank
    int rank = optimsoc_get_ctrank();

    if (rank==0) {
        // Rank 0 simply waits for all tiles to send their message
        printf("Wait for %d messages\n", optimsoc_get_numct()-1);
        while (hello_received < (optimsoc_get_numct()-1) ) {}

        // Conclude and print hello
        printf("Received all messages. Hello World!\n",rank,optimsoc_get_numct());
    } else {
        // The message is a one flit packet
        uint32_t buffer[1] = { 0 };

        // Set destination (tile 0)
        set_bits(&buffer[0],0,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);

        // Set class (0)
        set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);

        // Set sender as my rank
        set_bits(&buffer[0],optimsoc_get_ranktile(rank),OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);

        // Send the message
        optimsoc_mp_simple_send(1,(uint32_t*) buffer);
    }
}
