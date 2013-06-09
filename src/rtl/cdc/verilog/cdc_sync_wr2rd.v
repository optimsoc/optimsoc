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
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Mark Sagi, mark.sagi@mytum.de
 */
 module cdc_sync_wr2rd(/*AUTOARG*/
   // Outputs
   rdq2_wrptr,
   // Inputs
   rd_clk, rd_rst, wr_ptr
   );

   parameter ADDRSIZE = 4;

   input rd_clk, rd_rst;
   input [ADDRSIZE:0] wr_ptr;

   output reg [ADDRSIZE:0] rdq2_wrptr;

   // at least two registers are needed to minimize metastability
   reg [ADDRSIZE:0] rdq1_wrptr;

   always @(posedge rd_clk or negedge rd_rst) begin
      if (!rd_rst) begin
         rdq2_wrptr <= 0;
         rdq1_wrptr <= 0;
      end else begin
         rdq2_wrptr <= rdq1_wrptr;
         rdq1_wrptr <= wr_ptr;
      end
   end

endmodule
