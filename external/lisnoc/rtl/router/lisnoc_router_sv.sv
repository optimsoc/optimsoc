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
 * This is the SystemVerilog wrapper for the router.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc.svh"

module lisnoc_router_sv
  (
   lisnoc_link_if north_out,
   lisnoc_link_if north_in,
   lisnoc_link_if east_out,
   lisnoc_link_if east_in,
   lisnoc_link_if south_out,
   lisnoc_link_if south_in,
   lisnoc_link_if west_out,
   lisnoc_link_if west_in,
   lisnoc_link_if local_out,
   lisnoc_link_if local_in,
   input clk, rst
   );

   parameter data_width = 32;
   localparam flit_width = data_width+2;
   parameter destwidth = 5;
   parameter num_dests = 32;

   parameter vchannels = 1;

   localparam ports = 5;
   parameter [ports*num_dests-1:0] lookup = {num_dests{`SELECT_NONE}};

   wire [flit_width-1:0] north_out_flit_o;
   wire [vchannels-1:0]   north_out_valid_o;
   wire [vchannels-1:0]   north_out_ready_i;
   wire [flit_width-1:0] east_out_flit_o;
   wire [vchannels-1:0]   east_out_valid_o;
   wire [vchannels-1:0]   east_out_ready_i;
   wire [flit_width-1:0] south_out_flit_o;
   wire [vchannels-1:0]   south_out_valid_o;
   wire [vchannels-1:0]   south_out_ready_i;
   wire [flit_width-1:0] west_out_flit_o;
   wire [vchannels-1:0]   west_out_valid_o;
   wire [vchannels-1:0]   west_out_ready_i;
   wire [flit_width-1:0] local_out_flit_o;
   wire [vchannels-1:0]   local_out_valid_o;
   wire [vchannels-1:0]   local_out_ready_i;

   wire [flit_width-1:0] north_in_flit_i;
   wire [vchannels-1:0]   north_in_valid_i;
   wire [vchannels-1:0]   north_in_ready_o;
   wire [flit_width-1:0] east_in_flit_i;
   wire [vchannels-1:0]   east_in_valid_i;
   wire [vchannels-1:0]   east_in_ready_o;
   wire [flit_width-1:0] south_in_flit_i;
   wire [vchannels-1:0]   south_in_valid_i;
   wire [vchannels-1:0]   south_in_ready_o;
   wire [flit_width-1:0] west_in_flit_i;
   wire [vchannels-1:0]   west_in_valid_i;
   wire [vchannels-1:0]   west_in_ready_o;
   wire [flit_width-1:0] local_in_flit_i;
   wire [vchannels-1:0]   local_in_valid_i;
   wire [vchannels-1:0]   local_in_ready_o;

   assign north_in_flit_i  = north_in.flit;
   assign north_in_valid_i = north_in.valid;
   assign north_in.ready   = north_in_ready_o;
   assign east_in_flit_i   = east_in.flit;
   assign east_in_valid_i  = east_in.valid;
   assign east_in.ready    = east_in_ready_o;
   assign south_in_flit_i  = south_in.flit;
   assign south_in_valid_i = south_in.valid;
   assign south_in.ready   = south_in_ready_o;
   assign west_in_flit_i   = west_in.flit;
   assign west_in_valid_i  = west_in.valid;
   assign west_in.ready    = west_in_ready_o;
   assign local_in_flit_i  = local_in.flit;
   assign local_in_valid_i = local_in.valid;
   assign local_in.ready   = local_in_ready_o;

   assign north_out.flit    = north_out_flit_o;
   assign north_out.valid   = north_out_valid_o;
   assign north_out_ready_i = north_out.ready;
   assign east_out.flit     = east_out_flit_o;
   assign east_out.valid    = east_out_valid_o;
   assign east_out_ready_i  = east_out.ready;
   assign south_out.flit    = south_out_flit_o;
   assign south_out.valid   = south_out_valid_o;
   assign south_out_ready_i = south_out.ready;
   assign west_out.flit     = west_out_flit_o;
   assign west_out.valid    = west_out_valid_o;
   assign west_out_ready_i  = west_out.ready;
   assign local_out.flit    = local_out_flit_o;
   assign local_out.valid   = local_out_valid_o;
   assign local_out_ready_i = local_out.ready;

   lisnoc_router_2dgrid
     #(.vchannels(vchannels),.num_dests(num_dests),.lookup(lookup))
     router_v(/*AUTOINST*/
              // Outputs
              .north_out_flit_o         (north_out_flit_o[flit_width-1:0]),
              .north_out_valid_o        (north_out_valid_o[vchannels-1:0]),
              .east_out_flit_o          (east_out_flit_o[flit_width-1:0]),
              .east_out_valid_o         (east_out_valid_o[vchannels-1:0]),
              .south_out_flit_o         (south_out_flit_o[flit_width-1:0]),
              .south_out_valid_o        (south_out_valid_o[vchannels-1:0]),
              .west_out_flit_o          (west_out_flit_o[flit_width-1:0]),
              .west_out_valid_o         (west_out_valid_o[vchannels-1:0]),
              .local_out_flit_o         (local_out_flit_o[flit_width-1:0]),
              .local_out_valid_o        (local_out_valid_o[vchannels-1:0]),
              .north_in_ready_o         (north_in_ready_o[vchannels-1:0]),
              .east_in_ready_o          (east_in_ready_o[vchannels-1:0]),
              .south_in_ready_o         (south_in_ready_o[vchannels-1:0]),
              .west_in_ready_o          (west_in_ready_o[vchannels-1:0]),
              .local_in_ready_o         (local_in_ready_o[vchannels-1:0]),
              // Inputs
              .clk                      (clk),
              .rst                      (rst),
              .north_out_ready_i        (north_out_ready_i[vchannels-1:0]),
              .east_out_ready_i         (east_out_ready_i[vchannels-1:0]),
              .south_out_ready_i        (south_out_ready_i[vchannels-1:0]),
              .west_out_ready_i         (west_out_ready_i[vchannels-1:0]),
              .local_out_ready_i        (local_out_ready_i[vchannels-1:0]),
              .north_in_flit_i          (north_in_flit_i[flit_width-1:0]),
              .north_in_valid_i         (north_in_valid_i[vchannels-1:0]),
              .east_in_flit_i           (east_in_flit_i[flit_width-1:0]),
              .east_in_valid_i          (east_in_valid_i[vchannels-1:0]),
              .south_in_flit_i          (south_in_flit_i[flit_width-1:0]),
              .south_in_valid_i         (south_in_valid_i[vchannels-1:0]),
              .west_in_flit_i           (west_in_flit_i[flit_width-1:0]),
              .west_in_valid_i          (west_in_valid_i[vchannels-1:0]),
              .local_in_flit_i          (local_in_flit_i[flit_width-1:0]),
              .local_in_valid_i         (local_in_valid_i[vchannels-1:0]));


endmodule // lisnoc_router_sv

`include "lisnoc_undef.vh"
