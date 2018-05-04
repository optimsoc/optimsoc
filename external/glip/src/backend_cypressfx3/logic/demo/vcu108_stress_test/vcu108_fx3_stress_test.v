/* Copyright (c) 2017-2018 by the author(s)
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
 * GLIP FX3 stress test example for the Xilinx VCU108 board.
 *
 * Author(s):
 *   Max Koenen <max.koenen@tum.de>
 */

module vcu108_fx3_stress_test
#(
   parameter WIDTH = 16
)(
   // FX3 interface
   output            fx3_pclk,
   inout [WIDTH-1:0] fx3_dq,
   output            fx3_slcs_n,
   output            fx3_sloe_n,
   output            fx3_slrd_n,
   output            fx3_slwr_n,
   output            fx3_pktend_n,
   output [1:0]      fx3_a,
   input             fx3_flaga_n,
   input             fx3_flagb_n,
   input             fx3_flagc_n,
   input             fx3_flagd_n,
   input             fx3_com_rst,
   input             fx3_logic_rst,

   output [2:0]      fx3_pmode,

   // Status LEDs
   output            error_led,
   output            idle_led,
   
   // Stall flag
   input             stall_flag,
   
   // Error flag
   input             error_flag,

   // Stress test reset switch
   input             rst_sw,

   // Clock
   input             clk_n,
   input             clk_p
);

   wire clk100;
   wire clk50;

   // Clock buffering
   //------------------------------------
   wire clk300;

   IBUFDS clkin1_ibufds(
      .O  (clk300),
      .I  (clk_p),
      .IB (clk_n));

   // Generate 100MHz clock for the FX3
   vcu108_stress_test_clock
      #(.FREQ(32'd100_000_000))
   u_clock_100(
      .rst      (),
      .locked   (),
      .clk_in   (clk300),
      .clk_out  (clk100));

   // Generate 50MHz clock for the stress test logic to test cross domain clocking
   vcu108_stress_test_clock
      #(.FREQ(32'd50_000_000))
   u_clock_50(
      .rst      (),
      .locked   (),
      .clk_in   (clk300),
      .clk_out  (clk50));


   wire [WIDTH-1:0]  fifo_out_data;
   wire              fifo_out_valid;
   wire              fifo_out_ready;
   wire [WIDTH-1:0]  fifo_in_data;
   wire              fifo_in_valid;
   wire              fifo_in_ready;   

   wire              glip_logic_rst;

   glip_cypressfx3_toplevel
      #(.WIDTH(WIDTH))
   u_glib_cypressfx3(
      // Clock/Reset
      .clk        (clk50),
      .clk_io_100 (clk100),
      .rst        (),
   
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
      .fx3_pmode     (fx3_pmode),
   
      // GLIP FIFO Interface
      // Logic->Host
      .fifo_out_ready   (fifo_out_ready),
      .fifo_out_valid   (fifo_out_valid),
      .fifo_out_data    (fifo_out_data),
      // Host->Logic
      .fifo_in_valid    (fifo_in_valid),
      .fifo_in_data     (fifo_in_data),
      .fifo_in_ready    (fifo_in_ready),
      // GLIP Control Interface
      .com_rst          (),
      .ctrl_logic_rst   (glip_logic_rst));


   io_stress_test
      #(.WIDTH(WIDTH))
   u_stress_test(
      .clk              (clk50),
      .rst              (glip_logic_rst | rst_sw),
      .fifo_out_ready   (fifo_out_ready),
      .fifo_out_valid   (fifo_out_valid),
      .fifo_out_data    (fifo_out_data),
      .fifo_in_valid    (fifo_in_valid),
      .fifo_in_data     (fifo_in_data),
      .fifo_in_ready    (fifo_in_ready),
      .stall_flag       (stall_flag),
      .error_flag       (error_flag),
      .error            (error_led),
      .idle             (idle_led));

endmodule

// Local Variables:
// verilog-library-directories:("." "../../verilog/")
// verilog-auto-inst-param-value: t
// End:
