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
 * This is a simple unidirectional ring noc.
 * TODO use 2 virtual channels and adapt routing to break cycle
 *
 * Author(s):
 *   Andreas Lankes <andreas.lankes@tum.de>
 */


`include "lisnoc_def.vh"

module lisnoc_uni_ring
  (
   // Outputs
   local_out_flit, local_out_valid, local_in_ready,
   // Inputs
   clk, rst, local_out_ready, local_in_flit, local_in_valid
    );

   parameter routers = 5;
   parameter flit_data_width = 16;
   parameter flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;
   parameter ph_dest_width = 5;
   parameter router_in_fifo_length = 4;
   parameter router_out_fifo_length = 4;

   parameter vchannels = 1;

   input clk;
   input rst;

   output [routers*flit_width-1:0]  local_out_flit;
   output [routers*vchannels-1:0]   local_out_valid;
   input [routers*vchannels-1:0]    local_out_ready;

   input [routers*flit_width-1:0]   local_in_flit;
   input [routers*vchannels-1:0]    local_in_valid;
   output [routers*vchannels-1:0]   local_in_ready;

   // Array conversion
   wire [flit_width-1:0]   local_out_flit_array [0:routers-1];
   wire [vchannels-1:0]    local_out_valid_array [0:routers-1];
   wire [vchannels-1:0]    local_out_ready_array [0:routers-1];

   wire [flit_width-1:0]   local_in_flit_array [0:routers-1];
   wire [vchannels-1:0]    local_in_valid_array [0:routers-1];
   wire [vchannels-1:0]    local_in_ready_array [0:routers-1];



   // wire for links of the ring
   wire [flit_width-1:0]   ring_flit[0:routers-1];
   wire [vchannels-1:0]    ring_valid[0:routers-1];
   wire [vchannels-1:0]    ring_ready[0:routers-1];



   genvar p;

   generate
      for (p=0;p<routers;p=p+1) begin : local_outputs
         assign local_out_flit[(p+1)*flit_width-1:p*flit_width] = local_out_flit_array[p];
         assign local_out_valid[(p+1)*vchannels-1:p*vchannels]    = local_out_valid_array[p];
         assign local_out_ready_array[p] = local_out_ready[(p+1)*vchannels-1:p*vchannels];
      end
   endgenerate

   generate
      for (p=0;p<routers;p=p+1) begin : local_inputs
         assign local_in_flit_array[p]  = local_in_flit[(p+1)*flit_width-1:p*flit_width];
         assign local_in_valid_array[p] = local_in_valid[(p+1)*vchannels-1:p*vchannels];
         assign local_in_ready[(p+1)*vchannels-1:p*vchannels] = local_in_ready_array[p];
      end
   endgenerate



     lisnoc_router_uni_ring
            #(.num_dests(routers),.vchannels(vchannels),
               .lookup({2'b10, {routers-1{2'b01}}}),
               .flit_data_width(flit_data_width), .flit_type_width(flit_type_width),
               .ph_dest_width(ph_dest_width),
               .in_fifo_length(router_in_fifo_length),
               .out_fifo_length(router_out_fifo_length))
            ring_router_first (
               .clk(clk),
               .rst(rst),
               .ring_in_flit(ring_flit[routers-1]),
               .ring_in_valid(ring_valid[routers-1]),
               .ring_in_ready(ring_ready[routers-1]),
               .ring_out_flit(ring_flit[0]),
               .ring_out_valid(ring_valid[0]),
               .ring_out_ready(ring_ready[0]),
               .local_in_flit(local_in_flit_array[0]),
               .local_in_valid(local_in_valid_array[0]),
               .local_in_ready(local_in_ready_array[0]),
               .local_out_flit(local_out_flit_array[0]),
               .local_out_valid(local_out_valid_array[0]),
               .local_out_ready(local_out_ready_array[0])
            );

   generate
      for(p=1; p<routers-1; p=p+1) begin : ring_routers
         lisnoc_router_uni_ring
            #(.num_dests(routers),.vchannels(vchannels),
               .lookup({{p{2'b01}}, 2'b10, {routers-p-1{2'b01}}}), //does NOT work for p=0 or p=routers-1 !!!
               .flit_data_width(flit_data_width), .flit_type_width(flit_type_width),
               .ph_dest_width(ph_dest_width),
               .in_fifo_length(router_in_fifo_length),
               .out_fifo_length(router_out_fifo_length))
            ring_router (
               .clk(clk),
               .rst(rst),
               .ring_in_flit(ring_flit[p-1]),
               .ring_in_valid(ring_valid[p-1]),
               .ring_in_ready(ring_ready[p-1]),
               .ring_out_flit(ring_flit[p]),
               .ring_out_valid(ring_valid[p]),
               .ring_out_ready(ring_ready[p]),
               .local_in_flit(local_in_flit_array[p]),
               .local_in_valid(local_in_valid_array[p]),
               .local_in_ready(local_in_ready_array[p]),
               .local_out_flit(local_out_flit_array[p]),
               .local_out_valid(local_out_valid_array[p]),
               .local_out_ready(local_out_ready_array[p])
            );
      end
   endgenerate


        lisnoc_router_uni_ring
            #(.num_dests(routers),.vchannels(vchannels),
               .lookup({{routers-1{2'b01}}, 2'b10}),
               .flit_data_width(flit_data_width), .flit_type_width(flit_type_width),
               .ph_dest_width(ph_dest_width),
               .in_fifo_length(router_in_fifo_length),
               .out_fifo_length(router_out_fifo_length))
            ring_router_last (
               .clk(clk),
               .rst(rst),
               .ring_in_flit(ring_flit[routers-2]),
               .ring_in_valid(ring_valid[routers-2]),
               .ring_in_ready(ring_ready[routers-2]),
               .ring_out_flit(ring_flit[routers-1]),
               .ring_out_valid(ring_valid[routers-1]),
               .ring_out_ready(ring_ready[routers-1]),
               .local_in_flit(local_in_flit_array[routers-1]),
               .local_in_valid(local_in_valid_array[routers-1]),
               .local_in_ready(local_in_ready_array[routers-1]),
               .local_out_flit(local_out_flit_array[routers-1]),
               .local_out_valid(local_out_valid_array[routers-1]),
               .local_out_ready(local_out_ready_array[routers-1])
            );



endmodule

`include "lisnoc_undef.vh"
