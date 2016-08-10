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
 * Unit that gets credits from the upstream communication partner
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module debtor
  #(parameter WIDTH = 1'bx,
    parameter TRANCHE_WIDTH = 1'bx)
   (
    input                     clk,
    input                     rst,

    input                     payback,
    output                    owing,
    input [TRANCHE_WIDTH-1:0] tranche,
    input                     lend,

    output reg                error
    );

   reg [WIDTH-1:0]            credit;
   reg [WIDTH:0]              nxt_credit;
   
   assign owing = |credit;
   
   always @(posedge clk) begin
      if (rst) begin
         credit <= 0;
      end else begin
         credit <= nxt_credit[WIDTH-1:0];
      end
   end

   always @(*) begin
      nxt_credit = credit;
      error = 0;
      
      if (lend & !payback) begin
         nxt_credit = credit + tranche;
         error = nxt_credit[WIDTH];
      end else if (payback & !lend) begin
         nxt_credit = nxt_credit - 1;
         error = ~|credit;
      end else if (lend & payback) begin
         nxt_credit = nxt_credit + tranche - 1;
      end
   end // always @ (*)
      
endmodule // glip_debtor
