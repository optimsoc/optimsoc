/* Copyright (c) 2016 by the author(s)
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
 * Egress path. Multiplexes the credit messages into the data stream.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module glip_uart_control_egress
  (
   input            clk,
   input            rst,

   // FIFO interface input
   input [7:0]      in_data,
   input            in_valid,
   output reg       in_ready,

   // Interface to transmit module
   output reg [7:0] out_data,
   output reg       out_enable,
   input            out_done,

   // Sufficient credit to send data
   input            can_send,

   // A transfer is completed
   output           transfer,

   // Request to send a credit
   input [14:0]     credit,
   input            credit_en,
   output reg       credit_ack,

   // Error case
   output reg       error
   );

   // Only user transfers are counted
   assign transfer = in_valid & in_ready;

   localparam STATE_IDLE = 0;
   localparam STATE_PASSTHROUGH = 1;
   localparam STATE_PASSTHROUGH_REPEAT = 2;
   localparam STATE_SENDCREDIT1 = 3;
   localparam STATE_SENDCREDIT2 = 4;
   localparam STATE_SENDCREDIT3 = 5;
   
   reg [2:0]        state;
   reg [2:0]        nxt_state;

   // Sequential part of state machine
   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
      end else begin
         state <= nxt_state;
      end
   end

   // Combinational part of state machine
   always @(*) begin
      // Registers
      nxt_state = state;

      // Default values
      in_ready = 1'b0;
      out_data = 8'hx;
      out_enable = 1'b0;

      credit_ack = 1'b0;

      error = 0;

      case (state)
        STATE_IDLE: begin
           if (credit_en) begin
              // Start credit request
              nxt_state = STATE_SENDCREDIT1;
           end else if (can_send & in_valid) begin
              // Tranfer a user word
              nxt_state = STATE_PASSTHROUGH;
           end
        end
        STATE_PASSTHROUGH: begin
           // Pass-through data and start transfer
           out_data = in_data;
           out_enable = can_send;
           if (out_done) begin
              // In this cycle the transfer completed
              // Acknowledge
              in_ready = 1'b1;
              if (in_data == 8'hfe) begin
                 // 0xfe needs to be replicated
                 nxt_state = STATE_PASSTHROUGH_REPEAT;
              end else begin
                 // Accept new word or send credit
                 nxt_state = STATE_IDLE;
              end
           end
        end
        STATE_PASSTHROUGH_REPEAT: begin
           // Plainly repeat the item
           out_data = 8'hfe;
           out_enable = can_send;
           if (out_done) begin
              nxt_state = STATE_IDLE;
           end
        end
        STATE_SENDCREDIT1: begin
           // Send control indicator first
           out_data = 8'hfe;
           out_enable = 1;
           if (out_done) begin
              nxt_state = STATE_SENDCREDIT2;
           end
        end
        STATE_SENDCREDIT2: begin
           // bit 0 differs from 0xfe, upper 7 bits are payload
           out_data = { credit[14:8], 1'b1 };
           out_enable = 1;
           if (out_done) begin
              nxt_state = STATE_SENDCREDIT3;
           end
        end
        STATE_SENDCREDIT3: begin
           // last word with the remain of the credit
           out_data = { credit[7:0] };
           out_enable = 1;
           if (out_done) begin
              nxt_state = STATE_IDLE;
              credit_ack = 1'b1;
           end
        end
      endcase
   end
endmodule // glip_uart_control_egress
