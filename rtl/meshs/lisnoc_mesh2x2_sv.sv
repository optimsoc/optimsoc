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

`include "lisnoc_def_mc.vh"

module lisnoc_mesh2x2_sv
  (
   lisnoc_link_if link0_out,
   lisnoc_link_if link0_in,
   lisnoc_link_if link1_out,
   lisnoc_link_if link1_in,
   lisnoc_link_if link2_out,
   lisnoc_link_if link2_in,
   lisnoc_link_if link3_out,
   lisnoc_link_if link3_in,
   input clk, rst
   );

   parameter vchannels = 1;

   wire [`FLIT_WIDTH-1:0] link0_in_flit_i;
   wire [vchannels-1:0]   link0_in_valid_i;
   wire [vchannels-1:0]   link0_in_ready_o;
   wire [`FLIT_WIDTH-1:0] link0_out_flit_o;
   wire [vchannels-1:0]   link0_out_valid_o;
   wire [vchannels-1:0]   link0_out_ready_i;

   wire [`FLIT_WIDTH-1:0] link1_in_flit_i;
   wire [vchannels-1:0]   link1_in_valid_i;
   wire [vchannels-1:0]   link1_in_ready_o;
   wire [`FLIT_WIDTH-1:0] link1_out_flit_o;
   wire [vchannels-1:0]   link1_out_valid_o;
   wire [vchannels-1:0]   link1_out_ready_i;

   wire [`FLIT_WIDTH-1:0] link2_in_flit_i;
   wire [vchannels-1:0]   link2_in_valid_i;
   wire [vchannels-1:0]   link2_in_ready_o;
   wire [`FLIT_WIDTH-1:0] link2_out_flit_o;
   wire [vchannels-1:0]   link2_out_valid_o;
   wire [vchannels-1:0]   link2_out_ready_i;

   wire [`FLIT_WIDTH-1:0] link3_in_flit_i;
   wire [vchannels-1:0]   link3_in_valid_i;
   wire [vchannels-1:0]   link3_in_ready_o;
   wire [`FLIT_WIDTH-1:0] link3_out_flit_o;
   wire [vchannels-1:0]   link3_out_valid_o;
   wire [vchannels-1:0]   link3_out_ready_i;

   assign link0_out.flit    = link0_out_flit_o;
   assign link0_out.valid   = link0_out_valid_o;
   assign link0_out_ready_i = link0_out.ready;
   assign link1_out.flit    = link1_out_flit_o;
   assign link1_out.valid   = link1_out_valid_o;
   assign link1_out_ready_i = link1_out.ready;
   assign link2_out.flit    = link2_out_flit_o;
   assign link2_out.valid   = link2_out_valid_o;
   assign link2_out_ready_i = link2_out.ready;
   assign link3_out.flit    = link3_out_flit_o;
   assign link3_out.valid   = link3_out_valid_o;
   assign link3_out_ready_i = link3_out.ready;

   assign link0_in_flit_i  = link0_in.flit;
   assign link0_in_valid_i = link0_in.valid;
   assign link0_in.ready   = link0_in_ready_o;
   assign link1_in_flit_i  = link1_in.flit;
   assign link1_in_valid_i = link1_in.valid;
   assign link1_in.ready   = link1_in_ready_o;
   assign link2_in_flit_i  = link2_in.flit;
   assign link2_in_valid_i = link2_in.valid;
   assign link2_in.ready   = link2_in_ready_o;
   assign link3_in_flit_i  = link3_in.flit;
   assign link3_in_valid_i = link3_in.valid;
   assign link3_in.ready   = link3_in_ready_o;

   lisnoc_mesh2x2 #(.vchannels(vchannels))
   mesh2x2_v(/*AUTOINST*/
	     // Outputs
	     .link0_in_ready_o		(link0_in_ready_o[vchannels-1:0]),
	     .link0_out_flit_o		(link0_out_flit_o[`FLIT_WIDTH-1:0]),
	     .link0_out_valid_o		(link0_out_valid_o[vchannels-1:0]),
	     .link1_in_ready_o		(link1_in_ready_o[vchannels-1:0]),
	     .link1_out_flit_o		(link1_out_flit_o[`FLIT_WIDTH-1:0]),
	     .link1_out_valid_o		(link1_out_valid_o[vchannels-1:0]),
	     .link2_in_ready_o		(link2_in_ready_o[vchannels-1:0]),
	     .link2_out_flit_o		(link2_out_flit_o[`FLIT_WIDTH-1:0]),
	     .link2_out_valid_o		(link2_out_valid_o[vchannels-1:0]),
	     .link3_in_ready_o		(link3_in_ready_o[vchannels-1:0]),
	     .link3_out_flit_o		(link3_out_flit_o[`FLIT_WIDTH-1:0]),
	     .link3_out_valid_o		(link3_out_valid_o[vchannels-1:0]),
	     // Inputs
	     .clk			(clk),
	     .rst			(rst),
	     .link0_in_flit_i		(link0_in_flit_i[`FLIT_WIDTH-1:0]),
	     .link0_in_valid_i		(link0_in_valid_i[vchannels-1:0]),
	     .link0_out_ready_i		(link0_out_ready_i[vchannels-1:0]),
	     .link1_in_flit_i		(link1_in_flit_i[`FLIT_WIDTH-1:0]),
	     .link1_in_valid_i		(link1_in_valid_i[vchannels-1:0]),
	     .link1_out_ready_i		(link1_out_ready_i[vchannels-1:0]),
	     .link2_in_flit_i		(link2_in_flit_i[`FLIT_WIDTH-1:0]),
	     .link2_in_valid_i		(link2_in_valid_i[vchannels-1:0]),
	     .link2_out_ready_i		(link2_out_ready_i[vchannels-1:0]),
	     .link3_in_flit_i		(link3_in_flit_i[`FLIT_WIDTH-1:0]),
	     .link3_in_valid_i		(link3_in_valid_i[vchannels-1:0]),
	     .link3_out_ready_i		(link3_out_ready_i[vchannels-1:0]));




endmodule // lisnoc_mesh2x2_sv
