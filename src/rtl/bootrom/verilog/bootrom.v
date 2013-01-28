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
 * This is a simple ROM used at boot time in distributed memory systems
 * or similar.
 * 
 * (c) 2012 by the author(s)
 * 
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

module bootrom(/*AUTOARG*/
   // Outputs
   wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o,
   // Inputs
   clk, rst, wb_adr_i, wb_dat_i, wb_cyc_i, wb_stb_i, wb_sel_i
   );

   input clk;
   input rst;

   input [31:0] wb_adr_i;
   input [31:0] wb_dat_i;
   input        wb_cyc_i;
   input        wb_stb_i;
   input [3:0]  wb_sel_i;
   output reg [31:0] wb_dat_o;
   output reg    wb_ack_o;
   output        wb_err_o;
   output        wb_rty_o;

   always @(posedge clk) begin
      wb_ack_o <= wb_stb_i & ~wb_ack_o;
   end

   assign wb_err_o = 1'b0;
   assign wb_rty_o = 1'b0;

   always @(*) begin
      case(wb_adr_i[7:2])
`include "bootrom_code.v"
        default: wb_dat_o = 32'hx;
      endcase
   end

endmodule // bootrom
