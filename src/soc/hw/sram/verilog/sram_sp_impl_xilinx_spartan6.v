/* Copyright (c) 2012-2013 by the author(s)
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
 * This is the actual memory element implemented as Xilinx Spartan 6 block RAM
 * in the local tile memory.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module sram_sp_impl_xilinx_spartan6(/*AUTOARG*/
   // Outputs
   dout,
   // Inputs
   clk, rst, sel, addr, we, ce, din
   );

`include "optimsoc_functions.vh"

   // Memory size in bytes
   parameter MEM_SIZE  = 'hx;

   // Memory size in words
   localparam MEM_WORDS = MEM_SIZE >> 2;

   // 16 kbit BRAM is 2 kByte are 512 = 2^9 words
   // BLOCKNUM = ceil(MEM_WORDS/WORDS_PER_BLOCK)
   localparam BLOCKNUM = (MEM_WORDS >> 9) + |MEM_WORDS[8:0];

   /**
    * +-------+---------------+
    * | Block | Word in Block |
    * +-------+---------------+
    */

   // The width of the address bus
   localparam AW = clog2(MEM_WORDS);

   input          clk;
   input          rst;
   input [3:0]    sel;
   input [AW-1:0] addr;
   input          we;
   input          ce;
   input [31:0]   din;
   output [31:0]  dout;


   // The width of the block address
   localparam BLOCK_AW = clog2(BLOCKNUM);

   localparam WORD_AW = 9;

   wire [BLOCK_AW-1:0] block_addr;
   wire [WORD_AW-1:0]  word_addr;

   assign block_addr = addr >> WORD_AW;
   assign word_addr = addr[WORD_AW-1:0];

   wire [31:0] do_array [0:BLOCKNUM-1];

   assign dout = do_array[block_addr];

   generate
      genvar i;
      for (i=0;i<BLOCKNUM;i=i+1) begin
         RAMB16BWER
             #(.DATA_WIDTH_A (36))
         u_ram(.DOA    (do_array[i]),
               .DOPA   (),
               .DIA    (din),
               .DIPA   (4'h0),
               .ADDRA  ({word_addr,2'b00,3'b000}), // | word in block | byte in word | bit in byte |
               .WEA    (sel & {4{ce & we & (block_addr==i)}}),
               .ENA    (1'b1),
               .RSTA   (rst),
               .REGCEA (1'b0),
               .CLKA   (clk),
               .DOB    (),
               .DOPB   (),
               .DIB    (32'h0),
               .DIPB   (4'h0),
               .ADDRB  (14'h0),
               .WEB    (4'h0),
               .ENB    (1'b0),
               .RSTB   (1'b0),
               .REGCEB (1'b0),
               .CLKB   (1'b0));
      end
   endgenerate
endmodule
