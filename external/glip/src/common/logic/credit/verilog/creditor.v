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
 * Unit that gives credits to the upstream communication partner
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module creditor
  #(parameter WIDTH = 1'bx,
    parameter CREDIT_WIDTH = 1'bx,
    parameter INITIAL_VALUE = {WIDTH{1'bx}})
   (
    input                         clk,
    input                         rst,
    
    input                         payback,
    output reg [CREDIT_WIDTH-1:0] credit,
    input                         borrow,
    output reg                    grant,
    
    output                        error
    );
   
   reg [WIDTH-1:0]                resources = INITIAL_VALUE;
   reg [WIDTH:0]                  nxt_resources;
   reg [CREDIT_WIDTH-1:0]         nxt_credit;
   
   // Error is an overflow of the resources. The host cannot payback
   // more than originally granted.
   assign error = nxt_resources[WIDTH] | (resources > INITIAL_VALUE);

   // Sample registers for remaining resources and credit
   always @(posedge clk) begin
      if (rst) begin
         resources <= INITIAL_VALUE;
         credit <= 0;
      end else begin
         resources <= nxt_resources[WIDTH-1:0];
         credit <= nxt_credit;
      end
   end

   always @(*) begin
      nxt_resources = resources;
      nxt_credit = credit;
      
      grant = 0;
      
      if (payback) begin
         nxt_resources = resources + 1;
      end else if (borrow) begin
         grant = 1;
         if (WIDTH > CREDIT_WIDTH) begin
            if (|resources[WIDTH-1:CREDIT_WIDTH]) begin
               nxt_credit = {CREDIT_WIDTH{1'b1}};
            nxt_resources = resources - nxt_credit;
         end else begin
            nxt_credit = resources;
            nxt_resources = 0;
         end
         end else begin
            nxt_credit = { {CREDIT_WIDTH-WIDTH{1'b0}}, resources};
            nxt_resources = 0;
         end // else: !if(WIDTH > CREDIT_WIDTH)
      end
   end
   
endmodule // glip_creditgen
