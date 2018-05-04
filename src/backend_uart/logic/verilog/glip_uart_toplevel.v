/* Copyright (c) 2015-2017 by the author(s)
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
 * UART Toplevel
 *
 * This module handles the UART interface and puts incoming bytes to a
 * FIFO and vice versa. The module only supports 8N1 UART, meaning 8
 * bit, no parity and one stop bit. All baud rates are supported, but
 * be careful with low frequencies and large baud rates that the
 * tolerance of the rounded bit divisor (rounding error of
 * FREQ_CLK_IO/BAUD) is within 2%.
 *
 * The uart_* signals are seen from the side of this module, i.e. from the DCE
 * side. To connect them to a host, connect the signals in a crossed way.
 *
 *
 * Data Paths
 *
 *  All modules marked with * are in I/O clock domain clk_io, all others are
 *  are in the logic clock domain clk.
 *
 * Ingress
 *   u_receive* -> u_control* -> u_ingress_cdc -> u_ingress_upscale
 *   -> u_ingress_buffer
 *
 * Egress
 *   u_egress_downscale -> u_egress_cdc -> u_egress_buffer* -> u_control*
 *   -> u_transmit*
 *
 *
 * Parameters:
 *  - FREQ_CLK_IO: The frequency of clk_io
 *  - BAUD: Interface baud rate
 *  - WIDTH: Width of the fifo_* signals. Supported values: 8 and 16
 *  - BUFFER_OUT_DEPTH: size of the output buffer in bytes
 *
 * Author(s):
 *   Max Koenen <max.koenen@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module glip_uart_toplevel
   #(parameter FREQ_CLK_IO = 32'hx,
     parameter BAUD = 115200,
     parameter WIDTH = 8,
     parameter BUFFER_OUT_DEPTH = 4*1024)
   (
      // Clock & Reset
      input              clk, // Logic clock (GLIP default)
      input              clk_io, // I/O clock
      input              rst,

      output             com_rst,

      // GLIP FIFO Interface
      input [WIDTH-1:0]  fifo_out_data,
      input              fifo_out_valid,
      output             fifo_out_ready,
      output [WIDTH-1:0] fifo_in_data,
      output             fifo_in_valid,
      input              fifo_in_ready,

      // GLIP Control Interface
      output             ctrl_logic_rst,

      // UART Interface
      // All ports are seen from our side, i.e. from the DCE side
      input              uart_rx,
      output             uart_tx,
      input              uart_cts_n, // active low
      output             uart_rts_n, // active low

      // Error signal if failure on the line
      output reg         error
   );


   // Assert that the actual baud rate is within two percent of the specified
   // baud rate.
`ifndef SYNTHESIS
// synthesis translate_off
   integer divisor, baud_used, baud_err;
   real baud_err_percent;
   initial begin
      assign divisor = FREQ_CLK_IO / BAUD;
      assign baud_used = FREQ_CLK_IO / divisor;
      assign baud_err = (baud_used > BAUD ? baud_used - BAUD : BAUD - baud_used);
      assign baud_err_percent = $itor(baud_err) / $itor(BAUD);
      $display("%m: Using baud rate of %d, resulting in an error of %f percent from the specified baud rate of %d.",
               baud_used, baud_err_percent, BAUD);
      if (baud_err_percent > 0.02) begin
         $display("%m: Baud error larger than two percent.");
         $stop;
      end
   end
// synthesis translate on
`endif


   wire               transfer_in;

   wire [7:0]         receive_data;
   wire               receive_enable;
   wire               receive_error;

   wire [7:0]         ingress_buffer_data;
   wire               ingress_buffer_valid;
   wire               ingress_buffer_ready;

   wire [7:0]         ingress_ctrl_in_data;
   wire               ingress_ctrl_in_valid;
   wire               ingress_ctrl_in_ready;
   wire [7:0]         ingress_ctrl_out_data;
   wire               ingress_ctrl_out_valid;
   wire               ingress_ctrl_out_ready;

   wire               ingress_cdc_wr_full;
   wire               ingress_cdc_rd_empty;
   wire               ingress_cdc_rd_en;
   wire               ingress_cdc_wr_en;
   wire [7:0]         ingress_cdc_wr_data;
   wire [7:0]         ingress_cdc_rd_data;

   wire [7:0]         ingress_upscale_in_data;
   wire               ingress_upscale_in_valid;
   wire               ingress_upscale_in_ready;
   wire [WIDTH-1:0]   ingress_upscale_out_data;
   wire               ingress_upscale_out_valid;
   wire               ingress_upscale_out_ready;

   wire [WIDTH-1:0]   ingress_buffer_din;
   wire               ingress_buffer_wr_en;
   wire               ingress_buffer_full;
   wire               ingress_buffer_prog_full;
   wire               ingress_buffer_rd_en;
   wire [WIDTH-1:0]   ingress_buffer_dout;
   wire               ingress_buffer_empty;


   wire [7:0]         egress_ctrl_out_data;
   wire               egress_ctrl_out_enable;
   wire               egress_ctrl_out_done;
   wire [7:0]         egress_ctrl_in_data;
   wire               egress_ctrl_in_valid;
   wire               egress_ctrl_in_ready;


   wire [WIDTH-1:0]   egress_downscale_in_data;
   wire               egress_downscale_in_valid;
   wire               egress_downscale_in_ready;
   wire [7:0]         egress_downscale_out_data;
   wire               egress_downscale_out_valid;
   wire               egress_downscale_out_ready;


   wire               egress_cdc_wr_full;
   wire               egress_cdc_rd_empty;
   wire               egress_cdc_rd_en;
   wire               egress_cdc_wr_en;
   wire [7:0]         egress_cdc_rd_data;
   wire [7:0]         egress_cdc_wr_data;

   wire [7:0]         transmit_data;
   wire               transmit_done;
   wire               transmit_enable;

   wire               fifo_rst;

   reg [2:0]          fifo_en_io;
   reg [1:0]          fifo_rst_io ;
   reg [2:0]          fifo_en_logic;
   reg [1:0]          fifo_rst_logic;

   assign fifo_rst = fifo_rst_io[0] & fifo_rst_logic[0];

   // Generate delayed enable and rst signals for the different clocks
   always @(posedge clk_io) begin
      if (com_rst) begin
         fifo_en_io <= 3'b000;
         fifo_rst_io <= {1'b1, fifo_rst_io[1]};
      end else begin
         fifo_en_io <= {1'b1, fifo_en_io[2:1]};
         fifo_rst_io <= 2'b0;
      end
   end

   always @(posedge clk) begin
      if (com_rst) begin
         fifo_en_logic <= 3'b000;
         fifo_rst_logic <= {1'b1, fifo_rst_logic[1]};
      end else begin
         fifo_en_logic <= {1'b1, fifo_en_logic[2:1]};
         fifo_rst_logic <= 2'b0;
      end
   end


   // Generate error. Sticky when an error occured.
   wire          control_error;
   always @(posedge clk_io) begin
      if (com_rst) begin
         error <= 0;
      end else begin
         error <= error | receive_error | control_error;
      end
   end

   glip_uart_control
      #(.FIFO_CREDIT_WIDTH(12),
        .INPUT_FIFO_CREDIT(4090))
   u_control(
      .clk                       (clk_io),
      .rst                       (rst),

      .ingress_in_ready          (), // unused
      .ingress_in_data           (ingress_ctrl_in_data),
      .ingress_in_valid          (ingress_ctrl_in_valid),

      .ingress_out_data          (ingress_ctrl_out_data),
      .ingress_out_valid         (ingress_ctrl_out_valid),
      .ingress_out_ready         (ingress_ctrl_out_ready),

      .egress_in_ready           (egress_ctrl_in_ready),
      .egress_in_data            (egress_ctrl_in_data),
      .egress_in_valid           (egress_ctrl_in_valid),

      .egress_out_data           (egress_ctrl_out_data),
      .egress_out_enable         (egress_ctrl_out_enable),
      .egress_out_done           (egress_ctrl_out_done),

      .ctrl_logic_rst            (ctrl_logic_rst),
      .com_rst                   (com_rst),
      .error                     (control_error),

      .transfer_in               (transfer_in));

   assign transfer_in = ~ingress_cdc_rd_empty & ~ingress_cdc_wr_full;

   //------------------- ingress data path (host -> FPGA) ---------------------//

   glip_uart_receive
      #(.DIVISOR(FREQ_CLK_IO/BAUD))
   u_receive(
      .clk    (clk_io),
      .rst    (rst),

      .rx     (uart_rx),

      .enable (receive_enable),
      .data   (receive_data),
      .error  (receive_error));

   // connect receive -> control
   assign ingress_ctrl_in_valid = receive_enable;
   assign ingress_ctrl_in_data = receive_data;

   // connect control -> ingress_cdc
   assign ingress_cdc_wr_data = ingress_ctrl_out_data;
   assign ingress_cdc_wr_en = ingress_ctrl_out_valid & fifo_en_io[0];
   assign ingress_ctrl_out_ready = ~ingress_cdc_wr_full;

   fifo_dualclock_fwft
      #(.WIDTH(8),
      .DEPTH(16))
   u_ingress_cdc(
      // write side (clk_io)
      .wr_clk           (clk_io),
      .wr_rst           (fifo_rst),
      .full             (ingress_cdc_wr_full),
      .prog_full        (),
      .din              (ingress_cdc_wr_data),
      .wr_en            (ingress_cdc_wr_en),

      // read side (clk)
      .rd_clk           (clk),
      .rd_rst           (fifo_rst),
      .empty            (ingress_cdc_rd_empty),
      .prog_empty       (),
      .dout             (ingress_cdc_rd_data),
      .rd_en            (ingress_cdc_rd_en));

   // connect ingress_cdc -> ingress_upscale
   assign ingress_upscale_in_data = ingress_cdc_rd_data;
   assign ingress_upscale_in_valid = ~ingress_cdc_rd_empty;
   assign ingress_cdc_rd_en = ingress_upscale_in_ready;

   glip_upscale
      #(.IN_SIZE(8),
        .OUT_SIZE(WIDTH))
   u_ingress_upscale(
      .clk       (clk),
      .rst       (fifo_rst_logic[0]),
      .in_data   (ingress_upscale_in_data),
      .in_valid  (ingress_upscale_in_valid),
      .in_ready  (ingress_upscale_in_ready),
      .out_data  (ingress_upscale_out_data),
      .out_valid (ingress_upscale_out_valid),
      .out_ready (ingress_upscale_out_ready));


   // connect ingress_upscale -> ingress_buffer
   assign ingress_buffer_din = ingress_upscale_out_data;
   assign ingress_buffer_wr_en = ingress_upscale_out_valid & fifo_en_io[0];
   assign ingress_upscale_out_ready = ~ingress_buffer_full;

   fifo_singleclock_fwft
      #(.WIDTH(WIDTH),
        .DEPTH(32),
        .PROG_FULL(9'h006))
   u_ingress_buffer(
      .clk       (clk),
      .rst       (fifo_rst_logic[0]),

      .din       (ingress_buffer_din),
      .wr_en     (ingress_buffer_wr_en),
      .full      (ingress_buffer_full),
      .prog_full (ingress_buffer_almost_full),


      .rd_en     (ingress_buffer_rd_en),
      .dout      (ingress_buffer_dout),
      .empty     (ingress_buffer_empty));

   // connect ingress_buffer -> output interface (to be used in attached logic)
   assign fifo_in_data = ingress_buffer_dout;
   assign fifo_in_valid = ~ingress_buffer_empty;
   assign ingress_buffer_rd_en = fifo_in_ready;


   //-------------------------- egress data path --------------------------//


   // connect input interface -> u_downscale
   assign egress_downscale_in_data = fifo_out_data;
   assign egress_downscale_in_valid = fifo_out_valid;
   assign fifo_out_ready = egress_downscale_in_ready;

   glip_downscale
      #(.IN_SIZE(WIDTH),
        .OUT_SIZE(8))
   u_egress_downscale(
      .clk       (clk),
      .rst       (com_rst),
      .in_data   (egress_downscale_in_data),
      .in_valid  (egress_downscale_in_valid),
      .in_ready  (egress_downscale_in_ready),
      .out_data  (egress_downscale_out_data),
      .out_valid (egress_downscale_out_valid),
      .out_ready (egress_downscale_out_ready));


   // connect u_egress_downscale -> u_egress_cdc
   assign egress_cdc_wr_data = egress_downscale_out_data;
   assign egress_cdc_wr_en = egress_downscale_out_valid & fifo_en_logic[0];
   assign egress_downscale_out_ready = ~egress_cdc_wr_full;

   fifo_dualclock_fwft
      #(.WIDTH(8),
      .DEPTH(BUFFER_OUT_DEPTH))
   u_egress_cdc(
      .wr_clk           (clk),
      .wr_rst           (fifo_rst),
      .full             (egress_cdc_wr_full),
      .prog_full        (),
      .din              (egress_cdc_wr_data),
      .wr_en            (egress_cdc_wr_en),

      .rd_clk           (clk_io),
      .rd_rst           (fifo_rst),
      .empty            (egress_cdc_rd_empty),
      .prog_empty       (),
      .dout             (egress_cdc_rd_data),
      .rd_en            (egress_cdc_rd_en));

   // connect u_egress_cdc -> control
   assign egress_ctrl_in_data = egress_cdc_rd_data;
   assign egress_ctrl_in_valid = ~egress_cdc_rd_empty;
   assign egress_cdc_rd_en = egress_ctrl_in_ready;

   // connect control -> u_transmit
   assign transmit_data = egress_ctrl_out_data;
   assign transmit_enable = egress_ctrl_out_enable & ~uart_cts_n;
   assign egress_ctrl_out_done = transmit_done;


   glip_uart_transmit
      #(.DIVISOR(FREQ_CLK_IO/BAUD))
   u_transmit(
      .clk    (clk_io),
      .rst    (com_rst),

      .tx     (uart_tx),

      .done   (transmit_done),
      .data   (transmit_data),
      .enable (transmit_enable));

   // We are always ready to send
   assign uart_rts_n = 1'b0;

endmodule // glip_uart_toplevel
