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
 * This is the actual memory element implemented as Xilinx Virtex 5 block RAM
 * in the local tile memory.
 * 
 * (c) 2012-2013 by the author(s)
 * 
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

module ct_ram_xilinxbram_spartan6(/*AUTOARG*/
   // Outputs
   dat_o,
   // Inputs
   clk_i, rst_i, sel_i, adr_i, we_i, dat_i
   );

   parameter dw = 32;

   parameter aw = 23;
   parameter mem_size  = 8388608;

   localparam blocknum = mem_size >> 11; // 2^11 bytes per 8kB BRAM block

   input          clk_i;
   input          rst_i;
   input [3:0]    sel_i;
   input [aw-1:0] adr_i;
   input          we_i;
   input [31:0]   dat_i;
   output [31:0]  dat_o;
   
   wire [31:0]    do_array [0:blocknum-1];
   
   assign dat_o = do_array[adr_i[aw-1:8]];

   generate
      genvar i;
      for (i=0;i<blocknum;i=i+1) begin
         BRAM_SINGLE_MACRO
             #(.BRAM_SIZE("18Kb"),
               .DO_REG   (0),
               .DEVICE   ("SPARTAN6"),
               .READ_WIDTH (32),
               .WRITE_WIDTH (32))
         u_ram(.DO   (do_array[i]),
               .DI   (dat_i),
               .ADDR (adr_i[9:2]),
               .WE   (sel_i & {4{we_i & (adr_i[aw-1:8]==i)}}),
               .EN   (1'b1),
               .RST  (rst_i),
               .REGCE (1'b0),
               .CLK  (clk_i));
      end
   endgenerate
endmodule
