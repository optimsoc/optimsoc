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
 * Wishbone Bus Arbiter with two inputs
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module memtile_bus_arbiter(/*AUTOARG*/
   // Outputs
   gnt,
   // Inputs
   clk, rst, req
   );

   localparam GRANT0 = 1'd0;
   localparam GRANT1 = 1'd1;

   input clk;
   input rst;
   input  [1:0] req; // request input
   output [0:0] gnt; // grant output

   reg state;
   reg state_nxt;

   assign gnt = state;

   always @(posedge clk or posedge rst) begin
      if (rst) begin
         state <= GRANT0;
      end else begin
         state <= state_nxt;
      end
   end

   always @(state or req) begin
      state_nxt = state;

      case (state)
         GRANT0: begin
            if (!req[0]) begin
               if (req[1]) begin
                  state_nxt = GRANT1;
               end
            end
         end

         GRANT1: begin
            if (!req[1]) begin
               if (req[0]) begin
                  state_nxt = GRANT0;
               end
            end
         end
      endcase
   end
endmodule
