/* Copyright (c) 2016 by the author(s)
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
 * First-Word Fall Through version of the oh_fifo_sync
 *
 * Code adapted from http://www.billauer.co.il
 * 
 * Author(s):
 *    Max Koenen <max.koenen@tum.de>
 */

module fifo_sync_fwft #( 
   parameter DW        = 32,           // FIFO width
   parameter DEPTH     = 32,           // FIFO depth
   parameter PROG_FULL = (DEPTH/2),    // prog_full threshold
   parameter AW        = $clog2(DEPTH) // rd_count width
)(
   input               clk,
   input               nreset,
   input [DW-1:0]      din,
   input               wr_en,
   input               rd_en,
   output reg [DW-1:0] dout,
   output              full,
   output              prog_full,
   output              empty,
   output [AW-1:0]     rd_count
   );   

   reg              fifo_valid;
   reg              middle_valid;
   reg              dout_valid;
   reg [DW-1:0]     middle_dout;

   wire [DW-1:0]    fifo_dout;
   wire             fifo_empty;
   wire             fifo_rd_en;
   wire             will_update_middle;
   wire             will_update_dout;

   // normal non-FWFT FIFO
   oh_fifo_sync
        #(.DW(DW),.DEPTH(DEPTH),.PROG_FULL(PROG_FULL),.AW(AW))
   u_fifo_sync(
        .clk       (clk),
        .nreset    (nreset),
        .din       (din),
        .wr_en     (wr_en),
        .rd_en     (fifo_rd_en),
        .dout      (fifo_dout),
        .full      (full),
        .prog_full (prog_full),
        .empty     (fifo_empty),
        .rd_count  (rd_count));

   assign will_update_middle = fifo_valid && (middle_valid == will_update_dout);
   assign will_update_dout = (middle_valid || fifo_valid) && (rd_en || !dout_valid);
   assign fifo_rd_en = (!fifo_empty) && !(middle_valid && dout_valid && fifo_valid);
   assign empty = !dout_valid;

   always @(posedge clk) begin
      if (!nreset) begin
         fifo_valid <= 0;
         middle_valid <= 0;
         dout_valid <= 0;
         dout <= 0;
         middle_dout <= 0;
      end else begin
         if (will_update_middle) begin
              middle_dout <= fifo_dout;
          end
            
          if (will_update_dout) begin
             dout <= middle_valid ? middle_dout : fifo_dout;
          end
            
          if (fifo_rd_en) begin
             fifo_valid <= 1;
          end else if (will_update_middle || will_update_dout) begin
             fifo_valid <= 0;
          end
            
          if (will_update_middle) begin
             middle_valid <= 1;
          end else if (will_update_dout) begin
             middle_valid <= 0;
          end
            
          if (will_update_dout) begin
             dout_valid <= 1;
          end else if (rd_en) begin
             dout_valid <= 0;
          end
       end 
    end
endmodule
