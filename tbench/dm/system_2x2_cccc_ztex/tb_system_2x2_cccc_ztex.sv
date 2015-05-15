/* Copyright (c) 2012-2014 by the author(s)
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
 * Testbench for the system_2x2_cccc_dm module with debug system
 *
 * This system is equal to the system_2x2_cccc_ztex for the ZTEX boards, but
 * runs in simulation (ModelSim) and uses TCP for the host connection instead
 * of USB.
 *
 * Target: ModelSim
 * System: system_2x2_cccc_dm
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 *   Hans-Christian Wild <hans-christian.wild@mytum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

`timescale 1ns/1ps

`include "lisnoc16_def.vh"
`include "optimsoc_def.vh"
`include "dbg_config.vh"

module tb_system_2x2_cccc_ztex;

   // 32 MByte for each of the four compute tiles (128 MByte total)
   // The ZTEX 1.15 boards provide 128 MByte DDR2 memory, which is
   // used by the compute tile memories.
   localparam MEM_SIZE = 32*1024*1024;
   // Memory file, used only in simulation
   parameter MEM_FILE = "ct.vmem";

   // NoC configuration
   localparam NOC_FLIT_DATA_WIDTH = 32;
   localparam NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   localparam NOC_VCHANNELS = `VCHANNELS;

   // Debug NoC configuration
   localparam DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   localparam DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   localparam DBG_NOC_VCHANNELS = 2;
   localparam DBG_NOC_CONF_VCHANNEL = 0;
   localparam DBG_NOC_TRACE_VCHANNEL = 1;

   // system control signals
   wire sys_clk_disable;
   wire sys_clk_is_halted;

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

   wire glip_ctrl_logic_rst;

   // generate 48 MHz clock, as used on the ZTEX boards
   reg clk;
   initial begin
      clk = 1;
   end
   always clk = #10.41ns ~clk;

   clockmanager_ztex115
      u_clockmanager(.clk     (clk),
                     .rst     (glip_ctrl_logic_rst),
                     .clk_ct  (clk_sys),
                     .clk_dbg (clk_dbg),
                     .clk_noc (),
                     .clk_ddr (),
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
               .clk                        (clk_sys),
               .rst_sys                    (rst_sys),
               .rst_cpu                    (rst_cpu));

   // Host interface using GLIP
   wire        glip_fifo_out_valid;
   wire        glip_fifo_out_ready;
   wire [15:0] glip_fifo_out_data;
   wire        glip_fifo_in_valid;
   wire        glip_fifo_in_ready;
   wire [15:0] glip_fifo_in_data;

   wire glip_com_rst;

   glip_tcp_toplevel
      u_glip(// Clock and reset
             .clk                       (clk_dbg),

             // XXX: Feeding rst_sys to the rst input of GLIP is a workaround
             // In order to support hot-attach, GLIP has separate communication
             // and logic reset signals. But resetting the communication also
             // requires the Debug NoC -> FIFO communication to be reset to a
             // well-known starting point (i.e. the beginning of a NoC packet).
             // This is currently not implemented. If the communication starts
             // in the middle of a packet, there is no way for the receiver to
             // synchronize to the data stream. As a workaround, we simply
             // reset the whole system when connecting, thus no hot-attach
             // capability, and make sure we also reset the communication
             // interface when the system is reset.
             .rst                       (rst_sys),

             .com_rst                   (glip_com_rst),

             // FIFO Interface
             .fifo_out_ready            (glip_fifo_out_ready),
             .fifo_in_valid             (glip_fifo_in_valid),
             .fifo_in_data              (glip_fifo_in_data),
             .fifo_out_valid            (glip_fifo_out_valid),
             .fifo_out_data             (glip_fifo_out_data),
             .fifo_in_ready             (glip_fifo_in_ready),

             // Control interface
             .ctrl_logic_rst            (glip_ctrl_logic_rst));

   fifo_dbg_if
      #(.DBG_NOC_VCHANNELS(DBG_NOC_VCHANNELS),
        .DBG_NOC_USB_VCHANNEL(DBG_NOC_CONF_VCHANNEL))
      u_fifo_dbg_if(.clk(clk_dbg),
                    // This module is a "high level protocol" for GLIP, use the
                    // provided communication reset signal from GLIP; see also
                    // the comment on u_glip.rst above.
                    .rst(glip_com_rst),

                    // FIFO Interface
                    .fifo_out_valid     (glip_fifo_out_valid),
                    .fifo_out_data      (glip_fifo_out_data),
                    .fifo_in_ready      (glip_fifo_in_ready),
                    .fifo_out_ready     (glip_fifo_out_ready),
                    .fifo_in_valid      (glip_fifo_in_valid),
                    .fifo_in_data       (glip_fifo_in_data),

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
       .DBG_NOC_VCHANNELS               (DBG_NOC_VCHANNELS),
       .DBG_NOC_TRACE_VCHANNEL          (DBG_NOC_TRACE_VCHANNEL),
       .DBG_NOC_CONF_VCHANNEL           (DBG_NOC_CONF_VCHANNEL),
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
// verilog-library-directories:("../../../../src/rtl/*/verilog/" "../../../" "." )
// verilog-auto-inst-param-value: t
// End:
