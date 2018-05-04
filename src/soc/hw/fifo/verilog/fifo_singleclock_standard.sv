/* Copyright (c) 2017 by the author(s)
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
 * Synchronous Standard FIFO (one clock)
 *
 * The memory block in this FIFO is following the "RAM HDL Coding Guidelines"
 * of Xilinx (UG901) to enable placing the FIFO memory into block ram during
 * synthesis.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module fifo_singleclock_standard #(
   parameter WIDTH = 8,
   parameter DEPTH = 32,
   parameter PROG_FULL = DEPTH / 2
)(
   input                     clk,
   input                     rst,

   input [(WIDTH-1):0]       din,
   input                     wr_en,
   output                    full,
   output                    prog_full,

   output reg [(WIDTH-1):0]  dout,
   input                     rd_en,
   output                    empty
);
   localparam AW = $clog2(DEPTH);

   // ensure that parameters are set to allowed values
   initial begin
      if ((1 << $clog2(DEPTH)) != DEPTH) begin
         $fatal("fifo_singleclock_standard: the DEPTH must be a power of two.");
      end
   end

   reg [AW-1:0] wr_addr;
   reg [AW-1:0] rd_addr;
   wire         fifo_read;
   wire         fifo_write;
   reg [AW-1:0] rd_count;

   // generate control signals
   assign empty       = (rd_count[AW-1:0] == 0);
   assign prog_full   = (rd_count[AW-1:0] >= PROG_FULL);
   assign full        = (rd_count[AW-1:0] == (DEPTH-1));
   assign fifo_read   = rd_en & ~empty;
   assign fifo_write  = wr_en & ~full;

   // address logic
   always_ff @(posedge clk) begin
      if (rst) begin
         wr_addr[AW-1:0]   <= 'd0;
         rd_addr[AW-1:0]   <= 'b0;
         rd_count[AW-1:0]  <= 'b0;
      end else begin
         if (fifo_write & fifo_read) begin
            wr_addr[AW-1:0] <= wr_addr[AW-1:0] + 'd1;
            rd_addr[AW-1:0] <= rd_addr[AW-1:0] + 'd1;
         end else if (fifo_write) begin
            wr_addr[AW-1:0] <= wr_addr[AW-1:0]  + 'd1;
            rd_count[AW-1:0]<= rd_count[AW-1:0] + 'd1;
         end else if (fifo_read) begin
            rd_addr[AW-1:0] <= rd_addr[AW-1:0]  + 'd1;
            rd_count[AW-1:0]<= rd_count[AW-1:0] - 'd1;
         end
      end
   end

   // generic dual-port, single clock memory
   reg [WIDTH-1:0] ram [DEPTH-1:0];

   // write
   always_ff @(posedge clk) begin
      if (fifo_write) begin
         ram[wr_addr] <= din;
      end
   end

   // read
   always_ff @(posedge clk) begin
      if (fifo_read) begin
         dout <= ram[rd_addr];
      end
   end
endmodule
