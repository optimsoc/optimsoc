/* Copyright (c) 2016 by the author(s)
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
 * ============================================================================
 *
 * Author(s):
 *   Tim Fritzmann <tim.fritzmann@tum.de>
 */

`include "dbg_config.vh"
`include "diagnosis_config.vh"

import dii_package::dii_flit;

module osd_system_diagnosis
   #(
      parameter SYSTEMID='x,
      parameter NUM_MOD='x,
      parameter MAX_PKT_LEN=8)
   (
      input clk, rst,
      input [9:0] id,
      input dii_flit debug_in, output debug_in_ready,
      output dii_flit debug_out, input debug_out_ready,
      /* System Interface */
      input mor1kx_trace_exec trace_port,
      input [32-1:0]  sram_addr,
      input [4-1:0]   sram_ce,
      input           sram_we);

   logic        reg_request;
   logic        reg_write;
   logic [15:0] reg_addr;
   logic [1:0]  reg_size;
   logic [15:0] reg_wdata;
   logic        reg_ack;
   logic        reg_err;
   logic [15:0] reg_rdata;
   logic [15:0] config_reg [`DIAGNOSIS_CONF_FLITS_PER_ENTRY*`DIAGNOSIS_TOTAL_EVENTS_MAX*2:0];
   logic [`DIAGNOSIS_CONF_FLITS_PER_ENTRY*16*`DIAGNOSIS_TOTAL_EVENTS_MAX*2:0] conf_mem;
   logic [`DIAGNOSIS_TIMESTAMP_WIDTH-1:0] timestamp;

   dii_flit dp_out, dp_in;
   logic dp_out_ready, dp_in_ready;
   logic stall;

   /* REG ACCESS */
   osd_regaccess_layer
      #(
         .MODID(16'h6),
         .MODVERSION(16'h0),
         .MAX_REG_SIZE(16),
         .CAN_STALL(1))
   u_regaccess(
         .*,
         .module_in (dp_out),
         .module_in_ready (dp_out_ready),
         .module_out (dp_in),
         .module_out_ready (dp_in_ready));

   always @(*) begin
      reg_ack = 1;
      reg_rdata = 'x;
      reg_err = 0;

      case (reg_addr)
         16'h200: reg_rdata = 32;
         16'h201: reg_rdata = 16'h0;
         default: reg_err = 0;
      endcase // case (reg_addr)
   end // always @ (*)


   /* Assigning Config Register */
   always_ff @(posedge clk)
      if (reg_request & reg_write & (reg_addr >= 16'h200))
         config_reg[reg_addr-16'h200] <= reg_wdata;

   for (genvar i = 0 ; i <= `DIAGNOSIS_CONF_FLITS_PER_ENTRY*`DIAGNOSIS_TOTAL_EVENTS_MAX*2+1; i = i + 1) begin
      assign conf_mem[16*i+15:16*i] = config_reg[i];
   end

   /* Time Stamp Unit */
   osd_timestamp
      #(
         .WIDTH(32))
   u_timestamp(
         .clk  (clk),
         .rst  (rst),
         .enable (1),
         .timestamp (timestamp));

   /* System Diagnosis Module Implementation */
   diagnosis_system
   u_diagnosis_system(
         // Inputs
         .clk (clk),
         .rst (rst),
         .memaddr_val (sram_addr),
         .sram_ce (sram_ce),
         .sram_we (sram_we),
         .time_global (timestamp),
         .trace_port (trace_port),
         .dbgnoc_in_flit (dp_in),
         .dbgnoc_in_valid (1),
         .dbgnoc_out_ready (dp_out_ready),
         .conf_mem (conf_mem),
         // Outputs
         .dbgnoc_in_ready (dp_in_ready),
         .dbgnoc_out_flit (dp_out),
         .dbgnoc_out_valid ());
endmodule
