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
 * Testbench for system_simpledemo_pgas_ztex
 *
 * This testbench can be used to simulate the whole system as it runs on the
 * ZTEX 1.15 boards. Instead of using the USB interface for external
 * communication a TCP connection is used, emulating the Cypress EZUSB FX2
 * FIFO protocol that is used on the ZTEX board.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
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
