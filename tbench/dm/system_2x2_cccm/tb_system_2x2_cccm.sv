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
 * A testbench for a 2x2 CCCM distributed memory system
 *
 * All data coming from printf() calls ("simulated stdout") are written to
 * the file "stdout".
 * During the program run a full instruction trace is generated and saved
 * in the file "trace" if you set the parameter ENABLE_TRACE below.
 *
 * To run this simulation, build a software application (e.g. hello_simple) and
 * link the resulting .vmem file as ct.vmem into the folder containing this
 * file. All compute tiles will run the same software.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "dbg_config.vh"

module tb_system_2x2_cccm();

   reg clk;
   reg rst_sys;
   reg rst_cpu;

   // compute tile parameters (used for all compute tiles)
   parameter CT_MEM_FILE = "ct.vmem";
   parameter CT_MEM_SIZE = 256*1024; // 256 KByte

   parameter MT_MEM_FILE = "mt.vmem";
   parameter MT_MEM_SIZE = 1*1024*1024; // 1 MByte

   localparam NUM_TILES = 3;
   parameter CORES_PER_TILE = 1;
   localparam NUM_CORES = NUM_TILES * CORES_PER_TILE;
   
   // enable instruction trace output
   localparam ENABLE_TRACE = 0;

   wire [NUM_CORES-1:0] termination;

   wire [`DEBUG_TRACE_EXEC_WIDTH*NUM_CORES-1:0] trace;

   wire [`DEBUG_TRACE_EXEC_WIDTH-1:0] trace_array [0:NUM_CORES-1];
   wire                               trace_enable [0:NUM_CORES-1];
   wire [31:0]                        trace_pc [0:NUM_CORES-1];
   wire [31:0]                        trace_insn [0:NUM_CORES-1];
   wire                               trace_wben [0:NUM_CORES-1];
   wire [4:0]                         trace_wbreg [0:NUM_CORES-1];
   wire [31:0]                        trace_wbdata [0:NUM_CORES-1];
   wire [31:0]                        trace_r3 [0:NUM_CORES-1];
   
   genvar                          i;
   
   generate
      for (i = 0; i < NUM_CORES; i++) begin
         assign trace_array[i] = trace[(i+1)*`DEBUG_TRACE_EXEC_WIDTH-1:`DEBUG_TRACE_EXEC_WIDTH*i];
         assign trace_enable[i] = trace_array[i][`DEBUG_TRACE_EXEC_ENABLE_MSB:`DEBUG_TRACE_EXEC_ENABLE_LSB];
         assign trace_insn[i] = trace_array[i][`DEBUG_TRACE_EXEC_INSN_MSB:`DEBUG_TRACE_EXEC_INSN_LSB];
         assign trace_pc[i] = trace_array[i][`DEBUG_TRACE_EXEC_PC_MSB:`DEBUG_TRACE_EXEC_PC_LSB];
         assign trace_wben[i] = trace_array[i][`DEBUG_TRACE_EXEC_WBEN_MSB:`DEBUG_TRACE_EXEC_WBEN_LSB];
         assign trace_wbreg[i] = trace_array[i][`DEBUG_TRACE_EXEC_WBREG_MSB:`DEBUG_TRACE_EXEC_WBREG_LSB];
         assign trace_wbdata[i] = trace_array[i][`DEBUG_TRACE_EXEC_WBDATA_MSB:`DEBUG_TRACE_EXEC_WBDATA_LSB];

         r3_checker
           u_r3_checker(.clk   (clk),
                        .valid (trace_enable[i]),
                        .we    (trace_wben[i]),
                        .addr  (trace_wbreg[i]),
                        .data  (trace_wbdata[i]),
                        .r3    (trace_r3[i]));

         /* trace_monitor AUTO_TEMPLATE(
          .enable  (trace_enable[i]),
          .wb_pc   (trace_pc[i]),
          .wb_insn (trace_insn[i]),
          .r3      (trace_r3[i]),
          .supv    (),
          .termination  (termination[i]),
          .termination_all (termination),
          ); */
         trace_monitor
           #(.STDOUT_FILENAME    ({"stdout.", index2string(i)}),
             .TRACEFILE_FILENAME ({"trace.", index2string(i)}),
             .ENABLE_TRACE       (ENABLE_TRACE),
             .ID                 (i),
             .TERM_CROSS_NUM     (NUM_CORES))
         u_mon0(/*AUTOINST*/
                // Outputs
                .termination            (termination[i]),        // Templated
                // Inputs
                .clk                    (clk),
                .enable                 (trace_enable[i]),       // Templated
                .wb_pc                  (trace_pc[i]),           // Templated
                .wb_insn                (trace_insn[i]),         // Templated
                .r3                     (trace_r3[i]),           // Templated
                .termination_all        (termination));          // Templated
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
      #(.CORES        (CORES_PER_TILE),
        .CT_MEM_FILE     (CT_MEM_FILE),
        .CT_MEM_SIZE     (CT_MEM_SIZE))
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
     #(.MEM_SIZE (MT_MEM_SIZE),
       .MEM_FILE (MT_MEM_FILE),
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
   
   initial begin
      clk = 1'b1;
      rst_sys = 1'b1;
      rst_cpu = 1'b1;
      #15;
      rst_sys = 1'b0;
      rst_cpu = 1'b0;
   end

   always clk = #1.25 ~clk;

   `include "optimsoc_functions.vh"
   
endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
