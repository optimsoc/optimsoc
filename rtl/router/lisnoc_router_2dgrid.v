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
 * This is the 5-port router toplevel.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"

module lisnoc_router_2dgrid( /*AUTOARG*/
   // Outputs
   north_out_flit_o, north_out_valid_o, east_out_flit_o,
   east_out_valid_o, south_out_flit_o, south_out_valid_o,
   west_out_flit_o, west_out_valid_o, local_out_flit_o,
   local_out_valid_o, north_in_ready_o, east_in_ready_o,
   south_in_ready_o, west_in_ready_o, local_in_ready_o,
   // Inputs
   clk, rst, north_out_ready_i, east_out_ready_i, south_out_ready_i,
   west_out_ready_i, local_out_ready_i, north_in_flit_i,
   north_in_valid_i, east_in_flit_i, east_in_valid_i, south_in_flit_i,
   south_in_valid_i, west_in_flit_i, west_in_valid_i, local_in_flit_i,
   local_in_valid_i
   );

   parameter  flit_data_width = 32;
   parameter  flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;
   parameter  ph_dest_width = 5;
   parameter  num_dests = 16;

   parameter use_prio = 0;
   parameter ph_prio_width = 4;

   parameter vchannels = 2;

   localparam ports = 5;

   parameter in_fifo_length = 4;
   parameter out_fifo_length = 4;

   parameter [ports*num_dests-1:0] lookup = {num_dests{`SELECT_NONE}};

   input clk, rst;

   output [flit_width-1:0] north_out_flit_o;
   output [vchannels-1:0]   north_out_valid_o;
   input [vchannels-1:0]    north_out_ready_i;
   output [flit_width-1:0] east_out_flit_o;
   output [vchannels-1:0]   east_out_valid_o;
   input [vchannels-1:0]    east_out_ready_i;
   output [flit_width-1:0] south_out_flit_o;
   output [vchannels-1:0]   south_out_valid_o;
   input [vchannels-1:0]    south_out_ready_i;
   output [flit_width-1:0] west_out_flit_o;
   output [vchannels-1:0]   west_out_valid_o;
   input [vchannels-1:0]    west_out_ready_i;
   output [flit_width-1:0] local_out_flit_o;
   output [vchannels-1:0]   local_out_valid_o;
   input [vchannels-1:0]    local_out_ready_i;

   input [flit_width-1:0]  north_in_flit_i;
   input [vchannels-1:0]    north_in_valid_i;
   output [vchannels-1:0]   north_in_ready_o;
   input [flit_width-1:0]  east_in_flit_i;
   input [vchannels-1:0]    east_in_valid_i;
   output [vchannels-1:0]   east_in_ready_o;
   input [flit_width-1:0]  south_in_flit_i;
   input [vchannels-1:0]    south_in_valid_i;
   output [vchannels-1:0]   south_in_ready_o;
   input [flit_width-1:0]  west_in_flit_i;
   input [vchannels-1:0]    west_in_valid_i;
   output [vchannels-1:0]   west_in_ready_o;
   input [flit_width-1:0]  local_in_flit_i;
   input [vchannels-1:0]    local_in_valid_i;
   output [vchannels-1:0]   local_in_ready_o;

   /* lisnoc_router AUTO_TEMPLATE(
    .out_flit ({local_out_flit_o,west_out_flit_o,south_out_flit_o,east_out_flit_o,north_out_flit_o}),
    .out_valid ({local_out_valid_o,west_out_valid_o,south_out_valid_o,east_out_valid_o,north_out_valid_o}),
    .out_ready ({local_out_ready_i,west_out_ready_i,south_out_ready_i,east_out_ready_i,north_out_ready_i}),
    .in_flit ({local_in_flit_i,west_in_flit_i,south_in_flit_i,east_in_flit_i,north_in_flit_i}),
    .in_valid ({local_in_valid_i,west_in_valid_i,south_in_valid_i,east_in_valid_i,north_in_valid_i}),
    .in_ready ({local_in_ready_o,west_in_ready_o,south_in_ready_o,east_in_ready_o,north_in_ready_o}),
    ); */
   lisnoc_router #(.vchannels(vchannels),.input_ports(5),.output_ports(5),.lookup(lookup),.num_dests(num_dests),
                   .in_fifo_length(in_fifo_length),.out_fifo_length(out_fifo_length),
                   .flit_data_width(flit_data_width), .flit_type_width(flit_type_width), .ph_dest_width(ph_dest_width),.use_prio(use_prio),
                   .ph_prio_width(ph_prio_width))
   u_router(/*AUTOINST*/
            // Outputs
            .out_flit                   ({local_out_flit_o,west_out_flit_o,south_out_flit_o,east_out_flit_o,north_out_flit_o}), // Templated
            .out_valid                  ({local_out_valid_o,west_out_valid_o,south_out_valid_o,east_out_valid_o,north_out_valid_o}), // Templated
            .in_ready                   ({local_in_ready_o,west_in_ready_o,south_in_ready_o,east_in_ready_o,north_in_ready_o}), // Templated
            // Inputs
            .clk                        (clk),
            .rst                        (rst),
            .out_ready                  ({local_out_ready_i,west_out_ready_i,south_out_ready_i,east_out_ready_i,north_out_ready_i}), // Templated
            .in_flit                    ({local_in_flit_i,west_in_flit_i,south_in_flit_i,east_in_flit_i,north_in_flit_i}), // Templated
            .in_valid                   ({local_in_valid_i,west_in_valid_i,south_in_valid_i,east_in_valid_i,north_in_valid_i})); // Templated


endmodule // lisnoc_2dgrid_router

`include "lisnoc_undef.vh"
