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
 * =============================================================================
 *
 * System-level testbench for the system_2x2_cccc_nexys4ddr system
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module tb_system_2x2_cccc_nexys4ddr;

   logic clk, cpu_resetn;
   logic uart_rxd_out, uart_txd_in, uart_cts, uart_rts;

   wire [12:0] ddr2_addr;
   wire [2:0]  ddr2_ba;
   wire        ddr2_cas_n;
   wire        ddr2_ck_n;
   wire        ddr2_ck_p;
   wire        ddr2_cke;
   wire        ddr2_cs_n;
   wire [1:0]  ddr2_dm;
   wire [15:0] ddr2_dq;
   wire [1:0]  ddr2_dqs_n;
   wire [1:0]  ddr2_dqs_p;
   wire        ddr2_odt;
   wire        ddr2_ras_n;
   wire        ddr2_we_n;

   // System
   system_2x2_cccc_nexys4
     u_dut
       (.*);

   // Model of the Nexys 4 DDR board
   tb_nexys4ddr
     u_tb
       (.*);

endmodule
