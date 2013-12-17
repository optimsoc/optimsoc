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
 * Submodule of the Network Router Monitor (NRM): statistics collector
 *
 * This module collects statistics from a router by aggregating data from
 * all router links collected by nrm_link_statistics_collector.
 *
 * (c) 2011-2012 by the author(s)
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 *   Andreas Lankes <andreas.lankes@tum.de>
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
