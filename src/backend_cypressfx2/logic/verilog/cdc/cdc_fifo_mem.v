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
 module cdc_fifo_mem(/*AUTOARG*/
   // Outputs
   rd_data,
   // Inputs
   wr_data, wr_clken, wr_clk, wr_full, wr_addr, rd_addr
   );

   parameter DW = 32;
   parameter ADDRSIZE = 4;

   localparam DEPTH = 1<<ADDRSIZE;

   input [DW-1:0] wr_data;
   input wr_clken, wr_clk, wr_full;
   input [ADDRSIZE-1:0] wr_addr;
   input [ADDRSIZE-1:0] rd_addr;

   output [DW-1:0] rd_data;

   reg [DW-1:0] mem [0:DEPTH-1];

   assign rd_data = mem[rd_addr];

   always @(posedge wr_clk) begin
      if (wr_clken && !wr_full) begin
         mem[wr_addr] <= wr_data;
      end
   end

endmodule
