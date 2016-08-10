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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module bcdcounter
  (
   input            clk, rst,
   input [3:0]      carry_in,
   output reg [3:0] count,
   output reg       carry_out
   );

   reg [3:0]        nxt_count;
   reg              nxt_carry_out;

   reg [4:0]        sum;

   always @(*) begin
      sum = count + carry_in;
      nxt_carry_out = 0;
      
      if (sum > 9) begin
         nxt_carry_out = 1;
         sum = sum - 10;
      end

      nxt_count = sum[3:0];
   end

   always @(posedge clk) begin
      if (rst) begin
         count <= 0;
         carry_out <= 0;
      end else begin
         count <= nxt_count;
         carry_out <= nxt_carry_out;
      end
   end
endmodule // bcdcounter
