/* Copyright (c) 2013-2015 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */


/*
 * BASE+
 * +----------------------------+
 * | 0x0 R: tile id             |
 * +----------------------------+
 * | 0x4 R: number of tiles     |
 * +----------------------------+
 * | 0x8 R:                     |
 * +----------------------------+
 * | 0xc R: configuration       |
 * |        bit 0: mp_simple    |
 * |        bit 1: dma          |
 * +----------------------------+
 * | 0x10 R: core base id       |
 * +----------------------------+
 * | 0x14 R:                    |
 * +----------------------------+
 * | 0x18 R: domain core number |
 * +----------------------------+
 * | 0x1c R: global memory size |
 * +----------------------------+
 * | 0x20 R: global memory tile |
 * +----------------------------+
 * | 0x24 R: local memory size  |
 * +----------------------------+
 * | 0x28 R: number of compute  |
 * |         tiles              |
 * +----------------------------+
 * | 0x2c R: read a random seed |
 * +----------------------------+
 * .
 * .
 * +----------------------------+
 * | 0x200 R: list of compute   |
 * |          tiles             |
 * +----------------------------+
 */

module networkadapter_conf
  import optimsoc_config::*;
  #(parameter config_t CONFIG = 'x,
    parameter TILEID = 'x,
    parameter COREBASE = 'x
    )
  (
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

   reg [31:0] seed;

`ifdef verilator
   initial begin
      seed = $random();
   end
`else
   assign seed = 32'h0;
`endif

   localparam REG_TILEID   = 0;
   localparam REG_NUMTILES = 1;
   localparam REG_CONF   = 3;
   localparam REG_COREBASE = 4;
   localparam REG_DOMAIN_NUMCORES = 6;
   localparam REG_GMEM_SIZE = 7;
   localparam REG_GMEM_TILE = 8;
   localparam REG_LMEM_SIZE = 9;
   localparam REG_NUMCTS = 10;
   localparam REG_SEED = 11;

   localparam REG_CDC      = 10'h40;
   localparam REG_CDC_DYN  = 10'h41;
   localparam REG_CDC_CONF = 10'h42;

   localparam REG_CTLIST = 10'h80;

   localparam REGBIT_CONF_MPSIMPLE = 0;
   localparam REGBIT_CONF_DMA      = 1;

   input clk;
   input rst;

   input [15:0]      adr;
   input             we;
   input [31:0]      data_i;

   output reg [31:0] data;
   output            ack;
   output            rty;
   output            err;

   assign ack = ~|adr[15:12];
   assign err = ~ack;
   assign rty = 1'b0;

   // CDC configuration register
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
   output reg [2:0]         cdc_conf;
   output reg               cdc_enable;
 `endif
`endif

   wire [15:0]              ctlist_vector[0:63];

   genvar                   i;
   generate
      for (i = 0; i < 64; i = i + 1) begin : gen_ctlist_vector // array is indexed by the desired destination
         if (i < CONFIG.NUMCTS) begin
            // The entries of the ctlist_vector array are subranges from the parameter, where
            // the indexing is reversed (num_dests-i-1)!
            assign ctlist_vector[CONFIG.NUMCTS - i - 1] = CONFIG.CTLIST[i];
         end else begin
            // All other entries are unused and zero'ed.
            assign ctlist_vector[i] = 16'h0;
         end
      end
   endgenerate


   always @(*) begin
      if (adr[11:9] == REG_CTLIST[9:7]) begin
         if (adr[1]) begin
            data = {16'h0,ctlist_vector[adr[6:1]]};
         end else begin
            data = {ctlist_vector[adr[6:1]],16'h0};
         end
      end else begin
         case (adr[11:2])
           REG_TILEID: begin
              data = TILEID;
           end
           REG_NUMTILES: begin
              data = CONFIG.NUMTILES;
           end
           REG_CONF: begin
              data = 32'h0000_0000;
              data[REGBIT_CONF_MPSIMPLE] = CONFIG.NA_ENABLE_MPSIMPLE;
              data[REGBIT_CONF_DMA] = CONFIG.NA_ENABLE_DMA;
           end
           REG_COREBASE: begin
              data = 32'(COREBASE);
           end
           REG_DOMAIN_NUMCORES: begin
              data = CONFIG.CORES_PER_TILE;
           end
           REG_GMEM_SIZE: begin
              data = CONFIG.GMEM_SIZE;
           end
           REG_GMEM_TILE: begin
              data = CONFIG.GMEM_TILE;
           end
           REG_LMEM_SIZE: begin
              data = CONFIG.LMEM_SIZE;
           end
           REG_NUMCTS: begin
              data = CONFIG.NUMCTS;
           end
           REG_SEED: begin
              data = seed;
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
         endcase // case (adr[11:2])
      end
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
