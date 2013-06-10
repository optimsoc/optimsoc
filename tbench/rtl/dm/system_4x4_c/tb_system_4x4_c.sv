/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
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
 * A testbench for a 4x4 compute-tile-only distributed memory system
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
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 *    Philipp Wagner, philipp.wagner@tum.de
 */

`include "timescale.v"

module tb_system_4x4_c();

   reg clk;
   reg rst_sys;
   reg rst_cpu;

   // compute tile parameters (used for all compute tiles)
   localparam MEM_FILE = "ct.vmem";
   localparam MEM_SIZE = 1*1024*1024; // 1 MByte

   // enable instruction trace output
   localparam ENABLE_TRACE = 0;

   system_4x4_c_dm
      #(.MEM_FILE(MEM_FILE),
        .MEM_SIZE(MEM_SIZE))
      u_system(.clk                      (clk),
               .rst_sys                  (rst_sys),
               .rst_cpu                  (rst_cpu));

   wire [15:0] termination;

   function [23:0] index2string;
      input [11:0] index;
      integer hundreds;
      integer tens;
      integer ones;
      begin
         hundreds = (index) / 100;
         tens = (index - (hundreds * 100)) / 10;
         ones = (index - (hundreds * 100) - (tens * 10));
         index2string[23:16] = hundreds + 8'd48;
         index2string[15:8] = tens + 8'd48;
         index2string[7:0] = ones + 8'd48;
      end
   endfunction

   genvar i;
   generate
      for (i=0; i<16; i=i+1) begin : gen_mon
         trace_monitor
            #(.ID                              (i),
              .ENABLE_TRACE                    (ENABLE_TRACE),
              .STDOUT_FILENAME                 ({"stdout.", index2string(i)}),
              .TRACEFILE_FILENAME              ({"trace.", index2string(i)}),
              .TERM_CROSS_NUM                  (16))
             u_mon(// Outputs
                   .termination                (termination[i]),
                   // Inputs
                   .clk                        (clk),
                   .enable                     (~u_system.gen_ct[i].u_ct.u_core0.u_cpu.or1200_cpu.or1200_except.wb_freeze),
                   .wb_pc                      (u_system.gen_ct[i].u_ct.u_core0.u_cpu.or1200_cpu.or1200_except.wb_pc),
                   .wb_insn                    (u_system.gen_ct[i].u_ct.u_core0.u_cpu.or1200_cpu.or1200_ctrl.wb_insn),
                   .r3                         (u_system.gen_ct[i].u_ct.u_core0.u_cpu.or1200_cpu.or1200_rf.rf_a.mem[3]),
                   .termination_all            (termination));
      end
   endgenerate

   initial begin
      clk = 1'b1;
      rst_sys = 1'b1;
      rst_cpu = 1'b1;
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
