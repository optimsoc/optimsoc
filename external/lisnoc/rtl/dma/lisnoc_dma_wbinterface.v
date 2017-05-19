/* Copyright (c) 2015 by the author(s)
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
 * The wishbone slave interface to configure the DMA module.
 *
 * (c) 2011-2013 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 */

`include "lisnoc_dma_def.vh"

module lisnoc_dma_wbinterface(/*AUTOARG*/
   // Outputs
   wb_if_dat_o, wb_if_ack_o, if_write_req, if_write_pos,
   if_write_select, if_write_en, if_valid_pos, if_valid_set,
   if_valid_en, if_validrd_en,
   // Inputs
   clk, rst, wb_if_adr_i, wb_if_dat_i, wb_if_cyc_i, wb_if_stb_i,
   wb_if_we_i, done
   );

   parameter table_entries = 4;
//   localparam table_entries_ptrwidth = $clog2(table_entries);
   localparam table_entries_ptrwidth = 2;

   parameter tileid = 0; // TODO: remove

   input clk,rst;

   input [31:0]             wb_if_adr_i;
   input [31:0]             wb_if_dat_i;
   input                    wb_if_cyc_i;
   input                    wb_if_stb_i;
   input                    wb_if_we_i;
   output reg [31:0]        wb_if_dat_o;
   output                   wb_if_ack_o;

   output [`DMA_REQUEST_WIDTH-1:0]     if_write_req;
   output [table_entries_ptrwidth-1:0] if_write_pos;
   output [`DMA_REQMASK_WIDTH-1:0]     if_write_select;
   output                              if_write_en;

   // Interface read (status) interface
   output [table_entries_ptrwidth-1:0]    if_valid_pos;
   output                                 if_valid_set;
   output                                 if_valid_en;
   output                                 if_validrd_en;

   input [table_entries-1:0]              done;

   assign if_write_req = { wb_if_dat_i[`DMA_REQFIELD_LADDR_WIDTH-1:0],
                           wb_if_dat_i[`DMA_REQFIELD_SIZE_WIDTH-1:0],
                           wb_if_dat_i[`DMA_REQFIELD_RTILE_WIDTH-1:0],
                           wb_if_dat_i[`DMA_REQFIELD_RADDR_WIDTH-1:0],
                           wb_if_dat_i[0] };

   assign if_write_pos = wb_if_adr_i[table_entries_ptrwidth+4:5]; // ptrwidth MUST be <= 7 (=128 entries)
   assign if_write_en  = wb_if_cyc_i & wb_if_stb_i & wb_if_we_i;

   assign if_valid_pos = wb_if_adr_i[table_entries_ptrwidth+4:5]; // ptrwidth MUST be <= 7 (=128 entries)
   assign if_valid_en  = wb_if_cyc_i & wb_if_stb_i & (wb_if_adr_i[4:0] == 5'h14) & wb_if_we_i;
   assign if_validrd_en  = wb_if_cyc_i & wb_if_stb_i & (wb_if_adr_i[4:0] == 5'h14) & ~wb_if_we_i;
   assign if_valid_set = wb_if_we_i | (~wb_if_we_i & ~done[if_valid_pos]);

   assign wb_if_ack_o = wb_if_cyc_i & wb_if_stb_i;

   always @(*) begin
      if (wb_if_adr_i[4:0] == 5'h14) begin
         wb_if_dat_o = {31'h0,done[if_valid_pos]};
      end else begin
         wb_if_dat_o = 32'h0;
      end
   end

   genvar  i;
   // This assumes, that mask and address match
   generate
      for (i=0;i<`DMA_REQMASK_WIDTH;i=i+1) begin
         assign if_write_select[i] = (wb_if_adr_i[4:2] == i);
      end
   endgenerate

endmodule // lisnoc_dma_wbinterface

`include "lisnoc_dma_undef.vh"
