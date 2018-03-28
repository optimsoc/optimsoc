/* Copyright (c) 2010-2017 by the author(s)
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
 * RAM interface translation: Generic RAM interface to Wishbone
 *
 * See the README file in this directory for information about the common
 * ports and parameters used in all memory modules.
 *
 * The original code has been taken from wb_ram_b3.v, part of the ORPSoCv2
 * project on opencores.org.
 *
 * Author(s):
 *   Julius Baxter <julius@opencores.org> (original author)
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Markus Goehrle <markus.goehrle@tum.de>
 */

module wb2sram(/*AUTOARG*/
   // Outputs
   wb_ack_o, wb_err_o, wb_rty_o, wb_dat_o, sram_ce, sram_we,
   sram_waddr, sram_din, sram_sel,
   // Inputs
   wb_adr_i, wb_bte_i, wb_cti_i, wb_cyc_i, wb_dat_i, wb_sel_i,
   wb_stb_i, wb_we_i, wb_clk_i, wb_rst_i, sram_dout
   );

   import optimsoc_functions::*;

   // Memory parameters
   // data width (word size)
   // Valid values: 32, 16 and 8
   parameter DW = 32;

   // address width
   parameter AW = 32;

   // byte select width
   localparam SW = (DW == 32) ? 4 :
                   (DW == 16) ? 2 :
                   (DW ==  8) ? 1 : 'hx;

   /*
    * +--------------+--------------+
    * | word address | byte in word |
    * +--------------+--------------+
    *     WORD_AW         BYTE_AW
    *        +----- AW -----+
    */
   localparam BYTE_AW = SW >> 1;
   localparam WORD_AW = AW - BYTE_AW;

   // wishbone ports
   input [AW-1:0]       wb_adr_i;
   input [1:0]          wb_bte_i;
   input [2:0]          wb_cti_i;
   input                wb_cyc_i;
   input [DW-1:0]       wb_dat_i;
   input [SW-1:0]       wb_sel_i;
   input                wb_stb_i;
   input                wb_we_i;

   output               wb_ack_o;
   output               wb_err_o;
   output               wb_rty_o;
   output [DW-1:0]      wb_dat_o;

   input                wb_clk_i;
   input                wb_rst_i;

   wire [WORD_AW-1:0]   word_addr_in;
   assign word_addr_in = wb_adr_i[AW-1:BYTE_AW];

   // generic RAM ports
   output               sram_ce;
   output               sram_we;
   output [WORD_AW-1:0] sram_waddr;
   output [DW-1:0]      sram_din;
   output [SW-1:0]      sram_sel;
   input [DW-1:0]       sram_dout;

   reg [WORD_AW-1:0]         word_addr_reg;
   reg [WORD_AW-1:0]         word_addr;

   // Register to indicate if the cycle is a Wishbone B3-registered feedback
   // type access
   reg                  wb_b3_trans;
   wire                 wb_b3_trans_start, wb_b3_trans_stop;

   // Register to use for counting the addresses when doing burst accesses
   reg [WORD_AW-1:0]    burst_adr_counter;
   reg [2:0]            wb_cti_i_r;
   reg [1:0]            wb_bte_i_r;
   wire                 using_burst_adr;
   wire                 burst_access_wrong_wb_adr;


   // assignments from wb to memory
   assign sram_ce = 1'b1;
   assign sram_we = wb_we_i & wb_ack_o;
   assign sram_waddr = (wb_we_i) ? word_addr_reg : word_addr;
   assign sram_din = wb_dat_i;
   assign sram_sel = wb_sel_i;

   assign wb_dat_o = sram_dout;


   // Logic to detect if there's a burst access going on
   assign wb_b3_trans_start = ((wb_cti_i == 3'b001)|(wb_cti_i == 3'b010)) &
                              wb_stb_i & !wb_b3_trans;

   assign  wb_b3_trans_stop = (wb_cti_i == 3'b111) &
                              wb_stb_i & wb_b3_trans & wb_ack_o;

   always @(posedge wb_clk_i) begin
      if (wb_rst_i) begin
         wb_b3_trans <= 0;
      end else if (wb_b3_trans_start) begin
         wb_b3_trans <= 1;
      end else if (wb_b3_trans_stop) begin
         wb_b3_trans <= 0;
      end
   end

   // Burst address generation logic
   always @(*) begin
      if (wb_rst_i) begin
          burst_adr_counter = 0;
      end else begin
         burst_adr_counter = word_addr_reg;
         if (wb_b3_trans_start) begin
            burst_adr_counter = word_addr_in;
         end else if ((wb_cti_i_r == 3'b010) & wb_ack_o & wb_b3_trans) begin
            // Incrementing burst
            if (wb_bte_i_r == 2'b00) begin // Linear burst
               burst_adr_counter = word_addr_reg + 1;
            end
            if (wb_bte_i_r == 2'b01) begin // 4-beat wrap burst
               burst_adr_counter[1:0] = word_addr_reg[1:0] + 1;
            end
            if (wb_bte_i_r == 2'b10) begin // 8-beat wrap burst
               burst_adr_counter[2:0] = word_addr_reg[2:0] + 1;
            end
            if (wb_bte_i_r == 2'b11) begin // 16-beat wrap burst
               burst_adr_counter[3:0] = word_addr_reg[3:0] + 1;
            end
         end else if (!wb_ack_o & wb_b3_trans) begin
            burst_adr_counter = word_addr_reg;
         end
      end
   end


   // Register it locally
   always @(posedge wb_clk_i) begin
      wb_bte_i_r <= wb_bte_i;
   end

   always @(posedge wb_clk_i) begin
      wb_cti_i_r <= wb_cti_i;
   end

   assign using_burst_adr = wb_b3_trans;

   assign burst_access_wrong_wb_adr = (using_burst_adr & (word_addr_reg != word_addr_in));

   // Address logic
   always @(*) begin
      if (using_burst_adr) begin
         word_addr = burst_adr_counter;
      end else if (wb_cyc_i & wb_stb_i) begin
         word_addr = word_addr_in;
      end else begin
         word_addr = word_addr_reg;
      end
   end

   // Address registering logic
   always @(posedge wb_clk_i) begin
      if (wb_rst_i) begin
         word_addr_reg <= {WORD_AW{1'bx}};
      end else begin
         word_addr_reg <= word_addr;
      end
   end


   assign wb_rty_o = 0;

   // Ack Logic
   reg  wb_ack;
   reg  nxt_wb_ack;

   assign wb_ack_o = wb_ack & wb_stb_i & ~burst_access_wrong_wb_adr;

   always @(*) begin
      if (wb_cyc_i) begin
         if (wb_cti_i == 3'b000) begin
            // Classic cycle acks
            if (wb_stb_i) begin
               if (!wb_ack) begin
                  nxt_wb_ack = 1;
               end else begin
                  nxt_wb_ack = 0;
               end
            end else begin
               nxt_wb_ack = 0;
            end
         end else if ((wb_cti_i == 3'b001) ||
                      (wb_cti_i == 3'b010)) begin
            // Increment/constant address bursts
            if (wb_stb_i) begin
               nxt_wb_ack = 1;
            end else begin
               nxt_wb_ack = 0;
            end
         end else if (wb_cti_i == 3'b111) begin
            // End of cycle
            if (wb_stb_i) begin
               if (!wb_ack) begin
                  nxt_wb_ack = 1;
               end else begin
                  nxt_wb_ack = 0;
               end
            end else begin
               nxt_wb_ack = 0;
            end
         end else begin
            nxt_wb_ack = 0;
         end
      end else begin
         nxt_wb_ack = 0;
      end
   end // always @ begin

   always @(posedge wb_clk_i) begin
      if (wb_rst_i) begin
         wb_ack <= 1'b0;
      end else begin
         wb_ack <= nxt_wb_ack;
      end
   end

   assign wb_err_o = wb_ack & wb_stb_i & (burst_access_wrong_wb_adr);

endmodule
