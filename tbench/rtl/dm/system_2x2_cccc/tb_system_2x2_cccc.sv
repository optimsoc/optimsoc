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
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

`include "timescale.v"

module tb_system_2x2_cccc();

   reg clk;
   reg rst_sys;
   reg rst_cpu;

   // compute tile parameters (used for all compute tiles)
   localparam MEM_FILE = "ct.vmem";
   localparam MEM_SIZE = 1*1024*1024; // 1 MByte

   // enable instruction trace output
   localparam ENABLE_TRACE = 0;

   system_2x2_cccc_dm
      #(.MEM_FILE(MEM_FILE),
        .MEM_SIZE(MEM_SIZE))
      u_system(.clk                      (clk),
               .rst_sys                  (rst_sys),
               .rst_cpu                  (rst_cpu));

   wire [3:0] termination;

   genvar i;
   generate
      for (i=0; i<4; i=i+1) begin : gen_mon
         trace_monitor
            #(.ID                              (i),
              .ENABLE_TRACE                    (ENABLE_TRACE),
              .STDOUT_FILENAME                 ({"stdout.", i+48}), // i+48 converts int to chars
              .TRACEFILE_FILENAME              ({"trace.", i+48}),
              .TERM_CROSS_NUM                  (4))
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

endmodule // tb_system_2x2_ccmc

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
