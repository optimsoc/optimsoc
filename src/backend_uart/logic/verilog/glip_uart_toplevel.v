/* Copyright (c) 2015-2016 by the author(s)
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
 * Parameters:
 *  - FREQ_CLK_IO: The frequency of clk_io
 *  - BAUD: Interface baud rate
 *  - XILINX_TARGET_DEVICE: Xilinx device, allowed: "7SERIES"
 *
 *
 * Limitations:
 * - Only Xilinx 7 series devices are supported due to the use of
 *   Xilinx-specific dual-clock FIFO macros.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module glip_uart_toplevel
  #(parameter FREQ_CLK_IO = 32'hx,
    parameter BAUD = 115200,
    parameter WIDTH = 8,
    parameter BUFFER_OUT_DEPTH = 4*1024,
    parameter XILINX_TARGET_DEVICE = "7SERIES")
   (
    // Clock & Reset
    input              clk,     // Logic clock (GLIP default)
    input              clk_io,  // I/O clock
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

   wire [7:0]     fifo_out_data_scale;
   wire           fifo_out_valid_scale;
   wire           fifo_out_ready_scale;
   wire [7:0]     fifo_in_data_scale;
   wire           fifo_in_valid_scale;
   wire           fifo_in_ready_scale;

   wire [7:0]    ingress_in_data;
   wire          ingress_in_valid;
   wire          ingress_in_ready;
   wire [7:0]    ingress_out_data;
   wire          ingress_out_valid;
   wire          ingress_out_ready;
   wire [7:0]    ingress_buffer_data;
   wire          ingress_buffer_valid;
   wire          ingress_buffer_ready;
   wire [7:0]    egress_in_data;
   wire          egress_in_valid;
   wire          egress_in_ready;
   wire [7:0]    egress_in_buffered_data;
   wire          egress_in_buffered_valid;
   wire          egress_in_buffered_ready;
   wire [7:0]    egress_out_data;
   wire          egress_out_enable;
   wire          egress_out_done;

   wire          transfer_in;
   assign transfer_in = ingress_buffer_valid & ingress_buffer_ready;

   // Map FIFO signals to flow control
   wire          in_fifo_full;
   wire          in_fifo_empty;
   wire          in_buffer_almost_full;
   wire          in_buffer_empty;
   wire          out_fifo_full;
   wire          out_fifo_empty;
   wire          out_buffer_full;
   wire          out_buffer_empty;
   assign ingress_out_ready = ~in_buffer_almost_full;
   assign ingress_buffer_valid = ~in_buffer_empty;
   assign ingress_buffer_ready = ~in_fifo_full;
   assign fifo_in_valid_scale = ~in_fifo_empty;
   assign egress_in_valid = ~out_fifo_empty;
   assign fifo_out_ready_scale = ~out_fifo_full;
   assign egress_in_buffered_valid = ~out_buffer_empty;
   assign egress_in_ready = ~out_buffer_full;

   // We are always ready to send
   assign uart_rts_n = 1'b0;

   // FIFO enable and reset control
   // "WREN and RDEN must be held Low before and during the Reset cycle. In
   //  addition, WREN and RDEN should be held Low for two WRCLK and RDCLK
   //  cycles, respectively, after the Reset is deasserted to guarantee timing."
   //  [UG473 (v1.11), p 49,  Xilinx]

   wire          in_fifo_rden;
   wire          in_fifo_wren;
   wire          in_buffer_rden;
   wire          in_buffer_wren;
   wire          out_fifo_rden;
   wire          out_fifo_wren;
   wire          fifo_rst;

   reg [2:0]     fifo_en_io;
   reg [1:0]     fifo_rst_io ;
   reg [2:0]     fifo_en_logic;
   reg [1:0]     fifo_rst_logic;

   assign fifo_rst = fifo_rst_io[0] & fifo_rst_logic[0];
   assign in_fifo_rden = fifo_in_ready_scale & fifo_en_logic[0];
   assign in_fifo_wren = ingress_buffer_valid & fifo_en_io[0];
   assign in_buffer_rden = ingress_buffer_ready & fifo_en_io[0];
   assign in_buffer_wren =  ingress_out_valid & fifo_en_io[0];
   assign out_fifo_rden = egress_in_ready & fifo_en_io[0];
   assign out_fifo_wren = fifo_out_valid_scale & fifo_en_logic[0];

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
   wire          rcv_error;
   wire          control_error;
   always @(posedge clk_io) begin
      if (com_rst) begin
         error <= 0;
      end else begin
         error <= error | rcv_error | control_error;
      end
   end

   generate
      if (WIDTH == 8) begin
         assign fifo_out_data_scale = fifo_out_data;
         assign fifo_out_valid_scale = fifo_out_valid;
         assign fifo_out_ready = fifo_out_ready_scale;
         assign fifo_in_data = fifo_in_data_scale;
         assign fifo_in_valid = fifo_in_valid_scale;
         assign fifo_in_ready_scale = fifo_in_ready;
      end else if (WIDTH == 16) begin
         glip_upscale
           #(.IN_SIZE(8))
         u_upscale(.clk       (clk),
                   .rst       (com_rst),
                   .in_data   (fifo_in_data_scale),
                   .in_valid  (fifo_in_valid_scale),
                   .in_ready  (fifo_in_ready_scale),
                   .out_data  (fifo_in_data),
                   .out_valid (fifo_in_valid),
                   .out_ready (fifo_in_ready));

         glip_downscale
           #(.OUT_SIZE(8))
         u_downscale(.clk       (clk),
                     .rst       (com_rst),
                     .in_data   (fifo_out_data),
                     .in_valid  (fifo_out_valid),
                     .in_ready  (fifo_out_ready),
                     .out_data  (fifo_out_data_scale),
                     .out_valid (fifo_out_valid_scale),
                     .out_ready (fifo_out_ready_scale));

      end
   endgenerate

   /* glip_uart_control AUTO_TEMPLATE(
    .clk   (clk_io),
    .error (control_error),
    ); */
   glip_uart_control
     #(.FIFO_CREDIT_WIDTH(12),
       .INPUT_FIFO_CREDIT(4090))
   u_control(/*AUTOINST*/
             // Outputs
             .ingress_in_ready          (ingress_in_ready),
             .ingress_out_data          (ingress_out_data[7:0]),
             .ingress_out_valid         (ingress_out_valid),
             .egress_in_ready           (egress_in_buffered_ready),
             .egress_out_data           (egress_out_data[7:0]),
             .egress_out_enable         (egress_out_enable),
             .ctrl_logic_rst            (ctrl_logic_rst),
             .com_rst                   (com_rst),
             .error                     (control_error),         // Templated
             // Inputs
             .clk                       (clk_io),                // Templated
             .rst                       (rst),
             .ingress_in_data           (ingress_in_data[7:0]),
             .ingress_in_valid          (ingress_in_valid),
             .ingress_out_ready         (ingress_out_ready),
             .egress_in_data            (egress_in_buffered_data[7:0]),
             .egress_in_valid           (egress_in_buffered_valid),
             .egress_out_done           (egress_out_done),
             .transfer_in               (transfer_in));

   /* glip_uart_receive AUTO_TEMPLATE(
    .clk (clk_io),
    .rx  (uart_rx),
    .enable (ingress_in_valid),
    .data   (ingress_in_data),
    .error  (rcv_error),
    ); */
   glip_uart_receive
     #(.DIVISOR(FREQ_CLK_IO/BAUD))
   u_receive(/*AUTOINST*/
             // Outputs
             .enable                    (ingress_in_valid),      // Templated
             .data                      (ingress_in_data),       // Templated
             .error                     (rcv_error),             // Templated
             // Inputs
             .clk                       (clk_io),                // Templated
             .rst                       (rst),
             .rx                        (uart_rx));              // Templated

   /* glip_uart_transmit AUTO_TEMPLATE(
    .rst    (com_rst),
    .clk    (clk_io),
    .tx     (uart_tx),
    .done   (egress_out_done),
    .enable (egress_out_enable & ~uart_cts_n),
    .data   (egress_out_data[]),
    ); */
   glip_uart_transmit
     #(.DIVISOR(FREQ_CLK_IO/BAUD))
   u_transmit(/*AUTOINST*/
              // Outputs
              .tx                       (uart_tx),               // Templated
              .done                     (egress_out_done),       // Templated
              // Inputs
              .clk                      (clk_io),                // Templated
              .rst                      (com_rst),               // Templated
              .data                     (egress_out_data[7:0]),  // Templated
              .enable                   (egress_out_enable & ~uart_cts_n)); // Templated

   // Buffer uart -> logic
   FIFO_DUALCLOCK_MACRO
     #(.ALMOST_FULL_OFFSET(9'h006), // Sets almost full threshold
       .DATA_WIDTH(8), // Valid values are 1-72 (37-72 only valid when FIFO_SIZE="36Kb")
       .DEVICE(XILINX_TARGET_DEVICE), // Target device: "VIRTEX5", "VIRTEX6", "7SERIES"
       .FIFO_SIZE("36Kb"), // Target BRAM: "18Kb" or "36Kb"
       .FIRST_WORD_FALL_THROUGH("TRUE") // Sets the FIfor FWFT to "TRUE" or "FALSE"
       )
   in_buffer
     (.ALMOSTEMPTY (),
      .ALMOSTFULL  (in_buffer_almost_full),
      .DO          (ingress_buffer_data[7:0]),
      .EMPTY       (in_buffer_empty),
      .FULL        (),
      .RDCOUNT     (),
      .RDERR       (),
      .WRCOUNT     (),
      .WRERR       (),
      .DI          (ingress_out_data[7:0]),
      .RDCLK       (clk_io),
      .RDEN        (in_buffer_rden),
      .RST         (fifo_rst_io[0]),
      .WRCLK       (clk_io),
      .WREN        (in_buffer_wren)
      );


   // Clock domain crossing uart -> logic
   FIFO_DUALCLOCK_MACRO
     #(.ALMOST_FULL_OFFSET(9'h006), // Sets almost full threshold
       .ALMOST_EMPTY_OFFSET(9'h006),
       .DATA_WIDTH(8), // Valid values are 1-72 (37-72 only valid when FIFO_SIZE="36Kb")
       .DEVICE(XILINX_TARGET_DEVICE), // Target device: "VIRTEX5", "VIRTEX6", "7SERIES"
       .FIFO_SIZE("18Kb"), // Target BRAM: "18Kb" or "36Kb"
       .FIRST_WORD_FALL_THROUGH("TRUE") // Sets the FIfor FWFT to "TRUE" or "FALSE"
       )
   in_fifo
     (.ALMOSTEMPTY (),
      .ALMOSTFULL  (),
      .DO          (fifo_in_data_scale[7:0]),
      .EMPTY       (in_fifo_empty),
      .FULL        (in_fifo_full),
      .RDCOUNT     (),
      .RDERR       (),
      .WRCOUNT     (),
      .WRERR       (),
      .DI          (ingress_buffer_data[7:0]),
      .RDCLK       (clk),
      .RDEN        (in_fifo_rden),
      .RST         (fifo_rst),
      .WRCLK       (clk_io),
      .WREN        (in_fifo_wren)
      );

   // Clock domain crossing logic -> uart
   FIFO_DUALCLOCK_MACRO
     #(.ALMOST_EMPTY_OFFSET(9'h006), // Sets the almost empty threshold
       .ALMOST_FULL_OFFSET(9'h006),
       .DATA_WIDTH(8), // Valid values are 1-72 (37-72 only valid when FIFO_SIZE="36Kb")
       .DEVICE(XILINX_TARGET_DEVICE), // Target device: "VIRTEX5", "VIRTEX6", "7SERIES"
       .FIFO_SIZE("18Kb"), // Target BRAM: "18Kb" or "36Kb"
       .FIRST_WORD_FALL_THROUGH("TRUE") // Sets the FIfor FWFT to "TRUE" or "FALSE"
       )
   out_fifo
     (.ALMOSTEMPTY (),
      .ALMOSTFULL  (),
      .DO          (egress_in_data),
      .EMPTY       (out_fifo_empty),
      .FULL        (out_fifo_full),
      .RDCOUNT     (),
      .RDERR       (),
      .WRCOUNT     (),
      .WRERR       (),
      .DI          (fifo_out_data_scale[7:0]),
      .RDCLK       (clk_io),
      .RDEN        (out_fifo_rden),
      .RST         (fifo_rst),
      .WRCLK       (clk),
      .WREN        (out_fifo_wren)
      );

   oh_fifo_sync
     #(.DW(8),.DEPTH(BUFFER_OUT_DEPTH))
   u_out_buffer
     (.clk       (clk_io),
      .nreset    (!rst),
      .din       (egress_in_data),
      .wr_en     (egress_in_valid),
      .rd_en     (egress_in_buffered_ready),
      .dout      (egress_in_buffered_data),
      .full      (out_buffer_full),
      .prog_full (),
      .empty     (out_buffer_empty),
      .rd_count  ());

endmodule // glip_uart_toplevel
