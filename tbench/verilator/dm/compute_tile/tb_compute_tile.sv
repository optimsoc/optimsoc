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
 * A verilated testbench for a simple systems with only one compute tile.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "dbg_config.vh"

module tb_compute_tile(/*AUTOARG*/
   // Inputs
   clk, rst_sys, rst_cpu, cpu_stall
   );

   // NoC parameters
   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   parameter VCHANNELS = 3;

   input clk;
   input rst_sys;
   input rst_cpu;
   input cpu_stall;

   localparam NUMCORES = 1;

   wire [`DEBUG_TRACE_EXEC_WIDTH*NUMCORES-1:0] trace;

   wire [`DEBUG_TRACE_EXEC_WIDTH-1:0] trace_array [0:NUMCORES-1];
   wire                               trace_enable [0:NUMCORES-1] /*verilator public_flat_rd*/;
   wire [31:0]                        trace_insn [0:NUMCORES-1] /*verilator public_flat_rd*/;
   wire [31:0]                        trace_pc [0:NUMCORES-1] /*verilator public_flat_rd*/;
   wire                               trace_wben [0:NUMCORES-1];
   wire [4:0]                         trace_wbreg [0:NUMCORES-1];
   wire [31:0]                        trace_wbdata [0:NUMCORES-1];
   wire [31:0]                        trace_r3 [0:NUMCORES-1] /*verilator public_flat_rd*/;

   wire 		       trace_noc_egress_start /*verilator public_flat_rd*/;
   wire 		       trace_noc_egress_end /*verilator public_flat_rd*/;
   wire 		       trace_noc_ingress_start /*verilator public_flat_rd*/;
   wire 		       trace_noc_ingress_end /*verilator public_flat_rd*/;
   wire 		       trace_ct_egress_start /*verilator public_flat_rd*/;
   wire 		       trace_ct_egress_end /*verilator public_flat_rd*/;
   wire 		       trace_ct_ingress_irq /*verilator public_flat_rd*/;
   wire 		       trace_ct_ingress_start /*verilator public_flat_rd*/;
   wire 		       trace_ct_ingress_end /*verilator public_flat_rd*/;

   assign trace_noc_egress_start = u_compute_tile.u_na.u_mp_simple.trace_noc_egress_start;
   assign trace_noc_egress_end = u_compute_tile.u_na.u_mp_simple.trace_noc_egress_end;
   assign trace_noc_ingress_start = u_compute_tile.u_na.u_mp_simple.trace_noc_ingress_start;
   assign trace_noc_ingress_end = u_compute_tile.u_na.u_mp_simple.trace_noc_ingress_end;
   assign trace_ct_egress_start = u_compute_tile.u_na.u_mp_simple.trace_ct_egress_start;
   assign trace_ct_egress_end = u_compute_tile.u_na.u_mp_simple.trace_ct_egress_end;
   assign trace_ct_ingress_irq = u_compute_tile.u_na.u_mp_simple.trace_ct_ingress_irq;
   assign trace_ct_ingress_start = u_compute_tile.u_na.u_mp_simple.trace_ct_ingress_start;
   assign trace_ct_ingress_end = u_compute_tile.u_na.u_mp_simple.trace_ct_ingress_end;

   reg [NOC_FLIT_WIDTH-1:0] noc_in_flit;
   reg [VCHANNELS-1:0] noc_in_valid;
   wire [VCHANNELS-1:0] noc_in_ready;
   wire [NOC_FLIT_WIDTH-1:0] noc_out_flit;
   wire [VCHANNELS-1:0] noc_out_valid;
   reg [VCHANNELS-1:0] noc_out_ready;
 
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
   
   compute_tile_dm
      #(.ID(0),
        .CORES(1),
        .MEM_SIZE(1*1024*1024), // 1 MB
        .MEM_FILE("ct.vmem"))
      u_compute_tile(// Outputs
                     .noc_in_ready      (noc_in_ready[VCHANNELS-1:0]),
                     .noc_out_flit      (noc_out_flit[NOC_FLIT_WIDTH-1:0]),
                     .noc_out_valid     (noc_out_valid[VCHANNELS-1:0]),
                     // Inputs
                     .clk               (clk),
                     .rst_cpu           (rst_cpu),
                     .rst_sys           (rst_sys),
                     .noc_in_flit       (noc_in_flit[NOC_FLIT_WIDTH-1:0]),
                     .noc_in_valid      (noc_in_valid[VCHANNELS-1:0]),
                     .noc_out_ready     (noc_out_ready[VCHANNELS-1:0]),
                     .cpu_stall         (cpu_stall),
                     .trace             (trace));

endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:

