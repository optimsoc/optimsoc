/* Copyright (c) 2013 by the author(s)
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
 * Testbench for system_simpledemo_pgas_ztex
 *
 * This testbench can be used to simulate the whole system as it runs on the
 * ZTEX 1.15 boards. Instead of using the USB interface for external
 * communication a TCP connection is used, emulating the Cypress EZUSB FX2
 * FIFO protocol that is used on the ZTEX board.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`timescale 1ns/1ps

`include "dbg_config.vh"
`include "lisnoc16_def.vh"

module tb_system_simpledemo_pgas_ztex;

   reg clk;
   reg fx2_ifclk;
   wire reset;

   wire [15:0] fx2_fd;
   wire fx2_sloe;
   wire fx2_slrd;
   wire fx2_slwr;
   wire fx2_pktend;
   wire [1:0] fx2_fifoadr;
   wire fx2_flaga;
   wire fx2_flagb;
   wire fx2_flagc;
   wire fx2_flagd;


   initial begin
      clk = 1;
      fx2_ifclk = 1;
   end

   always clk = #10.41ns ~clk; // 48 MHz
   always fx2_ifclk = #16.66ns ~fx2_ifclk; // 30 MHz

   system_simpledemo_pgas_ztex
      u_system(.clk(clk),
               .rst(reset),
               .fx2_ifclk(fx2_ifclk),
               .fx2_fd(fx2_fd),
               .fx2_sloe(fx2_sloe),
               .fx2_slrd(fx2_slrd),
               .fx2_slwr(fx2_slwr),
               .fx2_pktend(fx2_pktend),
               .fx2_fifoadr(fx2_fifoadr),
               .fx2_flaga(fx2_flaga),
               .fx2_flagb(fx2_flagb),
               .fx2_flagc(fx2_flagc),
               .fx2_flagd(fx2_flagd));


   FX2EmulationDebugConnector
      u_debugconn(.fx2_ifclk(fx2_ifclk),
                  .fx2_fd(fx2_fd),
                  .fx2_sloe(fx2_sloe),
                  .fx2_slrd(fx2_slrd),
                  .fx2_slwr(fx2_slwr),
                  .fx2_pktend(fx2_pktend),
                  .fx2_fifoadr(fx2_fifoadr),
                  .fx2_flaga(fx2_flaga),
                  .fx2_flagb(fx2_flagb),
                  .fx2_flagc(fx2_flagc),
                  .fx2_flagd(fx2_flagd),
                  .reset(reset));

endmodule
