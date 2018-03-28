/* Copyright (c) 2012-2018 by the author(s)
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
 * A distributed memory system with all compute tiles
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module system_allct
   import optimsoc_config::*;
   import dii_package::dii_flit;
   #(parameter config_t CONFIG = 'x,
     parameter XDIM = 1,
     parameter YDIM = 1,
     localparam NODES = XDIM*YDIM)
  (
   input                 clk, rst,

   glip_channel c_glip_in,
   glip_channel c_glip_out,

   output [NODES*32-1:0] wb_ext_adr_i,
   output [NODES*1-1:0]  wb_ext_cyc_i,
   output [NODES*32-1:0] wb_ext_dat_i,
   output [NODES*4-1:0]  wb_ext_sel_i,
   output [NODES*1-1:0]  wb_ext_stb_i,
   output [NODES*1-1:0]  wb_ext_we_i,
   output [NODES*1-1:0]  wb_ext_cab_i,
   output [NODES*3-1:0]  wb_ext_cti_i,
   output [NODES*2-1:0]  wb_ext_bte_i,
   input [NODES*1-1:0]   wb_ext_ack_o,
   input [NODES*1-1:0]   wb_ext_rty_o,
   input [NODES*1-1:0]   wb_ext_err_o,
   input [NODES*32-1:0]  wb_ext_dat_o
   );

   dii_flit [1:0] debug_ring_in [0:NODES-1];
   dii_flit [1:0] debug_ring_out [0:NODES-1];
   logic [1:0] debug_ring_in_ready [0:NODES-1];
   logic [1:0] debug_ring_out_ready [0:NODES-1];

   dii_flit [1:0] debug_ring_out_last;
   logic [1:0] debug_ring_out_last_ready;

   logic       rst_sys, rst_cpu;

   debug_interface
      #(
         .SYSTEM_VENDOR_ID (2),
         .SYSTEM_DEVICE_ID (2),
         .NUM_MODULES (CONFIG.DEBUG_NUM_MODS),
         .MAX_PKT_LEN(CONFIG.DEBUG_MAX_PKT_LEN),
         .SUBNET_BITS(CONFIG.DEBUG_SUBNET_BITS),
         .LOCAL_SUBNET(CONFIG.DEBUG_LOCAL_SUBNET),
         .DEBUG_ROUTER_BUFFER_SIZE(CONFIG.DEBUG_ROUTER_BUFFER_SIZE)
      )
      u_debuginterface
        (
         .clk            (clk),
         .rst            (rst),
         .sys_rst        (rst_sys),
         .cpu_rst        (rst_cpu),
         .glip_in        (c_glip_in),
         .glip_out       (c_glip_out),
         .ring_out       (debug_ring_in[0]),
         .ring_out_ready (debug_ring_in_ready[0]),
         .ring_in        (debug_ring_out_last),
         .ring_in_ready  (debug_ring_out_last_ready)
      );

   localparam FLIT_WIDTH = CONFIG.NOC_FLIT_WIDTH;
   localparam CHANNELS = CONFIG.NOC_CHANNELS;

   // Flits from NoC->tiles
   wire [NODES-1:0][CHANNELS-1:0][FLIT_WIDTH-1:0] link_in_flit;
   wire [NODES-1:0][CHANNELS-1:0]                 link_in_last;
   wire [NODES-1:0][CHANNELS-1:0]                 link_in_valid;
   wire [NODES-1:0][CHANNELS-1:0]                 link_in_ready;

   // Flits from tiles->NoC
   wire [NODES-1:0][CHANNELS-1:0][FLIT_WIDTH-1:0] link_out_flit;
   wire [NODES-1:0][CHANNELS-1:0]                 link_out_last;
   wire [NODES-1:0][CHANNELS-1:0]                 link_out_valid;
   wire [NODES-1:0][CHANNELS-1:0]                 link_out_ready;

   noc_mesh
     #(.FLIT_WIDTH (FLIT_WIDTH), .X (XDIM), .Y (YDIM),
       .CHANNELS (CHANNELS), .ENABLE_VCHANNELS(CONFIG.NOC_ENABLE_VCHANNELS))
   u_noc
     (.*,
      .in_flit   (link_out_flit),
      .in_last   (link_out_last),
      .in_valid  (link_out_valid),
      .in_ready  (link_out_ready),
      .out_flit  (link_in_flit),
      .out_last  (link_in_last),
      .out_valid (link_in_valid),
      .out_ready (link_in_ready)
      );

   genvar x,y;
   generate
      for (y=0; y<YDIM; y=y+1) begin : gen_cty
         for (x=0; x<XDIM; x=x+1) begin : gen_ctx
            localparam integer n = y * XDIM + x;

            compute_tile_dm
                #(.CONFIG (CONFIG),
                  .ID(n),
               .COREBASE(n*CONFIG.CORES_PER_TILE),
               .DEBUG_BASEID((CONFIG.DEBUG_LOCAL_SUBNET << (16 - CONFIG.DEBUG_SUBNET_BITS))
                             + 1 + (n*CONFIG.DEBUG_MODS_PER_TILE)))
             u_ct(.clk                        (clk),
                  .rst_cpu                    (rst_cpu),
                  .rst_sys                    (rst_sys),
                  .rst_dbg                    (rst),
                  .debug_ring_in              (debug_ring_in[n]),
                  .debug_ring_in_ready        (debug_ring_in_ready[n]),
                  .debug_ring_out             (debug_ring_out[n]),
                  .debug_ring_out_ready       (debug_ring_out_ready[n]),

                  .wb_ext_ack_o               (wb_ext_ack_o[n]),
                  .wb_ext_rty_o               (wb_ext_rty_o[n]),
                  .wb_ext_err_o               (wb_ext_err_o[n]),
                  .wb_ext_dat_o               (wb_ext_dat_o[(n+1)*32-1:n*32]),
                  .wb_ext_adr_i               (wb_ext_adr_i[(n+1)*32-1:n*32]),
                  .wb_ext_cyc_i               (wb_ext_cyc_i[n]),
                  .wb_ext_dat_i               (wb_ext_dat_i[(n+1)*32-1:n*32]),
                  .wb_ext_sel_i               (wb_ext_sel_i[(n+1)*4-1:n*4]),
                  .wb_ext_stb_i               (wb_ext_stb_i[n]),
                  .wb_ext_we_i                (wb_ext_we_i[n]),
                  .wb_ext_cab_i               (wb_ext_cab_i[n]),
                  .wb_ext_cti_i               (wb_ext_cti_i[(n+1)*3-1:n*3]),
                  .wb_ext_bte_i               (wb_ext_bte_i[(n+1)*2-1:n*2]),

                  .noc_in_ready               (link_in_ready[n]),
                  .noc_out_flit               (link_out_flit[n]),
                  .noc_out_last               (link_out_last[n]),
                  .noc_out_valid              (link_out_valid[n]),

                  .noc_in_flit                (link_in_flit[n]),
                  .noc_in_last                (link_in_last[n]),
                  .noc_in_valid               (link_in_valid[n]),
                  .noc_out_ready              (link_out_ready[n]));

            // We are routing the debug in a meander
            if (n > 0) begin
               if (y % 2 == 0) begin
                  if (x == 0) begin
                     assign debug_ring_in[n] = debug_ring_out[(y-1)*XDIM];
                     assign debug_ring_out_ready[(y-1)*XDIM] = debug_ring_in_ready[n];
                  end else begin
                     assign debug_ring_in[n] = debug_ring_out[n-1];
                     assign debug_ring_out_ready[n-1] = debug_ring_in_ready[n];
                  end
               end else begin
                  if (x == XDIM - 1) begin
                     assign debug_ring_in[n] = debug_ring_out[n-XDIM];
                     assign debug_ring_out_ready[n-XDIM] = debug_ring_in_ready[n];
                  end else begin
                     assign debug_ring_in[n] = debug_ring_out[n+1];
                     assign debug_ring_out_ready[n+1] = debug_ring_in_ready[n];
                  end
               end
            end // if (n > 0)
         end // block: gen_ctx
      end // block: gen_cty

      if (YDIM % 2 == 0) begin
         assign debug_ring_out_last = debug_ring_out[NODES-XDIM];
         assign debug_ring_out_ready[NODES-XDIM] = debug_ring_out_last_ready;
      end else begin
         assign debug_ring_out_last = debug_ring_out[NODES-1];
         assign debug_ring_out_ready[NODES-1] = debug_ring_out_last_ready;
      end
   endgenerate

endmodule
