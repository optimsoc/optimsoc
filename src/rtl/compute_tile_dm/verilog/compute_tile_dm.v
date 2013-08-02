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
 * This is the compute tile for distributed memory systems.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

`include "lisnoc_def.vh"
`include "optimsoc_def.vh"

module compute_tile_dm(
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   trace_itm,
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   trace_stm,
`endif
   /*AUTOARG*/
   // Outputs
   noc_in_ready, noc_out_flit, noc_out_valid,
   // Inputs
   clk, rst_cpu, rst_sys, noc_in_flit, noc_in_valid, noc_out_ready,
   cpu_stall
   );

   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH+NOC_FLIT_TYPE_WIDTH;
   parameter VCHANNELS = `VCHANNELS;

   parameter ID       = 0;
   parameter CORES    = 1;
   /* memory size in bytes */
   parameter MEM_SIZE = 30*1024; // 30 kByte
   parameter MEM_FILE = "ct.vmem";

   parameter DMA_ENTRIES = 4;
   // TODO: make define out of it

   input clk;
   input rst_cpu, rst_sys;

   input [NOC_FLIT_WIDTH-1:0] noc_in_flit;
   input [VCHANNELS-1:0] noc_in_valid;
   output [VCHANNELS-1:0] noc_in_ready;
   output [NOC_FLIT_WIDTH-1:0] noc_out_flit;
   output [VCHANNELS-1:0] noc_out_valid;
   input [VCHANNELS-1:0] noc_out_ready;

   input cpu_stall;

`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   output [`DEBUG_ITM_PORTWIDTH-1:0] trace_itm;
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   output [`DEBUG_STM_PORTWIDTH-1:0] trace_stm;
`endif

   wire [31:0]   busms_adr_o[0:2];
   wire          busms_cyc_o[0:2];
   wire [31:0]   busms_dat_o[0:2];
   wire [3:0]    busms_sel_o[0:2];
   wire          busms_stb_o[0:2];
   wire          busms_we_o[0:2];
   wire          busms_cab_o[0:2];
   wire [2:0]    busms_cti_o[0:2];
   wire [1:0]    busms_bte_o[0:2];
   wire          busms_ack_i[0:2];
   wire          busms_rty_i[0:2];
   wire          busms_err_i[0:2];
   wire [31:0]   busms_dat_i[0:2];

   wire [31:0]   bussl_adr_i[0:2];
   wire          bussl_cyc_i[0:2];
   wire [31:0]   bussl_dat_i[0:2];
   wire [3:0]    bussl_sel_i[0:2];
   wire          bussl_stb_i[0:2];
   wire          bussl_we_i[0:2];
   wire          bussl_cab_i[0:2];
   wire [2:0]    bussl_cti_i[0:2];
   wire [1:0]    bussl_bte_i[0:2];
   wire          bussl_ack_o[0:2];
   wire          bussl_rty_o[0:2];
   wire          bussl_err_o[0:2];
   wire [31:0]   bussl_dat_o[0:2];

   wire [19:0]   pic_ints_i;
   assign pic_ints_i[19:4] = 17'h0;
   assign pic_ints_i[1:0] = 2'b00;

   /* or1200_module AUTO_TEMPLATE(
    .clk_i          (clk),
    .rst_i          (rst_cpu),
    .bus_clk_i          (clk),
    .bus_rst_i          (rst_cpu),
    .dbg_.*_o       (),
    .dbg_stall_i    (cpu_stall),
    .dbg_ewt_i      (1'b0),
    .dbg_stb_i      (1'b0),
    .dbg_we_i       (1'b0),
    .dbg_adr_i      (32'h00000000),
    .dbg_dat_i      (32'h00000000),
    .iwb_\(.*\)     (busms_\1[0][]),
    .dwb_\(.*\)     (busms_\1[1][]),
    ); */
   or1200_module
      #(.ID(0))
      u_core0 (
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
            .trace_itm                  (trace_itm[`DEBUG_ITM_PORTWIDTH-1:0]),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
            .trace_stm                  (trace_stm[`DEBUG_STM_PORTWIDTH-1:0]),
`endif
               /*AUTOINST*/
               // Outputs
               .dbg_lss_o               (),                      // Templated
               .dbg_is_o                (),                      // Templated
               .dbg_wp_o                (),                      // Templated
               .dbg_bp_o                (),                      // Templated
               .dbg_dat_o               (),                      // Templated
               .dbg_ack_o               (),                      // Templated
               .iwb_cyc_o               (busms_cyc_o[0]),        // Templated
               .iwb_adr_o               (busms_adr_o[0][31:0]),  // Templated
               .iwb_stb_o               (busms_stb_o[0]),        // Templated
               .iwb_we_o                (busms_we_o[0]),         // Templated
               .iwb_sel_o               (busms_sel_o[0][3:0]),   // Templated
               .iwb_dat_o               (busms_dat_o[0][31:0]),  // Templated
               .iwb_bte_o               (busms_bte_o[0][1:0]),   // Templated
               .iwb_cti_o               (busms_cti_o[0][2:0]),   // Templated
               .dwb_cyc_o               (busms_cyc_o[1]),        // Templated
               .dwb_adr_o               (busms_adr_o[1][31:0]),  // Templated
               .dwb_stb_o               (busms_stb_o[1]),        // Templated
               .dwb_we_o                (busms_we_o[1]),         // Templated
               .dwb_sel_o               (busms_sel_o[1][3:0]),   // Templated
               .dwb_dat_o               (busms_dat_o[1][31:0]),  // Templated
               .dwb_bte_o               (busms_bte_o[1][1:0]),   // Templated
               .dwb_cti_o               (busms_cti_o[1][2:0]),   // Templated
               // Inputs
               .clk_i                   (clk),                   // Templated
               .bus_clk_i               (clk),                   // Templated
               .rst_i                   (rst_cpu),               // Templated
               .bus_rst_i               (rst_cpu),               // Templated
               .dbg_stall_i             (cpu_stall),             // Templated
               .dbg_ewt_i               (1'b0),                  // Templated
               .dbg_stb_i               (1'b0),                  // Templated
               .dbg_we_i                (1'b0),                  // Templated
               .dbg_adr_i               (32'h00000000),          // Templated
               .dbg_dat_i               (32'h00000000),          // Templated
               .pic_ints_i              (pic_ints_i[19:0]),
               .iwb_ack_i               (busms_ack_i[0]),        // Templated
               .iwb_err_i               (busms_err_i[0]),        // Templated
               .iwb_rty_i               (busms_rty_i[0]),        // Templated
               .iwb_dat_i               (busms_dat_i[0][31:0]),  // Templated
               .dwb_ack_i               (busms_ack_i[1]),        // Templated
               .dwb_err_i               (busms_err_i[1]),        // Templated
               .dwb_rty_i               (busms_rty_i[1]),        // Templated
               .dwb_dat_i               (busms_dat_i[1][31:0]));  // Templated


   assign busms_cab_o[0] = 1'b0;
   assign busms_cab_o[1] = 1'b0;

   /* compute_tile_dm_bus AUTO_TEMPLATE(
    .clk_i      (clk),
    .rst_i      (rst_sys),
    .m_@_\(.*\)_o (busms_\2_i[\1][]),
    .m_@_\(.*\)_i (busms_\2_o[\1][]),
    .s_@_\(.*\)_o (bussl_\2_i[\1][]),
    .s_@_\(.*\)_i (bussl_\2_o[\1][]),
    ); */

   compute_tile_dm_bus
      #(.dw(32),
        .aw(32),
        .sw(4))
      u_bus(/*AUTOINST*/
            // Outputs
            .m_0_dat_o                  (busms_dat_i[0][31:0]),  // Templated
            .m_0_ack_o                  (busms_ack_i[0]),        // Templated
            .m_0_err_o                  (busms_err_i[0]),        // Templated
            .m_0_rty_o                  (busms_rty_i[0]),        // Templated
            .m_1_dat_o                  (busms_dat_i[1][31:0]),  // Templated
            .m_1_ack_o                  (busms_ack_i[1]),        // Templated
            .m_1_err_o                  (busms_err_i[1]),        // Templated
            .m_1_rty_o                  (busms_rty_i[1]),        // Templated
            .m_2_dat_o                  (busms_dat_i[2][31:0]),  // Templated
            .m_2_ack_o                  (busms_ack_i[2]),        // Templated
            .m_2_err_o                  (busms_err_i[2]),        // Templated
            .m_2_rty_o                  (busms_rty_i[2]),        // Templated
            .s_0_dat_o                  (bussl_dat_i[0][31:0]),  // Templated
            .s_0_adr_o                  (bussl_adr_i[0][31:0]),  // Templated
            .s_0_sel_o                  (bussl_sel_i[0][3:0]),   // Templated
            .s_0_we_o                   (bussl_we_i[0]),         // Templated
            .s_0_cyc_o                  (bussl_cyc_i[0]),        // Templated
            .s_0_stb_o                  (bussl_stb_i[0]),        // Templated
            .s_0_cab_o                  (bussl_cab_i[0]),        // Templated
            .s_0_cti_o                  (bussl_cti_i[0][2:0]),   // Templated
            .s_0_bte_o                  (bussl_bte_i[0][1:0]),   // Templated
            .s_1_dat_o                  (bussl_dat_i[1][31:0]),  // Templated
            .s_1_adr_o                  (bussl_adr_i[1][31:0]),  // Templated
            .s_1_sel_o                  (bussl_sel_i[1][3:0]),   // Templated
            .s_1_we_o                   (bussl_we_i[1]),         // Templated
            .s_1_cyc_o                  (bussl_cyc_i[1]),        // Templated
            .s_1_stb_o                  (bussl_stb_i[1]),        // Templated
            .s_1_cab_o                  (bussl_cab_i[1]),        // Templated
            .s_1_cti_o                  (bussl_cti_i[1][2:0]),   // Templated
            .s_1_bte_o                  (bussl_bte_i[1][1:0]),   // Templated
            .s_2_dat_o                  (bussl_dat_i[2][31:0]),  // Templated
            .s_2_adr_o                  (bussl_adr_i[2][31:0]),  // Templated
            .s_2_sel_o                  (bussl_sel_i[2][3:0]),   // Templated
            .s_2_we_o                   (bussl_we_i[2]),         // Templated
            .s_2_cyc_o                  (bussl_cyc_i[2]),        // Templated
            .s_2_stb_o                  (bussl_stb_i[2]),        // Templated
            .s_2_cab_o                  (bussl_cab_i[2]),        // Templated
            .s_2_cti_o                  (bussl_cti_i[2][2:0]),   // Templated
            .s_2_bte_o                  (bussl_bte_i[2][1:0]),   // Templated
            // Inputs
            .clk_i                      (clk),                   // Templated
            .rst_i                      (rst_sys),               // Templated
            .m_0_dat_i                  (busms_dat_o[0][31:0]),  // Templated
            .m_0_adr_i                  (busms_adr_o[0][31:0]),  // Templated
            .m_0_sel_i                  (busms_sel_o[0][3:0]),   // Templated
            .m_0_we_i                   (busms_we_o[0]),         // Templated
            .m_0_cyc_i                  (busms_cyc_o[0]),        // Templated
            .m_0_stb_i                  (busms_stb_o[0]),        // Templated
            .m_0_cab_i                  (busms_cab_o[0]),        // Templated
            .m_0_cti_i                  (busms_cti_o[0][2:0]),   // Templated
            .m_0_bte_i                  (busms_bte_o[0][1:0]),   // Templated
            .m_1_dat_i                  (busms_dat_o[1][31:0]),  // Templated
            .m_1_adr_i                  (busms_adr_o[1][31:0]),  // Templated
            .m_1_sel_i                  (busms_sel_o[1][3:0]),   // Templated
            .m_1_we_i                   (busms_we_o[1]),         // Templated
            .m_1_cyc_i                  (busms_cyc_o[1]),        // Templated
            .m_1_stb_i                  (busms_stb_o[1]),        // Templated
            .m_1_cab_i                  (busms_cab_o[1]),        // Templated
            .m_1_cti_i                  (busms_cti_o[1][2:0]),   // Templated
            .m_1_bte_i                  (busms_bte_o[1][1:0]),   // Templated
            .m_2_dat_i                  (busms_dat_o[2][31:0]),  // Templated
            .m_2_adr_i                  (busms_adr_o[2][31:0]),  // Templated
            .m_2_sel_i                  (busms_sel_o[2][3:0]),   // Templated
            .m_2_we_i                   (busms_we_o[2]),         // Templated
            .m_2_cyc_i                  (busms_cyc_o[2]),        // Templated
            .m_2_stb_i                  (busms_stb_o[2]),        // Templated
            .m_2_cab_i                  (busms_cab_o[2]),        // Templated
            .m_2_cti_i                  (busms_cti_o[2][2:0]),   // Templated
            .m_2_bte_i                  (busms_bte_o[2][1:0]),   // Templated
            .s_0_dat_i                  (bussl_dat_o[0][31:0]),  // Templated
            .s_0_ack_i                  (bussl_ack_o[0]),        // Templated
            .s_0_err_i                  (bussl_err_o[0]),        // Templated
            .s_0_rty_i                  (bussl_rty_o[0]),        // Templated
            .s_1_dat_i                  (bussl_dat_o[1][31:0]),  // Templated
            .s_1_ack_i                  (bussl_ack_o[1]),        // Templated
            .s_1_err_i                  (bussl_err_o[1]),        // Templated
            .s_1_rty_i                  (bussl_rty_o[1]),        // Templated
            .s_2_dat_i                  (bussl_dat_o[2][31:0]),  // Templated
            .s_2_ack_i                  (bussl_ack_o[2]),        // Templated
            .s_2_err_i                  (bussl_err_o[2]),        // Templated
            .s_2_rty_i                  (bussl_rty_o[2]));        // Templated

`ifndef OPTIMSOC_CTRAM_WIRES
   /* wb_sram_sp AUTO_TEMPLATE(
    .wb_clk_i  (clk),
    .wb_rst_i  (rst_sys),
    .wb_\(.*\) (bussl_\1[0]),
    ); */
   wb_sram_sp
      #(.DW(32),
        .AW(32),
        .MEM_SIZE(MEM_SIZE),
        .MEM_FILE(MEM_FILE))
      u_ram(/*AUTOINST*/
            // Outputs
            .wb_ack_o                   (bussl_ack_o[0]),        // Templated
            .wb_err_o                   (bussl_err_o[0]),        // Templated
            .wb_rty_o                   (bussl_rty_o[0]),        // Templated
            .wb_dat_o                   (bussl_dat_o[0]),        // Templated
            // Inputs
            .wb_adr_i                   (bussl_adr_i[0]),        // Templated
            .wb_bte_i                   (bussl_bte_i[0]),        // Templated
            .wb_cti_i                   (bussl_cti_i[0]),        // Templated
            .wb_cyc_i                   (bussl_cyc_i[0]),        // Templated
            .wb_dat_i                   (bussl_dat_i[0]),        // Templated
            .wb_sel_i                   (bussl_sel_i[0]),        // Templated
            .wb_stb_i                   (bussl_stb_i[0]),        // Templated
            .wb_we_i                    (bussl_we_i[0]),         // Templated
            .wb_clk_i                   (clk),                   // Templated
            .wb_rst_i                   (rst_sys));               // Templated
`endif

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
   assign pic_ints_i[3:2] = {na_irq[0],|na_irq[DMA_ENTRIES:1]};

   /* networkadapter_ct AUTO_TEMPLATE(
    .clk(clk),
    .rst(rst_sys),
    .wbs_\(.*\)   (bussl_\1[1]),
    .wbm_\(.*\)      (busms_\1[2]),
    .irq    (na_irq),
    );*/
   networkadapter_ct
      #(.tileid(ID),
        .dma_entries(DMA_ENTRIES),
        .vchannels(VCHANNELS),
        .noc_flit_width(NOC_FLIT_WIDTH))
      u_na(
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
           .cdc_conf                     (cdc_conf[2:0]),
           .cdc_enable                   (cdc_enable),
 `endif
`endif
           /*AUTOINST*/
           // Outputs
           .noc_in_ready                (noc_in_ready[VCHANNELS-1:0]),
           .noc_out_flit                (noc_out_flit[NOC_FLIT_WIDTH-1:0]),
           .noc_out_valid               (noc_out_valid[VCHANNELS-1:0]),
           .wbm_adr_o                   (busms_adr_o[2]),        // Templated
           .wbm_cyc_o                   (busms_cyc_o[2]),        // Templated
           .wbm_dat_o                   (busms_dat_o[2]),        // Templated
           .wbm_sel_o                   (busms_sel_o[2]),        // Templated
           .wbm_stb_o                   (busms_stb_o[2]),        // Templated
           .wbm_we_o                    (busms_we_o[2]),         // Templated
           .wbm_cab_o                   (busms_cab_o[2]),        // Templated
           .wbm_cti_o                   (busms_cti_o[2]),        // Templated
           .wbm_bte_o                   (busms_bte_o[2]),        // Templated
           .wbs_ack_o                   (bussl_ack_o[1]),        // Templated
           .wbs_rty_o                   (bussl_rty_o[1]),        // Templated
           .wbs_err_o                   (bussl_err_o[1]),        // Templated
           .wbs_dat_o                   (bussl_dat_o[1]),        // Templated
           .irq                         (na_irq),                // Templated
           // Inputs
           .clk                         (clk),                   // Templated
           .rst                         (rst_sys),               // Templated
           .noc_in_flit                 (noc_in_flit[NOC_FLIT_WIDTH-1:0]),
           .noc_in_valid                (noc_in_valid[VCHANNELS-1:0]),
           .noc_out_ready               (noc_out_ready[VCHANNELS-1:0]),
           .wbm_ack_i                   (busms_ack_i[2]),        // Templated
           .wbm_rty_i                   (busms_rty_i[2]),        // Templated
           .wbm_err_i                   (busms_err_i[2]),        // Templated
           .wbm_dat_i                   (busms_dat_i[2]),        // Templated
           .wbs_adr_i                   (bussl_adr_i[1]),        // Templated
           .wbs_cyc_i                   (bussl_cyc_i[1]),        // Templated
           .wbs_dat_i                   (bussl_dat_i[1]),        // Templated
           .wbs_sel_i                   (bussl_sel_i[1]),        // Templated
           .wbs_stb_i                   (bussl_stb_i[1]),        // Templated
           .wbs_we_i                    (bussl_we_i[1]),         // Templated
           .wbs_cab_i                   (bussl_cab_i[1]),        // Templated
           .wbs_cti_i                   (bussl_cti_i[1]),        // Templated
           .wbs_bte_i                   (bussl_bte_i[1]));        // Templated

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
                .wb_sel_i               (bussl_sel_i[2][3:0]));   // Templated


endmodule

// Local Variables:
// verilog-library-directories:("../../*/verilog/")
// verilog-auto-inst-param-value: t
// End:
