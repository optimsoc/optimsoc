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
 * Round-robin Wishbone bus arbiter
 *
 *
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */
module compute_tile_dm_bus_arbiter(/*AUTOARG*/
   // Outputs
   gnt,
   // Inputs
   clk, rst, req
   );

   parameter GRANT0 = 2'd0;
   parameter GRANT1 = 2'd1;
   parameter GRANT2 = 2'd2;

   input        clk;
   input        rst;
   input  [2:0] req; // request input
   output [1:0] gnt; // grant output

   reg [1:0] state;
   reg [1:0] state_nxt;

   assign gnt = state;

   always @(posedge clk or posedge rst) begin
      if (rst)
         state <= GRANT0;
      else begin
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
               end else if (req[2]) begin
                  state_nxt = GRANT2;
               end
            end
         end

         GRANT1: begin
            if (!req[1]) begin
               if (req[2]) begin
                  state_nxt = GRANT2;
               end else if (req[0]) begin
                  state_nxt = GRANT0;
               end
            end
         end

         GRANT2: begin
            if (!req[2]) begin
               if (req[0]) begin
                  state_nxt = GRANT0;
               end else if (req[1]) begin
                  state_nxt = GRANT1;
               end
            end
         end
      endcase
   end
endmodule
