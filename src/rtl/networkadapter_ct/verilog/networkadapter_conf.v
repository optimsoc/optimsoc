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
 * This modules provides the configuration information of the network
 * adapter to the software via memory mapped registers.
 * 
 * (c) 2012-2013 by the author(s)
 * 
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */


/*
 * BASE+
 * +-------------------------+
 * | 0x0 R: tile id          |
 * +-------------------------+
 * | 0x4 R: NoC x-dimension  |
 * +-------------------------+
 * | 0x8 R: NoC y-dimension  |
 * +-------------------------+
 * | 0xc R: configuration    |
 * |        bit 0: mp_simple |
 * |        bit 1: dma       |
 * +-------------------------+
 * 
 */

module networkadapter_conf(/*AUTOARG*/
   // Outputs
   data, ack, rty, err,
   // Inputs
   adr
   );

   parameter tileid = 0;
   parameter mp_simple_present = 0;
   parameter dma_present = 0;

   parameter noc_xdim = 4;
   parameter noc_ydim = 4;
   parameter num_dests = $clog2(noc_xdim*noc_ydim);

   parameter REG_TILEID = 0;
   parameter REG_XDIM   = 1;
   parameter REG_YDIM   = 2;
   parameter REG_CONF   = 3;
   parameter CONF_MPSIMPLE = 0;
   parameter CONF_DMA      = 1;
   
   input [15:0]      adr;
   output reg [31:0] data;
   output            ack;
   output            rty;
   output            err;

   assign ack = ~|adr[15:4] & ~|adr[1:0];
   assign err = ~ack;
   assign rty = 1'b0;
   
   always @(*) begin
      case (adr[3:2])
        REG_TILEID: begin
           data = tileid;
        end
        REG_XDIM: begin
           data = noc_xdim;
        end
        REG_YDIM: begin
           data = noc_ydim;
        end
        REG_CONF: begin
           data = 32'h0000_0000;
           data[CONF_MPSIMPLE] = mp_simple_present;
           data[CONF_DMA] = dma_present;
        end
      endcase
   end
   

endmodule // networkadapter_conf
