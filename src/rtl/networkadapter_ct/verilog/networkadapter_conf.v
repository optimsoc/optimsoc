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
 * |
 * .
 * .
 * |
 * +
 * | 0x200 
 */

module networkadapter_conf(
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
                           cdc_conf, cdc_enable,
 `endif
`endif
                           /*AUTOARG*/
   // Outputs
   data, ack, rty, err,
   // Inputs
   clk, rst, adr, we, data_i
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

   parameter REG_CDC      = 10'h80;
   parameter REG_CDC_DYN  = 10'h81;
   parameter REG_CDC_CONF = 10'h82;
   
   parameter CONF_MPSIMPLE = 0;
   parameter CONF_DMA      = 1;

   input clk;
   input rst;
   
   input [15:0]      adr;
   input             we;
   input [31:0]      data_i;
   
   output reg [31:0] data;
   output            ack;
   output            rty;
   output            err;

   assign ack = ~|adr[15:12] & ~|adr[1:0];
   assign err = ~ack;
   assign rty = 1'b0;

   // CDC configuration register
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
   output reg [2:0]         cdc_conf;
   output reg               cdc_enable;
 `endif
`endif 
  
   always @(*) begin
      case (adr[11:2])
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

        REG_CDC: begin
`ifdef OPTIMSOC_CLOCKDOMAINS
           data = 32'b1;
`else
           data = 32'b0;
`endif
        end
        REG_CDC_DYN: begin
`ifdef OPTIMSOC_CDC_DYNAMIC
           data = 32'b1;
`else
           data = 32'b0;
`endif
        end
        REG_CDC_CONF: begin
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC       
           data = cdc_conf;
 `else
           data = 32'hx;
 `endif
`else
           data = 32'hx;
`endif
        end            

        default: begin
           data = 32'hx;
        end
      endcase
   end

`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
   always @(posedge clk) begin
      if (rst) begin
         cdc_conf <= `OPTIMSOC_CDC_DYN_DEFAULT;
         cdc_enable <= 0;
      end else begin
         if ((adr[11:2]==REG_CDC_CONF) && we) begin
            cdc_conf <= data_i[2:0];
            cdc_enable <= 1;
         end else begin
            cdc_conf <= cdc_conf;
            cdc_enable <= 0;
         end
      end
   end // always @ (posedge clk)
 `endif //  `ifdef OPTIMSOC_CDC_DYNAMIC
`endif //  `ifdef OPTIMSOC_CLOCKDOMAINS
   
endmodule // networkadapter_conf