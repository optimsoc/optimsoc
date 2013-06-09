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
 * Submodule of the Network Router Monitor (NRM): statistics collector
 *
 * This module collects statistics from a router by aggregating data from
 * all router links collected by nrm_link_statistics_collector.
 *
 * (c) 2011-2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 *    Andreas Lankes, andreas.lankes@tum.de
 */

`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module nrm_statistics_collector (/*AUTOARG*/
   // Outputs
   trace_out, trace_out_valid,
   // Inputs
   clk, rst, noc32_router_link_in_flit, noc32_router_link_in_ready,
   noc32_router_link_in_valid, timestamp, sample_interval
   );

   // number of monitored links for this router
   parameter MONITORED_LINK_COUNT = 'hx;

   // width of the sample_interval input
   // consequently, the maximum sample interval is 2^SAMPLE_INTERVAL_WIDTH
   // Consider the maximum counter value of 255 (8 bit) when setting this value!
   parameter SAMPLE_INTERVAL_WIDTH = 'hx;

   // NoC interface (32 bit flit payload, aka lisnoc)
   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   parameter NOC_VCHANNELS = 3;

   input clk;
   input rst;

   // router links
   input [MONITORED_LINK_COUNT*NOC_FLIT_WIDTH-1:0] noc32_router_link_in_flit;
   input [MONITORED_LINK_COUNT*NOC_VCHANNELS-1:0] noc32_router_link_in_ready;
   input [MONITORED_LINK_COUNT*NOC_VCHANNELS-1:0] noc32_router_link_in_valid;

   // from the Global Timestamp Provider (GTP)
   input [`DBG_TIMESTAMP_WIDTH-1:0] timestamp;

   // to the ring buffer
   output [`DBG_TIMESTAMP_WIDTH+8*MONITORED_LINK_COUNT-1:0] trace_out;
   output trace_out_valid;

   // sample a new data point every sample_interval clock cycles
   input [SAMPLE_INTERVAL_WIDTH-1:0] sample_interval;

   reg [SAMPLE_INTERVAL_WIDTH-1:0] timer;

   reg sample;

   assign trace_out[`DBG_TIMESTAMP_WIDTH+8*MONITORED_LINK_COUNT-1:8*MONITORED_LINK_COUNT] = timestamp;
   assign trace_out_valid = sample;

   // Generate nrm_link_statistics_collector instances for each monitored link
   // of the router.
   generate
      genvar i;
      for (i = 0; i < MONITORED_LINK_COUNT; i = i + 1) begin: gen_link_stat_coll
         wire [NOC_FLIT_WIDTH-1:0] in_flit;
         wire [NOC_VCHANNELS-1:0] in_valid;
         wire [NOC_VCHANNELS-1:0] in_ready;

         assign in_flit = noc32_router_link_in_flit[(i+1)*NOC_FLIT_WIDTH-1:i*NOC_FLIT_WIDTH];
         assign in_valid = noc32_router_link_in_valid[(i+1)*NOC_VCHANNELS-1:i*NOC_VCHANNELS];
         assign in_ready = noc32_router_link_in_ready[(i+1)*NOC_VCHANNELS-1:i*NOC_VCHANNELS];

         wire [7:0] sent_flits;
         assign trace_out[(i+1)*8-1:i*8] = sent_flits;

         nrm_link_statistics_collector
            u_link_statistics_collector(.clk(clk),
                                        .rst(rst),

                                        .in_flit(in_flit),
                                        .in_ready(in_ready),
                                        .in_valid(in_valid),

                                        .sample(sample),
                                        .sent_flits(sent_flits));
      end
   endgenerate

   // generate sample signal
   always @ (posedge clk) begin
      if (rst) begin
         timer <= 0;
      end else begin
         if (sample_interval == 0) begin
            sample <= 0;
            timer <= 0;
         end else begin
            if (timer == sample_interval) begin
               sample <= 1;
               timer <= 0;
            end else begin
               sample <= 0;
               timer <= timer + 1;
            end
         end
      end
   end

endmodule
