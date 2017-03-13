/* Copyright (c) 2013-2017 by the author(s)
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
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

`include "lisnoc_def.vh"
import optimsoc::*;

module networkadapter_ct
  #(
    parameter config_t CONFIG = 'x,

    parameter TILEID = 'x,
    parameter COREBASE = 'x
    )
   (
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
    output [2:0]          cdc_conf,
    output                cdc_enable,
 `endif
`endif

    input clk, rst,

    input [CONFIG.NOC_FLIT_WIDTH-1:0]  noc_in_flit,
    input [CONFIG.NOC_VCHANNELS-1:0]   noc_in_valid,
    output [CONFIG.NOC_VCHANNELS-1:0]  noc_in_ready,
    output [CONFIG.NOC_FLIT_WIDTH-1:0] noc_out_flit,
    output [CONFIG.NOC_VCHANNELS-1:0]  noc_out_valid,
    input [CONFIG.NOC_VCHANNELS-1:0]   noc_out_ready,

    output [31:0]                      wbm_adr_o,
    output                             wbm_cyc_o,
    output [31:0]                      wbm_dat_o,
    output [3:0]                       wbm_sel_o,
    output                             wbm_stb_o,
    output                             wbm_we_o,
    output                             wbm_cab_o,
    output [2:0]                       wbm_cti_o,
    output [1:0]                       wbm_bte_o,
    input                              wbm_ack_i,
    input                              wbm_rty_i,
    input                              wbm_err_i,
    input [31:0]                       wbm_dat_i,

    input [31:0]                       wbs_adr_i,
    input                              wbs_cyc_i,
    input [31:0]                       wbs_dat_i,
    input [3:0]                        wbs_sel_i,
    input                              wbs_stb_i,
    input                              wbs_we_i,
    input                              wbs_cab_i,
    input [2:0]                        wbs_cti_i,
    input [1:0]                        wbs_bte_i,
    output                             wbs_ack_o,
    output                             wbs_rty_o,
    output                             wbs_err_o,
    output [31:0]                      wbs_dat_o,

    output [1:0]                       irq
    );

   localparam VCHANNELS = CONFIG.NOC_VCHANNELS;
   localparam FLIT_WIDTH = CONFIG.NOC_FLIT_WIDTH;
   localparam VC_MPSIMPLE = CONFIG.NOC_VC_MPSIMPLE;
   localparam VC_DMA_REQ = CONFIG.NOC_VC_DMA_REQ;
   localparam VC_DMA_RESP = CONFIG.NOC_VC_DMA_RESP;

   wire [VCHANNELS-1:0] mod_out_ready;
   wire [VCHANNELS-1:0] mod_out_valid;
   wire [FLIT_WIDTH-1:0] mod_out_flit[0:VCHANNELS-1];
   wire [VCHANNELS-1:0] mod_in_ready;
   wire [VCHANNELS-1:0] mod_in_valid;
   wire [FLIT_WIDTH-1:0] mod_in_flit[0:VCHANNELS-1];

   wire [FLIT_WIDTH*VCHANNELS-1:0] mod_out_flit_flat;

   assign mod_in_valid = noc_in_valid;
   assign noc_in_ready = mod_in_ready;

   generate
      genvar              v;
      for (v=0;v<VCHANNELS;v=v+1) begin
         assign mod_in_flit[v] = noc_in_flit;
         assign mod_out_flit_flat[(v+1)*FLIT_WIDTH-1:v*FLIT_WIDTH] = mod_out_flit[v];
      end
   endgenerate

   // The different interfaces at the bus slave
   //  slave 0: configuration
   //           NABASE + 0x000000
   //  slave 1: mp_simple
   //           NABASE + 0x100000
   //  slave 2: dma
   //           NABASE + 0x200000
   // If a slave is not present there is a gap

   localparam SLAVES   = 3; // This is the number of maximum slaves
   localparam ID_CONF     = 0;
   localparam ID_MPSIMPLE = 1;
   localparam ID_DMA      = 2;

   wire [32*SLAVES-1:0] wbif_adr_i;
   wire [32*SLAVES-1:0] wbif_dat_i;
   wire [SLAVES-1:0]    wbif_cyc_i;
   wire [SLAVES-1:0]    wbif_stb_i;
   wire [4*SLAVES-1:0]  wbif_sel_i;
   wire [SLAVES-1:0]    wbif_we_i;
   wire [SLAVES*3-1:0]  wbif_cti_i;
   wire [SLAVES*2-1:0]  wbif_bte_i;
   wire [32*SLAVES-1:0] wbif_dat_o;
   wire [SLAVES-1:0]    wbif_ack_o;
   wire [SLAVES-1:0]    wbif_err_o;
   wire [SLAVES-1:0]    wbif_rty_o;

   wb_decode
     #(.SLAVES(3), .DATA_WIDTH(32), .ADDR_WIDTH(24),
       .S0_RANGE_WIDTH(4), .S0_RANGE_MATCH(4'h0),
       .S1_RANGE_WIDTH(4), .S1_RANGE_MATCH(4'h1),
       .S2_RANGE_WIDTH(4), .S2_RANGE_MATCH(4'h2)
       )
   u_slavedecode
     (.clk_i (clk),
      .rst_i (rst),
      .m_adr_i (wbs_adr_i[23:0]),
      .m_dat_i (wbs_dat_i),
      .m_cyc_i (wbs_cyc_i),
      .m_stb_i (wbs_stb_i),
      .m_sel_i (wbs_sel_i),
      .m_we_i (wbs_we_i),
      .m_cti_i (wbs_cti_i),
      .m_bte_i (wbs_bte_i),
      .m_dat_o (wbs_dat_o),
      .m_ack_o (wbs_ack_o),
      .m_err_o (wbs_err_o),
      .m_rty_o (wbs_rty_o),
      .s_adr_o (wbif_adr_i),
      .s_dat_o (wbif_dat_i),
      .s_cyc_o (wbif_cyc_i),
      .s_stb_o (wbif_stb_i),
      .s_sel_o (wbif_sel_i),
      .s_we_o  (wbif_we_i),
      .s_cti_o (wbif_cti_i),
      .s_bte_o (wbif_bte_i),
      .s_dat_i (wbif_dat_o),
      .s_ack_i (wbif_ack_o),
      .s_err_i (wbif_err_o),
      .s_rty_i (wbif_rty_o));

   networkadapter_conf
     #(.CONFIG                (CONFIG),
       .TILEID                (TILEID),
       .COREBASE              (COREBASE))
   u_conf(
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
          .cdc_conf                     (cdc_conf[2:0]),
          .cdc_enable                   (cdc_enable),
 `endif
`endif
          // Outputs
          .data                         (wbif_dat_o[ID_CONF*32 +: 32]),
          .ack                          (wbif_ack_o[ID_CONF]),
          .rty                          (wbif_rty_o[ID_CONF]),
          .err                          (wbif_err_o[ID_CONF]),
          // Inputs
          .clk                          (clk),
          .rst                          (rst),
          .adr                          (wbs_adr_i[15:0]),
          .we                           (wbs_cyc_i & wbs_stb_i & wbs_we_i),
          .data_i                       (wbif_dat_i[ID_CONF*32 +: 32]));


   // just wire them statically for the moment
   assign wbif_rty_o[ID_MPSIMPLE] = 1'b0;
   assign wbif_err_o[ID_MPSIMPLE] = 1'b0;

   /* lisnoc_mp_simple_wb AUTO_TEMPLATE(
    .noc_out_flit  (mod_out_flit[VC_MPSIMPLE][FLIT_WIDTH-1:0]),
    .noc_out_valid (mod_out_valid[VC_MPSIMPLE]),
    .noc_out_ready (mod_out_ready[VC_MPSIMPLE]),
    .noc_in_flit  (mod_in_flit[VC_MPSIMPLE][FLIT_WIDTH-1:0]),
    .noc_in_valid (mod_in_valid[VC_MPSIMPLE]),
    .noc_in_ready (mod_in_ready[VC_MPSIMPLE]),
    .wb_dat_\(.*\)    (wbif_dat_\1[ID_MPSIMPLE*32 +: 32]),
    .wb_adr_i      (wbif_adr_i[ID_MPSIMPLE*32 +: 6]),
    .wb_\(.*\)    (wbif_\1[ID_MPSIMPLE]),
    .irq          (irq[1]),
    ); */

   lisnoc_mp_simple_wb
     #(.noc_data_width(32),.noc_type_width(2),.fifo_depth(16))
   u_mp_simple(/*AUTOINST*/
               // Outputs
               .noc_out_flit            (mod_out_flit[VC_MPSIMPLE][FLIT_WIDTH-1:0]), // Templated
               .noc_out_valid           (mod_out_valid[VC_MPSIMPLE]), // Templated
               .noc_in_ready            (mod_in_ready[VC_MPSIMPLE]), // Templated
               .wb_dat_o                (wbif_dat_o[ID_MPSIMPLE*32 +: 32]), // Templated
               .wb_ack_o                (wbif_ack_o[ID_MPSIMPLE]), // Templated
               .irq                     (irq[1]),                // Templated
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .noc_out_ready           (mod_out_ready[VC_MPSIMPLE]), // Templated
               .noc_in_flit             (mod_in_flit[VC_MPSIMPLE][FLIT_WIDTH-1:0]), // Templated
               .noc_in_valid            (mod_in_valid[VC_MPSIMPLE]), // Templated
               .wb_adr_i                (wbif_adr_i[ID_MPSIMPLE*32 +: 6]), // Templated
               .wb_we_i                 (wbif_we_i[ID_MPSIMPLE]), // Templated
               .wb_cyc_i                (wbif_cyc_i[ID_MPSIMPLE]), // Templated
               .wb_stb_i                (wbif_stb_i[ID_MPSIMPLE]), // Templated
               .wb_dat_i                (wbif_dat_i[ID_MPSIMPLE*32 +: 32])); // Templated
   generate
      if (CONFIG.NA_ENABLE_DMA) begin
         wire [3:0] irq_dma;
         assign irq[0] = |irq_dma;

         /* lisnoc_dma AUTO_TEMPLATE(
          .noc_in_req_ready (mod_in_ready[VC_DMA_REQ]),
          .noc_in_req_valid (mod_in_valid[VC_DMA_REQ]),
          .noc_in_req_flit  (mod_in_flit[VC_DMA_REQ][FLIT_WIDTH-1:0]),
          .noc_in_resp_ready (mod_in_ready[VC_DMA_RESP]),
          .noc_in_resp_valid (mod_in_valid[VC_DMA_RESP]),
          .noc_in_resp_flit  (mod_in_flit[VC_DMA_RESP][FLIT_WIDTH-1:0]),
          .noc_out_req_ready (mod_out_ready[VC_DMA_REQ]),
          .noc_out_req_valid (mod_out_valid[VC_DMA_REQ]),
          .noc_out_req_flit  (mod_out_flit[VC_DMA_REQ][FLIT_WIDTH-1:0]),
          .noc_out_resp_ready (mod_out_ready[VC_DMA_RESP]),
          .noc_out_resp_valid (mod_out_valid[VC_DMA_RESP]),
          .noc_out_resp_flit  (mod_out_flit[VC_DMA_RESP][FLIT_WIDTH-1:0]),
          .wb_if_dat_\(.*\)    (wbif_dat_\1[ID_DMA*32 +: 32]),
          .wb_if_adr_i    (wbif_adr_i[ID_DMA*32 +: 32]),
          .wb_if_\(.*\)    (wbif_\1[ID_DMA]),
          .wb_\(.*\)      (wbm_\1),
          .irq            (irq_dma),
          ); */
         lisnoc_dma
           #(.tileid(TILEID),.table_entries(CONFIG.NA_DMA_ENTRIES))
         u_dma(/*AUTOINST*/
               // Outputs
               .noc_in_req_ready        (mod_in_ready[VC_DMA_REQ]), // Templated
               .noc_in_resp_ready       (mod_in_ready[VC_DMA_RESP]), // Templated
               .noc_out_req_flit        (mod_out_flit[VC_DMA_REQ][FLIT_WIDTH-1:0]), // Templated
               .noc_out_req_valid       (mod_out_valid[VC_DMA_REQ]), // Templated
               .noc_out_resp_flit       (mod_out_flit[VC_DMA_RESP][FLIT_WIDTH-1:0]), // Templated
               .noc_out_resp_valid      (mod_out_valid[VC_DMA_RESP]), // Templated
               .wb_if_dat_o             (wbif_dat_o[ID_DMA*32 +: 32]), // Templated
               .wb_if_ack_o             (wbif_ack_o[ID_DMA]),    // Templated
               .wb_if_err_o             (wbif_err_o[ID_DMA]),    // Templated
               .wb_if_rty_o             (wbif_rty_o[ID_DMA]),    // Templated
               .wb_adr_o                (wbm_adr_o),             // Templated
               .wb_dat_o                (wbm_dat_o),             // Templated
               .wb_cyc_o                (wbm_cyc_o),             // Templated
               .wb_stb_o                (wbm_stb_o),             // Templated
               .wb_sel_o                (wbm_sel_o),             // Templated
               .wb_we_o                 (wbm_we_o),              // Templated
               .wb_cab_o                (wbm_cab_o),             // Templated
               .wb_cti_o                (wbm_cti_o),             // Templated
               .wb_bte_o                (wbm_bte_o),             // Templated
               .irq                     (irq_dma),               // Templated
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .noc_in_req_flit         (mod_in_flit[VC_DMA_REQ][FLIT_WIDTH-1:0]), // Templated
               .noc_in_req_valid        (mod_in_valid[VC_DMA_REQ]), // Templated
               .noc_in_resp_flit        (mod_in_flit[VC_DMA_RESP][FLIT_WIDTH-1:0]), // Templated
               .noc_in_resp_valid       (mod_in_valid[VC_DMA_RESP]), // Templated
               .noc_out_req_ready       (mod_out_ready[VC_DMA_REQ]), // Templated
               .noc_out_resp_ready      (mod_out_ready[VC_DMA_RESP]), // Templated
               .wb_if_adr_i             (wbif_adr_i[ID_DMA*32 +: 32]), // Templated
               .wb_if_dat_i             (wbif_dat_i[ID_DMA*32 +: 32]), // Templated
               .wb_if_cyc_i             (wbif_cyc_i[ID_DMA]),    // Templated
               .wb_if_stb_i             (wbif_stb_i[ID_DMA]),    // Templated
               .wb_if_we_i              (wbif_we_i[ID_DMA]),     // Templated
               .wb_dat_i                (wbm_dat_i),             // Templated
               .wb_ack_i                (wbm_ack_i));            // Templated
      end else begin // if (CONFIG.NA_ENABLE_DMA)
         assign irq[0] = 1'b0;
      end
   endgenerate

   generate
      if (VCHANNELS>1) begin
         /* lisnoc_router_output_arbiter AUTO_TEMPLATE(
          .fifo_valid_i (mod_out_valid[]),
          .fifo_ready_o (mod_out_ready[]),
          .fifo_flit_i  (mod_out_flit_flat[VCHANNELS*FLIT_WIDTH-1:0]),
          .link_valid_o (noc_out_valid[]),
          .link_flit_o (noc_out_flit[FLIT_WIDTH-1:0]),
          .link_ready_i (noc_out_ready[]),
          ); */
         lisnoc_router_output_arbiter
           #(.vchannels(VCHANNELS))
         u_arb(/*AUTOINST*/
               // Outputs
               .fifo_ready_o            (mod_out_ready[VCHANNELS-1:0]), // Templated
               .link_valid_o            (noc_out_valid[VCHANNELS-1:0]), // Templated
               .link_flit_o             (noc_out_flit[FLIT_WIDTH-1:0]), // Templated
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .fifo_valid_i            (mod_out_valid[VCHANNELS-1:0]), // Templated
               .fifo_flit_i             (mod_out_flit_flat[VCHANNELS*FLIT_WIDTH-1:0]), // Templated
               .link_ready_i            (noc_out_ready[VCHANNELS-1:0])); // Templated
      end // if (VCHANNELS>1)
      else begin
         assign noc_out_valid = mod_out_valid[0];
         assign noc_out_flit = mod_out_flit[0];
         assign mod_out_ready[0] = noc_out_ready;
      end // else: !if(VCHANNELS>1)
   endgenerate


endmodule // networkadapter_ct

// Local Variables:
// verilog-library-directories:("../../../../../external/lisnoc/rtl/dma/" "../../../../../external/lisnoc/rtl/mp_simple/" "../../../../../external/lisnoc/rtl/router/" ".")
// verilog-auto-inst-param-value: t
// End:
