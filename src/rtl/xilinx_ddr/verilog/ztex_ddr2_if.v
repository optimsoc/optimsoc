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
 * This is a wrapper module that instatiates the mig_39 DDR2 memory
 * interfaces and attaches one wishbone to MIG native interface 
 * adapters to each of the four client ports of the MIG and and offers
 * these wishbone bus connection to the outside world.
 * Therewith, this module may be attached to up to 4 different wishbone
 * buses as a slave device.
 * 
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Hans-Christian Wild, hans-christian.wild@mytum.de  
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */
 
module ztex_ddr2_if(/*AUTOARG*/
   // Outputs
   ddr2_calib_done, mcb3_dram_a, mcb3_dram_ba, mcb3_dram_ras_n,
   mcb3_dram_cas_n, mcb3_dram_we_n, mcb3_dram_cke, mcb3_dram_dm,
   mcb3_dram_udm, mcb3_dram_ck, mcb3_dram_ck_n, wbm0_ack_o,
   wbm0_err_o, wbm0_rty_o, wbm0_dat_o, wbm1_ack_o, wbm1_err_o,
   wbm1_rty_o, wbm1_dat_o, wbm2_ack_o, wbm2_err_o, wbm2_rty_o,
   wbm2_dat_o, wbm3_ack_o, wbm3_err_o, wbm3_rty_o, wbm3_dat_o,
   // Inouts
   mcb3_dram_dq, mcb3_dram_udqs, mcb3_dram_udqs_n, mcb3_rzq, mcb3_zio,
   mcb3_dram_dqs, mcb3_dram_dqs_n,
   // Inputs
   ddr2_clk, ddr2_rst, wbm0_clk_i, wbm0_rst_i, wbm0_dat_i, wbm0_adr_i,
   wbm0_bte_i, wbm0_cti_i, wbm0_cyc_i, wbm0_sel_i, wbm0_stb_i,
   wbm0_we_i, wbm1_clk_i, wbm1_rst_i, wbm1_dat_i, wbm1_adr_i,
   wbm1_bte_i, wbm1_cti_i, wbm1_cyc_i, wbm1_sel_i, wbm1_stb_i,
   wbm1_we_i, wbm2_clk_i, wbm2_rst_i, wbm2_dat_i, wbm2_adr_i,
   wbm2_bte_i, wbm2_cti_i, wbm2_cyc_i, wbm2_sel_i, wbm2_stb_i,
   wbm2_we_i, wbm3_dat_i, wbm3_adr_i, wbm3_bte_i, wbm3_cti_i,
   wbm3_cyc_i, wbm3_sel_i, wbm3_stb_i, wbm3_we_i, wbm3_clk_i,
   wbm3_rst_i
   );

   input ddr2_clk;
   input ddr2_rst;

   output ddr2_calib_done;

   parameter USE_WBPORT0 = 1;
   parameter USE_WBPORT1 = 0;
   parameter USE_WBPORT2 = 0;
   parameter USE_WBPORT3 = 0;
   
   // MCB connection
   inout [15:0] mcb3_dram_dq;
   output [12:0] mcb3_dram_a;
   output [2:0]  mcb3_dram_ba;
   output        mcb3_dram_ras_n;
   output        mcb3_dram_cas_n;
   output        mcb3_dram_we_n;
   output        mcb3_dram_cke;
   output        mcb3_dram_dm;
   inout         mcb3_dram_udqs;
   inout         mcb3_dram_udqs_n;
   inout         mcb3_rzq;
   inout         mcb3_zio;
   output        mcb3_dram_udm;
   inout         mcb3_dram_dqs;
   inout         mcb3_dram_dqs_n;
   output        mcb3_dram_ck;
   output        mcb3_dram_ck_n;

   // Wishbone connection #0
   input         wbm0_clk_i;
   input         wbm0_rst_i;
   input [31:0]  wbm0_dat_i;
   output        wbm0_ack_o;
   output        wbm0_err_o;
   output        wbm0_rty_o;
   input [31:0]  wbm0_adr_i;
   input [1:0]   wbm0_bte_i;
   input [2:0]   wbm0_cti_i;
   input         wbm0_cyc_i;
   output [31:0] wbm0_dat_o;
   input [3:0]   wbm0_sel_i;
   input         wbm0_stb_i;
   input         wbm0_we_i;

   // Wishbone connection #1
   input         wbm1_clk_i;
   input         wbm1_rst_i;
   input [31:0]  wbm1_dat_i;
   output        wbm1_ack_o;
   output        wbm1_err_o;
   output        wbm1_rty_o;
   input [31:0]  wbm1_adr_i;
   input [1:0]   wbm1_bte_i;
   input [2:0]   wbm1_cti_i;
   input         wbm1_cyc_i;
   output [31:0] wbm1_dat_o;
   input [3:0]   wbm1_sel_i;
   input         wbm1_stb_i;
   input         wbm1_we_i;

   // Wishbone connection #2
   input         wbm2_clk_i;
   input         wbm2_rst_i;
   input [31:0]  wbm2_dat_i;
   output        wbm2_ack_o;
   output        wbm2_err_o;
   output        wbm2_rty_o;
   input [31:0]  wbm2_adr_i;
   input [1:0]   wbm2_bte_i;
   input [2:0]   wbm2_cti_i;
   input         wbm2_cyc_i;
   output [31:0] wbm2_dat_o;
   input [3:0]   wbm2_sel_i;
   input         wbm2_stb_i;
   input         wbm2_we_i;

   // Wishbone connection #3
   input [31:0]  wbm3_dat_i;
   output        wbm3_ack_o;
   output        wbm3_err_o;
   output        wbm3_rty_o;
   input [31:0]  wbm3_adr_i;
   input [1:0]   wbm3_bte_i;
   input [2:0]   wbm3_cti_i;
   input         wbm3_cyc_i;
   output [31:0] wbm3_dat_o;
   input [3:0]   wbm3_sel_i;
   input         wbm3_stb_i;
   input         wbm3_we_i;
   input         wbm3_clk_i;
   input         wbm3_rst_i;

   // Wires
   wire [15:0]   mcb3_dram_dq;
   wire [12:0]   mcb3_dram_a;
   wire [2:0]    mcb3_dram_ba;
   wire          mcb3_dram_ras_n;
   wire          mcb3_dram_cas_n;
   wire          mcb3_dram_we_n;
   wire          mcb3_dram_cke;
   wire          mcb3_dram_dm;
   wire          mcb3_dram_udqs;
   wire          mcb3_dram_udqs_n;
   wire          mcb3_rzq;
   wire          mcb3_zio;
   wire          mcb3_dram_udm;
   wire          c3_calib_done;
   wire          c3_clk0;
   wire          c3_rst0;
   wire          mcb3_dram_dqs;
   wire          mcb3_dram_dqs_n;
   wire          mcb3_dram_ck;
   wire          mcb3_dram_ck_n;
   wire          c3_p0_cmd_clk;
   wire          c3_p0_cmd_en;
   wire [2:0]    c3_p0_cmd_instr;
   wire [5:0]    c3_p0_cmd_bl;
   wire [29:0]   c3_p0_cmd_byte_addr;
   wire          c3_p0_cmd_empty;
   wire          c3_p0_cmd_full;
   wire          c3_p0_wr_clk;
   wire          c3_p0_wr_en;
   wire [3:0]    c3_p0_wr_mask;
   wire [31:0]   c3_p0_wr_data;
   wire          c3_p0_wr_full;
   wire          c3_p0_wr_empty;
   wire [6:0]    c3_p0_wr_count;
   wire          c3_p0_wr_underrun;
   wire          c3_p0_wr_error;
   wire          c3_p0_rd_clk;
   wire          c3_p0_rd_en;
   wire [31:0]   c3_p0_rd_data;
   wire          c3_p0_rd_full;
   wire          c3_p0_rd_empty;
   wire [6:0]    c3_p0_rd_count;
   wire          c3_p0_rd_overflow;
   wire          c3_p0_rd_error;
   wire          c3_p1_cmd_clk;
   wire          c3_p1_cmd_en;
   wire [2:0]    c3_p1_cmd_instr;
   wire [5:0]    c3_p1_cmd_bl;
   wire [29:0]   c3_p1_cmd_byte_addr;
   wire          c3_p1_cmd_empty;
   wire          c3_p1_cmd_full;
   wire          c3_p1_wr_clk;
   wire          c3_p1_wr_en;
   wire [3:0]    c3_p1_wr_mask;
   wire [31:0]   c3_p1_wr_data;
   wire          c3_p1_wr_full;
   wire          c3_p1_wr_empty;
   wire [6:0]    c3_p1_wr_count;
   wire          c3_p1_wr_underrun;
   wire          c3_p1_wr_error;
   wire          c3_p1_rd_clk;
   wire          c3_p1_rd_en;
   wire [31:0]   c3_p1_rd_data;
   wire          c3_p1_rd_full;
   wire          c3_p1_rd_empty;
   wire [6:0]    c3_p1_rd_count;
   wire          c3_p1_rd_overflow;
   wire          c3_p1_rd_error;
   wire          c3_p2_cmd_clk;
   wire          c3_p2_cmd_en;
   wire [2:0]    c3_p2_cmd_instr;
   wire [5:0]    c3_p2_cmd_bl;
   wire [29:0]   c3_p2_cmd_byte_addr;
   wire          c3_p2_cmd_empty;
   wire          c3_p2_cmd_full;
   wire          c3_p2_wr_clk;
   wire          c3_p2_wr_en;
   wire [3:0]    c3_p2_wr_mask;
   wire [31:0]   c3_p2_wr_data;
   wire          c3_p2_wr_full;
   wire          c3_p2_wr_empty;
   wire [6:0]    c3_p2_wr_count;
   wire          c3_p2_wr_underrun;
   wire          c3_p2_wr_error;
   wire          c3_p2_rd_clk;
   wire          c3_p2_rd_en;
   wire [31:0]   c3_p2_rd_data;
   wire          c3_p2_rd_full;
   wire          c3_p2_rd_empty;
   wire [6:0]    c3_p2_rd_count;
   wire          c3_p2_rd_overflow;
   wire          c3_p2_rd_error;
   wire          c3_p3_cmd_clk;
   wire          c3_p3_cmd_en;
   wire [2:0]    c3_p3_cmd_instr;
   wire [5:0]    c3_p3_cmd_bl;
   wire [29:0]   c3_p3_cmd_byte_addr;
   wire          c3_p3_cmd_empty;
   wire          c3_p3_cmd_full;
   wire          c3_p3_wr_clk;
   wire          c3_p3_wr_en;
   wire [3:0]    c3_p3_wr_mask;
   wire [31:0]   c3_p3_wr_data;
   wire          c3_p3_wr_full;
   wire          c3_p3_wr_empty;
   wire [6:0]    c3_p3_wr_count;
   wire          c3_p3_wr_underrun;
   wire          c3_p3_wr_error;
   wire          c3_p3_rd_clk;
   wire          c3_p3_rd_en;
   wire [31:0]   c3_p3_rd_data;
   wire          c3_p3_rd_full;
   wire          c3_p3_rd_empty;
   wire [6:0]    c3_p3_rd_count;
   wire          c3_p3_rd_overflow;
   wire          c3_p3_rd_error;

   //-------------------------------------------------------------------------------------
   // Calibration DONE Logic
   //-------------------------------------------------------------------------------------
   //
   // The DDR2 MIG outputs a wierd ddr2_calib_done signal, as a ddr2_rst strobe will force
   // a recalibration (which is not defined in UG388) but with a certain delay with
   // respect to that strobe. The rising scenario is, that the system will start-up after
   // the reset and that the ddr2 mig will cause a second reset after a while. To avoid
   // that and to abstract from this behavior, this logic keeps the ddr2_calib_done low
   // until it is sure, that the calibration has finished.
   //-------------------------------------------------------------------------------------
   reg           ddr2_calib_done;               
   reg           ddr2_calib_has_started;

   always @(posedge ddr2_clk) begin
      if (ddr2_rst) begin
         ddr2_calib_done <= 0;
         ddr2_calib_has_started <= 0;
      end else if (!c3_calib_done) begin
         ddr2_calib_done <= 0;
         ddr2_calib_has_started <= 1;
      end else if (c3_calib_done && ddr2_calib_has_started) begin
         ddr2_calib_done <= 1;
         ddr2_calib_has_started <= 0;
      end
   end // always @ (posedge ddr2_clk)
   


   //-------------------------------------------------------------------------------------
   // WISHBONE wrapper instatiation
   //
   // The MIG offers four bidirectional ports so we instatiate four wishbone to memory
   // interfaces and adject each of them to one of these ports.
   //-------------------------------------------------------------------------------------
   
   /* wb_mig_if AUTO_TEMPLATE (
    .wb_\(.*\)(wbm@_\1),
    .\(.*\)_[io](c3_p@_\1),
    );*/
   generate
      if (USE_WBPORT0) begin : genwbport0
         wb_mig_if u_wb_mig_if0 (/*AUTOINST*/
                                 // Outputs
                                 .wb_dat_o              (wbm0_dat_o),    // Templated
                                 .wb_ack_o              (wbm0_ack_o),    // Templated
                                 .wb_err_o              (wbm0_err_o),    // Templated
                                 .wb_rty_o              (wbm0_rty_o),    // Templated
                                 .cmd_clk_o             (c3_p0_cmd_clk), // Templated
                                 .cmd_en_o              (c3_p0_cmd_en),  // Templated
                                 .cmd_instr_o           (c3_p0_cmd_instr), // Templated
                                 .cmd_bl_o              (c3_p0_cmd_bl),  // Templated
                                 .cmd_byte_addr_o       (c3_p0_cmd_byte_addr), // Templated
                                 .wr_clk_o              (c3_p0_wr_clk),  // Templated
                                 .wr_en_o               (c3_p0_wr_en),   // Templated
                                 .wr_mask_o             (c3_p0_wr_mask), // Templated
                                 .wr_data_o             (c3_p0_wr_data), // Templated
                                 .rd_clk_o              (c3_p0_rd_clk),  // Templated
                                 .rd_en_o               (c3_p0_rd_en),   // Templated
                                 // Inputs
                                 .wb_adr_i              (wbm0_adr_i),    // Templated
                                 .wb_bte_i              (wbm0_bte_i),    // Templated
                                 .wb_cti_i              (wbm0_cti_i),    // Templated
                                 .wb_cyc_i              (wbm0_cyc_i),    // Templated
                                 .wb_dat_i              (wbm0_dat_i),    // Templated
                                 .wb_sel_i              (wbm0_sel_i),    // Templated
                                 .wb_stb_i              (wbm0_stb_i),    // Templated
                                 .wb_we_i               (wbm0_we_i),     // Templated
                                 .wb_clk_i              (wbm0_clk_i),    // Templated
                                 .wb_rst_i              (wbm0_rst_i),    // Templated
                                 .cmd_empty_i           (c3_p0_cmd_empty), // Templated
                                 .cmd_full_i            (c3_p0_cmd_full), // Templated
                                 .wr_full_i             (c3_p0_wr_full), // Templated
                                 .wr_empty_i            (c3_p0_wr_empty), // Templated
                                 .wr_count_i            (c3_p0_wr_count), // Templated
                                 .wr_underrun_i         (c3_p0_wr_underrun), // Templated
                                 .wr_error_i            (c3_p0_wr_error), // Templated
                                 .rd_data_i             (c3_p0_rd_data), // Templated
                                 .rd_full_i             (c3_p0_rd_full), // Templated
                                 .rd_empty_i            (c3_p0_rd_empty), // Templated
                                 .rd_count_i            (c3_p0_rd_count), // Templated
                                 .rd_overflow_i         (c3_p0_rd_overflow), // Templated
                                 .rd_error_i            (c3_p0_rd_error)); // Templated
      end else begin // block: genwbport0
         assign wbm1_dat_o = 32'hx;
         assign {wbm1_ack_o, wbm1_rty_o, wbm1_err_o} = 3'b000;
         assign c3_p1_cmd_clk = 0;
         assign c3_p1_rd_clk = 0;
         assign c3_p1_wr_clk = 0;
      end
      
      if (USE_WBPORT1) begin : genwbport1
         wb_mig_if u_wb_mig_if1 (/*AUTOINST*/
                                 // Outputs
                                 .wb_dat_o              (wbm1_dat_o),    // Templated
                                 .wb_ack_o              (wbm1_ack_o),    // Templated
                                 .wb_err_o              (wbm1_err_o),    // Templated
                                 .wb_rty_o              (wbm1_rty_o),    // Templated
                                 .cmd_clk_o             (c3_p1_cmd_clk), // Templated
                                 .cmd_en_o              (c3_p1_cmd_en),  // Templated
                                 .cmd_instr_o           (c3_p1_cmd_instr), // Templated
                                 .cmd_bl_o              (c3_p1_cmd_bl),  // Templated
                                 .cmd_byte_addr_o       (c3_p1_cmd_byte_addr), // Templated
                                 .wr_clk_o              (c3_p1_wr_clk),  // Templated
                                 .wr_en_o               (c3_p1_wr_en),   // Templated
                                 .wr_mask_o             (c3_p1_wr_mask), // Templated
                                 .wr_data_o             (c3_p1_wr_data), // Templated
                                 .rd_clk_o              (c3_p1_rd_clk),  // Templated
                                 .rd_en_o               (c3_p1_rd_en),   // Templated
                                 // Inputs
                                 .wb_adr_i              (wbm1_adr_i),    // Templated
                                 .wb_bte_i              (wbm1_bte_i),    // Templated
                                 .wb_cti_i              (wbm1_cti_i),    // Templated
                                 .wb_cyc_i              (wbm1_cyc_i),    // Templated
                                 .wb_dat_i              (wbm1_dat_i),    // Templated
                                 .wb_sel_i              (wbm1_sel_i),    // Templated
                                 .wb_stb_i              (wbm1_stb_i),    // Templated
                                 .wb_we_i               (wbm1_we_i),     // Templated
                                 .wb_clk_i              (wbm1_clk_i),    // Templated
                                 .wb_rst_i              (wbm1_rst_i),    // Templated
                                 .cmd_empty_i           (c3_p1_cmd_empty), // Templated
                                 .cmd_full_i            (c3_p1_cmd_full), // Templated
                                 .wr_full_i             (c3_p1_wr_full), // Templated
                                 .wr_empty_i            (c3_p1_wr_empty), // Templated
                                 .wr_count_i            (c3_p1_wr_count), // Templated
                                 .wr_underrun_i         (c3_p1_wr_underrun), // Templated
                                 .wr_error_i            (c3_p1_wr_error), // Templated
                                 .rd_data_i             (c3_p1_rd_data), // Templated
                                 .rd_full_i             (c3_p1_rd_full), // Templated
                                 .rd_empty_i            (c3_p1_rd_empty), // Templated
                                 .rd_count_i            (c3_p1_rd_count), // Templated
                                 .rd_overflow_i         (c3_p1_rd_overflow), // Templated
                                 .rd_error_i            (c3_p1_rd_error)); // Templated
      end else begin // block: genwbport1
         assign wbm1_dat_o = 32'hx;
         assign {wbm1_ack_o, wbm1_rty_o, wbm1_err_o} = 3'b000;
         assign c3_p1_cmd_clk = 0;
         assign c3_p1_rd_clk = 0;
         assign c3_p1_wr_clk = 0;
      end

      if (USE_WBPORT2) begin : genwbport2
         wb_mig_if u_wb_mig_if2 (/*AUTOINST*/
                                 // Outputs
                                 .wb_dat_o              (wbm2_dat_o),    // Templated
                                 .wb_ack_o              (wbm2_ack_o),    // Templated
                                 .wb_err_o              (wbm2_err_o),    // Templated
                                 .wb_rty_o              (wbm2_rty_o),    // Templated
                                 .cmd_clk_o             (c3_p2_cmd_clk), // Templated
                                 .cmd_en_o              (c3_p2_cmd_en),  // Templated
                                 .cmd_instr_o           (c3_p2_cmd_instr), // Templated
                                 .cmd_bl_o              (c3_p2_cmd_bl),  // Templated
                                 .cmd_byte_addr_o       (c3_p2_cmd_byte_addr), // Templated
                                 .wr_clk_o              (c3_p2_wr_clk),  // Templated
                                 .wr_en_o               (c3_p2_wr_en),   // Templated
                                 .wr_mask_o             (c3_p2_wr_mask), // Templated
                                 .wr_data_o             (c3_p2_wr_data), // Templated
                                 .rd_clk_o              (c3_p2_rd_clk),  // Templated
                                 .rd_en_o               (c3_p2_rd_en),   // Templated
                                 // Inputs
                                 .wb_adr_i              (wbm2_adr_i),    // Templated
                                 .wb_bte_i              (wbm2_bte_i),    // Templated
                                 .wb_cti_i              (wbm2_cti_i),    // Templated
                                 .wb_cyc_i              (wbm2_cyc_i),    // Templated
                                 .wb_dat_i              (wbm2_dat_i),    // Templated
                                 .wb_sel_i              (wbm2_sel_i),    // Templated
                                 .wb_stb_i              (wbm2_stb_i),    // Templated
                                 .wb_we_i               (wbm2_we_i),     // Templated
                                 .wb_clk_i              (wbm2_clk_i),    // Templated
                                 .wb_rst_i              (wbm2_rst_i),    // Templated
                                 .cmd_empty_i           (c3_p2_cmd_empty), // Templated
                                 .cmd_full_i            (c3_p2_cmd_full), // Templated
                                 .wr_full_i             (c3_p2_wr_full), // Templated
                                 .wr_empty_i            (c3_p2_wr_empty), // Templated
                                 .wr_count_i            (c3_p2_wr_count), // Templated
                                 .wr_underrun_i         (c3_p2_wr_underrun), // Templated
                                 .wr_error_i            (c3_p2_wr_error), // Templated
                                 .rd_data_i             (c3_p2_rd_data), // Templated
                                 .rd_full_i             (c3_p2_rd_full), // Templated
                                 .rd_empty_i            (c3_p2_rd_empty), // Templated
                                 .rd_count_i            (c3_p2_rd_count), // Templated
                                 .rd_overflow_i         (c3_p2_rd_overflow), // Templated
                                 .rd_error_i            (c3_p2_rd_error)); // Templated
      end else begin // block: genwbport2
         assign wbm2_dat_o = 32'hx;
         assign {wbm2_ack_o, wbm2_rty_o, wbm2_err_o} = 3'b000;
         assign c3_p2_cmd_clk = 0;
         assign c3_p2_rd_clk = 0;
         assign c3_p2_wr_clk = 0;
      end

      if (USE_WBPORT3) begin : genwbport3
         wb_mig_if u_wb_mig_if3(/*AUTOINST*/
                                // Outputs
                                .wb_dat_o       (wbm3_dat_o),    // Templated
                                .wb_ack_o       (wbm3_ack_o),    // Templated
                                .wb_err_o       (wbm3_err_o),    // Templated
                                .wb_rty_o       (wbm3_rty_o),    // Templated
                                .cmd_clk_o      (c3_p3_cmd_clk), // Templated
                                .cmd_en_o       (c3_p3_cmd_en),  // Templated
                                .cmd_instr_o    (c3_p3_cmd_instr), // Templated
                                .cmd_bl_o       (c3_p3_cmd_bl),  // Templated
                                .cmd_byte_addr_o(c3_p3_cmd_byte_addr), // Templated
                                .wr_clk_o       (c3_p3_wr_clk),  // Templated
                                .wr_en_o        (c3_p3_wr_en),   // Templated
                                .wr_mask_o      (c3_p3_wr_mask), // Templated
                                .wr_data_o      (c3_p3_wr_data), // Templated
                                .rd_clk_o       (c3_p3_rd_clk),  // Templated
                                .rd_en_o        (c3_p3_rd_en),   // Templated
                                // Inputs
                                .wb_adr_i       (wbm3_adr_i),    // Templated
                                .wb_bte_i       (wbm3_bte_i),    // Templated
                                .wb_cti_i       (wbm3_cti_i),    // Templated
                                .wb_cyc_i       (wbm3_cyc_i),    // Templated
                                .wb_dat_i       (wbm3_dat_i),    // Templated
                                .wb_sel_i       (wbm3_sel_i),    // Templated
                                .wb_stb_i       (wbm3_stb_i),    // Templated
                                .wb_we_i        (wbm3_we_i),     // Templated
                                .wb_clk_i       (wbm3_clk_i),    // Templated
                                .wb_rst_i       (wbm3_rst_i),    // Templated
                                .cmd_empty_i    (c3_p3_cmd_empty), // Templated
                                .cmd_full_i     (c3_p3_cmd_full), // Templated
                                .wr_full_i      (c3_p3_wr_full), // Templated
                                .wr_empty_i     (c3_p3_wr_empty), // Templated
                                .wr_count_i     (c3_p3_wr_count), // Templated
                                .wr_underrun_i  (c3_p3_wr_underrun), // Templated
                                .wr_error_i     (c3_p3_wr_error), // Templated
                                .rd_data_i      (c3_p3_rd_data), // Templated
                                .rd_full_i      (c3_p3_rd_full), // Templated
                                .rd_empty_i     (c3_p3_rd_empty), // Templated
                                .rd_count_i     (c3_p3_rd_count), // Templated
                                .rd_overflow_i  (c3_p3_rd_overflow), // Templated
                                .rd_error_i     (c3_p3_rd_error)); // Templated
      end else begin // block: genwbport3
         assign wbm3_dat_o = 32'hx;
         assign {wbm3_ack_o, wbm3_rty_o, wbm3_err_o} = 3'b000;
         assign c3_p3_cmd_clk = 0;
         assign c3_p3_rd_clk = 0;
         assign c3_p3_wr_clk = 0;
      end
   endgenerate
   
   //-------------------------------------------------------------------------------------
   // DDR2 Memory Interface Generator Module Instatiation
   //-------------------------------------------------------------------------------------   

   /* ztex_1_15_mig_39 AUTO_TEMPLATE(
    .c3_sys_clk(ddr2_clk),
    .c3_sys_rst_i(ddr2_rst),
    .\(.*\)(\1), // suppress explict port widths
    );*/
   ztex_1_15_mig_39 u_mig_39(/*AUTOINST*/
                             // Outputs
                             .mcb3_dram_a       (mcb3_dram_a),   // Templated
                             .mcb3_dram_ba      (mcb3_dram_ba),  // Templated
                             .mcb3_dram_ras_n   (mcb3_dram_ras_n), // Templated
                             .mcb3_dram_cas_n   (mcb3_dram_cas_n), // Templated
                             .mcb3_dram_we_n    (mcb3_dram_we_n), // Templated
                             .mcb3_dram_cke     (mcb3_dram_cke), // Templated
                             .mcb3_dram_dm      (mcb3_dram_dm),  // Templated
                             .mcb3_dram_udm     (mcb3_dram_udm), // Templated
                             .c3_calib_done     (c3_calib_done), // Templated
                             .c3_clk0           (c3_clk0),       // Templated
                             .c3_rst0           (c3_rst0),       // Templated
                             .mcb3_dram_ck      (mcb3_dram_ck),  // Templated
                             .mcb3_dram_ck_n    (mcb3_dram_ck_n), // Templated
                             .c3_p0_cmd_empty   (c3_p0_cmd_empty), // Templated
                             .c3_p0_cmd_full    (c3_p0_cmd_full), // Templated
                             .c3_p0_wr_full     (c3_p0_wr_full), // Templated
                             .c3_p0_wr_empty    (c3_p0_wr_empty), // Templated
                             .c3_p0_wr_count    (c3_p0_wr_count), // Templated
                             .c3_p0_wr_underrun (c3_p0_wr_underrun), // Templated
                             .c3_p0_wr_error    (c3_p0_wr_error), // Templated
                             .c3_p0_rd_data     (c3_p0_rd_data), // Templated
                             .c3_p0_rd_full     (c3_p0_rd_full), // Templated
                             .c3_p0_rd_empty    (c3_p0_rd_empty), // Templated
                             .c3_p0_rd_count    (c3_p0_rd_count), // Templated
                             .c3_p0_rd_overflow (c3_p0_rd_overflow), // Templated
                             .c3_p0_rd_error    (c3_p0_rd_error), // Templated
                             .c3_p1_cmd_empty   (c3_p1_cmd_empty), // Templated
                             .c3_p1_cmd_full    (c3_p1_cmd_full), // Templated
                             .c3_p1_wr_full     (c3_p1_wr_full), // Templated
                             .c3_p1_wr_empty    (c3_p1_wr_empty), // Templated
                             .c3_p1_wr_count    (c3_p1_wr_count), // Templated
                             .c3_p1_wr_underrun (c3_p1_wr_underrun), // Templated
                             .c3_p1_wr_error    (c3_p1_wr_error), // Templated
                             .c3_p1_rd_data     (c3_p1_rd_data), // Templated
                             .c3_p1_rd_full     (c3_p1_rd_full), // Templated
                             .c3_p1_rd_empty    (c3_p1_rd_empty), // Templated
                             .c3_p1_rd_count    (c3_p1_rd_count), // Templated
                             .c3_p1_rd_overflow (c3_p1_rd_overflow), // Templated
                             .c3_p1_rd_error    (c3_p1_rd_error), // Templated
                             .c3_p2_cmd_empty   (c3_p2_cmd_empty), // Templated
                             .c3_p2_cmd_full    (c3_p2_cmd_full), // Templated
                             .c3_p2_wr_full     (c3_p2_wr_full), // Templated
                             .c3_p2_wr_empty    (c3_p2_wr_empty), // Templated
                             .c3_p2_wr_count    (c3_p2_wr_count), // Templated
                             .c3_p2_wr_underrun (c3_p2_wr_underrun), // Templated
                             .c3_p2_wr_error    (c3_p2_wr_error), // Templated
                             .c3_p2_rd_data     (c3_p2_rd_data), // Templated
                             .c3_p2_rd_full     (c3_p2_rd_full), // Templated
                             .c3_p2_rd_empty    (c3_p2_rd_empty), // Templated
                             .c3_p2_rd_count    (c3_p2_rd_count), // Templated
                             .c3_p2_rd_overflow (c3_p2_rd_overflow), // Templated
                             .c3_p2_rd_error    (c3_p2_rd_error), // Templated
                             .c3_p3_cmd_empty   (c3_p3_cmd_empty), // Templated
                             .c3_p3_cmd_full    (c3_p3_cmd_full), // Templated
                             .c3_p3_wr_full     (c3_p3_wr_full), // Templated
                             .c3_p3_wr_empty    (c3_p3_wr_empty), // Templated
                             .c3_p3_wr_count    (c3_p3_wr_count), // Templated
                             .c3_p3_wr_underrun (c3_p3_wr_underrun), // Templated
                             .c3_p3_wr_error    (c3_p3_wr_error), // Templated
                             .c3_p3_rd_data     (c3_p3_rd_data), // Templated
                             .c3_p3_rd_full     (c3_p3_rd_full), // Templated
                             .c3_p3_rd_empty    (c3_p3_rd_empty), // Templated
                             .c3_p3_rd_count    (c3_p3_rd_count), // Templated
                             .c3_p3_rd_overflow (c3_p3_rd_overflow), // Templated
                             .c3_p3_rd_error    (c3_p3_rd_error), // Templated
                             // Inouts
                             .mcb3_dram_dq      (mcb3_dram_dq),  // Templated
                             .mcb3_dram_udqs    (mcb3_dram_udqs), // Templated
                             .mcb3_dram_udqs_n  (mcb3_dram_udqs_n), // Templated
                             .mcb3_rzq          (mcb3_rzq),      // Templated
                             .mcb3_zio          (mcb3_zio),      // Templated
                             .mcb3_dram_dqs     (mcb3_dram_dqs), // Templated
                             .mcb3_dram_dqs_n   (mcb3_dram_dqs_n), // Templated
                             // Inputs
                             .c3_sys_clk        (ddr2_clk),      // Templated
                             .c3_sys_rst_i      (ddr2_rst),      // Templated
                             .c3_p0_cmd_clk     (c3_p0_cmd_clk), // Templated
                             .c3_p0_cmd_en      (c3_p0_cmd_en),  // Templated
                             .c3_p0_cmd_instr   (c3_p0_cmd_instr), // Templated
                             .c3_p0_cmd_bl      (c3_p0_cmd_bl),  // Templated
                             .c3_p0_cmd_byte_addr(c3_p0_cmd_byte_addr), // Templated
                             .c3_p0_wr_clk      (c3_p0_wr_clk),  // Templated
                             .c3_p0_wr_en       (c3_p0_wr_en),   // Templated
                             .c3_p0_wr_mask     (c3_p0_wr_mask), // Templated
                             .c3_p0_wr_data     (c3_p0_wr_data), // Templated
                             .c3_p0_rd_clk      (c3_p0_rd_clk),  // Templated
                             .c3_p0_rd_en       (c3_p0_rd_en),   // Templated
                             .c3_p1_cmd_clk     (c3_p1_cmd_clk), // Templated
                             .c3_p1_cmd_en      (c3_p1_cmd_en),  // Templated
                             .c3_p1_cmd_instr   (c3_p1_cmd_instr), // Templated
                             .c3_p1_cmd_bl      (c3_p1_cmd_bl),  // Templated
                             .c3_p1_cmd_byte_addr(c3_p1_cmd_byte_addr), // Templated
                             .c3_p1_wr_clk      (c3_p1_wr_clk),  // Templated
                             .c3_p1_wr_en       (c3_p1_wr_en),   // Templated
                             .c3_p1_wr_mask     (c3_p1_wr_mask), // Templated
                             .c3_p1_wr_data     (c3_p1_wr_data), // Templated
                             .c3_p1_rd_clk      (c3_p1_rd_clk),  // Templated
                             .c3_p1_rd_en       (c3_p1_rd_en),   // Templated
                             .c3_p2_cmd_clk     (c3_p2_cmd_clk), // Templated
                             .c3_p2_cmd_en      (c3_p2_cmd_en),  // Templated
                             .c3_p2_cmd_instr   (c3_p2_cmd_instr), // Templated
                             .c3_p2_cmd_bl      (c3_p2_cmd_bl),  // Templated
                             .c3_p2_cmd_byte_addr(c3_p2_cmd_byte_addr), // Templated
                             .c3_p2_wr_clk      (c3_p2_wr_clk),  // Templated
                             .c3_p2_wr_en       (c3_p2_wr_en),   // Templated
                             .c3_p2_wr_mask     (c3_p2_wr_mask), // Templated
                             .c3_p2_wr_data     (c3_p2_wr_data), // Templated
                             .c3_p2_rd_clk      (c3_p2_rd_clk),  // Templated
                             .c3_p2_rd_en       (c3_p2_rd_en),   // Templated
                             .c3_p3_cmd_clk     (c3_p3_cmd_clk), // Templated
                             .c3_p3_cmd_en      (c3_p3_cmd_en),  // Templated
                             .c3_p3_cmd_instr   (c3_p3_cmd_instr), // Templated
                             .c3_p3_cmd_bl      (c3_p3_cmd_bl),  // Templated
                             .c3_p3_cmd_byte_addr(c3_p3_cmd_byte_addr), // Templated
                             .c3_p3_wr_clk      (c3_p3_wr_clk),  // Templated
                             .c3_p3_wr_en       (c3_p3_wr_en),   // Templated
                             .c3_p3_wr_mask     (c3_p3_wr_mask), // Templated
                             .c3_p3_wr_data     (c3_p3_wr_data), // Templated
                             .c3_p3_rd_clk      (c3_p3_rd_clk),  // Templated
                             .c3_p3_rd_en       (c3_p3_rd_en));  // Templated
   
endmodule // ztex115b_ddr2_mig

// Local Variables:
// verilog-library-directories:("../../*/verilog/" "*")
// verilog-auto-inst-param-value: t
// End: