/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using OpTiMSoC in your projects at
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
 *
 * Top-level module for a 2x2 CCCC distributed memory system running on a
 * ZTEX 1.15d board
 *
 * Hardware target: ZTEX 1.15b/d USB-FPGA Boards
 * System: system_2x2_cccc_dm
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 *    Michael Tempelmeier, michael.tempelmeier@tum.de
 *    Hans-Christian Wild, hans-christian.wild@mytum.de
 *    Philipp Wagner, philipp.wagner@tum.de
 */

`include "lisnoc16_def.vh"
`include "optimsoc_def.vh"
`include "dbg_config.vh"

module system_2x2_cccc_ztex(
   /*AUTOARG*/
   // Outputs
   fx2_sloe, fx2_slrd, fx2_slwr, fx2_pktend, fx2_fifoadr,
   // Inouts
   fx2_fd,
   // Inputs
   clk, rst, fx2_ifclk, fx2_flaga, fx2_flagb, fx2_flagc, fx2_flagd
   );

   // 128 kByte for each of the four compute tiles (4*128 kByte total)
   localparam MEM_SIZE = 128*1024;
   // Memory file, used only in simulation
   localparam MEM_FILE = "ct.vmem";

   // NoC configuration
   localparam NOC_FLIT_DATA_WIDTH = 32;
   localparam NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   localparam NOC_VCHANNELS = `VCHANNELS;

   // Debug NoC configuration
   localparam DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   localparam DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   localparam DBG_NOC_VCHANNELS = 1;

   localparam DEBUG_ROUTER_COUNT = 4;
   localparam DEBUG_ROUTER_LINKS_PER_ROUTER = 1;

   // Clock and resets inputs
   input clk;
   input rst;

   // FX2 USB interface
   input fx2_ifclk;
   inout [15:0] fx2_fd;
   output fx2_sloe;
   output fx2_slrd;
   output fx2_slwr;
   output fx2_pktend;
   output [1:0] fx2_fifoadr;
   input fx2_flaga;
   input fx2_flagb;
   input fx2_flagc;
   input fx2_flagd;

   // system control signals
   wire sys_clk_disable;
   wire sys_clk_is_halted;

   /*
    * Manually insert I/O buffers
    * When using DDR2 memory automatic I/O Insertation in Synplify needs to
    * be disabled.
    */
   wire rst_buf;
   wire fx2_flaga_buf;
   wire fx2_flagb_buf;
   wire fx2_flagc_buf;
   wire fx2_flagd_buf;
   wire fx2_sloe_buf;
   wire fx2_slrd_buf;
   wire fx2_slwr_buf;
   wire fx2_pktend_buf;
   wire [1:0] fx2_fifoadr_buf;
   wire [15:0] fx2_fd_in_buf;
   wire [15:0] fx2_fd_out_buf;
   wire fx2_ifclk_buf;

   // clock
   assign fx2_ifclk_buf = fx2_ifclk;

   // input
   assign rst_buf = rst;
   assign fx2_flaga_buf = fx2_flaga;
   assign fx2_flagb_buf = fx2_flagb;
   assign fx2_flagc_buf = fx2_flagc;
   assign fx2_flagd_buf = fx2_flagd;

   // output
   assign fx2_sloe = fx2_sloe_buf;
   assign fx2_slrd = fx2_slrd_buf;
   assign fx2_slwr = fx2_slwr_buf;
   assign fx2_pktend = fx2_pktend_buf;
   assign fx2_fifoadr = fx2_fifoadr_buf;

   // inout
   assign fx2_fd_in_buf = fx2_fd;
   assign fx2_fd = (~fx2_slwr ? fx2_fd_out_buf : 16'hz);


   // system control
   wire cpu_reset;
   wire cpu_stall;
   wire start_cpu;

   // connection to the USB interface
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;

   wire clk_sys;
   wire clk_dbg;
   wire rst_sys;
   wire rst_cpu;

   clockmanager_ztex115
      u_clockmanager(.clk     (clk),
                     .rst     (rst_buf),
                     .clk_ct  (clk_sys),
                     .clk_dbg (clk_dbg),
                     .clk_noc (),
                     .rst_sys  (rst_sys),
                     .rst_cpu  (rst_cpu),
                     .cpu_reset (cpu_reset),
                     .cpu_start (start_cpu),
                     .sys_halt (sys_clk_disable),
                     .sys_is_halted (sys_clk_is_halted));

   wire [`DEBUG_ITM_PORTWIDTH*4-1:0] trace_itm;
   wire [`DEBUG_STM_PORTWIDTH*4-1:0] trace_stm;


`ifdef OPTIMSOC_DEBUG_ENABLE_MAM
   wire [4*32-1:0] wb_mam_adr_o;
   wire [4*1-1:0]  wb_mam_cyc_o;
   wire [4*32-1:0] wb_mam_dat_o;
   wire [4*4-1:0]  wb_mam_sel_o;
   wire [4*1-1:0]  wb_mam_stb_o;
   wire [4*1-1:0]  wb_mam_we_o;
   wire [4*1-1:0]  wb_mam_cab_o;
   wire [4*3-1:0]  wb_mam_cti_o;
   wire [4*2-1:0]  wb_mam_bte_o;
   wire [4*1-1:0]  wb_mam_ack_i;
   wire [4*1-1:0]  wb_mam_rty_i;
   wire [4*1-1:0]  wb_mam_err_i;
   wire [4*32-1:0] wb_mam_dat_i;
`endif

   /* system_2x2_cccc_dm AUTO_TEMPLATE(
    .clk (clk_sys),
    .rst_sys (rst_sys),
    .rst_cpu (rst_cpu),
    ); */
   system_2x2_cccc_dm
      #(.MEM_SIZE(MEM_SIZE),
        .MEM_FILE(MEM_FILE))
      u_system(
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
               .trace_itm                  (trace_itm[4*`DEBUG_ITM_PORTWIDTH-1:0]),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
               .trace_stm                  (trace_stm[4*`DEBUG_STM_PORTWIDTH-1:0]),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_MAM
               .wb_mam_ack_i               (wb_mam_ack_i),
               .wb_mam_rty_i               (wb_mam_rty_i),
               .wb_mam_err_i               (wb_mam_err_i),
               .wb_mam_dat_i               (wb_mam_dat_i),
               .wb_mam_adr_o               (wb_mam_adr_o),
               .wb_mam_cyc_o               (wb_mam_cyc_o),
               .wb_mam_dat_o               (wb_mam_dat_o),
               .wb_mam_sel_o               (wb_mam_sel_o),
               .wb_mam_stb_o               (wb_mam_stb_o),
               .wb_mam_we_o                (wb_mam_we_o),
               .wb_mam_cab_o               (wb_mam_cab_o),
               .wb_mam_cti_o               (wb_mam_cti_o),
               .wb_mam_bte_o               (wb_mam_bte_o),
`endif
               /*AUTOINST*/
               // Inputs
               .clk                     (clk_sys),               // Templated
               .rst_sys                 (rst_sys),               // Templated
               .rst_cpu                 (rst_cpu));               // Templated

   // USB interface
   usb_dbg_if
      u_usb(.clk_sys(clk_dbg),
            .rst(rst_sys),

            // FX2 interface
            .fx2_clk(fx2_ifclk_buf),
            .fx2_epout_fifo_empty(fx2_flaga_buf),
            .fx2_epin_fifo_almost_full(fx2_flagd_buf),
            .fx2_epin_fifo_full(fx2_flagc_buf),
            .fx2_slrd(fx2_slrd_buf),
            .fx2_slwr(fx2_slwr_buf),
            .fx2_sloe(fx2_sloe_buf),
            .fx2_pktend(fx2_pktend_buf),
            .fx2_fifoadr(fx2_fifoadr_buf),
            .fx2_fd_out(fx2_fd_out_buf),
            .fx2_fd_in(fx2_fd_in_buf),

            // Debug NoC interface
            .dbgnoc_out_ready(dbgnoc_in_ready),
            .dbgnoc_out_flit(dbgnoc_in_flit),
            .dbgnoc_out_valid(dbgnoc_in_valid),
            .dbgnoc_in_ready(dbgnoc_out_ready),
            .dbgnoc_in_flit(dbgnoc_out_flit),
            .dbgnoc_in_valid(dbgnoc_out_valid));

   // debug system
   debug_system
     #(
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
       .DEBUG_ITM_CORE_COUNT            (4),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
       .DEBUG_STM_CORE_COUNT            (4),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_NRM
       .DEBUG_ROUTER_COUNT              (4),
       .DEBUG_ROUTER_LINKS_PER_ROUTER   (1),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_NCM
       .OPTIMSOC_DEBUG_NCM_ID           (1),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_MAM
       .DEBUG_MAM_MEMORY_COUNT          (4),
`endif
       .NOC_VCHANNELS                   (`VCHANNELS),
       .NOC_USED_VCHANNEL               (`VCHANNEL_LSU_REQ),
       .SYSTEM_IDENTIFIER (16'hce75)) // FIXME: Change system identifier
      u_dbg_system(.clk                 (clk_dbg),
                   .rst                 (rst_sys),

                   // System control
                   .sys_clk_disable     (sys_clk_disable),
                   .cpu_reset           (cpu_reset),
                   .cpu_stall           (cpu_stall),
                   .start_cpu           (start_cpu),
                   .sys_clk_is_halted   (sys_clk_is_halted),

`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
                   .itm_ports_flat      (trace_itm),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
                   .stm_ports_flat      (trace_stm),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_MAM
                   .wb_mam_ack_i        (wb_mam_ack_i),
                   .wb_mam_rty_i        (wb_mam_rty_i),
                   .wb_mam_err_i        (wb_mam_err_i),
                   .wb_mam_dat_i        (wb_mam_dat_i),
                   .wb_mam_adr_o        (wb_mam_adr_o),
                   .wb_mam_cyc_o        (wb_mam_cyc_o),
                   .wb_mam_dat_o        (wb_mam_dat_o),
                   .wb_mam_sel_o        (wb_mam_sel_o),
                   .wb_mam_stb_o        (wb_mam_stb_o),
                   .wb_mam_we_o         (wb_mam_we_o),
                   .wb_mam_cab_o        (wb_mam_cab_o),
                   .wb_mam_cti_o        (wb_mam_cti_o),
                   .wb_mam_bte_o        (wb_mam_bte_o),
`endif
                   // Debug NoC link to USB
                   .dbgnoc_in_flit      (dbgnoc_in_flit),
                   .dbgnoc_in_valid     (dbgnoc_in_valid),
                   .dbgnoc_in_ready     (dbgnoc_in_ready),
                   .dbgnoc_out_flit     (dbgnoc_out_flit),
                   .dbgnoc_out_valid    (dbgnoc_out_valid),
                   .dbgnoc_out_ready    (dbgnoc_out_ready));

   `include "optimsoc_functions.vh"
endmodule

// Local Variables:
// verilog-library-directories:("../../../src/rtl/*/verilog/" "../../../" "." )
// verilog-auto-inst-param-value: t
// End:
