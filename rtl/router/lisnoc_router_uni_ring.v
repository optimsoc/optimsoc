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
 * This is the 2x2-port router for the unidirectional ring noc.
 *
 * Author(s):
 *   Andreas Lankes <andreas.lankes@tum.de>
 */

`include "lisnoc_def.vh"

module lisnoc_router_uni_ring( /*AUTOARG*/
   ring_in_flit, ring_in_valid, ring_in_ready,
   ring_out_flit, ring_out_valid, ring_out_ready,
   local_in_flit, local_in_valid, local_in_ready,
   local_out_flit, local_out_valid, local_out_ready,
   clk, rst
   );

   parameter flit_data_width = 32;
   parameter flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;
   localparam ports = 2;
   parameter ph_dest_width = 5;
   parameter num_dests = 32;

   parameter vchannels = 1;

   parameter in_fifo_length = 4;
   parameter out_fifo_length = 4;

   parameter [ports*num_dests-1:0] lookup = {num_dests{`SELECT_NONE }};

   input clk, rst;


   output [flit_width-1:0]  ring_out_flit;
   output [vchannels-1:0]   ring_out_valid;
   input [vchannels-1:0]    ring_out_ready;

   input [flit_width-1:0]   ring_in_flit;
   input [vchannels-1:0]    ring_in_valid;
   output [vchannels-1:0]   ring_in_ready;

   output [flit_width-1:0]  local_out_flit;
   output [vchannels-1:0]   local_out_valid;
   input [vchannels-1:0]    local_out_ready;

   input [flit_width-1:0]   local_in_flit;
   input [vchannels-1:0]    local_in_valid;
   output [vchannels-1:0]   local_in_ready;

   lisnoc_router
      #(.vchannels(vchannels),.input_ports(ports),.output_ports(ports),
        .lookup(lookup),.num_dests(num_dests), .flit_data_width(flit_data_width),
        .flit_type_width(flit_type_width), .ph_dest_width(ph_dest_width),
        .in_fifo_length(in_fifo_length), .out_fifo_length(out_fifo_length))
      u_router(.clk       (clk),
               .rst       (rst),

               // noc output interfaces
               .out_flit  ({local_out_flit, ring_out_flit}),
               .out_valid ({local_out_valid, ring_out_valid}),
               .out_ready ({local_out_ready, ring_out_ready}),

               // noc input interfaces
               .in_ready  ({local_in_ready, ring_in_ready}),
               .in_flit   ({local_in_flit, ring_in_flit}),
               .in_valid  ({local_in_valid, ring_in_valid}));


endmodule // lisnoc_router

`include "lisnoc_undef.vh"
