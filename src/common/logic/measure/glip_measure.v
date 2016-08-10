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
 * Measure how many triggers occur in one second
 *
 * Parameters:
 *  - FREQ: The frequency of the design, to match the second
 *  - DIGITS: Number of digits to emit
 *  - OFFSET: The power of ten offset (digits) to skip, useful if you
 *            have limited display width
 *  - STEP: Step count on each trigger, usually power of 2
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module glip_measure
  #(parameter FREQ = 32'hx,
    parameter DIGITS = 8,
    parameter OFFSET = 0,
    parameter STEP = 4'd1)
   (
    input                     clk,
    input                     rst,

    // High when count this cycle
    input                     trigger,

    // Emit digits (4 bit each)
    output reg [DIGITS*4-1:0] digits,

    // Emit overflow
    output reg                overflow
    );

   // Total number of digits
   localparam TOTAL = DIGITS + OFFSET;

   wire [TOTAL-1:0]           carry; // Carry between digits
   wire                       sample; // Sample trigger (every second)
   reg [31:0]                 count; // Counter for sampling
   wire [DIGITS*4-1:0]        sample_digits; // Counter until sampled

   // Sample logic
   // Every second we update the digits to give Byte/s
   always @(posedge clk) begin
      if (sample) begin
         overflow <= carry[TOTAL-1];
         digits <= sample_digits;
      end
   end

   // Sample on reset or whenever we counted down (1s)
   assign sample = rst | (count == 0);

   // Perpetual countdown
   always @(posedge clk) begin
      if (rst) begin
         count <= FREQ;
      end else begin
         if (count == 0) begin
            count <= FREQ;
         end else begin
            count <= count - 1;
         end
      end
   end

   // Generate the set of BCD counters
   genvar                i;
   generate
      for (i = 0; i < TOTAL; i = i + 1) begin
         wire [3:0] carry_in;
         wire [3:0] mydigits;

         if (i == 0) begin
            // The first one carries in the STEP on a trigger
            assign carry_in = trigger ? STEP : 4'd0;
         end else begin
            // All others carry in their pre-decessors carry
            assign carry_in = { 3'b000, carry[i-1]};
         end

         if (i > OFFSET) begin
            // Assign the relevant digits to get sampled, we attribute the
            // OFFSET here and don't care about the lower digits.
            assign sample_digits[(i-OFFSET+1)*4-1:(i-OFFSET)*4] = mydigits;
         end

         bcdcounter
           u_count(.clk       (clk),
                   .rst       (sample),
                   .carry_in  (carry_in[3:0]),
                   .count     (mydigits[3:0]),
                   .carry_out (carry[i]));
      end
   endgenerate
endmodule // glip_measure
