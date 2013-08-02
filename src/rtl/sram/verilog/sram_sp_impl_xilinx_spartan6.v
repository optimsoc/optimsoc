/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * This is the actual memory element implemented as Xilinx Spartan 6 block RAM
 * in the local tile memory.
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
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

   wire [31:0]    do_array [0:BLOCKNUM-1];

   assign dout = do_array[block_addr];

   generate
      genvar i;
      for (i=0;i<BLOCKNUM;i=i+1) begin
         RAMB16BWER
             #(.DATA_WIDTH_A (36))
         u_ram(.DOA    (do_array[i]),
               .DIA    (din),
               .DIPA   (4'h0),
               .ADDRA  ({word_addr,2'b00,3'b000}), // | word in block | byte in word | bit in byte |
               .WEA    (sel & {4{ce & we & (block_addr==i)}}),
               .ENA    (1'b1),
               .RSTA   (rst),
               .REGCEA (1'b0),
               .CLKA   (clk),
               .DOB    (),
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
