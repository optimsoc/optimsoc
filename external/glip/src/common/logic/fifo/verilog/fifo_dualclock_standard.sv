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
 * Dual clock FIFO with programmable full/empty flags
 *
 * The PROG_FULL / PROG_EMPTY flags are limited to a value of 8 in order to keep
 * the logic at a reasonable size.
 *
 * Author(s):
 *   Max Koenen <max.koenen@tum.de>
 *
 *
 * Based on dual_clock_fifo.v by Stefan Kristiansson
 * Copyright (c) 2012, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * All rights reserved.
 *
 * Based on vga_fifo_dc.v in Richard Herveille's VGA/LCD core
 * Copyright (C) 2001 Richard Herveille <richard@asics.ws>
 *
 * Redistribution and use in source and non-source forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in non-source form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS WORK IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * WORK, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

module fifo_dualclock_standard #(
   parameter WIDTH = 8,
   parameter DEPTH = 32,
   parameter PROG_FULL = 0,
   parameter PROG_EMPTY = 0
)(
   input wire              wr_rst,
   input wire              wr_clk,
   input wire              wr_en,
   input wire [WIDTH-1:0]  din,

   input wire              rd_rst,
   input wire              rd_clk,
   input wire              rd_en,
   output reg [WIDTH-1:0]  dout,

   output reg              full,
   output                  prog_full,
   output reg              empty,
   output                  prog_empty
);

   localparam AW = $clog2(DEPTH);

   // ensure that parameters are set to allowed values
   initial begin
      if ((1 << $clog2(DEPTH)) != DEPTH) begin
         $fatal("fifo_dualclock_standard: the DEPTH must be a power of two.");
      end
      if (PROG_FULL > 8 || PROG_EMPTY > 8) begin
         $fatal("fifo_dualclock_standard: PROG_FULL and PROG_EMPTY must be eight or lower.");
      end
   end   

   reg [AW-1:0] wr_addr;
   reg [AW-1:0] wr_addr_gray;
   reg [AW-1:0] wr_addr_gray_rd;
   reg [AW-1:0] wr_addr_gray_rd_r;
   reg [AW-1:0] rd_addr;
   reg [AW-1:0] rd_addr_gray;
   reg [AW-1:0] rd_addr_gray_wr;
   reg [AW-1:0] rd_addr_gray_wr_r;

   // generate prog_full signal
   generate
      if (PROG_FULL == 0) begin
         assign prog_full = full;
      end else begin
         wire [PROG_FULL:0][AW-1:0]   prog_full_addr;
         logic [PROG_FULL:0] prog_full_part;
         genvar i;
         for (i = 0; i <= PROG_FULL; i++) begin
            assign prog_full_addr[i] = wr_addr + i;
         end
         always @(posedge wr_clk) begin
            integer i;
            if (wr_rst) begin
               for (i = 0; i <= PROG_FULL; i++) begin
                  prog_full_part[i] <= 0;
               end               
            end else begin
               for (i = 0; i <= PROG_FULL; i++) begin
                  prog_full_part[i] <= gray_conv(prog_full_addr[i] + 2) == rd_addr_gray_wr_r;
               end
            end
         end
         assign prog_full = |prog_full_part | full;
      end
   endgenerate

   // generate prog_empty signal
   generate
      if (PROG_EMPTY == 0) begin
         assign prog_empty = empty;
      end else begin
         wire [PROG_EMPTY:0][AW-1:0]   prog_empty_addr;
         logic [PROG_EMPTY:0] prog_empty_part;
         genvar i;
         for (i = 0; i <= PROG_EMPTY; i++) begin
            assign prog_empty_addr[i] = rd_addr + i;
         end
         always @(posedge rd_clk) begin
            integer i;
            if (rd_rst) begin
               for (i = 0; i <= PROG_EMPTY; i++) begin
                  prog_empty_part[i] <= 0;
               end               
            end else begin
               for (i = 0; i <= PROG_EMPTY; i++) begin
                  prog_empty_part[i] <= gray_conv(prog_empty_addr[i] + 1) == wr_addr_gray_rd_r;
               end
            end
         end
         assign prog_empty = |prog_empty_part | empty;
      end
   endgenerate   

   function [AW-1:0] gray_conv;
      input [AW-1:0] in;
      begin
         gray_conv = {in[AW-1],
               in[AW-2:0] ^ in[AW-1:1]};
      end
   endfunction

   always @(posedge wr_clk) begin
      if (wr_rst) begin
         wr_addr <= 0;
         wr_addr_gray <= 0;
      end else if (wr_en) begin
         wr_addr <= wr_addr + 1'b1;
         wr_addr_gray <= gray_conv(wr_addr + 1'b1);
      end
   end

   // synchronize read address to write clock domain
   always @(posedge wr_clk) begin
      if (wr_rst) begin
         rd_addr_gray_wr <= 0;
         rd_addr_gray_wr_r <= 0;
      end else begin
         rd_addr_gray_wr <= rd_addr_gray;
         rd_addr_gray_wr_r <= rd_addr_gray_wr;
      end
   end

   always @(posedge wr_clk)
      if (wr_rst)
         full <= 0;
      else if (wr_en)
         full <= gray_conv(wr_addr + 2) == rd_addr_gray_wr_r;
      else
         full <= full & (gray_conv(wr_addr + 1'b1) == rd_addr_gray_wr_r);

   always @(posedge rd_clk) begin
      if (rd_rst) begin
         rd_addr <= 0;
         rd_addr_gray <= 0;
      end else if (rd_en) begin
         rd_addr <= rd_addr + 1'b1;
         rd_addr_gray <= gray_conv(rd_addr + 1'b1);
      end
   end

   // synchronize write address to read clock domain
   always @(posedge rd_clk) begin
      if (rd_rst) begin
         wr_addr_gray_rd <= 0;
         wr_addr_gray_rd_r <= 0;
      end else begin
         wr_addr_gray_rd <= wr_addr_gray;
         wr_addr_gray_rd_r <= wr_addr_gray_rd;
      end
   end

   always @(posedge rd_clk)
      if (rd_rst)
         empty <= 1'b1;
      else if (rd_en)
         empty <= gray_conv(rd_addr + 1) == wr_addr_gray_rd_r;
      else
         empty <= empty & (gray_conv(rd_addr) == wr_addr_gray_rd_r);

   // generate dual clocked memory
   reg [WIDTH-1:0] mem[(1<<AW)-1:0];

   always @(posedge rd_clk)
      if (rd_en)
         dout <= mem[rd_addr];

   always @(posedge wr_clk)
      if (wr_en)
         mem[wr_addr] <= din;
endmodule