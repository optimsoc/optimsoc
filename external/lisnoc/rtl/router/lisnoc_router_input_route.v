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
 * =============================================================================
 *
 * This file implements the routing stage on the input ports.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"

module lisnoc_router_input_route (/*AUTOARG*/
   // Outputs
   fifo_ready, switch_request, switch_flit,
   // Inputs
   clk, rst, fifo_flit, fifo_valid, switch_read
   );

   parameter flit_data_width = 32;
   parameter flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;
   parameter ph_dest_width = 5;

   // The number of destinations is a parameter of each port.
   // It should in general be equal for all routers in a NoC and
   // must be within the range defined by FLIT_DEST_WIDTH.
   parameter num_dests = 1;

   // The number of directions of each router is the number of output ports.
   parameter directions = 1;

   // The externally defined destination->direction lookup
   // It is the concatenation of directions-width elements starting
   // with destiantion 0:
   //  { destination0_direction, destination1_direction, ... }
   parameter [directions*num_dests-1:0] lookup = {num_dests*directions{1'b0}};

   // General ports
   input clk;
   input rst;

   // The fifo interface
   input [flit_width-1:0] fifo_flit;  // current FIFO output
   input                   fifo_valid; // current output valid
   output                  fifo_ready; // current output has been registered

   // The switch interface
   output reg [directions-1:0]  switch_request;     // direction requests
   wire [directions-1:0]        nxt_switch_request; // combinatorial signal for this variable
   output reg [flit_width-1:0] switch_flit;        // corresponding flit
   input [directions-1:0]       switch_read;        // destination acknowledge

   // The aggregated read signal, because there is only one read reply
   wire   read;
   assign read = |switch_read;

   // Internal state
   reg                      active;     // there is a transfer in progress
   wire                     nxt_active; // combinatorial input to transfer state

   reg [directions-1:0]     cur_select;     // This stores the current selection
   wire [directions-1:0]    nxt_cur_select; // combinational signal

   // The lookup vector is generated for better code readability and
   // is in fact the lookup parameter in another representation
   wire [directions-1:0]    lookup_vector [0:num_dests-1];

   // generate this representation
   genvar                   i;
   generate
      for(i=0;i<num_dests;i=i+1) begin // array is indexed by the desired destination
         // The entries of this array are subranges from the parameter, where
         // the indexing is reversed (num_dests-i-1)!
         assign lookup_vector[num_dests-i-1] = lookup[(i+1)*directions-1:i*directions];
      end
   endgenerate

   // Some bit selections for better readability below
   wire [flit_data_width-1:0] flit_header;
   assign flit_header = fifo_flit[flit_data_width-1:0];

   wire [ph_dest_width-1:0]     flit_dest;
   assign flit_dest = flit_header[flit_data_width-1:flit_data_width-ph_dest_width];

   wire 			is_last;
   assign is_last = fifo_flit[flit_data_width+1];

   reg                         is_first;
   wire                        nxt_is_first;

   // Generating the current destination selection
   assign nxt_cur_select = ( // If there is no transfer we are waiting for or the active is finished ..
                             (~active || (active && read && is_first)) &&
                             // .. and this is valid..
                             (fifo_valid)
                             // .. take selection from the lookup vector
                             ) ? lookup_vector[flit_dest] :
                           // take current value otherwise

                           cur_select;

   // Generate the request for the output
   assign nxt_switch_request =  // When the next cycle is an active route ..
               nxt_active ?
               // .. issue the current route request
               nxt_cur_select
               // .. and nothing otherwise.
               : {directions{1'b0}};

   // The route is active when ..
   assign nxt_active = // .. the FIFO has a request or ..
               fifo_valid |
               // .. the current request is not finished.
               (active & ~read);

   assign nxt_is_first = active && read && is_last;

   // Pop from FIFO when active and successfully routed or not active and something waiting in FIFO.
   assign fifo_ready = (active ? read : 1 ) & fifo_valid;

   // TODO: The line above is a combinational loop from FIFO output to FIFO input.
   // The line below removes this, by simply signaling whether we in principle accept
   // new flits.
   // assign fifo_ready = ~active | read;

   // Register stuff with the clock edge
   always @(posedge clk) begin
      if (rst) begin
         active         <= 1'b0;
         switch_request <= {directions{1'b0}};
         cur_select     <= {directions{1'b0}};
         is_first       <= 0;
      end else begin
         cur_select     <= nxt_cur_select;
         switch_request <= nxt_switch_request;
         active         <= nxt_active;
         is_first       <= nxt_is_first;

         // Additionally check whether
         if ((active && read) || !active)
            switch_flit    <= fifo_flit;
         end
   end

endmodule // noc_router_input_decode

`include "lisnoc_undef.vh"
