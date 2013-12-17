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
 * Network Router Monitor (NRM): collect data from a single LISNoC router
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

module nrm(/*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready,
   sys_clk_disable, trigger_out,
   // Inputs
   clk, rst, timestamp, dbgnoc_out_ready, dbgnoc_in_flit,
   dbgnoc_in_valid, noc32_router_link_in_flit,
   noc32_router_link_in_ready, noc32_router_link_in_valid, trigger_in
   );

   // ID of this router (used in enumeration to identify the router)
   parameter ROUTER_ID = 'hx;

   // number of links monitored for this router
   parameter MONITORED_LINK_COUNT = 'hx;

   /*
    * collect flit statistics every n cycles (default value, can be set on
    * run-time in a configuration register)
    * 5 flits are required per data transfer, make sure that you got enough
    * bandwidth for all NRM modules in the Debug NoC
    *
    * Example:
    * 4 NRM modules, sample interval of 100 cycles => (4*5)/100 = 20 % of the
    * best-case Debug NoC bandwidth is used by the NRM modules!
    */
   parameter STAT_DEFAULT_SAMPLE_INTERVAL = 16'd0;

   // default setting of the minimum flitcount trigger
   parameter CONF_DEFAULT_MIN_FLITCNT = 8'h0;

   // NoC interface (32 bit flit payload, aka lisnoc)
   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   parameter NOC_VCHANNELS = 3;

   // Debug NoC interface (16 bit flit payload, aka lisnoc16)
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;
   parameter DBG_NOC_PH_CLASS_WIDTH = `PACKET16_CLASS_WIDTH;
   localparam DBG_NOC_PH_ID_WIDTH = DBG_NOC_DATA_WIDTH - DBG_NOC_PH_DEST_WIDTH - DBG_NOC_PH_CLASS_WIDTH;
   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_TRACE_VCHANNEL = 0;
   parameter DBG_NOC_CONF_VCHANNEL = 0;

   // size of the configuration memory (16 bit words)
   localparam CONF_MEM_SIZE = 4;

   input clk;
   input rst;

   // from the Global Timestamp Provider (GTP)
   input [`DBG_TIMESTAMP_WIDTH-1:0] timestamp;

   // Debug NoC interface
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // NoC Router Interface
   input [MONITORED_LINK_COUNT*NOC_FLIT_WIDTH-1:0] noc32_router_link_in_flit;
   input [MONITORED_LINK_COUNT*NOC_VCHANNELS-1:0] noc32_router_link_in_ready;
   input [MONITORED_LINK_COUNT*NOC_VCHANNELS-1:0] noc32_router_link_in_valid;

   // system control interface
   output sys_clk_disable;

   // trigger interface
   input trigger_in;
   output trigger_out;

   // configuration memory
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_out;

   // un-flatten conf_mem_flat_out to conf_mem_out
   wire [15:0] conf_mem_out [CONF_MEM_SIZE-1:0];
   genvar i;
   generate
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem_out
         assign conf_mem_out[i] = conf_mem_flat_out[((i+1)*16)-1:i*16];
      end
   endgenerate

   wire [`DBG_TIMESTAMP_WIDTH+8*MONITORED_LINK_COUNT-1:0] stat_trace;
   wire [`DBG_TIMESTAMP_WIDTH+8*MONITORED_LINK_COUNT-1:0] stat_trace_delayed;
   wire stat_trace_valid;
   wire trace_enable;

   // Debug NoC interface: send trace data, send and receive config data
   nrm_dbgnoc_if
      #(.CONF_MEM_SIZE(CONF_MEM_SIZE),
        .DBG_NOC_TRACE_VCHANNEL(DBG_NOC_TRACE_VCHANNEL),
        .DBG_NOC_CONF_VCHANNEL(DBG_NOC_CONF_VCHANNEL),
        .DBG_NOC_VCHANNELS(DBG_NOC_VCHANNELS),
        .ROUTER_ID(ROUTER_ID),
        .MONITORED_LINK_COUNT(MONITORED_LINK_COUNT),
        .STAT_DEFAULT_SAMPLE_INTERVAL(STAT_DEFAULT_SAMPLE_INTERVAL),
        .CONF_DEFAULT_MIN_FLITCNT(CONF_DEFAULT_MIN_FLITCNT))
      u_dbgnoc_if(.clk(clk),
                  .rst(rst),

                  .dbgnoc_out_flit(dbgnoc_out_flit),
                  .dbgnoc_out_valid(dbgnoc_out_valid),
                  .dbgnoc_out_ready(dbgnoc_out_ready),

                  .dbgnoc_in_flit(dbgnoc_in_flit),
                  .dbgnoc_in_valid(dbgnoc_in_valid),
                  .dbgnoc_in_ready(dbgnoc_in_ready),

                  .sys_clk_disable(sys_clk_disable),

                  .trace_in(stat_trace_delayed),
                  .trace_in_valid(trace_enable),

                  .conf_mem_flat_out(conf_mem_flat_out));

   // statistics collector
   nrm_statistics_collector
      #(.MONITORED_LINK_COUNT(MONITORED_LINK_COUNT),
        .SAMPLE_INTERVAL_WIDTH(16)) // one word = one config register
      u_statistics_collector(.clk(clk),
                             .rst(rst),

                             .timestamp(timestamp[`DBG_TIMESTAMP_WIDTH-1:0]),
                             .sample_interval(conf_mem_out[2]),

                             .noc32_router_link_in_flit(noc32_router_link_in_flit[MONITORED_LINK_COUNT*NOC_FLIT_WIDTH-1:0]),
                             .noc32_router_link_in_ready(noc32_router_link_in_ready[MONITORED_LINK_COUNT*NOC_VCHANNELS-1:0]),
                             .noc32_router_link_in_valid(noc32_router_link_in_valid[MONITORED_LINK_COUNT*NOC_VCHANNELS-1:0]),

                             .trace_out         (stat_trace),
                             .trace_out_valid   (stat_trace_valid));

   // trace qualification (trigger logic)
   nrm_qualificator
      #(.CONF_MEM_SIZE(CONF_MEM_SIZE),
        .MONITORED_LINK_COUNT(MONITORED_LINK_COUNT))
      u_trace_qualificator(.clk(clk),
                           .rst(rst),
                           .trace_in(stat_trace),
                           .trace_in_valid(stat_trace_valid),
                           .trace_enable(trace_enable),
                           .conf_mem_flat(conf_mem_flat_out),
                           .trigger_in(trigger_in),
                           .trigger_out(trigger_out));

   // delay shift register
   debug_data_sr
      #(.DATA_WIDTH(`DBG_TIMESTAMP_WIDTH+32),
        .DELAY_CYCLES(`DBG_TRIGGER_DELAY))
      u_data_sr(.clk(clk),
                .rst(rst),
                .din(stat_trace),
                .dout(stat_trace_delayed));

endmodule
