/* Copyright (c) 2013 by the author(s)
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
 * 2-FlipFlop Synchronizer
 *
 * Author(s):
 *   Mark Sagi <mark.sagi@mytum.de>
 */
module cdc_sync_2ff (/*AUTOARG*/
   // Outputs
   rd_p,
   // Inputs
   rd_clk, wr_p, rst
   );

   input rd_clk;
   input wr_p;
   input rst;

   output rd_p;

   reg ff1;
   reg ff2;

   always @(posedge rd_clk or posedge rst) begin
      if (rst == 1'b1) begin
         {ff1, ff2}  <= {1'b0, 1'b0};
      end else begin
         {ff1, ff2}  <= {wr_p, ff1};
      end
   end

   assign rd_p = ff2;
endmodule
