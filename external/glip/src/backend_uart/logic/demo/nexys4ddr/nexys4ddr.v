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
 * Nexys 4 DDR board demo
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module nexys4ddr
  (
   // 100 MHz clock & CPU Reset (active low) button
   input        clk,
   input        rstn,

   // UART Interface
   // Signal names are DIFFERENT THAN IN THE DIGILENT DEFAULT CONSTRAINT FILE!
   // The signals here are named from a FPGA point-of-view.
   input        uart_rx,
   output       uart_tx,
   input        uart_cts, // active low
   output       uart_rts, // active low

   // Slide switches SW0-SW3
   input [2:0]  switch,

   // 7 Segment display
   output       CA,
   output       CB,
   output       CC,
   output       CD,
   output       CE,
   output       CF,
   output       CG,
   output       DP,
   output [7:0] AN,

   output reg   redled
   );

   parameter WIDTH = 16;

   localparam FREQ = 100000000; // frequency of clk [Hz]
   localparam BAUD = 12000000;

   wire         rst;
   assign rst = ~rstn;

   wire [WIDTH-1:0] in_data;
   wire             in_valid;
   reg              in_ready;
   reg [WIDTH-1:0]  out_data;
   reg              out_valid;
   wire             out_ready;

   always @(*) begin
      casez(switch[1:0])
        // Loopback mode
        2'b00: begin
           out_data = in_data;
           out_valid = in_valid;
           in_ready = out_ready;
        end
        2'b01: begin
           in_ready = 1'b0;
           out_data = 16'habcd;
           out_valid = 1'b1;
        end
        2'b1?: begin
           out_valid = 1'b0;
           out_data = 16'hx;
           in_ready = switch[2];
        end
      endcase // case (switches[1:0])
   end

   wire error;
   reg  error_reg;

   always @(posedge clk) begin
      if (rst) begin
         error_reg <= 0;
         redled <= 0;
      end else begin
         error_reg <= error_reg | error;
         if (error_reg) begin
            redled <= ~redled;
         end
      end
   end

   wire ctrl_logic_rst;

   glip_uart_toplevel
     #(.FREQ_CLK_IO(FREQ),
       .BAUD(BAUD),
       .WIDTH(WIDTH))
   u_uart(.clk_io         (clk),
          .clk            (clk),
          .rst            (rst),
          .uart_rx        (uart_rx),
          .uart_tx        (uart_tx),
          .uart_cts_n     (uart_cts),
          .uart_rts_n     (uart_rts),
          .error          (error),
          .ctrl_logic_rst (ctrl_logic_rst),
          .com_rst        (com_rst),
          .fifo_in_data   (in_data),
          .fifo_in_valid  (in_valid),
          .fifo_in_ready  (in_ready),
          .fifo_out_data  (out_data),
          .fifo_out_valid (out_valid),
          .fifo_out_ready (out_ready));

   wire [8*7-1:0] digits;
   wire           overflow;

   glip_measure_sevensegment
     #(.FREQ(FREQ), .DIGITS(8), .OFFSET(0), .STEP(WIDTH/8))
   u_measure(.clk      (clk),
             .rst      (ctrl_logic_rst),
             .trigger  ((in_valid & in_ready) | (out_valid & out_ready)),
             .digits   (digits),
             .overflow (overflow));

   nexys4ddr_display
     #(.FREQ(FREQ))
   u_display(.clk       (clk),
             .rst       (ctrl_logic_rst),
             .digits    (digits),
             .decpoints (8'b00001000),
             .CA        (CA),
             .CB        (CB),
             .CC        (CC),
             .CD        (CD),
             .CE        (CE),
             .CF        (CF),
             .CG        (CG),
             .DP        (DP),
             .AN        (AN));

endmodule // nexys4ddr
