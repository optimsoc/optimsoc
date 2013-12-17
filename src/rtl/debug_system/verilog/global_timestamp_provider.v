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
 * The Global Timestamp Provider (GTP) Module
 *
 * This module provides all other debug units with a global timestamp with
 * single-clock-cycle resolution to be able to exactly reconstruct the original
 * order of the collected information.
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 */

`include "dbg_config.vh"

module global_timestamp_provider(/*AUTOARG*/
   // Outputs
   timestamp,
   // Inputs
   clk, rst
   );

   parameter TIMESTAMP_WIDTH = `DBG_TIMESTAMP_WIDTH;
   parameter RESET_VALUE = 0;

   input clk;
   input rst;

   output reg[TIMESTAMP_WIDTH-1:0] timestamp;

   always @(posedge clk) begin
      if (rst) begin
         timestamp <= RESET_VALUE;
      end else begin
         timestamp <= timestamp + 1;
      end
   end

endmodule
