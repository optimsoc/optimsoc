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
 * Upscale a FIFO interface
 *
 * parameter:
 *  - IN_SIZE: input width in bits. The output will have the double width.
 * 
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module glip_upscale
  #(parameter IN_SIZE=8)
   (input clk, rst,

    input [IN_SIZE-1:0]    in_data,
    input                  in_valid,
    output                 in_ready,

    output [IN_SIZE*2-1:0] out_data,
    output                 out_valid,
    input                  out_ready);

   /* 0 while storing the first part and 1 when emitting */
   reg                      scale;
   /* Store upper part for second cycle */
   reg [IN_SIZE-1:0]        upper;

   /* Ready to store on first part and then passthrough in second */
   assign in_ready = !scale | out_ready;
   /* Valid in second part */
   assign out_valid = scale & in_valid;
   /* Assemble data */
   assign out_data = { upper, in_data };
   
   always @(posedge clk) begin
      if (rst) begin
         scale <= 0;
      end else if (scale & in_valid & in_ready) begin
         scale <= 0;
      end else if (!scale & in_valid & in_ready) begin
         scale <= 1;
      end
   end

   always @(posedge clk) begin
      if (in_valid & in_ready) begin
         upper <= in_data;
      end
   end
endmodule // glip_upscale

