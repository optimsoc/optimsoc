/* Copyright (c) 2012-2015 by the author(s)
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
 * A testbench for a simple systems with only one compute tile
 *
 * Parameters:
 *   USE_DEBUG:
 *     Enable the OSD-based debug system.
 *
 *   NUM_CORES:
 *     Number of CPU cores inside the compute tile (default: 1)
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "dbg_config.vh"
`include "optimsoc_def.vh"

import dii_package::dii_flit;
import opensocdebug::mor1kx_trace_exec;

module tb_system_2x2_cccc(
`ifdef verilator
   input clk,
   input rst
`endif
   );

   parameter USE_DEBUG = 0;

   parameter NUM_CORES = 1;

   // NoC parameters
   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;

   parameter VCHANNELS = 3;

   logic rst_sys, rst_cpu;

   logic cpu_stall;
   assign cpu_stall = 0;

// In Verilator, we feed clk and rst from the C++ toplevel, in ModelSim & Co.
// these signals are generated inside this testbench.
`ifndef verilator
   reg clk;
   reg rst;
`endif

   // Reset signals
   // In simulations with debug system, these signals can be triggered through
   // the host software. In simulations without debug systems, we only rely on
   // the global reset signal.
   generate
      if (USE_DEBUG == 0) begin
         assign rst_sys = rst;
         assign rst_cpu = rst;
      end
   endgenerate

   glip_channel c_glip_in(.*);
   glip_channel c_glip_out(.*);

   logic com_rst, logic_rst;

   if (USE_DEBUG == 1) begin
      // TCP communication interface (simulation only)
      glip_tcp_toplevel
        u_glip
          (
           .*,
           .clk_io    (clk),
           .clk_logic (clk),
           .fifo_in   (c_glip_in),
           .fifo_out  (c_glip_out)
           );
   end // if (USE_DEBUG == 1)

   // Monitor system behavior in simulation
   localparam NUM_TILES = 4;

   genvar t;
   genvar i;

   wire [NUM_CORES*NUM_TILES-1:0] termination;

   generate
      for (t = 0; t < NUM_TILES; t = t + 1) begin

         logic [31:0] trace_r3 [0:NUM_CORES-1];
         mor1kx_trace_exec [NUM_CORES-1:0] trace;
         assign trace = u_system.gen_ct[t].u_ct.trace;

         for (i = 0; i < NUM_CORES; i = i + 1) begin
            r3_checker
               u_r3_checker(
                  .clk(clk),
                  .valid(trace[i].valid),
                  .we (trace[i].wben),
                  .addr (trace[i].wbreg),
                  .data (trace[i].wbdata),
                  .r3 (trace_r3[i])
               );

            trace_monitor
               #(
                  .STDOUT_FILENAME({"stdout.",index2string((t*NUM_CORES)+i)}),
                  .TRACEFILE_FILENAME({"trace.",index2string((t*NUM_CORES)+i)}),
                  .ENABLE_TRACE(0),
                  .ID((t*NUM_CORES)+i),
                  .TERM_CROSS_NUM(NUM_TILES*NUM_CORES)
               )
               u_mon0(
                  .termination            (termination[(t*NUM_CORES)+i]),
                  .clk                    (clk),
                  .enable                 (trace[i].valid),
                  .wb_pc                  (trace[i].pc),
                  .wb_insn                (trace[i].insn),
                  .r3                     (trace_r3[i]),
                  .termination_all        (termination)
              );
         end

      end
   endgenerate

   system_2x2_cccc_dm
     #(.USE_DEBUG(USE_DEBUG),
       .CORES(NUM_CORES),
       .MEM_SIZE(32 * 1024 * 1024 /* 32 MB per tile = 128 MB total */))
   u_system
     (.clk (clk),
      .rst (rst | logic_rst),
      .c_glip_in (c_glip_in),
      .c_glip_out (c_glip_out)
      );

// Generate testbench signals.
// In Verilator, these signals are generated in the C++ toplevel testbench
`ifndef verilator
   initial begin
      clk = 1'b1;
      rst = 1'b1;
      noc_out_ready = {VCHANNELS{1'b1}};
      noc_in_valid = '0;
      #15;
      rst = 1'b0;
   end

   always clk = #1.25 ~clk;
`endif

   `include "optimsoc_functions.vh"
endmodule

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
