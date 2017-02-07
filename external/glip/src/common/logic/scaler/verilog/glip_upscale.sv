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
 * Only scaling factors 1:1 and 2:1 are supported.
 *
 * parameter:
 *  - IN_SIZE: input width in bits.
 *  - OUT_SIZE: output width in bits.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module glip_upscale
  #(parameter IN_SIZE=8,
    parameter OUT_SIZE = IN_SIZE * 2)
   (input clk, rst,

    input [IN_SIZE-1:0]    in_data,
    input                  in_valid,
    output                 in_ready,

    output [OUT_SIZE-1:0]  out_data,
    output                 out_valid,
    input                  out_ready);

   initial begin
      assert(OUT_SIZE == IN_SIZE || OUT_SIZE == IN_SIZE * 2) 
         else $fatal(1, "Only 1:1 and 1:2 scaling factors are supported.");
   end

   generate
      if (OUT_SIZE == IN_SIZE) begin
         assign out_data = in_data;
         assign out_valid = in_valid;
         assign in_ready = out_ready;
      end else if (OUT_SIZE == IN_SIZE * 2) begin
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
      end
   endgenerate
endmodule // glip_upscale

