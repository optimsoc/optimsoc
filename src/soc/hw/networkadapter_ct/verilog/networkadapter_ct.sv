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
 * This is the network adapter for compute tiles. It is configurable to
 * contain different elements, e.g. message passing or DMA.
 *
 * (c) 2012-2016 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

`include "lisnoc_def.vh"
import optimsoc::*;

module networkadapter_ct
  #(
    parameter config_t CONFIG = 'x,

    parameter TILEID = 'x,
    parameter COREBASE = 'x,

    parameter ENABLE_MPSIMPLE = 1,
    parameter ENABLE_DMA      = 1,

    parameter DMA_GENERATE_INTERRUPT = 1,
    parameter DMA_ENTRIES = 4
    )
   (
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
                         cdc_conf, cdc_enable,
 `endif
`endif
                         /*AUTOARG*/
   // Outputs
   noc_in_ready, noc_out_flit, noc_out_valid, wbm_adr_o, wbm_cyc_o,
   wbm_dat_o, wbm_sel_o, wbm_stb_o, wbm_we_o, wbm_cab_o, wbm_cti_o,
   wbm_bte_o, wbs_ack_o, wbs_rty_o, wbs_err_o, wbs_dat_o, irq,
   // Inputs
   clk, rst, noc_in_flit, noc_in_valid, noc_out_ready, wbm_ack_i,
   wbm_rty_i, wbm_err_i, wbm_dat_i, wbs_adr_i, wbs_cyc_i, wbs_dat_i,
   wbs_sel_i, wbs_stb_i, wbs_we_i, wbs_cab_i, wbs_cti_i, wbs_bte_i
   );

   input clk, rst;

   input [CONFIG.NOC_FLIT_WIDTH-1:0]  noc_in_flit;
   input [CONFIG.NOC_VCHANNELS-1:0]   noc_in_valid;
   output [CONFIG.NOC_VCHANNELS-1:0]  noc_in_ready;
   output [CONFIG.NOC_FLIT_WIDTH-1:0] noc_out_flit;
   output [CONFIG.NOC_VCHANNELS-1:0]  noc_out_valid;
   input [CONFIG.NOC_VCHANNELS-1:0]   noc_out_ready;

   output [31:0]                      wbm_adr_o;
   output                             wbm_cyc_o;
   output [31:0]                      wbm_dat_o;
   output [3:0]                       wbm_sel_o;
   output                             wbm_stb_o;
   output                             wbm_we_o;
   output                             wbm_cab_o;
   output [2:0]                       wbm_cti_o;
   output [1:0]                       wbm_bte_o;
   input                              wbm_ack_i;
   input                              wbm_rty_i;
   input                              wbm_err_i;
   input [31:0]                       wbm_dat_i;

   input [31:0]                       wbs_adr_i;
   input                              wbs_cyc_i;
   input [31:0]                       wbs_dat_i;
   input [3:0]                        wbs_sel_i;
   input                              wbs_stb_i;
   input                              wbs_we_i;
   input                              wbs_cab_i;
   input [2:0]                        wbs_cti_i;
   input [1:0]                        wbs_bte_i;
   output                             wbs_ack_o;
   output                             wbs_rty_o;
   output                             wbs_err_o;
   output [31:0]                      wbs_dat_o;

   /*
    *  +---+-..-+----+
    *  |   dma  | mp |
    *  +---+-..-+----+
    * dma_entries 1  (0)
    */
   output [DMA_ENTRIES:0]          irq;

`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
   output [2:0]          cdc_conf;
   output                cdc_enable;
 `endif
`endif

   wire [CONFIG.NOC_VCHANNELS-1:0] mod_out_ready;
   wire [CONFIG.NOC_VCHANNELS-1:0] mod_out_valid;
   wire [CONFIG.NOC_FLIT_WIDTH-1:0] mod_out_flit[0:CONFIG.NOC_VCHANNELS-1];
   wire [CONFIG.NOC_VCHANNELS-1:0] mod_in_ready;
   wire [CONFIG.NOC_VCHANNELS-1:0] mod_in_valid;
   wire [CONFIG.NOC_FLIT_WIDTH-1:0] mod_in_flit[0:CONFIG.NOC_VCHANNELS-1];

   wire [CONFIG.NOC_FLIT_WIDTH*CONFIG.NOC_VCHANNELS-1:0] mod_out_flit_flat;

   assign mod_in_valid = noc_in_valid;
   assign noc_in_ready = mod_in_ready;

   generate
      genvar              v;
      for (v=0;v<CONFIG.NOC_VCHANNELS;v=v+1) begin
         assign mod_in_flit[v] = noc_in_flit;
         assign mod_out_flit_flat[(v+1)*CONFIG.NOC_FLIT_WIDTH-1:v*CONFIG.NOC_FLIT_WIDTH] = mod_out_flit[v];
      end
   endgenerate

   // The different interfaces at the bus slave
   //  slave 0: configuration
   //           NABASE + 0x000000
   //  slave 1: mp_simple
   //           NABASE + 0x100000
   //  slave 2: dma
   //           NABASE + 0x200000

   localparam IFSLAVES = 3;
   localparam IFCONF     = 0;
   localparam IFMPSIMPLE = 1;
   localparam IFDMA      = 2;

   reg [IFSLAVES-1:0] sselect;
   always @(*) begin
      sselect[IFCONF]     = (wbs_adr_i[21:20] == 2'b00);
      sselect[IFMPSIMPLE] = (wbs_adr_i[21:20] == 2'b01);
      sselect[IFDMA]      = (wbs_adr_i[21:20] == 2'b10);
   end

   wire        wbif_ack_o[0:IFSLAVES-1];
   wire        wbif_rty_o[0:IFSLAVES-1];
   wire        wbif_err_o[0:IFSLAVES-1];
   wire [31:0] wbif_dat_o[0:IFSLAVES-1];

   assign wbs_ack_o = wbs_cyc_i & wbs_stb_i &
                      ((sselect[IFCONF]     & wbif_ack_o[IFCONF]) |
                       (sselect[IFMPSIMPLE] & wbif_ack_o[IFMPSIMPLE]) |
                       (sselect[IFDMA]      & wbif_ack_o[IFDMA]));
   assign wbs_rty_o = wbs_cyc_i & wbs_stb_i &
                      ((sselect[IFCONF]     & wbif_rty_o[IFCONF]) |
                       (sselect[IFMPSIMPLE] & wbif_rty_o[IFMPSIMPLE]) |
                       (sselect[IFDMA]      & wbif_rty_o[IFDMA]));
   assign wbs_err_o = wbs_cyc_i & wbs_stb_i &
                      ((sselect[IFCONF]     & wbif_err_o[IFCONF]) |
                       (sselect[IFMPSIMPLE] & wbif_err_o[IFMPSIMPLE]) |
                       (sselect[IFDMA]      & wbif_err_o[IFDMA]));

   assign wbs_dat_o = sselect[IFCONF] ? wbif_dat_o[IFCONF] :
                      sselect[IFMPSIMPLE] ? wbif_dat_o[IFMPSIMPLE] :
                      sselect[IFDMA] ? wbif_dat_o[IFDMA] : wbif_dat_o[IFCONF];


   networkadapter_conf
     #(.CONFIG                (CONFIG),
       .TILEID                (TILEID),
       .COREBASE              (COREBASE),
       .CONF_MPSIMPLE_PRESENT (ENABLE_MPSIMPLE),
       .CONF_DMA_PRESENT      (ENABLE_DMA))
   u_conf(
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
          .cdc_conf                     (cdc_conf[2:0]),
          .cdc_enable                   (cdc_enable),
 `endif
`endif
          // Outputs
          .data                         (wbif_dat_o[IFCONF]),
          .ack                          (wbif_ack_o[IFCONF]),
          .rty                          (wbif_rty_o[IFCONF]),
          .err                          (wbif_err_o[IFCONF]),
          // Inputs
          .clk                          (clk),
          .rst                          (rst),
          .adr                          (wbs_adr_i[15:0]),
          .we                           (wbs_cyc_i & wbs_stb_i & wbs_we_i),
          .data_i                       (wbs_dat_i));


   // just wire them statically for the moment
   assign wbif_rty_o[IFMPSIMPLE] = 1'b0;
   assign wbif_err_o[IFMPSIMPLE] = 1'b0;

   /* lisnoc_mp_simple_wb AUTO_TEMPLATE(
    .noc_out_flit  (mod_out_flit[CONFIG.NOC_VC_MPSIMPLE][CONFIG.NOC_FLIT_WIDTH-1:0]),
    .noc_out_valid (mod_out_valid[CONFIG.NOC_VC_MPSIMPLE]),
    .noc_out_ready (mod_out_ready[CONFIG.NOC_VC_MPSIMPLE]),
    .noc_in_flit  (mod_in_flit[CONFIG.NOC_VC_MPSIMPLE][CONFIG.NOC_FLIT_WIDTH-1:0]),
    .noc_in_valid (mod_in_valid[CONFIG.NOC_VC_MPSIMPLE]),
    .noc_in_ready (mod_in_ready[CONFIG.NOC_VC_MPSIMPLE]),
    .wb_cyc_i     (wbs_cyc_i & sselect[IFMPSIMPLE]),
    .wb_stb_i     (wbs_stb_i & sselect[IFMPSIMPLE]),
    .wb_ack_o     (wbif_ack_o[IFMPSIMPLE]),
    .wb_dat_o     (wbif_dat_o[IFMPSIMPLE]),
    .wb_adr_i     (wbs_adr_i[5:0]),
    .wb_\(.*\)      (wbs_\1),
    .irq          (irq[0]),
    ); */

   lisnoc_mp_simple_wb
     #(.noc_data_width(32),.noc_type_width(2),.fifo_depth(16))
   u_mp_simple(/*AUTOINST*/
               // Outputs
               .noc_out_flit            (mod_out_flit[CONFIG.NOC_VC_MPSIMPLE][CONFIG.NOC_FLIT_WIDTH-1:0]), // Templated
               .noc_out_valid           (mod_out_valid[CONFIG.NOC_VC_MPSIMPLE]), // Templated
               .noc_in_ready            (mod_in_ready[CONFIG.NOC_VC_MPSIMPLE]), // Templated
               .wb_dat_o                (wbif_dat_o[IFMPSIMPLE]), // Templated
               .wb_ack_o                (wbif_ack_o[IFMPSIMPLE]), // Templated
               .irq                     (irq[0]),                // Templated
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .noc_out_ready           (mod_out_ready[CONFIG.NOC_VC_MPSIMPLE]), // Templated
               .noc_in_flit             (mod_in_flit[CONFIG.NOC_VC_MPSIMPLE][CONFIG.NOC_FLIT_WIDTH-1:0]), // Templated
               .noc_in_valid            (mod_in_valid[CONFIG.NOC_VC_MPSIMPLE]), // Templated
               .wb_adr_i                (wbs_adr_i[5:0]),        // Templated
               .wb_we_i                 (wbs_we_i),              // Templated
               .wb_cyc_i                (wbs_cyc_i & sselect[IFMPSIMPLE]), // Templated
               .wb_stb_i                (wbs_stb_i & sselect[IFMPSIMPLE]), // Templated
               .wb_dat_i                (wbs_dat_i));            // Templated

   generate
      if (ENABLE_DMA) begin
         /* lisnoc_dma AUTO_TEMPLATE(
          .noc_in_req_ready (mod_in_ready[CONFIG.NOC_VC_DMA_REQ]),
          .noc_in_req_valid (mod_in_valid[CONFIG.NOC_VC_DMA_REQ]),
          .noc_in_req_flit  (mod_in_flit[CONFIG.NOC_VC_DMA_REQ][CONFIG.NOC_FLIT_WIDTH-1:0]),
          .noc_in_resp_ready (mod_in_ready[CONFIG.NOC_VC_DMA_RESP]),
          .noc_in_resp_valid (mod_in_valid[CONFIG.NOC_VC_DMA_RESP]),
          .noc_in_resp_flit  (mod_in_flit[CONFIG.NOC_VC_DMA_RESP][CONFIG.NOC_FLIT_WIDTH-1:0]),
          .noc_out_req_ready (mod_out_ready[CONFIG.NOC_VC_DMA_REQ]),
          .noc_out_req_valid (mod_out_valid[CONFIG.NOC_VC_DMA_REQ]),
          .noc_out_req_flit  (mod_out_flit[CONFIG.NOC_VC_DMA_REQ][CONFIG.NOC_FLIT_WIDTH-1:0]),
          .noc_out_resp_ready (mod_out_ready[CONFIG.NOC_VC_DMA_RESP]),
          .noc_out_resp_valid (mod_out_valid[CONFIG.NOC_VC_DMA_RESP]),
          .noc_out_resp_flit  (mod_out_flit[CONFIG.NOC_VC_DMA_RESP][CONFIG.NOC_FLIT_WIDTH-1:0]),
          .wb_if_cyc_i    (wbs_cyc_i & sselect[IFDMA]),
          .wb_if_stb_i    (wbs_stb_i & sselect[IFDMA]),
          .wb_if_dat_o    (wbif_dat_o[IFDMA]),
          .wb_if_ack_o    (wbif_ack_o[IFDMA]),
          .wb_if_rty_o    (wbif_rty_o[IFDMA]),
          .wb_if_err_o    (wbif_err_o[IFDMA]),
          .wb_if_\(.*\)   (wbs_\1),
          .wb_\(.*\)      (wbm_\1),
          .irq            (irq[DMA_ENTRIES:1]),
          ); */
         lisnoc_dma
           #(.tileid(TILEID),.table_entries(DMA_ENTRIES))
         u_dma(/*AUTOINST*/
               // Outputs
               .noc_in_req_ready        (mod_in_ready[CONFIG.NOC_VC_DMA_REQ]), // Templated
               .noc_in_resp_ready       (mod_in_ready[CONFIG.NOC_VC_DMA_RESP]), // Templated
               .noc_out_req_flit        (mod_out_flit[CONFIG.NOC_VC_DMA_REQ][CONFIG.NOC_FLIT_WIDTH-1:0]), // Templated
               .noc_out_req_valid       (mod_out_valid[CONFIG.NOC_VC_DMA_REQ]), // Templated
               .noc_out_resp_flit       (mod_out_flit[CONFIG.NOC_VC_DMA_RESP][CONFIG.NOC_FLIT_WIDTH-1:0]), // Templated
               .noc_out_resp_valid      (mod_out_valid[CONFIG.NOC_VC_DMA_RESP]), // Templated
               .wb_if_dat_o             (wbif_dat_o[IFDMA]),     // Templated
               .wb_if_ack_o             (wbif_ack_o[IFDMA]),     // Templated
               .wb_if_err_o             (wbif_err_o[IFDMA]),     // Templated
               .wb_if_rty_o             (wbif_rty_o[IFDMA]),     // Templated
               .wb_adr_o                (wbm_adr_o),             // Templated
               .wb_dat_o                (wbm_dat_o),             // Templated
               .wb_cyc_o                (wbm_cyc_o),             // Templated
               .wb_stb_o                (wbm_stb_o),             // Templated
               .wb_sel_o                (wbm_sel_o),             // Templated
               .wb_we_o                 (wbm_we_o),              // Templated
               .wb_cab_o                (wbm_cab_o),             // Templated
               .wb_cti_o                (wbm_cti_o),             // Templated
               .wb_bte_o                (wbm_bte_o),             // Templated
               .irq                     (irq[DMA_ENTRIES:1]),    // Templated
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .noc_in_req_flit         (mod_in_flit[CONFIG.NOC_VC_DMA_REQ][CONFIG.NOC_FLIT_WIDTH-1:0]), // Templated
               .noc_in_req_valid        (mod_in_valid[CONFIG.NOC_VC_DMA_REQ]), // Templated
               .noc_in_resp_flit        (mod_in_flit[CONFIG.NOC_VC_DMA_RESP][CONFIG.NOC_FLIT_WIDTH-1:0]), // Templated
               .noc_in_resp_valid       (mod_in_valid[CONFIG.NOC_VC_DMA_RESP]), // Templated
               .noc_out_req_ready       (mod_out_ready[CONFIG.NOC_VC_DMA_REQ]), // Templated
               .noc_out_resp_ready      (mod_out_ready[CONFIG.NOC_VC_DMA_RESP]), // Templated
               .wb_if_adr_i             (wbs_adr_i),             // Templated
               .wb_if_dat_i             (wbs_dat_i),             // Templated
               .wb_if_cyc_i             (wbs_cyc_i & sselect[IFDMA]), // Templated
               .wb_if_stb_i             (wbs_stb_i & sselect[IFDMA]), // Templated
               .wb_if_we_i              (wbs_we_i),              // Templated
               .wb_dat_i                (wbm_dat_i),             // Templated
               .wb_ack_i                (wbm_ack_i));            // Templated
      end
   endgenerate

   generate
      if (CONFIG.NOC_VCHANNELS>1) begin
         /* lisnoc_router_output_arbiter AUTO_TEMPLATE(
          .fifo_valid_i (mod_out_valid[]),
          .fifo_ready_o (mod_out_ready[]),
          .fifo_flit_i  (mod_out_flit_flat[CONFIG.NOC_VCHANNELS*CONFIG.NOC_FLIT_WIDTH-1:0]),
          .link_valid_o (noc_out_valid[]),
          .link_flit_o (noc_out_flit[CONFIG.NOC_FLIT_WIDTH-1:0]),
          .link_ready_i (noc_out_ready[]),
          ); */
         lisnoc_router_output_arbiter
           #(.vchannels(CONFIG.NOC_VCHANNELS))
         u_arb(/*AUTOINST*/
               // Outputs
               .fifo_ready_o            (mod_out_ready[(CONFIG.NOC_VCHANNELS)-1:0]), // Templated
               .link_valid_o            (noc_out_valid[(CONFIG.NOC_VCHANNELS)-1:0]), // Templated
               .link_flit_o             (noc_out_flit[CONFIG.NOC_FLIT_WIDTH-1:0]), // Templated
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .fifo_valid_i            (mod_out_valid[(CONFIG.NOC_VCHANNELS)-1:0]), // Templated
               .fifo_flit_i             (mod_out_flit_flat[CONFIG.NOC_VCHANNELS*CONFIG.NOC_FLIT_WIDTH-1:0]), // Templated
               .link_ready_i            (noc_out_ready[(CONFIG.NOC_VCHANNELS)-1:0])); // Templated
      end // if (CONFIG.NOC_VCHANNELS>1)
      else begin
         assign noc_out_valid = mod_out_valid[0];
         assign noc_out_flit = mod_out_flit[0];
         assign mod_out_ready[0] = noc_out_ready;
      end // else: !if(CONFIG.NOC_VCHANNELS>1)
   endgenerate


endmodule // networkadapter_ct

// Local Variables:
// verilog-library-directories:("../../../../../external/lisnoc/rtl/dma/" "../../../../../external/lisnoc/rtl/router/" ".")
// verilog-auto-inst-param-value: t
// End:
