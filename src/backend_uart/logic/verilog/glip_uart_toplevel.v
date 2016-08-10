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
 * FREQ/BAUD) is within 2%.
 * 
 * Parameters:
 *  - FREQ: The frequency of the design, to match the second
 *  - BAUD: Interface baud rate
 *  - XILINX_TARGET_DEVICE: Xilinx device, allowed: "7SERIES"
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module glip_uart_toplevel
  #(parameter FREQ = 32'hx,
    parameter BAUD = 115200,
    parameter WIDTH = 8,
    parameter XILINX_TARGET_DEVICE = "7SERIES")
   (
    // Clock & Reset
    input              clk_io,
    input              clk_logic,
    input              rst,

    // GLIP FIFO Interface
    input [WIDTH-1:0]  fifo_out_data,
    input              fifo_out_valid,
    output             fifo_out_ready,
    output [WIDTH-1:0] fifo_in_data,
    output             fifo_in_valid,
    input              fifo_in_ready,

    // GLIP Control Interface
    output             logic_rst,
    output             com_rst,
    
    // UART Interface
    input              uart_rx,
    output             uart_tx,
    input              uart_cts,
    output             uart_rts,
    
    // Error signal if failure on the line
    output reg         error
    );

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
   assign ingress_out_ready = ~in_buffer_almost_full;
   assign ingress_buffer_valid = ~in_buffer_empty;
   assign ingress_buffer_ready = ~in_fifo_full;
   assign fifo_in_valid_scale = ~in_fifo_empty;
   assign egress_in_valid = ~out_fifo_empty;
   assign fifo_out_ready_scale = ~out_fifo_full;

   assign uart_rts = 0;

   // Generate error. Sticky when an error occured.
   wire          rcv_error;
   wire          control_error;
   always @(posedge clk_io) begin
      if (rst | com_rst) begin
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
         u_upscale(.clk       (clk_io),
                   .rst       (com_rst),
                   .in_data   (fifo_in_data_scale),
                   .in_valid  (fifo_in_valid_scale),
                   .in_ready  (fifo_in_ready_scale),
                   .out_data  (fifo_in_data),
                   .out_valid (fifo_in_valid),
                   .out_ready (fifo_in_ready));

         glip_downscale
           #(.OUT_SIZE(8))
         u_downscale(.clk       (clk_io),
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
       .INPUT_FIFO_CREDIT(4090),
       .FREQ(FREQ))
   u_control(/*AUTOINST*/
             // Outputs
             .ingress_in_ready          (ingress_in_ready),
             .ingress_out_data          (ingress_out_data[7:0]),
             .ingress_out_valid         (ingress_out_valid),
             .egress_in_ready           (egress_in_ready),
             .egress_out_data           (egress_out_data[7:0]),
             .egress_out_enable         (egress_out_enable),
             .logic_rst                 (logic_rst),
             .com_rst                   (com_rst),
             .error                     (control_error),         // Templated
             // Inputs
             .clk                       (clk_io),                // Templated
             .rst                       (rst),
             .ingress_in_data           (ingress_in_data[7:0]),
             .ingress_in_valid          (ingress_in_valid),
             .ingress_out_ready         (ingress_out_ready),
             .egress_in_data            (egress_in_data[7:0]),
             .egress_in_valid           (egress_in_valid),
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
     #(.DIVISOR(FREQ/BAUD))
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
    .enable (egress_out_enable & ~uart_cts),
    .data   (egress_out_data[]),
    ); */
   glip_uart_transmit
     #(.DIVISOR(FREQ/BAUD))
   u_transmit(/*AUTOINST*/
              // Outputs
              .tx                       (uart_tx),               // Templated
              .done                     (egress_out_done),       // Templated
              // Inputs
              .clk                      (clk_io),                // Templated
              .rst                      (com_rst),               // Templated
              .data                     (egress_out_data[7:0]),  // Templated
              .enable                   (egress_out_enable & ~uart_cts)); // Templated

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
      .RDEN        (ingress_buffer_ready),
      .RST         (com_rst),
      .WRCLK       (clk_io),
      .WREN        (ingress_out_valid)
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
      .RDCLK       (clk_logic),
      .RDEN        (fifo_in_ready_scale),
      .RST         (com_rst),
      .WRCLK       (clk_io),
      .WREN        (ingress_buffer_valid)
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
      .RDEN        (egress_in_ready),
      .RST         (com_rst),
      .WRCLK       (clk_logic),
      .WREN        (fifo_out_valid_scale)
      );
   
endmodule // glip_uart_toplevel
