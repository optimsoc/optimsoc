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
 * A testbench for a 2x2 CCCC distributed memory system
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

module tb_system_2x2_cccc();

   reg clk;
   reg rst_sys;
   reg rst_cpu;

   // compute tile parameters (used for all compute tiles)
   localparam MEM_FILE = "ct.vmem";
   localparam MEM_SIZE = 1*1024*1024; // 1 MByte

   localparam NUM_CORES = 4;   
   
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
      for (i = 0; i < 4; i++) begin
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
           #(.STDOUT_FILENAME                 ({"stdout.", index2string(i)}),
             .TRACEFILE_FILENAME              ({"trace.", index2string(i)}),
             .ENABLE_TRACE(1),
             .ID(i),
             .TERM_CROSS_NUM(NUM_CORES))
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

   system_2x2_cccc_dm
      #(.MEM_FILE(MEM_FILE),
        .MEM_SIZE(MEM_SIZE))
      u_system(.clk                      (clk),
               .rst_sys                  (rst_sys),
               .rst_cpu                  (rst_cpu),
               .trace                (trace));
   
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
