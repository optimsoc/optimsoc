/* Copyright (c) 2013 by the author(s)
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
 * Submodule of the Network Router Monitor (NRM): link statistics collector
 *
 * This module collects statistics from a single router link.
 *
 * TODO: Add virtual channel support. Currently, a flit is counted if it happens
 *       on any of the virtual channels.
 * TODO: Handle counter overflows.
 *
 * (c) 2011-2012 by the author(s)
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 *   Andreas Lankes <andreas.lankes@tum.de>
 */

`include "dbg_config.vh"

module nrm_link_statistics_collector (/*AUTOARG*/
   // Outputs
   sent_flits,
   // Inputs
   clk, rst, in_flit, in_ready, in_valid, sample
   );

   // NoC interface (32 bit flit payload, aka lisnoc)
   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   parameter NOC_VCHANNELS = 3;

   input clk;
   input rst;

   // a single link from a NoC router to collect statistics from
   input [NOC_FLIT_WIDTH-1:0] in_flit;
   input [NOC_VCHANNELS-1:0] in_ready;
   input [NOC_VCHANNELS-1:0] in_valid;

   // number of sent flits since the last sample signal
   output reg [7:0] sent_flits;
   // sample sent_flits and restart count with 0
   input sample;

   // TODO: Add proper virtual channel support!
   wire flit_sent;
   assign flit_sent = |(in_ready & in_valid);

   // flit sent counter
   reg [7:0] sent_flits_counter;

   always @ (posedge clk) begin
      if (rst) begin
         sent_flits_counter <= 0;
         sent_flits <= 0;
      end else begin
         if (flit_sent & sample) begin
            sent_flits <= sent_flits_counter + 1;
            sent_flits_counter <= 0;
         end else if (sample) begin
            sent_flits <= sent_flits_counter;
            sent_flits_counter <= 0;
         end else if (flit_sent) begin
            sent_flits_counter <= sent_flits_counter + 1;
         end else begin
            sent_flits_counter <= sent_flits_counter;
         end
      end
   end
endmodule
