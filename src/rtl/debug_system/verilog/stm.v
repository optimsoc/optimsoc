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
 * Software Trace Module (STM): software trace for a single CPU core
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 *    Michael Tempelmeier, michael.tempelmeier@mytum.de
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
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

   input [`DEBUG_STM_PORTWIDTH-1:0] trace_port;

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
   
   // Control signals from the traced CPU core
   wire [4:0]  rf_addrw;
   wire [31:0] rf_dataw;
   wire        rf_we;
   wire        cpu_wb_freeze;
   wire [31:0] cpu_wb_insn;

   assign rf_addrw      = trace_port[`DEBUG_STM_RF_ADDRW_MSB:`DEBUG_STM_RF_ADDRW_LSB];
   assign rf_dataw      = trace_port[`DEBUG_STM_RF_DATAW_MSB:`DEBUG_STM_RF_DATAW_LSB];
   assign rf_we         = trace_port[`DEBUG_STM_RF_WE_MSB:`DEBUG_STM_RF_WE_LSB];
   assign cpu_wb_freeze = trace_port[`DEBUG_STM_WB_FREEZE_MSB:`DEBUG_STM_WB_FREEZE_LSB];
   assign cpu_wb_insn   = trace_port[`DEBUG_STM_WB_INSN_MSB:`DEBUG_STM_WB_INSN_LSB];
      
   // connection wires between the individual modules
   wire [`DBG_TIMESTAMP_WIDTH+32+16-1:0] trace;
   wire                                  trace_valid;
   wire [`DBG_TIMESTAMP_WIDTH+32+16-1:0] trace_delayed;
   wire                                  trace_delayed_valid;
  
   stm_trace_collector
      u_trace_collector(.clk(clk),
                        .rst(rst),
`ifdef OPTIMSOC_CLOCKDOMAINS
                        .clk_cdc (clk_cdc),
`endif
                        .trace_out(trace),
                        .trace_enable(trace_valid),
                        .rf_addrw(rf_addrw),
                        .rf_dataw(rf_dataw),
                        .rf_we(rf_we),
                        .cpu_wb_insn(cpu_wb_insn),
                        .cpu_wb_freeze(cpu_wb_freeze|sys_clk_is_halted), 
                        .timestamp(timestamp));

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
