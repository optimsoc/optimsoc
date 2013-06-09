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
 * This is the actual memory element implemented plainly in the local
 * tile memory.
 * 
 * (c) 2012-2013 by the author(s)
 * 
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

module bram_infer(/*AUTOARG*/
   // Outputs
   dat_o,
   // Inputs
   clk_i, sel_i, adr_i_r, adr_i, we_i, dat_i
   );

   parameter dw = 32;

   parameter aw = 23;
   /* memory size in bytes */
   parameter mem_size  = 'hx;
   /* memory size in words */
   localparam mem_size_words = mem_size * 8 / dw;

   input clk_i;
   input [3:0] sel_i;
   input [aw-3:0] adr_i_r;
   input [aw-3:0] adr_i;
   input          we_i;
   input [31:0]   dat_i;
   output [31:0]  dat_o;

   bram_256kB
     u_bram(
            .clka(clk_i), // input clka
            .wea(sel_i&{4{we_i}}), // input [3 : 0] wea
            .addra(adr_i[14:0]), // input [14 : 0] addra
            .dina(dat_i), // input [31 : 0] dina
            .douta(dat_o) // output [31 : 0] douta
            );

   
/* -----\/----- EXCLUDED -----\/-----
   reg [7:0]         mem0 [0:mem_size_words-1] /-*synthesis syn_ramstyle = "no_rw_check" *-/;
   reg [15:8]        mem1 [0:mem_size_words-1] /-*synthesis syn_ramstyle = "no_rw_check" *-/;
   reg [23:16]       mem2 [0:mem_size_words-1] /-*synthesis syn_ramstyle = "no_rw_check" *-/;
   reg [31:24]       mem3 [0:mem_size_words-1] /-*synthesis syn_ramstyle = "no_rw_check" *-/;

   assign dat_o = {mem3[adr_i_r],mem2[adr_i_r],mem1[adr_i_r],mem0[adr_i_r]};
   
   // Write logic
   always @ (posedge clk_i) begin
      if (we_i & sel_i[3]) 
         mem3[adr_i_r] <= dat_i[31:24];
      if (we_i & sel_i[2]) 
         mem2[adr_i_r] <= dat_i[23:16];
      if (we_i & sel_i[1]) 
         mem1[adr_i_r] <= dat_i[15:8];
      if (we_i & sel_i[0]) 
         mem0[adr_i_r] <= dat_i[7:0];
   end
 -----/\----- EXCLUDED -----/\----- */

endmodule