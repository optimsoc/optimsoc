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
 * Author(s):
 *   Mark Sagi <mark.sagi@mytum.de>
 */
 module cdc_sync_wr2rd(/*AUTOARG*/
   // Outputs
   rdq2_wrptr,
   // Inputs
   rd_clk, rd_rst, wr_ptr
   );

   parameter ADDRSIZE = 4;

   input rd_clk, rd_rst;
   input [ADDRSIZE:0] wr_ptr;

   output reg [ADDRSIZE:0] rdq2_wrptr;

   // at least two registers are needed to minimize metastability
   reg [ADDRSIZE:0] rdq1_wrptr;

   always @(posedge rd_clk or negedge rd_rst) begin
      if (!rd_rst) begin
         rdq2_wrptr <= 0;
         rdq1_wrptr <= 0;
      end else begin
         rdq2_wrptr <= rdq1_wrptr;
         rdq1_wrptr <= wr_ptr;
      end
   end

endmodule
