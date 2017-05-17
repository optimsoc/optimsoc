/* Copyright (c) 2017 by the author(s)
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
 * GLIP Looback example for the Xilinx KC705 board
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Max Koenen <max.koenen@tum.de>
 */

module vcu108_loopback
  #(
    parameter WIDTH = 16
    )
  (
   // FX3 interface
   output 	     fx3_pclk,
   inout [WIDTH-1:0] fx3_dq,
   output 	     fx3_slcs_n,
   output 	     fx3_sloe_n,
   output 	     fx3_slrd_n,
   output 	     fx3_slwr_n,
   output 	     fx3_pktend_n,
   output [1:0]  fx3_a,
   input 	     fx3_flaga_n,
   input 	     fx3_flagb_n,
   input 	     fx3_flagc_n,
   input 	     fx3_flagd_n,
   input 	     fx3_com_rst,
   input 	     fx3_logic_rst,

   output [2:0]   fx3_pmode,

   // User logic
   input 	     clk_n,
   input 	     clk_p
   );

   wire  clk;
   wire  clk_locked;

   // The actual frequency is calculated as:    f = 1200MHz / (1200 / FREQ)
   // in which (1200 / FREQ) is cropped to an integer.
   // Working frequencies in MHz are: 100, 92 4/13, 85 5/7, 80, ...
   // Frequencies 80MHz > f > 63MHz produce wrong output from the FX3 and should be avoided.
   localparam FREQ = 32'd100_000_000; // choose maximum frequency of 100 MHz

   vcu108_loopback_clock
      #(.FREQ(FREQ))
   u_clock (
      .rst(),
      .locked(),
      .clk_in_p (clk_p),
      .clk_in_n (clk_n),
      .clk_out  (clk));

   wire [WIDTH-1:0]  loop_data;
   wire 	     loop_valid;
   wire 	     loop_ready;

   wire glip_logic_rst;

   glip_cypressfx3_toplevel
      #(.WIDTH(WIDTH),
      .FREQ_CLK_IO(FREQ))
   u_glib_cypressfx3(
      // Clock/Reset
      .clk     (clk),
      .clk_io  (clk),
      .rst     (),

      // Cypress FX3 ports
      .fx3_pclk      (fx3_pclk),
      .fx3_dq        (fx3_dq),
      .fx3_slcs_n    (fx3_slcs_n),
      .fx3_sloe_n    (fx3_sloe_n),
      .fx3_slrd_n    (fx3_slrd_n),
      .fx3_slwr_n    (fx3_slwr_n),
      .fx3_pktend_n  (fx3_pktend_n),
      .fx3_a         (fx3_a[1:0]),
      .fx3_flaga_n   (fx3_flaga_n),
      .fx3_flagb_n   (fx3_flagb_n),
      .fx3_flagc_n   (fx3_flagc_n),
      .fx3_flagd_n   (fx3_flagd_n),
      .fx3_com_rst   (fx3_com_rst),
      .fx3_logic_rst (fx3_logic_rst),
      .fx3_pmode (fx3_pmode),

      // GLIP FIFO Interface
      // Logic->Host
      .fifo_out_ready   (loop_ready),
      .fifo_out_valid   (loop_valid),
      .fifo_out_data    (loop_data),
      // Host->Logic
      .fifo_in_valid    (loop_valid),
      .fifo_in_data     (loop_data),
      .fifo_in_ready    (loop_ready),
      // GLIP Control Interface
      .com_rst          (),
      .ctrl_logic_rst   (glip_logic_rst));


endmodule

// Local Variables:
// verilog-library-directories:("." "../../verilog/")
// verilog-auto-inst-param-value: t
// End:
