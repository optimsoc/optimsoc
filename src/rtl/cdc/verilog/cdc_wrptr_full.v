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
module cdc_wrptr_full(/*AUTOARG*/
   // Outputs
   wr_full, wr_addr, wr_ptr,
   // Inputs
   wrq2_rdptr, wr_inc, wr_clk, wr_rst
   );

   parameter ADDRSIZE = 4;

   input [ADDRSIZE:0] wrq2_rdptr;
   input wr_inc, wr_clk, wr_rst;

   output reg              wr_full;
   output [ADDRSIZE-1:0]   wr_addr; //memory write-address pointer
   output reg [ADDRSIZE:0] wr_ptr;

   reg [ADDRSIZE:0] wbin;
   wire [ADDRSIZE:0] wgraynext, wbinnext;

   //Gray pointer
   always @(posedge wr_clk or negedge wr_rst) begin
      if (!wr_rst) begin
         {wbin, wr_ptr} <= 0;
      end else begin
         {wbin, wr_ptr} <= {wbinnext, wgraynext};
      end
   end

   assign wr_addr = wbin[ADDRSIZE-1:0];

   assign wbinnext = wbin + (wr_inc & ~wr_full);
   assign wgraynext = (wbinnext>>1) ^ wbinnext;

   // Full test:
   // If MSB and MSB-1 bit of wgraynext pointer != wrq2_rdptr
   // AND MSB-2:0 bits of wgraynext pointer == wrq2_rdptr
   // => FIFO Full
   wire wfull_val;
   assign wfull_val = (wgraynext == {~wrq2_rdptr[ADDRSIZE:ADDRSIZE-1],
                                     wrq2_rdptr[ADDRSIZE-2:0]});

   always @(posedge wr_clk or negedge wr_rst) begin
      if (!wr_rst) begin
         wr_full <= 1'b0;
      end else begin
         wr_full <= wfull_val;
      end
   end

endmodule
