/* Copyright (c) 2013-2016 by the author(s)
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
 * This is the compute tile for distributed memory systems.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

`include "lisnoc_def.vh"

import dii_package::dii_flit;

import opensocdebug::mor1kx_trace_exec;
import optimsoc::config_t;

module compute_tile_dm
  #(
    parameter config_t CONFIG = 'x,

    parameter ID       = 'x,
    parameter COREBASE = 'x,

    parameter DEBUG_BASEID = 'x,

    parameter MEM_FILE = 'x
    )
   (
   input dii_flit [1:0] debug_ring_in,
   output [1:0] debug_ring_in_ready,
   output dii_flit [1:0] debug_ring_out,
   input [1:0] debug_ring_out_ready,

   output [31:0] wb_ext_adr_i,
   output        wb_ext_cyc_i,
   output [31:0] wb_ext_dat_i,
   output [3:0]  wb_ext_sel_i,
   output        wb_ext_stb_i,
   output        wb_ext_we_i,
   output        wb_ext_cab_i,
   output [2:0]  wb_ext_cti_i,
   output [1:0]  wb_ext_bte_i,
   input         wb_ext_ack_o,
   input         wb_ext_rty_o,
   input         wb_ext_err_o,
   input [31:0]  wb_ext_dat_o,

   input clk,
   input rst_cpu, rst_sys, rst_dbg,

   input [CONFIG.NOC_FLIT_WIDTH-1:0] noc_in_flit,
   input [CONFIG.NOC_VCHANNELS-1:0] noc_in_valid,
   output [CONFIG.NOC_VCHANNELS-1:0] noc_in_ready,
   output [CONFIG.NOC_FLIT_WIDTH-1:0] noc_out_flit,
   output [CONFIG.NOC_VCHANNELS-1:0] noc_out_valid,
   input [CONFIG.NOC_VCHANNELS-1:0] noc_out_ready
   );

   localparam NR_MASTERS = CONFIG.CORES_PER_TILE * 2 + 1;
   localparam NR_SLAVES = 3;

   localparam NA_ENABLE_DMA = 1;
   localparam DMA_ENTRIES = 4;

   mor1kx_trace_exec [CONFIG.CORES_PER_TILE-1:0] trace;

   wire wb_mem_clk_i, wb_mem_rst_i;
   assign wb_mem_clk_i = clk;
   assign wb_mem_rst_i = rst_sys;

   logic [31:0] wb_mem_adr_i;
   logic        wb_mem_cyc_i;
   logic [31:0] wb_mem_dat_i;
   logic [3:0]  wb_mem_sel_i;
   logic        wb_mem_stb_i;
   logic        wb_mem_we_i;
   logic        wb_mem_cab_i;
   logic [2:0]  wb_mem_cti_i;
   logic [1:0]  wb_mem_bte_i;
   logic         wb_mem_ack_o;
   logic         wb_mem_rty_o;
   logic         wb_mem_err_o;
   logic [31:0]  wb_mem_dat_o;

   dii_flit [CONFIG.DEBUG_NUM_MODS-1:0] dii_in;
   logic [CONFIG.DEBUG_NUM_MODS-1:0]    dii_in_ready;
   dii_flit [CONFIG.DEBUG_NUM_MODS-1:0] dii_out;
   logic [CONFIG.DEBUG_NUM_MODS-1:0]    dii_out_ready;

   generate
      if (CONFIG.USE_DEBUG == 1) begin

         genvar i;
         logic [CONFIG.DEBUG_NUM_MODS-1:0][9:0] id_map;
         for (i = 0; i < CONFIG.DEBUG_NUM_MODS; i = i+1) begin
            assign id_map[i][9:0] = 10'(DEBUG_BASEID+i);
         end

         debug_ring_expand
           #(.PORTS(CONFIG.DEBUG_MODS_PER_TILE))
         u_debug_ring_segment
           (.*,
            .rst           (rst_dbg),
            .ext_in        (debug_ring_in),
            .ext_in_ready  (debug_ring_in_ready),
            .ext_out       (debug_ring_out),
            .ext_out_ready (debug_ring_out_ready));
      end // if (USE_DEBUG)
   endgenerate

   wire [31:0]   busms_adr_o[0:NR_MASTERS-1];
   wire          busms_cyc_o[0:NR_MASTERS-1];
   wire [31:0]   busms_dat_o[0:NR_MASTERS-1];
   wire [3:0]    busms_sel_o[0:NR_MASTERS-1];
   wire          busms_stb_o[0:NR_MASTERS-1];
   wire          busms_we_o[0:NR_MASTERS-1];
   wire          busms_cab_o[0:NR_MASTERS-1];
   wire [2:0]    busms_cti_o[0:NR_MASTERS-1];
   wire [1:0]    busms_bte_o[0:NR_MASTERS-1];
   wire          busms_ack_i[0:NR_MASTERS-1];
   wire          busms_rty_i[0:NR_MASTERS-1];
   wire          busms_err_i[0:NR_MASTERS-1];
   wire [31:0]   busms_dat_i[0:NR_MASTERS-1];

   wire [31:0]   bussl_adr_i[0:NR_SLAVES-1];
   wire          bussl_cyc_i[0:NR_SLAVES-1];
   wire [31:0]   bussl_dat_i[0:NR_SLAVES-1];
   wire [3:0]    bussl_sel_i[0:NR_SLAVES-1];
   wire          bussl_stb_i[0:NR_SLAVES-1];
   wire          bussl_we_i[0:NR_SLAVES-1];
   wire          bussl_cab_i[0:NR_SLAVES-1];
   wire [2:0]    bussl_cti_i[0:NR_SLAVES-1];
   wire [1:0]    bussl_bte_i[0:NR_SLAVES-1];
   wire          bussl_ack_o[0:NR_SLAVES-1];
   wire          bussl_rty_o[0:NR_SLAVES-1];
   wire          bussl_err_o[0:NR_SLAVES-1];
   wire [31:0]   bussl_dat_o[0:NR_SLAVES-1];

   wire          snoop_enable;
   wire [31:0]   snoop_adr;

   wire [31:0]   pic_ints_i [0:CONFIG.CORES_PER_TILE-1];
   assign pic_ints_i[0][31:4] = 17'h0;
   assign pic_ints_i[0][1:0] = 2'b00;

   genvar        c, m;

   wire [32*NR_MASTERS-1:0] busms_adr_o_flat;
   wire [NR_MASTERS-1:0]    busms_cyc_o_flat;
   wire [32*NR_MASTERS-1:0] busms_dat_o_flat;
   wire [4*NR_MASTERS-1:0]  busms_sel_o_flat;
   wire [NR_MASTERS-1:0]    busms_stb_o_flat;
   wire [NR_MASTERS-1:0]    busms_we_o_flat;
   wire [NR_MASTERS-1:0]    busms_cab_o_flat;
   wire [3*NR_MASTERS-1:0]  busms_cti_o_flat;
   wire [2*NR_MASTERS-1:0]  busms_bte_o_flat;
   wire [NR_MASTERS-1:0]    busms_ack_i_flat;
   wire [NR_MASTERS-1:0]    busms_rty_i_flat;
   wire [NR_MASTERS-1:0]    busms_err_i_flat;
   wire [32*NR_MASTERS-1:0] busms_dat_i_flat;

   generate
      for (m = 0; m < NR_MASTERS; m = m + 1) begin : gen_busms_flat
         assign busms_adr_o_flat[32*(m+1)-1:32*m] = busms_adr_o[m];
         assign busms_cyc_o_flat[m] = busms_cyc_o[m];
         assign busms_dat_o_flat[32*(m+1)-1:32*m] = busms_dat_o[m];
         assign busms_sel_o_flat[4*(m+1)-1:4*m] = busms_sel_o[m];
         assign busms_stb_o_flat[m] = busms_stb_o[m];
         assign busms_we_o_flat[m] = busms_we_o[m];
         assign busms_cab_o_flat[m] = busms_cab_o[m];
         assign busms_cti_o_flat[3*(m+1)-1:3*m] = busms_cti_o[m];
         assign busms_bte_o_flat[2*(m+1)-1:2*m] = busms_bte_o[m];
         assign busms_ack_i[m] = busms_ack_i_flat[m];
         assign busms_rty_i[m] = busms_rty_i_flat[m];
         assign busms_err_i[m] = busms_err_i_flat[m];
         assign busms_dat_i[m] = busms_dat_i_flat[32*(m+1)-1:32*m];
      end
   endgenerate

   generate
      for (c = 1; c < CONFIG.CORES_PER_TILE; c = c + 1) begin
         assign pic_ints_i[c] = 31'h0;
      end
   endgenerate


   generate
      for (c = 0; c < CONFIG.CORES_PER_TILE; c = c + 1) begin : gen_cores
         /* mor1kx_module AUTO_TEMPLATE(
          .clk_i          (clk),
          .rst_i          (rst_cpu),
          .bus_clk_i      (clk),
          .bus_rst_i      (rst_cpu),
          .dbg_.*_o       (),
          .dbg_stall_i    (1'b0),
          .dbg_ewt_i      (1'b0),
          .dbg_stb_i      (1'b0),
          .dbg_we_i       (1'b0),
          .dbg_adr_i      (32'h00000000),
          .dbg_dat_i      (32'h00000000),
          .iwb_\(.*\)     (busms_\1[c*2][]),
          .dwb_\(.*\)     (busms_\1[c*2+1][]),
          .pic_ints_i     (pic_ints_i[c]),
          .snoop_enable_i (snoop_enable),
          .snoop_adr_i    (snoop_adr),
          .trace_exec     (trace[c]),
          ); */
         mor1kx_module
               #(.ID(c),
                 .NUMCORES(CONFIG.CORES_PER_TILE))
         u_core (
                 /*AUTOINST*/
                 // Interfaces
                 .trace_exec            (trace[c]),              // Templated
                 // Outputs
                 .dbg_lss_o             (),                      // Templated
                 .dbg_is_o              (),                      // Templated
                 .dbg_wp_o              (),                      // Templated
                 .dbg_bp_o              (),                      // Templated
                 .dbg_dat_o             (),                      // Templated
                 .dbg_ack_o             (),                      // Templated
                 .iwb_cyc_o             (busms_cyc_o[c*2]),      // Templated
                 .iwb_adr_o             (busms_adr_o[c*2][31:0]), // Templated
                 .iwb_stb_o             (busms_stb_o[c*2]),      // Templated
                 .iwb_we_o              (busms_we_o[c*2]),       // Templated
                 .iwb_sel_o             (busms_sel_o[c*2][3:0]), // Templated
                 .iwb_dat_o             (busms_dat_o[c*2][31:0]), // Templated
                 .iwb_bte_o             (busms_bte_o[c*2][1:0]), // Templated
                 .iwb_cti_o             (busms_cti_o[c*2][2:0]), // Templated
                 .dwb_cyc_o             (busms_cyc_o[c*2+1]),    // Templated
                 .dwb_adr_o             (busms_adr_o[c*2+1][31:0]), // Templated
                 .dwb_stb_o             (busms_stb_o[c*2+1]),    // Templated
                 .dwb_we_o              (busms_we_o[c*2+1]),     // Templated
                 .dwb_sel_o             (busms_sel_o[c*2+1][3:0]), // Templated
                 .dwb_dat_o             (busms_dat_o[c*2+1][31:0]), // Templated
                 .dwb_bte_o             (busms_bte_o[c*2+1][1:0]), // Templated
                 .dwb_cti_o             (busms_cti_o[c*2+1][2:0]), // Templated
                 // Inputs
                 .clk_i                 (clk),                   // Templated
                 .bus_clk_i             (clk),                   // Templated
                 .rst_i                 (rst_cpu),               // Templated
                 .bus_rst_i             (rst_cpu),               // Templated
                 .dbg_stall_i           (1'b0),                  // Templated
                 .dbg_ewt_i             (1'b0),                  // Templated
                 .dbg_stb_i             (1'b0),                  // Templated
                 .dbg_we_i              (1'b0),                  // Templated
                 .dbg_adr_i             (32'h00000000),          // Templated
                 .dbg_dat_i             (32'h00000000),          // Templated
                 .pic_ints_i            (pic_ints_i[c]),         // Templated
                 .iwb_ack_i             (busms_ack_i[c*2]),      // Templated
                 .iwb_err_i             (busms_err_i[c*2]),      // Templated
                 .iwb_rty_i             (busms_rty_i[c*2]),      // Templated
                 .iwb_dat_i             (busms_dat_i[c*2][31:0]), // Templated
                 .dwb_ack_i             (busms_ack_i[c*2+1]),    // Templated
                 .dwb_err_i             (busms_err_i[c*2+1]),    // Templated
                 .dwb_rty_i             (busms_rty_i[c*2+1]),    // Templated
                 .dwb_dat_i             (busms_dat_i[c*2+1][31:0]), // Templated
                 .snoop_enable_i        (snoop_enable),          // Templated
                 .snoop_adr_i           (snoop_adr));            // Templated


         assign busms_cab_o[c*2] = 1'b0;
         assign busms_cab_o[c*2+1] = 1'b0;

         if (CONFIG.USE_DEBUG == 1) begin
            osd_stm_mor1kx
              u_stm
                (.clk  (clk),
                 .rst  (rst_dbg),
                 .id   (DEBUG_BASEID + 1 + c*CONFIG.DEBUG_MODS_PER_CORE),
                 .debug_in (dii_out[1+c*CONFIG.DEBUG_MODS_PER_CORE]),
                 .debug_in_ready (dii_out_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE]),
                 .debug_out (dii_in[1+c*CONFIG.DEBUG_MODS_PER_CORE]),
                 .debug_out_ready (dii_in_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE]),
                 .trace_port (trace[c]));

            osd_ctm_mor1kx
              u_ctm
                (.clk  (clk),
                 .rst  (rst_dbg),
                 .id   (DEBUG_BASEID + 1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1),
                 .debug_in (dii_out[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1]),
                 .debug_in_ready (dii_out_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1]),
                 .debug_out (dii_in[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1]),
                 .debug_out_ready (dii_in_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1]),
                 .trace_port (trace[c]));

      	    osd_system_diagnosis #(
            	.SYSTEMID (ID),
         	.NUM_MOD  (5))
            u_system_diagnosis (
         	.clk(clk),
         	.rst(rst_dbg),

         	.id  (DEBUG_BASEID + 1 + c*CONFIG.DEBUG_MODS_PER_CORE + 2),
         	.debug_in   (dii_out[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 2]),
         	.debug_in_ready (dii_out_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 2]),
         	.debug_out (dii_in[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 2]),
         	.debug_out_ready (dii_in_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 2]),
         	// System Interface
		.trace_port (trace[c]),
         	.sram_addr (wb_mem_adr_i),
        	.sram_ce (1'b1),
        	.sram_we (wb_mem_we_i));

         end
      end
   endgenerate

   /* wb_bus_b3 AUTO_TEMPLATE(
    .clk_i      (clk),
    .rst_i      (rst_sys),
    .m_\(.*\)_o (busms_\1_i_flat),
    .m_\(.*\)_i (busms_\1_o_flat),
    .s_\(.*\)_o ({bussl_\1_i[2],bussl_\1_i[1],bussl_\1_i[0]}),
    .s_\(.*\)_i ({bussl_\1_o[2],bussl_\1_o[1],bussl_\1_o[0]}),
    .snoop_en_o (snoop_enable),
    .snoop_adr_o (snoop_adr),
    .bus_hold (1'b0),
    .bus_hold_ack (),
    ); */
   wb_bus_b3
     #(.MASTERS(NR_MASTERS),.SLAVES(NR_SLAVES),
       .S0_RANGE_WIDTH(1),.S0_RANGE_MATCH(1'h0),
       .S1_RANGE_WIDTH(4),.S1_RANGE_MATCH(4'he),
       .S2_RANGE_WIDTH(4),.S2_RANGE_MATCH(4'hf))
   u_bus(/*AUTOINST*/
         // Outputs
         .m_dat_o                       (busms_dat_i_flat),      // Templated
         .m_ack_o                       (busms_ack_i_flat),      // Templated
         .m_err_o                       (busms_err_i_flat),      // Templated
         .m_rty_o                       (busms_rty_i_flat),      // Templated
         .s_adr_o                       ({bussl_adr_i[2],bussl_adr_i[1],bussl_adr_i[0]}), // Templated
         .s_dat_o                       ({bussl_dat_i[2],bussl_dat_i[1],bussl_dat_i[0]}), // Templated
         .s_cyc_o                       ({bussl_cyc_i[2],bussl_cyc_i[1],bussl_cyc_i[0]}), // Templated
         .s_stb_o                       ({bussl_stb_i[2],bussl_stb_i[1],bussl_stb_i[0]}), // Templated
         .s_sel_o                       ({bussl_sel_i[2],bussl_sel_i[1],bussl_sel_i[0]}), // Templated
         .s_we_o                        ({bussl_we_i[2],bussl_we_i[1],bussl_we_i[0]}), // Templated
         .s_cti_o                       ({bussl_cti_i[2],bussl_cti_i[1],bussl_cti_i[0]}), // Templated
         .s_bte_o                       ({bussl_bte_i[2],bussl_bte_i[1],bussl_bte_i[0]}), // Templated
         .snoop_adr_o                   (snoop_adr),             // Templated
         .snoop_en_o                    (snoop_enable),          // Templated
         .bus_hold_ack                  (),                      // Templated
         // Inputs
         .clk_i                         (clk),                   // Templated
         .rst_i                         (rst_sys),               // Templated
         .m_adr_i                       (busms_adr_o_flat),      // Templated
         .m_dat_i                       (busms_dat_o_flat),      // Templated
         .m_cyc_i                       (busms_cyc_o_flat),      // Templated
         .m_stb_i                       (busms_stb_o_flat),      // Templated
         .m_sel_i                       (busms_sel_o_flat),      // Templated
         .m_we_i                        (busms_we_o_flat),       // Templated
         .m_cti_i                       (busms_cti_o_flat),      // Templated
         .m_bte_i                       (busms_bte_o_flat),      // Templated
         .s_dat_i                       ({bussl_dat_o[2],bussl_dat_o[1],bussl_dat_o[0]}), // Templated
         .s_ack_i                       ({bussl_ack_o[2],bussl_ack_o[1],bussl_ack_o[0]}), // Templated
         .s_err_i                       ({bussl_err_o[2],bussl_err_o[1],bussl_err_o[0]}), // Templated
         .s_rty_i                       ({bussl_rty_o[2],bussl_rty_o[1],bussl_rty_o[0]}), // Templated
         .bus_hold                      (1'b0));                         // Templated

   //MAM - WB adapter signals
   logic          mam_stb_o;
   logic          mam_cyc_o;
   logic          mam_ack_i;
   logic          mam_err_i;
   logic          mam_rty_i;
   logic          mam_we_o;
   logic [31:0]   mam_addr_o;
   logic [31:0]   mam_dat_o;
   logic [31:0]   mam_dat_i;
   logic [2:0]    mam_cti_o;
   logic [1:0]    mam_bte_o;
   logic [3:0]    mam_sel_o;

   if (CONFIG.USE_DEBUG == 1) begin
      //MAM
      osd_mam_wb #(
           .DATA_WIDTH(32),
           .MAX_PKT_LEN(8),
           .MEM_SIZE0(CONFIG.LMEM_SIZE),
           .BASE_ADDR0(0))
      u_mam_wb(
           .clk_i(clk),
           .rst_i(rst_dbg),
           .debug_in(dii_out[0]),
           .debug_in_ready(dii_out_ready[0]),
           .debug_out(dii_in[0]),
           .debug_out_ready(dii_in_ready[0]),
           .id (DEBUG_BASEID),
           .stb_o(mam_stb_o),
           .cyc_o(mam_cyc_o),
           .ack_i(mam_ack_i),
           .we_o(mam_we_o),
           .addr_o(mam_addr_o),
           .dat_o(mam_dat_o),
           .dat_i(mam_dat_i),
           .cti_o(mam_cti_o),
           .bte_o(mam_bte_o),
           .sel_o(mam_sel_o));

   end //if (USE_DEBUG == 1)

   /* mam_wb_adapter AUTO_TEMPLATE(
    .wb_in_clk_i  (clk),
    .wb_in_rst_i  (rst_sys),
    .wb_in_\(.*\) (bussl_\1[0]),
    .wb_out_\(.*\) (wb_mem_\1),
    .wb_mam_\(.*\) (wb_mam_\1),
    ); */
   mam_wb_adapter
      #(.DW(32),
        .AW(32))
      u_mam_wb_adapter(
                       .wb_mam_adr_o    (mam_addr_o),
                       .wb_mam_cyc_o    (mam_cyc_o),
                       .wb_mam_dat_o    (mam_dat_o),
                       .wb_mam_sel_o    (mam_sel_o),
                       .wb_mam_stb_o    (mam_stb_o),
                       .wb_mam_we_o     (mam_we_o),
                       .wb_mam_cab_o    (1'b0),
                       .wb_mam_cti_o    (mam_cti_o),
                       .wb_mam_bte_o    (mam_bte_o),
                       .wb_mam_ack_i    (mam_ack_i),
                       .wb_mam_rty_i    (mam_rty_i),
                       .wb_mam_err_i    (mam_err_i),
                       .wb_mam_dat_i    (mam_dat_i),
                       /*AUTOINST*/
                       // Outputs
                       .wb_in_ack_o     (bussl_ack_o[0]),        // Templated
                       .wb_in_err_o     (bussl_err_o[0]),        // Templated
                       .wb_in_rty_o     (bussl_rty_o[0]),        // Templated
                       .wb_in_dat_o     (bussl_dat_o[0]),        // Templated
                       .wb_out_adr_i    (wb_mem_adr_i),          // Templated
                       .wb_out_bte_i    (wb_mem_bte_i),          // Templated
                       .wb_out_cti_i    (wb_mem_cti_i),          // Templated
                       .wb_out_cyc_i    (wb_mem_cyc_i),          // Templated
                       .wb_out_dat_i    (wb_mem_dat_i),          // Templated
                       .wb_out_sel_i    (wb_mem_sel_i),          // Templated
                       .wb_out_stb_i    (wb_mem_stb_i),          // Templated
                       .wb_out_we_i     (wb_mem_we_i),           // Templated
                       .wb_out_clk_i    (wb_mem_clk_i),          // Templated
                       .wb_out_rst_i    (wb_mem_rst_i),          // Templated
                       // Inputs
                       .wb_in_adr_i     (bussl_adr_i[0]),        // Templated
                       .wb_in_bte_i     (bussl_bte_i[0]),        // Templated
                       .wb_in_cti_i     (bussl_cti_i[0]),        // Templated
                       .wb_in_cyc_i     (bussl_cyc_i[0]),        // Templated
                       .wb_in_dat_i     (bussl_dat_i[0]),        // Templated
                       .wb_in_sel_i     (bussl_sel_i[0]),        // Templated
                       .wb_in_stb_i     (bussl_stb_i[0]),        // Templated
                       .wb_in_we_i      (bussl_we_i[0]),         // Templated
                       .wb_in_clk_i     (clk),                   // Templated
                       .wb_in_rst_i     (rst_sys),               // Templated
                       .wb_out_ack_o    (wb_mem_ack_o),          // Templated
                       .wb_out_err_o    (wb_mem_err_o),          // Templated
                       .wb_out_rty_o    (wb_mem_rty_o),          // Templated
                       .wb_out_dat_o    (wb_mem_dat_o));                 // Templated

   generate
      if ((CONFIG.MEMORY_ACCESS == DISTRIBUTED) &&
       (CONFIG.LMEM_STYLE == PLAIN)) begin : gen_sram
         /* wb_sram_sp AUTO_TEMPLATE(
          .wb_\(.*\) (wb_mem_\1),
          ); */
         wb_sram_sp
           #(.DW(32),
             .AW(32),
             .MEM_SIZE(CONFIG.LMEM_SIZE),
             .MEM_FILE(MEM_FILE),
             .MEM_IMPL_TYPE("PLAIN")
             )
         u_ram(/*AUTOINST*/
               // Outputs
               .wb_ack_o                   (wb_mem_ack_o),          // Templated
               .wb_err_o                   (wb_mem_err_o),          // Templated
               .wb_rty_o                   (wb_mem_rty_o),          // Templated
               .wb_dat_o                   (wb_mem_dat_o),          // Templated
               // Inputs
               .wb_adr_i                   (wb_mem_adr_i),          // Templated
               .wb_bte_i                   (wb_mem_bte_i),          // Templated
               .wb_cti_i                   (wb_mem_cti_i),          // Templated
               .wb_cyc_i                   (wb_mem_cyc_i),          // Templated
               .wb_dat_i                   (wb_mem_dat_i),          // Templated
               .wb_sel_i                   (wb_mem_sel_i),          // Templated
               .wb_stb_i                   (wb_mem_stb_i),          // Templated
               .wb_we_i                    (wb_mem_we_i),           // Templated
               .wb_clk_i                   (wb_mem_clk_i),          // Templated
               .wb_rst_i                   (wb_mem_rst_i));                 // Templated
      end else begin // if ((CONFIG.MEMORY_ACCESS == DISTRIBUTED) &&...
         assign wb_ext_adr_i = wb_mem_adr_i;
         assign wb_ext_bte_i = wb_mem_bte_i;
         assign wb_ext_cti_i = wb_mem_cti_i;
         assign wb_ext_cyc_i = wb_mem_cyc_i;
         assign wb_ext_dat_i = wb_mem_dat_i;
         assign wb_ext_sel_i = wb_mem_sel_i;
         assign wb_ext_stb_i = wb_mem_stb_i;
         assign wb_ext_we_i = wb_mem_we_i;
         assign wb_mem_ack_o = wb_ext_ack_o;
         assign wb_mem_rty_o = wb_ext_rty_o;
         assign wb_mem_err_o = wb_ext_err_o;
         assign wb_mem_dat_o = wb_ext_dat_o;
      end // else: !if((CONFIG.MEMORY_ACCESS == DISTRIBUTED) &&...
   endgenerate

   wire [DMA_ENTRIES:0] na_irq;

   /*
    *  +---+-..-+----+
    *  |   dma  | mp |
    *  +---+-..-+----+
    * dma_entries 1  (0)
    *
    * map to irq lines of cpu
    *
    *  +----+-----+
    *  | mp | dma |
    *  +----+-----+
    *    3     2
    */
   assign pic_ints_i[0][3:2] = {na_irq[0],|na_irq[DMA_ENTRIES:1]};


   /* networkadapter_ct AUTO_TEMPLATE(
    .clk(clk),
    .rst(rst_sys),
    .wbs_\(.*\)   (bussl_\1[1]),
    .wbm_\(.*\)      (busms_\1[NR_MASTERS-1]),
    .irq    (na_irq),
    );*/
   networkadapter_ct
      #(.CONFIG(CONFIG),
        .TILEID(ID),
        .ENABLE_MPSIMPLE(1),
        .ENABLE_DMA(NA_ENABLE_DMA),
        .DMA_ENTRIES(DMA_ENTRIES),
        .COREBASE(COREBASE))
      u_na(
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
           .cdc_conf                     (cdc_conf[2:0]),
           .cdc_enable                   (cdc_enable),
 `endif
`endif
           /*AUTOINST*/
           // Outputs
           .noc_in_ready                (noc_in_ready[(CONFIG.NOC_VCHANNELS)-1:0]),
           .noc_out_flit                (noc_out_flit[(CONFIG.NOC_FLIT_WIDTH)-1:0]),
           .noc_out_valid               (noc_out_valid[(CONFIG.NOC_VCHANNELS)-1:0]),
           .wbm_adr_o                   (busms_adr_o[NR_MASTERS-1]), // Templated
           .wbm_cyc_o                   (busms_cyc_o[NR_MASTERS-1]), // Templated
           .wbm_dat_o                   (busms_dat_o[NR_MASTERS-1]), // Templated
           .wbm_sel_o                   (busms_sel_o[NR_MASTERS-1]), // Templated
           .wbm_stb_o                   (busms_stb_o[NR_MASTERS-1]), // Templated
           .wbm_we_o                    (busms_we_o[NR_MASTERS-1]), // Templated
           .wbm_cab_o                   (busms_cab_o[NR_MASTERS-1]), // Templated
           .wbm_cti_o                   (busms_cti_o[NR_MASTERS-1]), // Templated
           .wbm_bte_o                   (busms_bte_o[NR_MASTERS-1]), // Templated
           .wbs_ack_o                   (bussl_ack_o[1]),        // Templated
           .wbs_rty_o                   (bussl_rty_o[1]),        // Templated
           .wbs_err_o                   (bussl_err_o[1]),        // Templated
           .wbs_dat_o                   (bussl_dat_o[1]),        // Templated
           .irq                         (na_irq),                // Templated
           // Inputs
           .clk                         (clk),                   // Templated
           .rst                         (rst_sys),               // Templated
           .noc_in_flit                 (noc_in_flit[(CONFIG.NOC_FLIT_WIDTH)-1:0]),
           .noc_in_valid                (noc_in_valid[(CONFIG.NOC_VCHANNELS)-1:0]),
           .noc_out_ready               (noc_out_ready[(CONFIG.NOC_VCHANNELS)-1:0]),
           .wbm_ack_i                   (busms_ack_i[NR_MASTERS-1]), // Templated
           .wbm_rty_i                   (busms_rty_i[NR_MASTERS-1]), // Templated
           .wbm_err_i                   (busms_err_i[NR_MASTERS-1]), // Templated
           .wbm_dat_i                   (busms_dat_i[NR_MASTERS-1]), // Templated
           .wbs_adr_i                   (bussl_adr_i[1]),        // Templated
           .wbs_cyc_i                   (bussl_cyc_i[1]),        // Templated
           .wbs_dat_i                   (bussl_dat_i[1]),        // Templated
           .wbs_sel_i                   (bussl_sel_i[1]),        // Templated
           .wbs_stb_i                   (bussl_stb_i[1]),        // Templated
           .wbs_we_i                    (bussl_we_i[1]),         // Templated
           .wbs_cab_i                   (bussl_cab_i[1]),        // Templated
           .wbs_cti_i                   (bussl_cti_i[1]),        // Templated
           .wbs_bte_i                   (bussl_bte_i[1]));       // Templated

   /* bootrom AUTO_TEMPLATE(
    .clk(clk),
    .rst(rst_sys),
    .wb_dat_o (bussl_dat_o[2][]),
    .wb_ack_o (bussl_ack_o[2][]),
    .wb_err_o (bussl_err_o[2][]),
    .wb_rty_o (bussl_rty_o[2][]),
    .wb_adr_i (bussl_adr_i[2][]),
    .wb_dat_i (bussl_dat_i[2][]),
    .wb_cyc_i (bussl_cyc_i[2][]),
    .wb_stb_i (bussl_stb_i[2][]),
    .wb_sel_i (bussl_sel_i[2][]),
    ); */
   bootrom
      u_bootrom(/*AUTOINST*/
                // Outputs
                .wb_dat_o               (bussl_dat_o[2][31:0]),  // Templated
                .wb_ack_o               (bussl_ack_o[2]),        // Templated
                .wb_err_o               (bussl_err_o[2]),        // Templated
                .wb_rty_o               (bussl_rty_o[2]),        // Templated
                // Inputs
                .clk                    (clk),                   // Templated
                .rst                    (rst_sys),               // Templated
                .wb_adr_i               (bussl_adr_i[2][31:0]),  // Templated
                .wb_dat_i               (bussl_dat_i[2][31:0]),  // Templated
                .wb_cyc_i               (bussl_cyc_i[2]),        // Templated
                .wb_stb_i               (bussl_stb_i[2]),        // Templated
                .wb_sel_i               (bussl_sel_i[2][3:0]));  // Templated

endmodule

// Local Variables:
// verilog-library-directories:("../../*/verilog/" "../../../../../external/osd/hardware/*/*/common/")
// verilog-auto-inst-param-value: t
// End:
