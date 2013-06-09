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
module cdc_fifo(/*AUTOARG*/
   // Outputs
   wr_full, rd_empty, rd_data,
   // Inputs
   wr_clk, rd_clk, wr_rst, rd_rst, rd_en, wr_en, wr_data
   );

   parameter DW = 32;
   parameter ADDRSIZE = 4;

   // inputs
   input wr_clk;
   input rd_clk;

   input wr_rst;
   input rd_rst;

   input rd_en;
   input wr_en;

   input [DW-1:0] wr_data;

   // outputs
   output          wr_full;
   output          rd_empty;
   output [DW-1:0] rd_data;

   wire [ADDRSIZE-1:0] wr_addr, rd_addr;
   wire [ADDRSIZE:0]   wr_ptr, rd_ptr, wrq2_rdptr, rdq2_wrptr;

   cdc_sync_rd2wr
      #(.ADDRSIZE(ADDRSIZE))
      u_sync_rd2wr(.wrq2_rdptr(wrq2_rdptr[ADDRSIZE:0]),
                   .rd_ptr(rd_ptr[ADDRSIZE:0]),
                   .wr_clk(wr_clk),
                   .wr_rst(wr_rst));

   cdc_sync_wr2rd
      #(.ADDRSIZE(ADDRSIZE))
      u_sync_wr2rd(.rdq2_wrptr(rdq2_wrptr[ADDRSIZE:0]),
                   .wr_ptr(wr_ptr[ADDRSIZE:0]),
                   .rd_clk(rd_clk),
                   .rd_rst(rd_rst));

   cdc_fifo_mem
      #(.DW(DW),
        .ADDRSIZE(ADDRSIZE))
      u_fifo_mem(.rd_data(rd_data[DW-1:0]),
                 .wr_data(wr_data[DW-1:0]),
                 .wr_addr(wr_addr[ADDRSIZE-1:0]),
                 .rd_addr(rd_addr[ADDRSIZE-1:0]),
                 .wr_clken(wr_en),
                 .wr_full(wr_full),
                 .wr_clk(wr_clk));

   cdc_rdptr_empty
      #(.ADDRSIZE(ADDRSIZE))
      u_rdptr_empty(.rd_empty(rd_empty),
                    .rd_addr(rd_addr[ADDRSIZE-1:0]),
                    .rd_ptr(rd_ptr[ADDRSIZE:0]),
                    .rdq2_wrptr(rdq2_wrptr[ADDRSIZE:0]),
                    .rd_inc(rd_en),
                    .rd_clk(rd_clk),
                    .rd_rst(rd_rst));

   cdc_wrptr_full
      #(.ADDRSIZE(ADDRSIZE))
      u_wrptr_full(.wr_full(wr_full),
                   .wr_addr(wr_addr[ADDRSIZE-1:0]),
                   .wr_ptr(wr_ptr[ADDRSIZE:0]),
                   .wrq2_rdptr(wrq2_rdptr[ADDRSIZE:0]),
                   .wr_inc(wr_en),
                   .wr_clk(wr_clk),
                   .wr_rst(wr_rst));

endmodule
