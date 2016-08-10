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
 * Model of the Nexys 4 DDR board
 *
 * Supported board features:
 * - DDR
 * - UART
 *
 * Limitations:
 * - UART: HW handshaking (CTS/RTS) is not modeled.
 * - UART: Baud rate is fixed at 115200 baud
 * - DRAM: it's slooooow
 *
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

// Keep this timescale to match DRAM model
`timescale 1ps / 1ps

module tb_nexys4ddr (
   // 100 MHz system clock
   output reg   clk,

   // push button "CPU RESET" (C12)
   output reg   cpu_resetn,

   // UART (signals names from a DTE/PC point of view)
   output       uart_txd_in,
   input        uart_rxd_out,
   input        uart_cts,
   output       uart_rts,

   // DRAM
   input [12:0]         ddr2_addr,
   input [2:0]          ddr2_ba,
   input                ddr2_cas_n,
   input                ddr2_ck_n,
   input                ddr2_ck_p,
   input                ddr2_cke,
   input                ddr2_cs_n,
   input [1:0]          ddr2_dm,
   inout [15:0]         ddr2_dq,
   inout [1:0]          ddr2_dqs_n,
   inout [1:0]          ddr2_dqs_p,
   input                ddr2_odt,
   input                ddr2_ras_n,
   input                ddr2_we_n
   );

   // generate 100 MHz board clock
   always
     #5000 clk = ~clk;

   initial begin
      u_ddr2.reset_task;
      clk = 0;

      // XXX: is this how it works on the board? Isn't cpu_resetn just a button?
      cpu_resetn = 0;
      @(negedge glbl.GSR);
      #100000
      cpu_resetn = 1;
   end

   ddr2
      u_ddr2(
         .ck      (ddr2_ck_p),
         .ck_n    (ddr2_ck_n),
         .cke     (ddr2_cke),
         .cs_n    (ddr2_cs_n),
         .ras_n   (ddr2_ras_n),
         .cas_n   (ddr2_cas_n),
         .we_n    (ddr2_we_n),
         .dm_rdqs (ddr2_dm),
         .ba      (ddr2_ba),
         .addr    (ddr2_addr),
         .dq      (ddr2_dq),
         .dqs     (ddr2_dqs_p),
         .dqs_n   (ddr2_dqs_n),
         .rdqs_n  (),
         .odt     (ddr2_odt));

   // UART model
   uartdpi
      #(
         .BAUD(115200),
         .FREQ(100000000))
      u_uartdpi(
         .clk (clk),
         .tx  (uart_txd_in),
         .rx  (uart_rxd_out),
         .rst (glbl.GSR));

   assign uart_rtsn = 1'b0;

endmodule // tb_nexys4
