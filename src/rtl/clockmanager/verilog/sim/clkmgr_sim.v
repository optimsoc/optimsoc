/* Copyright (c) 2015 by the author(s)
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
 * Clock manager to be used in simulation
 *
 * This clock manager can create multiples and fractions of a existing clock,
 * if the original period is given.
 * To support all kinds of fractions and multiples, new clocks are generated
 * based on the CLK_PERIOD parameter. No phase relationship between input and
 * output clock is specified.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module clkmgr_sim(/*AUTOARG*/
   // Outputs
   clk_out,
   // Inputs
   clk, rst
   );

   parameter CLK_PERIOD = 10;
   parameter DIV = 1;
   parameter MUL = 1;

   input clk;
   input rst;

   output reg clk_out;


   // multiplied clock
   reg clk_int_mul;
   initial begin
      clk_int_mul = 0;
   end
   always clk_int_mul = #((CLK_PERIOD+0.0) / (MUL+0.0) / 2.0) ~clk_int_mul;

   // divided clock
   reg[clog2_width(DIV * 2):0] clk_int_cnt;
   always @(clk_int_mul) begin
      if (rst) begin
         clk_out <= clk;
         clk_int_cnt <= 0;
      end else begin
         if (clk_int_cnt == 0) begin
            clk_out <= ~clk_out;
         end
         clk_int_cnt <= (clk_int_cnt + 1) % DIV;
      end
   end

   `include "optimsoc_functions.vh"
endmodule
