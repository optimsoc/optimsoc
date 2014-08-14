/* Copyright (c) 2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * =============================================================================
 * 
 * This modules provides the configuration information of the network
 * adapter to the software via memory mapped registers.
 * 
 * (c) 2012-2013 by the author(s)
 * 
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */


/*
 * BASE+
 * +----------------------------+
 * | 0x0 R: tile id             |
 * +----------------------------+
 * | 0x4 R: NoC x-dimension     |
 * +----------------------------+
 * | 0x8 R: NoC y-dimension     |
 * +----------------------------+
 * | 0xc R: configuration       |
 * |        bit 0: mp_simple    |
 * |        bit 1: dma          |
 * +----------------------------+
 * | 0x10 R: core base id       |
 * +----------------------------+
 * | 0x14 R: tile core number   |
 * +----------------------------+
 * | 0x18 R: domain core number |
 * +----------------------------+
 * | 0x1c R: global memory size |
 * +----------------------------+
 * | 0x20 R: global memory tile |
 * +----------------------------+
 * |
 * .
 * .
 * |
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

   parameter COREBASE = 0;
   parameter NUMCORES = 32'hx;
   parameter DOMAIN_NUMCORES = 32'hx;
   parameter GLOBAL_MEMORY_SIZE = 32'h0;
   parameter GLOBAL_MEMORY_TILE = 32'hx;

   parameter REG_TILEID = 0;
   parameter REG_XDIM   = 1;
   parameter REG_YDIM   = 2;
   parameter REG_CONF   = 3;
   localparam REG_COREBASE = 4;
   localparam REG_NUMCORES = 5;
   localparam REG_DOMAIN_NUMCORES = 6;
   localparam REG_GMEM_SIZE = 7;
   localparam REG_GMEM_TILE = 8;  
   
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
        REG_COREBASE: begin
           data = COREBASE;
        end
        REG_NUMCORES: begin
           data = NUMCORES;
        end
        REG_DOMAIN_NUMCORES: begin
           data = DOMAIN_NUMCORES;
        end
        REG_GMEM_SIZE: begin
           data = GLOBAL_MEMORY_SIZE;
        end
        REG_GMEM_TILE: begin
           data = GLOBAL_MEMORY_TILE;
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
