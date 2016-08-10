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
 * UART Transmitter
 * 
 * Parameters:
 *  - DIVISOR: Number of clock cycles per UART bit
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module glip_uart_transmit
  #(
    parameter DIVISOR = 1'bx
    )
   (
    // Clock & Reset
    input       clk,
    input       rst,

    // Output register
    output reg  tx,

    // Input data and acknowledge
    input [7:0] data,
    input       enable,
    output reg  done
    );

   // State machine
   localparam STATE_IDLE  = 0;
   localparam STATE_START = 1;
   localparam STATE_BITS  = 2;
   localparam STATE_STOP  = 3;

   reg [1:0]    state;
   reg [1:0]    nxt_state;

   // Count down the clock edges per bit
   reg [31:0]   divcounter;
   reg [31:0]   nxt_divcounter;

   // Count up the bit position
   reg [2:0]    bitpos;
   reg [2:0]    nxt_bitpos;

   // Always emit signal at end of bit
   wire         nextedge;
   assign nextedge = (divcounter == 0);

   // Sequential part of state machine
   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
      end else begin
         state <= nxt_state;
      end
      divcounter <= nxt_divcounter;
      bitpos <= nxt_bitpos;
   end

   // Combinational part of state machine
   always @(*) begin
      // Defaults for registers
      nxt_state = state;
      nxt_bitpos = bitpos;

      if (divcounter == 0) begin
         // Auto-reset divisor counter
         nxt_divcounter = DIVISOR - 1;
      end else begin
         // else count down
         nxt_divcounter = divcounter - 1;
      end

      // Default output
      done = 0;
      
      case (state)
        STATE_IDLE: begin
           tx = 1;
           if (enable) begin
              // Start triggered
              nxt_state = STATE_START;
              nxt_divcounter = DIVISOR - 1;
           end
        end
        STATE_START: begin
           // Start bit
           tx = 0;
           if (nextedge) begin
              nxt_state = STATE_BITS;
              nxt_bitpos = 0;
           end
        end
        STATE_BITS: begin
           // Emit bit
           tx = data[bitpos];
           if (nextedge) begin
              // Count up bits
              nxt_bitpos = bitpos + 1;
              if (bitpos == 7) begin
                 nxt_state = STATE_STOP;
              end
           end
        end
        STATE_STOP: begin
           // Stop bit
           tx = 1;
           if (nextedge) begin
              nxt_state = STATE_IDLE;
              // Acknowledge byte
              done = 1;
           end
        end
      endcase // case (state)
   end   
endmodule // glip_uart_transmit
