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
 * Ingress path. Filters control messages from the stream.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module glip_uart_control_ingress
  (
   input             clk,
   input             rst,

   // Both FIFO interfaces
   input [7:0]       in_data,
   input             in_valid,
   output            in_ready,
   output [7:0]      out_data,
   output reg        out_valid,
   input             out_ready,

   // Count transfers for credits
   output            transfer,

   // Credit control message detected
   output reg        credit_en,
   output reg [13:0] credit_val,

   // Logic reset control message detected
   output reg        logic_rst_en,
   output reg        logic_rst_val,

   // Communication reset control message detected
   output reg        com_rst_en,
   output reg        com_rst_val,

   // Error case
   output            error
   );

   // Just bypass data, the out_enable filters
   assign out_data = in_data;

   // User transfers count only
   assign transfer = out_valid & out_ready;

   // We are always ready. Due to the credit-based flow control there
   // shall never be backpressure here.
   assign in_ready = 1'b1;

   // An FSM error or backpressure are error cases
   reg               fsm_error;
   assign error = ~out_ready | fsm_error;
   
   localparam STATE_PASSTHROUGH = 0;
   localparam STATE_MATCH = 1;
   localparam STATE_CREDIT = 2;

   reg [1:0]         state;
   reg [1:0]         nxt_state;
   reg [5:0]         credit_first;
   reg [5:0]         nxt_credit_first;

   // Sequential part of state machine
   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_PASSTHROUGH;
      end else begin
         state <= nxt_state;
      end
      // This is never used in the first cycle after reset. Can use
      // any reset value
      credit_first <= nxt_credit_first;
   end

   // Data matches the marker
   wire in_data_match = (in_data == 8'hfe);
   
   always @(*) begin
      // Register defaults
      nxt_state = state;
      nxt_credit_first = credit_first;

      // Default outputs
      out_valid = 1'b0;
      fsm_error = 1'b0;
      logic_rst_en = 1'b0;
      com_rst_en = 1'b0;
      logic_rst_val = 1'b0;
      com_rst_val = 1'b0;
      credit_en = 1'b0;
      credit_val = 14'hx;
      
      case(state)
        STATE_PASSTHROUGH: begin
           if (in_valid) begin
              // Input data received
              if (in_data_match) begin
                 // We match the filter condition, filter
                 nxt_state = STATE_MATCH;
              end else begin
                 // Otherwise just forward data
                 out_valid = 1'b1;
              end
           end
        end
        STATE_MATCH: begin
           if (in_valid) begin
              // Next data item
              if (~in_data[0]) begin
                 // If bit 0 is not set this is the repeated data item
                 // Verify that this is true and continue data stream
                 fsm_error = !in_data_match;
                 nxt_state = STATE_PASSTHROUGH;
                 out_valid = 1'b1;
              end else if (~in_data[7]) begin
                 // The first bit is set for credit messages Store the
                 // data in the register to assemble the full length
                 // after we received the next word
                 nxt_credit_first = in_data[6:1];
                 nxt_state = STATE_CREDIT;
              end else begin
                 // The first bit is not set for other control
                 // messages. Currently we only have the rst control
                 // message. Bit 1 is the new state of the rst
                 // register
                 logic_rst_en = ~in_data[2];
                 com_rst_en = in_data[2];
                 logic_rst_val = in_data[1];
                 com_rst_val = in_data[1];

                 // Go back to data stream bypassing
                 nxt_state = STATE_PASSTHROUGH;
              end
           end
        end // case: STATE_MATCH
        STATE_CREDIT: begin
           if (in_valid) begin
              // This is the extra word for the credit. Emit this and
              // the previously saved higher bits
              credit_val = {credit_first, in_data};
              credit_en = 1'b1;
              // Go back to data stream bypassing
              nxt_state = STATE_PASSTHROUGH;
           end
        end
      endcase // case (state)
   end
endmodule // glip_uart_control_egress
