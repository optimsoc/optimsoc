/* Copyright (c) 2013-2018 by the author(s)
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
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
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
   input             clk,
   input             rst,

   input [15:0]      adr,
   input             en,
   input             we,
   input [31:0]      data_i,

   output reg [31:0] data,
   output reg        ack,
   output            rty,
   output reg        err
   );

   reg [31:0] seed = 0;

`ifdef verilator
   initial begin
      seed = $random();
   end
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

   localparam REG_CTLIST = 10'h80;

   localparam REGBIT_CONF_MPSIMPLE = 0;
   localparam REGBIT_CONF_DMA      = 1;

   assign rty = 1'b0;

   always @(posedge clk) begin
      ack <= 0;
      err <= 0;

      if (en) begin
         if (we) begin
            err <= 1;
         end else begin
            ack <= ~ack;
            if (adr[11:9] == REG_CTLIST[9:7]) begin
               data <= CONFIG.CTLIST[adr[6:2]*32 +: 32];
               ack <= 1;
            end else begin
              case (adr[11:2])
                REG_TILEID: begin
                   data <= TILEID;
                end
                REG_NUMTILES: begin
                   data <= CONFIG.NUMTILES;
                end
                REG_CONF: begin
                 data <= 32'h0000_0000;
                 data[REGBIT_CONF_MPSIMPLE] <= CONFIG.NA_ENABLE_MPSIMPLE;
                 data[REGBIT_CONF_DMA] <= CONFIG.NA_ENABLE_DMA;
                end
                REG_COREBASE: begin
                   data <= 32'(COREBASE);
                end
                REG_DOMAIN_NUMCORES: begin
                   data <= CONFIG.CORES_PER_TILE;
                end
                REG_GMEM_SIZE: begin
                   data <= CONFIG.GMEM_SIZE;
                end
                REG_GMEM_TILE: begin
                   data <= CONFIG.GMEM_TILE;
                end
                REG_LMEM_SIZE: begin
                   data <= CONFIG.LMEM_SIZE;
                end
                REG_NUMCTS: begin
                   data <= CONFIG.NUMCTS;
                end
                REG_SEED: begin
                   data <= seed;
                end
                default: begin
                   ack <= 0;
                   err <= 1;
                   data <= 32'hx;
                end
              endcase // case (adr[11:2])
            end // else: !if(adr[11:9] == REG_CTLIST[9:7])
         end
      end
   end // always @ (posedge clk)
endmodule // networkadapter_conf
