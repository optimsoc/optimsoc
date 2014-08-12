/* Copyright (c) 2012-2013 by the author(s)
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
 * A verilated testbench for a simple system with 4 compute tiles
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "dbg_config.vh"

module tb_system_2x2_cccm(/*AUTOARG*/
   // Inputs
   clk, rst_sys, rst_cpu
   );

   input clk;
   input rst_sys;
   input rst_cpu;

   localparam NUMCORES = 3;
   
   wire [`DEBUG_TRACE_EXEC_WIDTH*NUMCORES-1:0] trace;

   wire [`DEBUG_TRACE_EXEC_WIDTH-1:0] trace_array [0:NUMCORES-1];
   wire                            trace_enable [0:NUMCORES-1] /*verilator public_flat_rd*/;
   wire [31:0]                     trace_insn [0:NUMCORES-1] /*verilator public_flat_rd*/;
   wire [31:0]                     trace_pc [0:NUMCORES-1] /*verilator public_flat_rd*/;
   wire                            trace_wben [0:NUMCORES-1];
   wire [4:0]                      trace_wbreg [0:NUMCORES-1];
   wire [31:0]                     trace_wbdata [0:NUMCORES-1];
   wire [31:0]                     trace_r3 [0:NUMCORES-1] /*verilator public_flat_rd*/;
   
   genvar                          i;
   
   generate
      for (i = 0; i < NUMCORES; i++) begin
         assign trace_array[i]  = trace[(i+1)*`DEBUG_TRACE_EXEC_WIDTH-1:`DEBUG_TRACE_EXEC_WIDTH*i];
         assign trace_enable[i] = trace_array[i][`DEBUG_TRACE_EXEC_ENABLE_MSB:`DEBUG_TRACE_EXEC_ENABLE_LSB];
         assign trace_insn[i]   = trace_array[i][`DEBUG_TRACE_EXEC_INSN_MSB:`DEBUG_TRACE_EXEC_INSN_LSB];
         assign trace_pc[i]     = trace_array[i][`DEBUG_TRACE_EXEC_PC_MSB:`DEBUG_TRACE_EXEC_PC_LSB];
         assign trace_wben[i]   = trace_array[i][`DEBUG_TRACE_EXEC_WBEN_MSB:`DEBUG_TRACE_EXEC_WBEN_LSB];
         assign trace_wbreg[i]  = trace_array[i][`DEBUG_TRACE_EXEC_WBREG_MSB:`DEBUG_TRACE_EXEC_WBREG_LSB];
         assign trace_wbdata[i] = trace_array[i][`DEBUG_TRACE_EXEC_WBDATA_MSB:`DEBUG_TRACE_EXEC_WBDATA_LSB];

         r3_checker
           u_r3_checker(.clk(clk),
                        .valid(trace_enable[i]),
                        .we (trace_wben[i]),
                        .addr (trace_wbreg[i]),
                        .data (trace_wbdata[i]),
                        .r3 (trace_r3[i]));
      end
   endgenerate
   
   wire [31:0]     mt3_adr_o;
   wire            mt3_cyc_o;
   wire [31:0]     mt3_dat_o;
   wire [3:0]      mt3_sel_o;
   wire            mt3_stb_o;
   wire            mt3_we_o;
   wire [2:0]      mt3_cti_o;
   wire [1:0]      mt3_bte_o;
   wire            mt3_ack_i;
   wire            mt3_rty_i;
   wire            mt3_err_i;
   wire [31:0]     mt3_dat_i;

   system_2x2_cccm_dm
     u_system(.clk       (clk),
              .rst_sys   (rst_sys),
              .rst_cpu   (rst_cpu),
              .trace     (trace),
              .mt3_adr_o (mt3_adr_o),
              .mt3_cyc_o (mt3_cyc_o),
              .mt3_dat_o (mt3_dat_o),
              .mt3_sel_o (mt3_sel_o),
              .mt3_stb_o (mt3_stb_o),
              .mt3_we_o  (mt3_we_o),
              .mt3_cti_o (mt3_cti_o),
              .mt3_bte_o (mt3_bte_o),
              .mt3_ack_i (mt3_ack_i),
              .mt3_err_i (mt3_err_i),
              .mt3_rty_i (mt3_rty_i),
              .mt3_dat_i (mt3_dat_i));

   wb_sram_sp
     #(.MEM_SIZE (1*1024*1024),
       .MEM_FILE ("mt.vmem"),
       .MEM_IMPL_TYPE ("PLAIN"))
   u_ram(.wb_adr_i (mt3_adr_o),
         .wb_bte_i (mt3_bte_o),
         .wb_cti_i (mt3_cti_o),
         .wb_cyc_i (mt3_cyc_o),
         .wb_dat_i (mt3_dat_o),
         .wb_sel_i (mt3_sel_o),
         .wb_stb_i (mt3_stb_o),
         .wb_we_i  (mt3_we_o),
         .wb_ack_o (mt3_ack_i),
         .wb_err_o (mt3_err_i),
         .wb_rty_o (mt3_rty_i),
         .wb_dat_o (mt3_dat_i),
         .wb_clk_i (clk),
         .wb_rst_i (rst_sys));
   
endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
