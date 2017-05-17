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

module kc705_loopback
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
   input 	     clk_p, 
   
   output 	     lcd_en,
   output 	     lcd_rs,
   output 	     lcd_rw,
   output [3:0]  lcd_data
   );

   wire 	clk;

   // Frequencies 85MHz > f > 63MHz produce wrong output from the FX3 and should be avoided.
   // If WIDTH == 32 the maximum frequency is 100MHz.
   // If WIDTH == 16 the maximum frequency that currently works is 63MHz.
   localparam FREQ = 32'd60000000;

   wire clk_locked;

   kc705_loopback_clock
      #(.FREQ(FREQ))
   u_clock (
      .clk_in_p (clk_p),
      .clk_in_n (clk_n),
      .clk_out  (clk),
      .locked   (clk_locked),
      .rst      (fx3_com_rst));
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


   reg [7:0]   display_data;
   reg         display_en;
   reg         display_row;
   reg [3:0]   display_col;

   lcd
      #(.FREQ(FREQ))
   u_lcd(
      // Outputs
      .data	(lcd_data[3:0]),
      .en		(lcd_en),
      .rw		(lcd_rw),
      .rs		(lcd_rs),
      // Inputs
      .clk		(clk),
      .rst		(glip_logic_rst),
      .in_data	(display_data),
      .in_enable	(display_en),
      .in_pos	(display_col),
      .in_row	(display_row));
   
   wire [3:0]		sample_3;
   wire [3:0]		sample_4;
   wire [3:0]		sample_5;
   wire [3:0]		sample_6;
   wire [3:0]		sample_7;
   wire [3:0]		sample_8;
   
   reg [31:0] 	count;
   
   always @(posedge clk) begin
      if (count == FREQ) begin
         count <= 0;
      end else begin
         count <= count + 1;
      end

      case (count)
         0: display_data <= 8'h67; // g
         1: display_data <= 8'h6c; // l
         2: display_data <= 8'h69; // i
         3: display_data <= 8'h70; // p
         4: display_data <= 8'h20; //
         5: display_data <= 8'h6c; // l
         6: display_data <= 8'h6f; // o
         7: display_data <= 8'h6f; // o
         8: display_data <= 8'h70; // p
         9: display_data <= 8'h62; // b
         10: display_data <= 8'h61; // a
         11: display_data <= 8'h63; // c
         12: display_data <= 8'h6b; // k
         13: display_data <= 8'h20; //
         14: display_data <= 8'h20; //
         15: display_data <= 8'h23; // #
         16: display_data <= 8'h20; //
         17: display_data <= 8'h20; //
         18: display_data <= 8'h20; //
         19: display_data <= 8'h30 + sample_8; //
         20: display_data <= 8'h30 + sample_7; // -
         21: display_data <= 8'h30 + sample_6; // -
         22: display_data <= 8'h2e; // .
         23: display_data <= 8'h30 + sample_5; // -
         24: display_data <= 8'h30 + sample_4; // -
         25: display_data <= 8'h30 + sample_3; // -
         26: display_data <= 8'h20; //
         27: display_data <= 8'h4d; // M
         28: display_data <= 8'h42; // B
         29: display_data <= 8'h2f; // /
         30: display_data <= 8'h73; // s
         31: display_data <= 8'h20; //
         default: display_data <= 8'hx;
      endcase
      display_en <= (count < 32);
      display_row <= count[4];
      display_col <= count[3:0];
   end

   measure_count
      #(.STEP((WIDTH == 16) ? 2 : 4))
   u_count(
      .clk(clk),
      .rst(fx3_logic_rst | (count == FREQ)),
      .valid(loop_valid && loop_ready),
      .sample_3(sample_3),
      .sample_4(sample_4),
      .sample_5(sample_5),
      .sample_6(sample_6),
      .sample_7(sample_7),
      .sample_8(sample_8));
   
endmodule

// Local Variables:
// verilog-library-directories:("." "../../verilog/")
// verilog-auto-inst-param-value: t
// End:
