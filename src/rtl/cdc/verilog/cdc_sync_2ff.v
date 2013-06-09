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
 * 2-FlipFlop Synchronizer
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Mark Sagi, mark.sagi@mytum.de
 */
module cdc_sync_2ff (/*AUTOARG*/
   // Outputs
   rd_p,
   // Inputs
   rd_clk, wr_p, rst
   );

   input rd_clk;
   input wr_p;
   input rst;

   output rd_p;

   reg ff1;
   reg ff2;

   always @(posedge rd_clk or posedge rst) begin
      if (rst == 1'b1) begin
         {ff1, ff2}  <= {1'b0, 1'b0};
      end else begin
         {ff1, ff2}  <= {wr_p, ff1};
      end
   end

   assign rd_p = ff2;
endmodule
