/* Copyright (c) 2018 by the author(s)
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
 * Dual clock First-Word Fall-Through (FWFT) FIFO programmable full/empty
 * flags
 *
 * This FIFO implementation wraps the dual clock FIFO with standard read 
 * characteristics to have first-word fall-through read characteristics.
 *
 * Author(s):
 *   Max Koenen <max.koenen@tum.de>
 */
module fifo_dualclock_fwft #(
   parameter WIDTH = 8,
   parameter DEPTH = 32,
   parameter PROG_FULL = 0,
   parameter PROG_EMPTY = 0
)(
   input                      wr_clk,
   input                      wr_rst,
   input                      wr_en,
   input [(WIDTH-1):0]        din,

   input                      rd_clk,
   input                      rd_rst,
   input                      rd_en,
   output reg [(WIDTH-1):0]   dout,

   output                     full,
   output                     prog_full,
   output                     empty,
   output                     prog_empty
);

   reg                  fifo_valid, middle_valid, dout_valid;
   reg [(WIDTH-1):0]    middle_dout;

   wire [(WIDTH-1):0]   fifo_dout;
   wire                 fifo_empty, fifo_rd_en;
   wire                 fifo_prog_empty;
   wire                 will_update_middle, will_update_dout;

   // dual clock FIFO with standard (non-FWFT) read characteristics
   fifo_dualclock_standard
   #(.WIDTH(WIDTH),
     .DEPTH(DEPTH),
     .PROG_FULL(PROG_FULL),
     .PROG_EMPTY(PROG_EMPTY >= 2 ? PROG_EMPTY - 2 : 0))
   u_fifo(
      .wr_rst(wr_rst),
      .wr_clk(wr_clk),
      .wr_en(wr_en & ~full),
      .din(din),

      .rd_rst(rd_rst),
      .rd_clk(rd_clk),
      .rd_en(fifo_rd_en),
      .dout(fifo_dout),

      .full(full),
      .prog_full(prog_full),
      .empty(fifo_empty),
      .prog_empty(fifo_prog_empty)
   );

   // create FWFT FIFO out of non-FWFT FIFO
   // public domain code from Eli Billauer
   // see http://www.billauer.co.il/reg_fifo.html
   assign will_update_middle = fifo_valid && (middle_valid == will_update_dout);
   assign will_update_dout = (middle_valid || fifo_valid) && (rd_en || !dout_valid);
   assign fifo_rd_en = (!fifo_empty) && !(middle_valid && dout_valid && fifo_valid);
   assign empty = !dout_valid;

   always_ff @(posedge rd_clk) begin
      if (rd_rst) begin
         fifo_valid <= 0;
         middle_valid <= 0;
         dout_valid <= 0;
         dout <= 0;
         middle_dout <= 0;
      end else begin
         if (will_update_middle)
            middle_dout <= fifo_dout;

         if (will_update_dout)
            dout <= middle_valid ? middle_dout : fifo_dout;

         if (fifo_rd_en)
            fifo_valid <= 1;
         else if (will_update_middle || will_update_dout)
            fifo_valid <= 0;

         if (will_update_middle)
            middle_valid <= 1;
         else if (will_update_dout)
            middle_valid <= 0;

         if (will_update_dout)
            dout_valid <= 1;
         else if (rd_en)
            dout_valid <= 0;
      end
   end

   // generate prog_emtpy flag. Necessary to account for FWFT-logic.
   generate
      if (PROG_EMPTY == 0)
         assign prog_empty = empty;
      else if (PROG_EMPTY == 1)
         assign prog_empty = (dout_valid & ~middle_valid & ~fifo_valid) | empty;
      else
         assign prog_empty = fifo_prog_empty;
   endgenerate
endmodule
