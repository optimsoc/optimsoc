/* Copyright (c) 2014 by the author(s)
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
 * GLIP Looback example for the ZTEX 1.15 boards
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */
module ztex_115_loopback(/*AUTOARG*/
   // Outputs
   fx2_sloe_n, fx2_slrd_n, fx2_slwr_n, fx2_pktend_n, fx2_fifoadr, //led,
   // Inouts
   fx2_fd,
   // Inputs
   fx2_ifclk, fx2_com_rst, fx2_logic_rst, fx2_flaga_n, fx2_flagb_n,
   fx2_flagc_n, fx2_flagd_n, clk
   );

   // FX2 interface
   input        fx2_ifclk;
   input        fx2_com_rst;
   input        fx2_logic_rst;
   inout [15:0] fx2_fd;
   output       fx2_sloe_n;
   output       fx2_slrd_n;
   output       fx2_slwr_n;
   output       fx2_pktend_n;
   output [1:0] fx2_fifoadr;
   input        fx2_flaga_n;
   input        fx2_flagb_n;
   input        fx2_flagc_n;
   input        fx2_flagd_n;

   // User logic
   input        clk;

   wire [15:0]  loop_data;
   wire         loop_valid;
   wire         loop_ready;

   glip_cypressfx2_toplevel
      u_glib_cypressfx2(.fifo_out_ready (loop_ready),
                        .fifo_in_valid  (loop_valid),
                        .fifo_in_data   (loop_data),
                        .fifo_out_valid (loop_valid),
                        .fifo_out_data  (loop_data),
                        .fifo_in_ready  (loop_ready),

                        .rst            (1'b0),
                        .com_rst        (),
                        .ctrl_logic_rst (),

                        /*AUTOINST*/
                        // Outputs
                        .fx2_sloe_n     (fx2_sloe_n),
                        .fx2_slrd_n     (fx2_slrd_n),
                        .fx2_slwr_n     (fx2_slwr_n),
                        .fx2_pktend_n   (fx2_pktend_n),
                        .fx2_fifoadr    (fx2_fifoadr[1:0]),
                        // Inouts
                        .fx2_fd         (fx2_fd[15:0]),
                        // Inputs
                        .fx2_ifclk      (fx2_ifclk),
                        .fx2_com_rst    (fx2_com_rst),
                        .fx2_logic_rst  (fx2_logic_rst),
                        .fx2_flaga_n    (fx2_flaga_n),
                        .fx2_flagb_n    (fx2_flagb_n),
                        .fx2_flagc_n    (fx2_flagc_n),
                        .fx2_flagd_n    (fx2_flagd_n),
                        .clk            (clk));

endmodule

// Local Variables:
// verilog-library-directories:("." "../../verilog/")
// verilog-auto-inst-param-value: t
// End:
