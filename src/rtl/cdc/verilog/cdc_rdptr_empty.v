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
 * A clock-domain crossing FIFO
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Mark Sagi, mark.sagi@mytum.de
 */
module cdc_rdptr_empty(/*AUTOARG*/
   // Outputs
   rd_empty, rd_addr, rd_ptr,
   // Inputs
   rdq2_wrptr, rd_inc, rd_clk, rd_rst
   );

   parameter ADDRSIZE = 4;

   // synced n bit gray pointer for FIFO empty determination
   input [ADDRSIZE:0] rdq2_wrptr;
   input rd_inc, rd_clk, rd_rst;

   // '1' if FIFO empty
   output reg rd_empty;
   // n-1 bit binary pointer for memory addressing
   output [ADDRSIZE-1:0] rd_addr;
   // n bit gray pointer for FIFO full determination
   output reg [ADDRSIZE:0] rd_ptr;

   reg [ADDRSIZE:0]  rd_bin;
   wire [ADDRSIZE:0] rdgraynext, rdbinnext;

   // graystyle pointer
   always @(posedge rd_clk or negedge rd_rst) begin
      if (!rd_rst) begin
         {rd_bin, rd_ptr} <= 0;
      end else begin
         {rd_bin, rd_ptr} <= {rdbinnext, rdgraynext};
      end
   end

   assign rd_addr = rd_bin[ADDRSIZE-1:0];

   assign rdbinnext = rd_bin + (rd_inc & ~rd_empty);
   assign rdgraynext = (rdbinnext>>1) ^ rdbinnext;

   // FIFO empty if next rdptr == synchronized wrptr or on reset
   wire rdempty_val;
   assign rdempty_val = (rdgraynext == rdq2_wrptr);

   always @(posedge rd_clk or negedge rd_rst)
   begin
      if (!rd_rst) begin
         rd_empty <= 1'b1;
      end else begin
         rd_empty <= rdempty_val;
      end
   end

endmodule
