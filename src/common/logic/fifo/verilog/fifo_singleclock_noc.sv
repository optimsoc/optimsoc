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
 * Synchronous FWFT FIFO with NoC port naming
 *
 * In the NoC, we use slightly different signal naming than in regular FIFOs.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

// synchronous FWFT FIFO with NoC naming (nothing else changed)
module fifo_singleclock_noc #(
   parameter WIDTH = 34,
   parameter DEPTH = 16
)(
   input clk,
   input rst,

   // FIFO input side
   input  [WIDTH-1:0] in_flit,
   input  in_valid,
   output in_ready,

   // FIFO output side
   output [WIDTH-1:0] out_flit,
   output out_valid,
   input  out_ready
);


   wire [(WIDTH-1):0]  din;
   wire                wr_en;
   wire                full;

   wire [(WIDTH-1):0]  dout;
   wire                rd_en;
   wire                empty;

   // Synchronous FWFT FIFO
   fifo_singleclock_fwft
      #(
         .WIDTH(WIDTH),
         .DEPTH(DEPTH)
      )
      u_fifo (
         .clk(clk),
         .rst(rst),

         .din(din),
         .wr_en(wr_en),
         .full(full),
         .prog_full(), // unused

         .dout(dout),
         .rd_en(rd_en),
         .empty(empty)
      );

   // map wire names from NoC naming to normal FIFO naming
   assign din = in_flit;
   assign wr_en = in_valid;
   assign in_ready = ~full;

   assign out_flit = dout;
   assign out_valid = ~empty;
   assign rd_en = out_ready;

endmodule
