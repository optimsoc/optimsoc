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
 * A clock-domain crossing FIFO
 *
 * Author(s):
 *   Mark Sagi <mark.sagi@mytum.de>
 */
module cdc_rdptr_empty(/*AUTOARG*/
   // Outputs
   rd_empty, rd_addr, rd_ptr,
   // Inputs
   rdq2_wrptr, rd_inc, rd_clk, rd_rst
   );

   parameter ADDRSIZE = 4;

   // synced n bit gray pointer for FIFO empty determination
   input [ADDRSIZE:0] rdq2_wrptr;
   input rd_inc, rd_clk, rd_rst;

   // '1' if FIFO empty
   output reg rd_empty;
   // n-1 bit binary pointer for memory addressing
   output [ADDRSIZE-1:0] rd_addr;
   // n bit gray pointer for FIFO full determination
   output reg [ADDRSIZE:0] rd_ptr;

   reg [ADDRSIZE:0]  rd_bin;
   wire [ADDRSIZE:0] rdgraynext, rdbinnext;

   // graystyle pointer
   always @(posedge rd_clk or negedge rd_rst) begin
      if (!rd_rst) begin
         {rd_bin, rd_ptr} <= 0;
      end else begin
         {rd_bin, rd_ptr} <= {rdbinnext, rdgraynext};
      end
   end

   assign rd_addr = rd_bin[ADDRSIZE-1:0];

   assign rdbinnext = rd_bin + (rd_inc & ~rd_empty);
   assign rdgraynext = (rdbinnext>>1) ^ rdbinnext;

   // FIFO empty if next rdptr == synchronized wrptr or on reset
   wire rdempty_val;
   assign rdempty_val = (rdgraynext == rdq2_wrptr);

   always @(posedge rd_clk or negedge rd_rst)
   begin
      if (!rd_rst) begin
         rd_empty <= 1'b1;
      end else begin
         rd_empty <= rdempty_val;
      end
   end

endmodule
