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
 * Instruction Trace Module (ITM): instruction trace for a single CPU core
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module itm(
`ifdef OPTIMSOC_CLOCKDOMAINS
           clk_cdc,
`endif
           /*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready,
   sys_clk_disable, trigger_out,
   // Inputs
   clk, rst, timestamp, trace_port, dbgnoc_out_ready, dbgnoc_in_flit,
   dbgnoc_in_valid, sys_clk_is_halted, trigger_in
   );

   // compressed trace
   localparam INSTR_COUNT_WIDTH = 8;
   localparam COMPRESSED_TRACE_WIDTH = `DBG_TIMESTAMP_WIDTH + 32 + INSTR_COUNT_WIDTH;

   // Debug NoC
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;

   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_TRACE_VCHANNEL = 0;
   parameter DBG_NOC_CONF_VCHANNEL = 0;


   // size of the configuration memory (16 bit words)
   localparam CONF_MEM_SIZE = 6;

   parameter CORE_ID = 'hx;

   input clk;
   input rst;
`ifdef OPTIMSOC_CLOCKDOMAINS
   input clk_cdc;
`endif

   // from the Global Timestamp Provider (GTP)
   input [`DBG_TIMESTAMP_WIDTH-1:0] timestamp;

   input [`DEBUG_ITM_PORTWIDTH-1:0] trace_port;

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

   // trigger interface
   output trigger_out;
   input trigger_in;

   // Control signals from the traced CPU core
   wire [31:0] cpu_wb_pc;
   wire        cpu_wb_freeze;
   assign cpu_wb_pc = trace_port[31:0];
   assign cpu_wb_freeze = trace_port[32]|sys_clk_is_halted;

   // connection wires between the individual modules
   wire [`DBG_TIMESTAMP_WIDTH+32-1:0] uncompressed_trace;
   wire [`DBG_TIMESTAMP_WIDTH+32-1:0] uncompressed_trace_delayed;
   wire [COMPRESSED_TRACE_WIDTH-1:0] compressed_trace;
   wire compressed_trace_valid;
   wire trace_enable;

   wire [16*CONF_MEM_SIZE-1:0] conf_mem_flat_out;

   itm_trace_collector
      u_trace_collector(.clk(clk),
                        .rst(rst),
`ifdef OPTIMSOC_CLOCKDOMAINS
                        .clk_cdc (clk_cdc),
`endif
                        .trace_out(uncompressed_trace),
                        .cpu_wb_pc(cpu_wb_pc),
                        .cpu_wb_freeze(cpu_wb_freeze),
                        .timestamp(timestamp));

   debug_data_sr
      #(.DATA_WIDTH(`DBG_TIMESTAMP_WIDTH+32),
        .DELAY_CYCLES(`DBG_TRIGGER_DELAY))
      u_data_sr(.clk(clk),
                .rst(rst),
                .din(uncompressed_trace),
                .dout(uncompressed_trace_delayed));

   itm_trace_compression
      u_trace_compression(.clk(clk),
                          .rst(rst),
                          .trace_in_valid(trace_enable),
                          .trace_in(uncompressed_trace_delayed),
                          .trace_out_compressed(compressed_trace),
                          .trace_out_compressed_valid(compressed_trace_valid));

   itm_dbgnoc_if
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

                  .trace_in(compressed_trace),
                  .trace_in_valid(compressed_trace_valid),

                  .conf_mem_flat_out(conf_mem_flat_out));

   itm_trace_qualificator
      #(.CONF_MEM_SIZE(CONF_MEM_SIZE))
      u_trace_qualificator(.clk(clk),
                           .rst(rst),
                           .trace_in(uncompressed_trace),
                           .trace_enable(trace_enable),
                           .conf_mem_flat(conf_mem_flat_out),
                           .trigger_out(trigger_out),
                           .trigger_in(trigger_in));

endmodule
