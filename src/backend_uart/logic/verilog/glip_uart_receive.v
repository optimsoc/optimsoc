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
 * UART Receiver
 * 
 * Parameters:
 *  - DIVISOR: Number of clock cycles per UART bit
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module glip_uart_receive
  #(
    parameter DIVISOR = 1'bx
    )
   (
    // Clock & Reset
    input            clk,
    input            rst,

    // UART input
    input            rx,

    // Byte output or error
    output reg       enable,
    output reg [7:0] data,
    output reg       error
    );

   // Data is a register of the detected bits
   reg [7:0]         nxt_data;

   // State machine
   reg [1:0]         state;
   reg [1:0]         nxt_state;
   
   localparam STATE_IDLE  = 0;
   localparam STATE_START = 1;
   localparam STATE_BITS  = 2;
   localparam STATE_STOP  = 3;

   // Count down the clock edges per bit
   reg [31:0]        divcounter;
   reg [31:0]        nxt_divcounter;

   // Count up the bit position
   reg [2:0]         bitpos;
   reg [2:0]         nxt_bitpos;

   // Register the last two rx bits. This is needed to detect the
   // start edge, and we previously faced the issue of a race when
   // comparing rx and reg_rx. Hence we sample and compare sampled
   // bits
   reg [1:0]         rx_reg;

   // Detect edges in the RX signal
   wire              rxedge;
   wire              rxnegedge;
   assign rxedge = ^rx_reg;
   assign rxnegedge = rxedge & !rx_reg[0];

   // Helpers for mid and end of a bit
   wire              bitend;
   wire              bitmid;
   assign bitend = (divcounter == 0);   
   assign bitmid = (divcounter == (DIVISOR >> 1)); 

   // Sequential part of state machine
   always @(posedge clk) begin
      if (rst) begin
         divcounter <= 0;
         state <= STATE_IDLE;
         rx_reg <= { rx, rx };
      end else begin
         state <= nxt_state;
         rx_reg <= {rx_reg[0], rx}; // Shift in RX sample
         divcounter <= nxt_divcounter;
      end
      bitpos <= nxt_bitpos;
      data <= nxt_data;
   end // always @ (posedge clk)

   // Combinational part of state machine
   always @(*) begin
      // Defaults for registers
      nxt_state = state;
      nxt_bitpos = bitpos;
      nxt_data = data;
      
      if (bitend) begin
         // Auto-reset divisor counter
         nxt_divcounter = DIVISOR - 1;
      end else begin
         // else count down
         nxt_divcounter = divcounter - 1;
      end

      // Default output
      enable = 0;
      error = 0;
      
      case (state)
        STATE_IDLE: begin
           if (rxnegedge) begin
              // Detected negative edge, this is a start bit
              nxt_state = STATE_START;
              nxt_divcounter = DIVISOR - 1;
           end
        end

        STATE_START: begin
           if (bitend) begin
              // End of start bit detected
              nxt_state = STATE_BITS;
              nxt_bitpos = 0;
           end
        end

        STATE_BITS: begin
           if (bitmid) begin
              // Sample at mid of bit
              nxt_data[bitpos] = rx_reg[0];
              nxt_bitpos = bitpos + 1;
              if (bitpos == 7) begin
                 // End of frame
                 nxt_state = STATE_STOP;
              end
           end
        end

        STATE_STOP: begin
           if (bitmid) begin
              // Frame was valid if stop bit is 1
              enable = rx_reg[0];
              error = ~rx_reg[0];
              nxt_state = STATE_IDLE;
           end
        end
      endcase // case (state)
   end // always @ (*)

endmodule // glip_uart_receive
