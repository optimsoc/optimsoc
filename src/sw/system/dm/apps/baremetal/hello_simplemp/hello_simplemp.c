/**
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
 * Hello world example that demonstrates simple message passing.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include <stdio.h> // For printf

#include <optimsoc.h>
#include <or1k-support.h>
#include <optimsoc-baremetal.h>
#include <assert.h>

// In this example every rank except 0 sends a simple message to rank 0.
// Rank 0 will output a hello world for each sender and wait until all
// of them arrived. Then it replies to all of them to terminate.

// Used by rank 0 to count all received hello messages
unsigned int volatile hello_received = 0;

int rank = 0;

// This function is called by the driver on receival of a message
void recv(unsigned int *buffer,int len) {
    int source_tile, source_rank;

    assert(len==1); // This is always one flit

    // Extract tile id from flit
    source_tile = extract_bits(buffer[0],OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);

    // Calculate rank from tile
    source_rank = optimsoc_tilerank(source_tile);

    // Print hello for this
    printf("Hello World from %d!\n",source_rank);

    // Count up received messages
    hello_received++;
}

// The main function
void main() {
    // Trace to the debug system
    optimsoc_trace_definesection(0, "Init");
    optimsoc_trace_definesection(1, "Print & Send");
    optimsoc_trace_definesection(2, "Wait for all");
    optimsoc_trace_definesection(3, "Wait for reply");
    optimsoc_trace_definesection(4, "Send replies");
    optimsoc_trace_definesection(5, "Done");
    optimsoc_trace_section(0);

    // Initialize optimsoc library
    optimsoc_init(0);
    optimsoc_mp_simple_init();
    or1k_enable_interrupts();

    // Add handler for received messages (of class 0)
    optimsoc_mp_simple_addhandler(0,&recv);

    // Determine tiles rank
    //    int rank = optimsoc_ctrank();
    int rank = optimsoc_get_tileid();

    if (rank==0) {
        // Trace
        optimsoc_trace_section(2);
  
        // Rank 0 simply waits for all tiles to send their message
        while (hello_received < (optimsoc_ctnum()-1) ) {}

        // Conclude and print hello
        printf("Received all messages. Hello World!\n",rank,optimsoc_ctnum());
	
        optimsoc_trace_section(4);
        
	for (int d=1; d <= hello_received; d++) {
	    // The message is a one flit packet
	    uint32_t buffer[1] = { 0 };

	    // Set destination (tile 0)
	    set_bits(&buffer[0],d,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);

	    // Set class (0)
	    set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
	    
	    // Set sender as my rank
	    set_bits(&buffer[0],optimsoc_ranktile(rank),OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
	    
	    // Send the message
	    optimsoc_mp_simple_send(1,(uint32_t*) buffer);
	}

	optimsoc_trace_section(5);
    } else {
        // Trace
        optimsoc_trace_section(1);

        // The message is a one flit packet
        uint32_t buffer[1] = { 0 };

        // Set destination (tile 0)
        set_bits(&buffer[0],0,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);

        // Set class (0)
        set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);

        // Set sender as my rank
        set_bits(&buffer[0],optimsoc_ranktile(rank),OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);

        // Send the message
        optimsoc_mp_simple_send(1,(uint32_t*) buffer);

	optimsoc_trace_section(3);

        while(hello_received == 0) {}
	
	optimsoc_trace_section(5);	
    }
}

