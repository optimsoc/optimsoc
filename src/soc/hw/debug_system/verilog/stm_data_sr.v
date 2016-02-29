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
 * Submodule of the Software Trace Module (STM): the data shift register
 *
 * All incoming data is delayed by DELAY_CYCLES before passing it to the output.
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 */

`include "dbg_config.vh"

module stm_data_sr(/*AUTOARG*/
   // Outputs
   dout,
   // Inputs
   clk, rst, din
   );

   parameter DELAY_CYCLES = `DBG_TRIGGER_DELAY;
   parameter DATA_WIDTH = `DBG_TIMESTAMP_WIDTH+32+16;

   input clk;
   input rst;

   input [DATA_WIDTH-1:0] din;
   output [DATA_WIDTH-1:0] dout;

   reg [(DATA_WIDTH*(DELAY_CYCLES+1))-1:0] sr;

   assign dout = sr[(DATA_WIDTH*(DELAY_CYCLES+1))-1:(DATA_WIDTH*DELAY_CYCLES)];

   always @ (posedge clk) begin
      if (rst) begin
         sr <= 0;
      end else begin
         sr <= sr << DATA_WIDTH;
         sr[DATA_WIDTH-1:0] <= din;
      end
   end
endmodule
