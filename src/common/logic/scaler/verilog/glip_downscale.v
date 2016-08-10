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
 * Sownscale a FIFO interface
 *
 * parameter:
 *  - OUT_SIZE: Output width in bits. The input will have the double width.
 * 
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module glip_downscale
  #(parameter OUT_SIZE=16)
   (input clk, rst,

    input [OUT_SIZE*2-1:0] in_data,
    input                  in_valid,
    output                 in_ready,

    output [OUT_SIZE-1:0]  out_data,
    output                 out_valid,
    input                  out_ready);

   /* 0 when passthrough and 1 when emitting lower part */
   reg                     scale;
   /* Store lower part for emitting in second transfer */
   reg [OUT_SIZE-1:0]      lower;

   /* Ready during passthrough */
   assign in_ready = !scale & out_ready;
   /* Valid during passthrough or second transfer */
   assign out_valid = scale | in_valid;
   /* Passthrough in first and stored lower in second transfer */
   assign out_data = scale ? lower : in_data[OUT_SIZE*2-1:OUT_SIZE];
   
   always @(posedge clk) begin
      if (rst) begin
         scale <= 0;
      end else if (scale & out_valid & out_ready) begin
         scale <= 0;
      end else if (!scale & in_valid & in_ready) begin
         scale <= 1;
      end
   end

   always @(posedge clk) begin
      if (in_valid & in_ready) begin
         lower <= in_data[OUT_SIZE-1:0];
      end
   end
   
endmodule // glip_downscale

