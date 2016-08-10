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
 * Measure the number of triggers in a second, and display it on a seven 
 * segment display.
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

module glip_measure_sevensegment
  #(parameter FREQ = 32'hx,
    parameter DIGITS = 8,
    parameter OFFSET = 0,
    parameter STEP = 1)
  (
   input                 clk,
   input                 rst,
   
   input                 trigger,
   
   output [DIGITS*7-1:0] digits,
   
   output                overflow
   );

   wire [DIGITS*4-1:0]   bcd_digits;
   
   glip_measure
     #(.FREQ(FREQ), .DIGITS(DIGITS), .OFFSET(OFFSET), .STEP(STEP))
     u_measure(.clk      (clk),
           .rst      (rst),
           .trigger  (trigger),
           .digits   (bcd_digits),
           .overflow (overflow));

   genvar i;

   generate
      for (i = 0; i < DIGITS; i = i + 1) begin
         sevensegment
            u_seg(.in  (bcd_digits[(i+1)*4-1:i*4]),
                  .out (digits[(i+1)*7-1:i*7]));
      end
   endgenerate
   
endmodule // glip_measure_sevensegment
