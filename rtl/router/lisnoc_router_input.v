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
 * This module is an input port. It instantiates a FIFO and a decode module
 * for each virtual channel and aggregates the interfaces.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 * TODO:
 *  - when FIFO is empty, the flit can go to decode directly
 */

`include "lisnoc_def.vh"

module lisnoc_router_input( /*AUTOARG*/
   // Outputs
   link_ready, switch_request, switch_flit,
   // Inputs
   clk, rst, link_flit, link_valid, switch_read
   );

   parameter flit_data_width = 32;
   parameter flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;
   parameter ph_dest_width = 5;

   parameter vchannels = 1;

   parameter ports = 5;

   parameter fifo_length = 4;

   // The number of destinations is a parameter of each port.
   // It should in general be equal for all routers in a NoC and
   // must be within the range defined by FLIT_DEST_WIDTH.
   parameter num_dests = 1;

   // The externally defined destination->direction lookup
   // It is the concatenation of directions-width elements starting
   // with destination 0:
   //  { destination0_direction, destination1_direction, ... }
   parameter [ports*num_dests-1:0] lookup = {num_dests{`SELECT_NONE }};

   // Generic stuff
   input clk, rst;

   // The link interface
   input [flit_width-1:0] link_flit;  // current flit
   input [vchannels-1:0]   link_valid; // current valid for which channel
   output [vchannels-1:0]  link_ready; // notification when registered

   // The switch interface
   output [ports*vchannels-1:0]       switch_request;     // direction requests
   output [flit_width*vchannels-1:0] switch_flit;        // corresponding flit
   input [ports*vchannels-1:0]        switch_read;        // destination acknowledge

   wire [ports-1:0]                   switch_request_array [0:vchannels-1];
   wire [flit_width-1:0]              switch_flit_array [0:vchannels-1];
   wire [ports-1:0]                   switch_read_array [0:vchannels-1];

   genvar v;

   generate
      for (v=0;v<vchannels;v=v+1) begin: vchannel
         wire                   fifo_valid;
         wire [flit_width-1:0] fifo_flit;
         wire                   fifo_ready;

         assign switch_request[(v+1)*ports-1:v*ports] = switch_request_array[v];
         assign switch_flit[(v+1)*flit_width-1:v*flit_width] = switch_flit_array[v];
         assign switch_read_array[v] = switch_read[(v+1)*ports-1:v*ports];

         /* lisnoc_fifo AUTO_TEMPLATE (
          .in_ready  (link_ready[v]),
          .out_flit  (fifo_flit[flit_width-1:0]),
          .out_valid (fifo_valid),
          .in_flit   (link_flit),
          .in_valid  (link_valid[v]),
          .out_ready (fifo_ready),
          ); */
         lisnoc_fifo #(.LENGTH(fifo_length),.flit_data_width(flit_data_width),
         .flit_type_width(flit_type_width))
           fifo (/*AUTOINST*/
                 // Outputs
                 .in_ready              (link_ready[v]),         // Templated
                 .out_flit              (fifo_flit[flit_width-1:0]), // Templated
                 .out_valid             (fifo_valid),            // Templated
                 // Inputs
                 .clk                   (clk),
                 .rst                   (rst),
                 .in_flit               (link_flit),             // Templated
                 .in_valid              (link_valid[v]),         // Templated
                 .out_ready             (fifo_ready));           // Templated

         lisnoc_router_input_route
           # (.num_dests(num_dests),.lookup(lookup),
              .flit_data_width(flit_data_width),.flit_type_width(flit_type_width),
         .ph_dest_width(ph_dest_width), .directions(ports))
         route(// Outputs
                .fifo_ready     (fifo_ready),
                .switch_request (switch_request_array[v]),
                .switch_flit    (switch_flit_array[v]),
                // Inputs
                .clk            (clk),
                .rst            (rst),
                .switch_read    (switch_read_array[v]),
                .fifo_flit      (fifo_flit),
                .fifo_valid     (fifo_valid));

      end // for (i=0;i<vchannels;i=i+1)

   endgenerate

endmodule // noc_router_input

`include "lisnoc_undef.vh"
