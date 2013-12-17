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
 module cdc_sync_rd2wr(/*AUTOARG*/
   // Outputs
   wrq2_rdptr,
   // Inputs
   rd_ptr, wr_clk, wr_rst
   );

   parameter ADDRSIZE = 4;

   input wr_clk, wr_rst;
   input [ADDRSIZE:0] rd_ptr;

   output [ADDRSIZE:0] wrq2_rdptr;

   reg [ADDRSIZE:0] wrq2_rdptr;
   reg [ADDRSIZE:0] wrq1_rdptr;
   wire [ADDRSIZE:0] rd_ptr;
   wire wr_clk, wr_rst;

   always @(posedge wr_clk or negedge wr_rst) begin
      if (!wr_rst) begin
         wrq2_rdptr <= 0;
         wrq1_rdptr <= 0;
      end else begin
         {wrq2_rdptr, wrq1_rdptr} <=  {wrq1_rdptr,rd_ptr};
      end
   end

endmodule
