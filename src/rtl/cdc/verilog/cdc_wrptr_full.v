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
module cdc_wrptr_full(/*AUTOARG*/
   // Outputs
   wr_full, wr_addr, wr_ptr,
   // Inputs
   wrq2_rdptr, wr_inc, wr_clk, wr_rst
   );

   parameter ADDRSIZE = 4;

   input [ADDRSIZE:0] wrq2_rdptr;
   input wr_inc, wr_clk, wr_rst;

   output reg              wr_full;
   output [ADDRSIZE-1:0]   wr_addr; //memory write-address pointer
   output reg [ADDRSIZE:0] wr_ptr;

   reg [ADDRSIZE:0] wbin;
   wire [ADDRSIZE:0] wgraynext, wbinnext;

   //Gray pointer
   always @(posedge wr_clk or negedge wr_rst) begin
      if (!wr_rst) begin
         {wbin, wr_ptr} <= 0;
      end else begin
         {wbin, wr_ptr} <= {wbinnext, wgraynext};
      end
   end

   assign wr_addr = wbin[ADDRSIZE-1:0];

   assign wbinnext = wbin + (wr_inc & ~wr_full);
   assign wgraynext = (wbinnext>>1) ^ wbinnext;

   // Full test:
   // If MSB and MSB-1 bit of wgraynext pointer != wrq2_rdptr
   // AND MSB-2:0 bits of wgraynext pointer == wrq2_rdptr
   // => FIFO Full
   wire wfull_val;
   assign wfull_val = (wgraynext == {~wrq2_rdptr[ADDRSIZE:ADDRSIZE-1],
                                     wrq2_rdptr[ADDRSIZE-2:0]});

   always @(posedge wr_clk or negedge wr_rst) begin
      if (!wr_rst) begin
         wr_full <= 1'b0;
      end else begin
         wr_full <= wfull_val;
      end
   end

endmodule
