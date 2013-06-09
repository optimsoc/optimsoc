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
 module cdc_sync_rd2wr(/*AUTOARG*/
   // Outputs
   wrq2_rdptr,
   // Inputs
   rd_ptr, wr_clk, wr_rst
   );

   parameter ADDRSIZE = 4;

   input wr_clk, wr_rst;
   input [ADDRSIZE:0] rd_ptr;

   output [ADDRSIZE:0] wrq2_rdptr;

   reg [ADDRSIZE:0] wrq2_rdptr;
   reg [ADDRSIZE:0] wrq1_rdptr;
   wire [ADDRSIZE:0] rd_ptr;
   wire wr_clk, wr_rst;

   always @(posedge wr_clk or negedge wr_rst) begin
      if (!wr_rst) begin
         wrq2_rdptr <= 0;
         wrq1_rdptr <= 0;
      end else begin
         {wrq2_rdptr, wrq1_rdptr} <=  {wrq1_rdptr,rd_ptr};
      end
   end

endmodule
