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
module cdc_fifo(/*AUTOARG*/
   // Outputs
   wr_full, rd_empty, rd_data,
   // Inputs
   wr_clk, rd_clk, wr_rst, rd_rst, rd_en, wr_en, wr_data
   );

   parameter DW = 32;
   parameter ADDRSIZE = 4;

   // inputs
   input wr_clk;
   input rd_clk;

   input wr_rst;
   input rd_rst;

   input rd_en;
   input wr_en;

   input [DW-1:0] wr_data;

   // outputs
   output          wr_full;
   output          rd_empty;
   output [DW-1:0] rd_data;

   wire [ADDRSIZE-1:0] wr_addr, rd_addr;
   wire [ADDRSIZE:0]   wr_ptr, rd_ptr, wrq2_rdptr, rdq2_wrptr;

   cdc_sync_rd2wr
      #(.ADDRSIZE(ADDRSIZE))
      u_sync_rd2wr(.wrq2_rdptr(wrq2_rdptr[ADDRSIZE:0]),
                   .rd_ptr(rd_ptr[ADDRSIZE:0]),
                   .wr_clk(wr_clk),
                   .wr_rst(wr_rst));

   cdc_sync_wr2rd
      #(.ADDRSIZE(ADDRSIZE))
      u_sync_wr2rd(.rdq2_wrptr(rdq2_wrptr[ADDRSIZE:0]),
                   .wr_ptr(wr_ptr[ADDRSIZE:0]),
                   .rd_clk(rd_clk),
                   .rd_rst(rd_rst));

   cdc_fifo_mem
      #(.DW(DW),
        .ADDRSIZE(ADDRSIZE))
      u_fifo_mem(.rd_data(rd_data[DW-1:0]),
                 .wr_data(wr_data[DW-1:0]),
                 .wr_addr(wr_addr[ADDRSIZE-1:0]),
                 .rd_addr(rd_addr[ADDRSIZE-1:0]),
                 .wr_clken(wr_en),
                 .wr_full(wr_full),
                 .wr_clk(wr_clk));

   cdc_rdptr_empty
      #(.ADDRSIZE(ADDRSIZE))
      u_rdptr_empty(.rd_empty(rd_empty),
                    .rd_addr(rd_addr[ADDRSIZE-1:0]),
                    .rd_ptr(rd_ptr[ADDRSIZE:0]),
                    .rdq2_wrptr(rdq2_wrptr[ADDRSIZE:0]),
                    .rd_inc(rd_en),
                    .rd_clk(rd_clk),
                    .rd_rst(rd_rst));

   cdc_wrptr_full
      #(.ADDRSIZE(ADDRSIZE))
      u_wrptr_full(.wr_full(wr_full),
                   .wr_addr(wr_addr[ADDRSIZE-1:0]),
                   .wr_ptr(wr_ptr[ADDRSIZE:0]),
                   .wrq2_rdptr(wrq2_rdptr[ADDRSIZE:0]),
                   .wr_inc(wr_en),
                   .wr_clk(wr_clk),
                   .wr_rst(wr_rst));

endmodule
