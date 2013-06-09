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
 module cdc_fifo_mem(/*AUTOARG*/
   // Outputs
   rd_data,
   // Inputs
   wr_data, wr_clken, wr_clk, wr_full, wr_addr, rd_addr
   );

   parameter DW = 32;
   parameter ADDRSIZE = 4;

   localparam DEPTH = 1<<ADDRSIZE;

   input [DW-1:0] wr_data;
   input wr_clken, wr_clk, wr_full;
   input [ADDRSIZE-1:0] wr_addr;
   input [ADDRSIZE-1:0] rd_addr;

   output [DW-1:0] rd_data;

   reg [DW-1:0] mem [0:DEPTH-1];

   assign rd_data = mem[rd_addr];

   always @(posedge wr_clk) begin
      if (wr_clken && !wr_full) begin
         mem[wr_addr] <= wr_data;
      end
   end

endmodule
