/* Copyright (c) 2012-2014 by the author(s)
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
 * A simple test system with only a single compute tile
 *
 * During the program run a full instruction trace is generated and saved
 * in the file "trace". All data coming from printf() calls ("simulated stdout")
 * are written to the file "stdout".
 *
 * To run this simulation, build a software application (e.g. hello_simple) and
 * link the resulting .vmem file as ct.vmem into the folder containing this
 * file.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "dbg_config.vh"

module tb_compute_tile();

   // NoC parameters
   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   parameter VCHANNELS = 3;

   reg clk;
   reg rst_sys;
   reg rst_cpu;
   reg cpu_stall;

   reg [NOC_FLIT_WIDTH-1:0] noc_in_flit;
   reg [VCHANNELS-1:0] noc_in_valid;
   wire [VCHANNELS-1:0] noc_in_ready;
   wire [NOC_FLIT_WIDTH-1:0] noc_out_flit;
   wire [VCHANNELS-1:0] noc_out_valid;
   reg [VCHANNELS-1:0] noc_out_ready;

   parameter NUMCORES = 1;

   wire [`DEBUG_ITM_PORTWIDTH-1:0] trace_itm;
   wire [`DEBUG_STM_PORTWIDTH-1:0] trace_stm;

   wire [`DEBUG_STM_PORTWIDTH-1:0] trace_stm_array  [0:NUMCORES-1]; 
   wire                            trace_stm_enable [0:NUMCORES-1];
   wire [31:0]                     trace_stm_insn   [0:NUMCORES-1];
   wire                            trace_stm_wben   [0:NUMCORES-1];
   wire [4:0]                      trace_stm_wbreg  [0:NUMCORES-1];
   wire [31:0]                     trace_stm_wbdata [0:NUMCORES-1];
   wire [31:0]                     trace_stm_r3     [0:NUMCORES-1];

   genvar                          i;
   
   generate
      for (i = 0; i < NUMCORES; i++) begin
         assign trace_stm_array[i] = trace_stm[(i+1)*`DEBUG_STM_PORTWIDTH-1:`DEBUG_STM_PORTWIDTH*i];
         assign trace_stm_enable[i] = trace_stm_array[i][`DEBUG_STM_ENABLE_MSB:`DEBUG_STM_ENABLE_LSB];
         assign trace_stm_insn[i] = trace_stm_array[i][`DEBUG_STM_INSN_MSB:`DEBUG_STM_INSN_LSB];
         assign trace_stm_wben[i] = trace_stm_array[i][`DEBUG_STM_WB_MSB:`DEBUG_STM_WB_LSB];
         assign trace_stm_wbreg[i] = trace_stm_array[i][`DEBUG_STM_WBREG_MSB:`DEBUG_STM_WBREG_LSB];
         assign trace_stm_wbdata[i] = trace_stm_array[i][`DEBUG_STM_WBDATA_MSB:`DEBUG_STM_WBDATA_LSB];

         r3_checker
           u_r3_checker(.clk(clk),
                        .valid(trace_stm_enable[i]),
                        .we (trace_stm_wben[i]),
                        .addr (trace_stm_wbreg[i]),
                        .data (trace_stm_wbdata[i]),
                        .r3 (trace_stm_r3[i]));
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
                     .trace_itm         (trace_itm),
                     .trace_stm         (trace_stm),
                     // Inputs
                     .clk               (clk),
                     .rst_cpu           (rst_cpu),
                     .rst_sys           (rst_sys),
                     .noc_in_flit       (noc_in_flit[NOC_FLIT_WIDTH-1:0]),
                     .noc_in_valid      (noc_in_valid[VCHANNELS-1:0]),
                     .noc_out_ready     (noc_out_ready[VCHANNELS-1:0]),
                     .cpu_stall         (cpu_stall));

   wire termination;

   /* trace_monitor AUTO_TEMPLATE(
    .enable  (trace_stm_enable[0]),
    .wb_pc   (trace_itm[31:0]),
    .wb_insn (trace_stm_insn[0]),
    .r3      (trace_stm_r3[0]),
    .supv    (),
    .termination  (termination),
    .termination_all (termination),
    ); */
   trace_monitor
      #(.STDOUT_FILENAME("stdout"),
        .TRACEFILE_FILENAME("trace"),
        .ENABLE_TRACE(1))
      u_mon0(/*AUTOINST*/
             // Outputs
             .termination               (termination),           // Templated
             // Inputs
             .clk                       (clk),
             .enable                    (trace_stm_enable[0]),   // Templated
             .wb_pc                     (trace_itm[31:0]),       // Templated
             .wb_insn                   (trace_stm_insn[0]),     // Templated
             .r3                        (trace_stm_r3[0]),       // Templated
             .termination_all           (termination));          // Templated



   initial begin
      clk = 1'b1;
      rst_sys = 1'b1;
      rst_cpu = 1'b1;
      noc_out_ready = {VCHANNELS{1'b1}};
      noc_in_valid = '0;
      cpu_stall = 0;
      #15;
      rst_sys = 1'b0;
      rst_cpu = 1'b0;
   end

   always clk = #1.25 ~clk;

endmodule

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
