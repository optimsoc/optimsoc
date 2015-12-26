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

#include <optimsoc-baremetal.h>
#include <or1k-support.h>

#include <assert.h>
#include <stdio.h>  // For printf
#include <stdlib.h> // For malloc
#include <string.h> // For memcpy

// The dimensions of the heat area
#define XSIZE 10
#define YSIZE 10

// The number of iterations
#define ITERATIONS 10

// As this problem is embarrasingly parallel, it is quite easy to just split the
// area as a matrix. As the direct neighbor elements are required in the
// calculation of the next value, the borders of each matrix need to be send to
// the neighbors.
// For example assume a 30x20 area and six cores are used for calculation.
// The work can then be split as
//   x>    10         10         10
//  y +----------+----------+----------+
//  v |          |          |          |
//    |  CPU 0   |  CPU 1   |  CPU 2   | 10
//    |          |          |          |
//    +----------+----------+----------+
//    |          |          |          |
//    |  CPU 3   |  CPU 4   |  CPU 5   | 10
//    |          |          |          |
//    +----------+----------+----------+
//
// The update value of each element is the average of the current values of the four
// direct neighbors, so that for example CPU 1 has to send:
//   * all (0,y) for y=(0,..,9) to CPU 0
//   * all (x,9) for x=(0,..,9) to CPU 4
//   * all (9,y) for y=(0,..,9) to CPU 2

// The split of the work to the processors is determined by this vector.
// Worksharing of up to 16 cores is supported at the moment. Each value
// of processor cores has a tupel containing the number of rows and cols
// of the grid.
//
//   N=1      N=2          N=3          N=4              N=5
//  +---+  +---+---+  +---+---+---+  +---+---+  +---+---+---+---+---+
//  |   |  |   |   |  |   |   |   |  |   |   |  |   |   |   |   |   |
//  +---+  +---+---+  +---+---+---+  +---+---+  +---+---+---+---+---+
//                                   |   |   |
//                                   +---+---+
//       N=6            N=7              N=8
//  +---+---+---+  +---+---+---+  +---+---+---+---+
//  |   |   |   |  |   |   |   |  |   |   |   |   |
//  +---+---+---+  +---+---+---+  +---+---+---+---+   etc.
//  |   |   |   |  |   |   |   |  |   |   |   |   |
//  +---+---+---+  +---+---+---+  +---+---+---+---+
//
// In some cases (e.g., 7) the rectangular structure can not be efficiently
// mapped. In such cases some processors just remain inactive.

unsigned int workshare[16][2] = { {1,1}, {2,1}, {3,1}, {2,2}, {5,1},
        {3,2}, {3,2}, {4,2}, {3,3}, {5,2},
        {5,2}, {4,3}, {4,3}, {7,2}, {5,3}, {4,4} };

// We have two matrices for the calculation. One holds the current values, the
// other the next values of the elements. To avoid copying, the current calculation
// and the previous matrix are just changes by pointing to the other one.
float *matrix[2];

// The result matrix contains all elements (not only the grid of this core (rank)
// and is only processed by core 0 (rank 0).
float *result;

// Points to the matrix we currently process on (of float *matrix[2] above).
int curmatrix;

// This core's rank and the total number of cores
int rank,total;

// Number of rows and cols (as from workshare array)
int xcount,ycount;

// Our col and row
int col,row;

// This rank processes on its grid subset, but when committing the results needs
// to know its absolute position.
int xbase,ybase;

// Helping variables: Does this rank's grid lie on one of the boundaries of the
// entire grid?
int leftbound,rightbound,topbound,bottombound;

// x and y dimensions of the grid of this rank (not necessary equal on all!)
int xdim,ydim;

// The actual matrix of this rank is larger than these dimensions as it also
// holds the direct neighbors of all elements. Those are either calculated by
// the neighboring ranks or are fixed values (boundary condition).
// The actual elements are therefore adressed with offset of 1, leading to
// 1,..,xdim as values for the elements plus 0 and xdim+1 as neighbor elements.
//
// Macro that helps adressing the elements in this matrix
#define POS(x,y) ((y)*(xdim+2)+(x))

// This function (see below) executes the iterations and coordinates the ranks
void heat();

// This function handles incoming messages from other ranks
void recv(unsigned int *buffer,int len);

// Barrier for all cores
void barrier();

// Here we go..
int main() {
    // Initialize the OpTiMSoC library
    optimsoc_init(0);

    optimsoc_mp_simple_init();

    // Add a handler for class 0 packets to the mpsimple message passing
    // driver. The driver will execute recv (see definition above) each
    // time a packet arrives.
    optimsoc_mp_simple_addhandler(0,&recv);

    or1k_interrupts_enable();

    // Determine my rank and total number
    rank = optimsoc_get_ctrank();
    total = optimsoc_get_numct();

    // Determine number of rows and columns
    xcount = workshare[total-1][0];
    ycount = workshare[total-1][1];

    // Define tracing
    optimsoc_trace_definesection(0,"initialization");
    optimsoc_trace_definesection(1,"compute");
    optimsoc_trace_definesection(2,"update");
    optimsoc_trace_definesection(3,"barrier");

    optimsoc_trace_section(0);

    // If this rank is not part of the grid, just quit..
    if (rank >= xcount*ycount)
        return -1;

    // Determine this ranks column and row
    col = rank % xcount;
    for(row=0;(row+1)*xcount<=rank;row++) {}

    // Determine whether this rank is on one of the boundaries
    leftbound = (col==0);
    rightbound = (col==xcount-1);
    topbound = (row==0);
    bottombound = (row==ycount-1);

    // The default for xdim and ydim is simple but rounded
    xdim = (XSIZE/xcount);
    ydim = (YSIZE/ycount);

    // The base address is easily calculated based on this
    xbase = xdim * col;
    ybase = ydim * row;

    // And the last in a row and in a column need to get their
    // dimensions adjusted to match the rounding
    if (rightbound) xdim = XSIZE - xdim*(xcount-1);
    if (bottombound) ydim = YSIZE - ydim*(ycount-1);

    // We start with matrix 0  as initial value and matrix 1
    // for the first calculation
    curmatrix = 1;

    // Inititalize sufficient memory (remember extra rows and cols)
    matrix[0] = malloc(sizeof(float)*(xdim+2)*(ydim+2));
    matrix[1] = malloc(sizeof(float)*(xdim+2)*(ydim+2));

    // Initialize all with zeros
    for (int x=0;x<xdim+2;x++) {
        for (int y=0;y<ydim+2;y++) {
            matrix[0][POS(x,y)] = 0;
            matrix[1][POS(x,y)] = 0;
        }
    }

    // To see any heat distribution we need a hot spot, that
    // is here on the upper left corner.
    if (rank==0) {
        matrix[0][POS(0,0)] = 9.99;
        matrix[0][POS(1,0)] = 9.99;
        matrix[0][POS(0,1)] = 9.99;
        matrix[0][POS(0,2)] = 9.99;

        matrix[1][POS(0,0)] = 9.99;
        matrix[1][POS(1,0)] = 9.99;
        matrix[1][POS(0,1)] = 9.99;
        matrix[1][POS(0,2)] = 9.99;
    }

    // Allocate memory for the result matrix
    if (rank==0) {
        result = malloc(XSIZE*YSIZE*sizeof(float));
    }

    // Call the heat function that does all processing
    heat();

    // Print results after all iterations from core 0
    if (rank==0) {
        for (int y=0;y<YSIZE;y++) {
            for (int x=0;x<XSIZE;x++) {
                printf("%.2f ",result[x+y*XSIZE]);
            }
            printf("\n");
        }
    }
    return 0;
}

// The heat function does the actual processing and communication
// It updates the next matrix values and then sends its border results
// to the respective neighbors. Then it waits until all ranks finished
// the iteration and then repeats this for the defined number of iterations.
//
// Three types of messages are used:
//   Type 1: Barrier reached (from ALL\{0} to 0) and continue (0 to ALL\{0})
//           Message format:
//              +------+---+-----+-----------+-------+
//              | dest | 0 | src | undefined | 1     |
//              +------+---+-----+-----------+(1)-(0)+
//
//   Type 2: Boundary result element. The boundaries are enumerated. At the
//           moment all elements are transfered separately (inefficient).
//              +------+---+-----+-----------+----------+-------+
//              | dest | 0 | src | undefined | boundary | 2     |
//              +------+---+-----+-----------+(3)----(2)+(1)-(0)+
//              |   position on boundary (either x or y)        |
//              +-----------------------------------------------+
//              |   element value                               |
//              +-----------------------------------------------+
//
//   Type 3: Result element. For simplicity we send a struct every time.
//              +------+---+-----+-----------+-------+
//              | 0    | 0 | src | undefined | 3     |
//              +------+---+-----+-----------+(1)-(0)+
//              |   x position                       |
//              +------------------------------------+
//              |   y position                       |
//              +------------------------------------+
//              |   element value                    |
//              +------------------------------------+

#define MSG_TYPE_BARRIER  1
#define MSG_TYPE_BOUNDARY 2
#define MSG_TYPE_RESULT   3

// Enumerate the boundaries
#define BOUNDARY_TOP    0
#define BOUNDARY_RIGHT  1
#define BOUNDARY_BOTTOM 2
#define BOUNDARY_LEFT   3

// The heat calculation
void heat() {
    // Verify curmatrix is set correctly
    curmatrix = 1;
    // This points to the other matrix from the previous step
    int other = 0;

    for (int n=1;n<=ITERATIONS;n++) {
#if 0
        // Enable if you want an updated matrix output every iteration
        for (int x=0;x<xdim+2;x++) {
            printf("(%.2f) ",matrix[other][POS(x,0)]);
        }
        printf("\n");
        for (int y=1;y<ydim+1;y++) {
            printf("(%.2f) ",matrix[other][POS(0,y)]);
            for (int x=1;x<xdim+1;x++) {
                printf(" %.2f  ",matrix[other][POS(x,y)]);
            }
            printf("(%.2f) ",matrix[other][POS(xdim+1,y)]);
            printf("\n");
        }
        for (int x=0;x<xdim+2;x++) {
            printf("(%.2f) ",matrix[other][POS(x,ydim+1)]);
        }
        printf("\n");
#endif

        optimsoc_trace_section(1);
        printf("Start iteration %d\n",n);

        // Calculate all new elements based on the previous values
        for (int x=1;x<xdim+1;x++)
            for (int y=1;y<ydim+1;y++)
                matrix[curmatrix][POS(x,y)] = 0.25 * (matrix[other][POS(x-1,y)] +
                        matrix[other][POS(x+1,y)] +
                        matrix[other][POS(x,y-1)] +
                        matrix[other][POS(x,y+1)]);
        printf("Finished iteration %d\n", n);

        optimsoc_trace_section(2);

        // Now we send the results to the other ranks
        if (!topbound) {
            // If one is above us

            // Message buffer
            uint32_t buffer[3];

            // Assemble the header
            // Find tile id
            set_bits(&buffer[0],optimsoc_get_ranktile(rank-xcount),OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
            // Set class
            set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
            // Sender is this rank
            set_bits(&buffer[0],rank,OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
            // Type is boundary data
            set_bits(&buffer[0],MSG_TYPE_BOUNDARY,1,0);
            // Boundary is other ranks bottom
            set_bits(&buffer[0],BOUNDARY_BOTTOM,3,2);

            // Only the payload varies, send them successively
            for (int x=1;x<xdim+1;x++) {
                // Set position in boundary
                buffer[1] = x;
                // Copy element to payload in buffer
                memcpy(&buffer[2],&matrix[curmatrix][POS(x,1)],4);
                // Call library to send the element
                optimsoc_mp_simple_send(3,buffer);
            }
        }

        if (!bottombound) {
            // Same as above to the rank below this one
            uint32_t buffer[3];
            set_bits(&buffer[0],optimsoc_get_ranktile(rank+xcount),OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
            set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
            set_bits(&buffer[0],rank,OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
            set_bits(&buffer[0],MSG_TYPE_BOUNDARY,1,0);
            set_bits(&buffer[0],BOUNDARY_TOP,3,2);
            for (int x=1;x<xdim+1;x++) {
                buffer[1] = x;
                memcpy(&buffer[2],&matrix[curmatrix][POS(x,ydim)],4);
                optimsoc_mp_simple_send(3,buffer);
            }
        }

        if (!leftbound) {
            // Same as above to the rank left of this one
            uint32_t buffer[3];
            set_bits(&buffer[0],optimsoc_get_ranktile(rank-1),OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
            set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
            set_bits(&buffer[0],rank,OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
            set_bits(&buffer[0],MSG_TYPE_BOUNDARY,1,0);
            set_bits(&buffer[0],BOUNDARY_RIGHT,3,2);
            for (int y=1;y<ydim+1;y++) {
                buffer[1] = y;
                memcpy(&buffer[2],&matrix[curmatrix][POS(1,y)],4);
                optimsoc_mp_simple_send(3,buffer);
            }
        }

        if (!rightbound) {
            // Same as above to the rank right of this one
            uint32_t buffer[3];
            set_bits(&buffer[0],optimsoc_get_ranktile(rank+1),OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
            set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
            set_bits(&buffer[0],rank,OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
            set_bits(&buffer[0],MSG_TYPE_BOUNDARY,1,0);
            set_bits(&buffer[0],BOUNDARY_LEFT,3,2);
            for (int y=1;y<ydim+1;y++) {
                buffer[1] = y;
                memcpy(&buffer[2],&matrix[curmatrix][POS(xdim,y)],4);
                optimsoc_mp_simple_send(3,buffer);
            }
        }

        // Wait for all other ranks to reach this point
        optimsoc_trace_section(3);

        barrier();


        // Change matrices
        // (1-0=1, 1-1=0)
        curmatrix = 1 - curmatrix;
        other = 1 - other;
    }

    // When all iterations are done, assemble the result
    if (rank==0) {
        // rank 0 sets it directly
        for (int x=1;x<xdim+1;x++) {
            for (int y=1;y<ydim+1;y++) {
                result[x-1+(y-1)*XSIZE] = matrix[other][POS(x,y)];
            }
        }
    } else {
        // the other ranks send their results to rank 0

        // The buffer contains the message described above
        uint32_t buffer[4];

        // Assemble header
        // Destination is rank 0
        set_bits(&buffer[0],0,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
        // Class is 0
        set_bits(&buffer[0],0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
        // Sender is this rank
        set_bits(&buffer[0],rank,OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
        // This is a result element
        set_bits(&buffer[0],MSG_TYPE_RESULT,1,0);

        // Send each element now
        for (int x=1;x<xdim+1;x++) {
            for (int y=1;y<ydim+1;y++) {
                // Set x and y position of element on complete grid
                buffer[1] = xbase+x-1;
                buffer[2] = ybase+y-1;
                // Copy value
                memcpy(&buffer[3],&matrix[other][POS(x,y)],4);
                // Send this element
                optimsoc_mp_simple_send(4,buffer);
            }
        }
    }

    // Final barrier to ensure the result is complete
    barrier();
}

// Above we registered a function that is called for each incoming packet
// The handling determines the message type and performs the respective actions.
//
// Barriers
//  Two message subtypes exist within this type depending on the directions
//   * Ranks != 0 send a message as they reach the barrier
//   * Rank zero sends messages to all ranks when it observed all ranks
//  The actual barrier implementation can be found below (barrier()).
//  Two variables are used to signal the messages:
//  Count ranks that reached the barrier (in rank 0)
unsigned volatile barrier_count = 0;
//  Signal that the barrier is finished (other ranks)
int volatile barrier_continue = 0;

// Boundary data
//  The neighbors send their data for the boundary one after each other.
//  On receiving we store the element at the given position

// Result data
//  Stores the data at the given positions

void recv(unsigned int *buffer,int len) {
    assert(len>0); // Just to be sure
    assert(buffer); // Just to be sure

    // Extract type
    int type = extract_bits(buffer[0],1,0);
    if (type==MSG_TYPE_BARRIER) {
        assert(len==1); // Length is always 1

        if (rank==0) {
            // Rank 0 counts all
            barrier_count++;
        } else {
            // The other ranks know they can continue now
            barrier_continue = 1;
        }
    } else if (type==MSG_TYPE_BOUNDARY) {
        assert(len==3); // Length is always 3

        // Extract boundary
        int bound = extract_bits(buffer[0],3,2);

        // Extract the position on the boundary
        int pos = buffer[1];

        // Extract the value
        float v;
        memcpy(&v,&buffer[2],4);

        // Store element in boundary
        if (bound == BOUNDARY_TOP) {
            matrix[curmatrix][POS(pos,0)] = v;
        } else if (bound == BOUNDARY_RIGHT) {
            matrix[curmatrix][POS(xdim+1,pos)] = v;
        } else if (bound == BOUNDARY_BOTTOM) {
            matrix[curmatrix][POS(pos,ydim+1)] = v;
        } else if (bound == BOUNDARY_LEFT) {
            matrix[curmatrix][POS(0,pos)] = v;
        }
    } else if (type==MSG_TYPE_RESULT) {
        assert(len==4); // Length is always 4

        // Extract position and value
        int x = buffer[1];
        int y = buffer[2];
        float v;
        memcpy(&v,&buffer[3],4);

        // Store in the result buffer
        result[x+y*XSIZE] = v;
    }
}

// The barrier function blocks for each rank until all ranks entered it
// For rank zero it waits until the others send their message and the counter
// was increased in the recv(..) function. When the counter finally reached the
// number of all ranks it sends the continue messages to all ranks.
// The other ranks send a message and wait until they received the continue
// message which is signaled by recv.

void barrier() {
    if (rank==0) {
      printf("Rank 0 in barrier\n");
        // Rank zero waits for the others
        while (barrier_count != xcount*ycount-1) {}
        barrier_count = 0; // Reset the count

        for (int i=1;i<xcount*ycount;i++) {
            // Send message to all ranks
            uint32_t buffer = 0;
            // Assemble header
            set_bits(&buffer,i,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
            set_bits(&buffer,0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
            set_bits(&buffer,0,OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
            set_bits(&buffer,MSG_TYPE_BARRIER,1,0);
            optimsoc_mp_simple_send(1,&buffer);
        }
    } else {
        // Send message to rank 0
        uint32_t buffer = 0;
        // Assemble header
        set_bits(&buffer,0,OPTIMSOC_DEST_MSB,OPTIMSOC_DEST_LSB);
        set_bits(&buffer,0,OPTIMSOC_CLASS_MSB,OPTIMSOC_CLASS_LSB);
        set_bits(&buffer,rank,OPTIMSOC_SRC_MSB,OPTIMSOC_SRC_LSB);
        set_bits(&buffer,MSG_TYPE_BARRIER,1,0); // is a barrier
        optimsoc_mp_simple_send(1,&buffer);

        // Wait until we received the message of rank 0
        while (barrier_continue==0) {}
        // And reset signal
        barrier_continue = 0;
    }
}
