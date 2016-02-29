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
 * Software Trace Module (STM): software trace for a single CPU core
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 *   Michael Tempelmeier <michael.tempelmeier@mytum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module stm(
`ifdef OPTIMSOC_CLOCKDOMAINS
           clk_cdc,
`endif
           /*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready,
   sys_clk_disable,
   // Inputs
   clk, rst, timestamp, trace_port, dbgnoc_out_ready, dbgnoc_in_flit,
   dbgnoc_in_valid, sys_clk_is_halted
   );

   // Debug NoC
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;

   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_TRACE_VCHANNEL = 0;
   parameter DBG_NOC_CONF_VCHANNEL = 0;
   
   // size of the configuration memory (16 bit words)
   localparam CONF_MEM_SIZE = 2;

   parameter CORE_ID = 'hx;

   input clk;
   input rst;
`ifdef OPTIMSOC_CLOCKDOMAINS
   input clk_cdc;
`endif

   // from the Global Timestamp Provider (GTP)
   input [`DBG_TIMESTAMP_WIDTH-1:0] timestamp;

   input [`DEBUG_TRACE_EXEC_WIDTH-1:0] trace_port;

   // Debug NoC interface
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // system control interface
   output sys_clk_disable;
   input  sys_clk_is_halted;
   
   // connection wires between the individual modules
   wire [`DBG_TIMESTAMP_WIDTH+32+16-1:0] trace;
   wire                                  trace_valid;
   wire [`DBG_TIMESTAMP_WIDTH+32+16-1:0] trace_delayed;
   wire                                  trace_delayed_valid;
  
   stm_trace_collector
      u_trace_collector(.clk            (clk),
                        .rst            (rst),
`ifdef OPTIMSOC_CLOCKDOMAINS
                        .clk_cdc        (clk_cdc),
`endif
                        .trace_out      (trace),
                        .trace_enable   (trace_valid),
                        .trace_port     (trace_port),
                        .sys_clk_halted (sys_clk_is_halted),
                        .timestamp      (timestamp));

   stm_data_sr
     #(.DELAY_CYCLES(`DBG_TRIGGER_DELAY),
       .DATA_WIDTH(`DBG_TIMESTAMP_WIDTH+32+16+1)) //timestamp + R3 + wb_insn_LSB + valid
      u_data_sr(.clk(clk),
                .rst(rst),
                .din({trace, trace_valid}),
                .dout({trace_delayed, trace_delayed_valid}));

   stm_dbgnoc_if
      #(.CONF_MEM_SIZE(CONF_MEM_SIZE),
        .DBG_NOC_TRACE_VCHANNEL(DBG_NOC_TRACE_VCHANNEL),
        .DBG_NOC_CONF_VCHANNEL(DBG_NOC_CONF_VCHANNEL),
        .DBG_NOC_VCHANNELS(DBG_NOC_VCHANNELS),
        .CORE_ID(CORE_ID))
      u_dbgnoc_if(.clk(clk),
                  .rst(rst),

                  .dbgnoc_out_flit(dbgnoc_out_flit),
                  .dbgnoc_out_valid(dbgnoc_out_valid),
                  .dbgnoc_out_ready(dbgnoc_out_ready),

                  .dbgnoc_in_flit(dbgnoc_in_flit),
                  .dbgnoc_in_valid(dbgnoc_in_valid),
                  .dbgnoc_in_ready(dbgnoc_in_ready),

                  .sys_clk_disable(sys_clk_disable),

                  .trace_in(trace_delayed),
                  .trace_in_valid(trace_delayed_valid),

                  .conf_mem_flat_out());

  endmodule
