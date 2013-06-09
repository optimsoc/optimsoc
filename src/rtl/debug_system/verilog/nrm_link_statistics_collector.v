/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using OpTiMSoC in your projects at
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
 *    Philipp Wagner, mail@philipp-wagner.com
 *    Andreas Lankes, andreas.lankes@tum.de
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
