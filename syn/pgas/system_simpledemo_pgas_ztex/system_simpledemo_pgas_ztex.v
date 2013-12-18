/* Copyright (c) 2012-2013 by the author(s)
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
 * Top-level module for a simple PGAS demo system
 *
 * Hardware target: ZTEX 1.15b/d USB-FPGA Boards
 * System: system_simpledemo_pgas
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 *   Hans-Christian Wild <hans-christian.wild@mytum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

`include "lisnoc16_def.vh"
`include "optimsoc_def.vh"
`include "dbg_config.vh"

module system_simpledemo_pgas_ztex(
`ifdef OPTIMSOC_MAINMEM_DDR                        
   mcb3_dram_a, mcb3_dram_ba, mcb3_dram_ras_n, mcb3_dram_cas_n,
   mcb3_dram_we_n, mcb3_dram_cke, mcb3_dram_dm, mcb3_dram_udm,
   mcb3_dram_ck, mcb3_dram_ck_n, mcb3_dram_dq, mcb3_dram_udqs,
   mcb3_dram_udqs_n, mcb3_rzq, mcb3_zio, mcb3_dram_dqs,
   mcb3_dram_dqs_n,
`endif
   /*AUTOARG*/
   // Outputs
   fx2_sloe, fx2_slrd, fx2_slwr, fx2_pktend, fx2_fifoadr,
   // Inouts
   fx2_fd,
   // Inputs
   clk, rst, fx2_ifclk, fx2_flaga, fx2_flagb, fx2_flagc, fx2_flagd
   );

`ifdef OPTIMSOC_MAINMEM_DDR
   parameter MEMORY_SIZE = 128*1024*1024; // 128 MByte
   localparam PGAS_ADDRW = 32 - clog2(MEMORY_SIZE); // This is the part that is overwritten
`else // BRAM
   parameter MEMORY_SIZE = 256*1024; // 256 kByte
   parameter PGAS_ADDRW = 32 - clog2(MEMORY_SIZE); // This is the part that is overwritten
`endif

   // NoC configuration
   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH + NOC_FLIT_TYPE_WIDTH;
   parameter NOC_VCHANNELS = `VCHANNELS;

   // Debug NoC configuration
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_VCHANNELS = 1;

   parameter DEBUG_ROUTER_COUNT = 1;
   parameter DEBUG_ROUTER_LINKS_PER_ROUTER = 1;

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

   // MCB connection
`ifdef OPTIMSOC_MAINMEM_DDR
   inout [15:0]  mcb3_dram_dq;
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
`endif

   // clocks
   wire clk_ddr2;

   // system control signals
   wire sys_clk_disable;
   wire sys_clk_is_halted;
   assign sys_clk_is_halted = sys_clk_disable;

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

   // connect external interface with debug system
   wire [NOC_FLIT_WIDTH-1:0] ext_link_in_flit;
   wire [NOC_VCHANNELS-1:0] ext_link_in_valid;
   wire [NOC_VCHANNELS-1:0] ext_link_in_ready;
   wire [NOC_FLIT_WIDTH-1:0] ext_link_out_flit;
   wire [NOC_VCHANNELS-1:0] ext_link_out_valid;
   wire [NOC_VCHANNELS-1:0] ext_link_out_ready;

   // connection to the USB interface
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   wire [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;

   // DDR2 memory
   wire ddr2_calib_done;
`ifndef OPTIMSOC_MAINMEM_DDR
   assign ddr2_calib_done = 1'b1;
`endif

   wire clk_ct;
   wire clk_noc;
   wire clk_dbg;
   wire rst_sys;
   wire rst_cpu;

`ifdef OPTIMSOC_CDC_DYNAMIC
   wire [2:0] cdc_conf;
   wire       cdc_enable;
`endif

   clockmanager_ztex115
`ifdef OPTIMSOC_MAINMEM_DDR
     #(.ENABLE_DDR_CLOCK(1))
`endif
      u_clockmanager(
`ifdef OPTIMSOC_CDC_DYNAMIC
                     .cdc_conf (cdc_conf),
                     .cdc_enable (cdc_enable),
`endif
`ifdef OPTIMSOC_MAINMEM_DDR
                     .clk_ddr (clk_ddr2),
`else
                     .clk_ddr (),
`endif
                     .clk     (clk),
                     .rst     (rst_buf),
                     .clk_ct  (clk_ct),
                     .clk_dbg (clk_dbg),
                     .clk_noc (clk_noc),
                     .rst_sys  (rst_sys),
                     .rst_cpu  (rst_cpu),
                     .sys_halt (sys_clk_disable),
                     .cpu_reset (cpu_reset),
                     .cpu_start (start_cpu));


   wire [31:0] wb_mt_dat_o;
   wire        wb_mt_ack_o;
   wire        wb_mt_rty_o;
   wire        wb_mt_err_o;
   wire [31:0] wb_mt_adr_i;
   wire [1:0]  wb_mt_bte_i;
   wire [2:0]  wb_mt_cti_i;
   wire        wb_mt_cyc_i;
   wire [31:0] wb_mt_dat_i;
   wire [3:0]  wb_mt_sel_i;
   wire        wb_mt_stb_i;
   wire        wb_mt_we_i;

   wire [`DEBUG_ITM_PORTWIDTH-1:0] trace_itm;
   wire [`DEBUG_STM_PORTWIDTH-1:0] trace_stm;

   /* system_simpledemo_pgas AUTO_TEMPLATE(
    .ht1_\(.*\) (ext_link_\1),
    .clk (clk_sys_gated),
    .clk_uart (clk_sys),
    .rst_sys (rst_sys),
    .rst_cpu (rst_cpu),
    .wb_mt2_\(.*\)_i (wb_mt_\1_o),
    .wb_mt2_\(.*\)_o (wb_mt_\1_i),
    ); */
   system_simpledemo_pgas
      #(.PGAS_ADDRW (PGAS_ADDRW))
      u_system(
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
               .trace_itm                  (trace_itm[`DEBUG_ITM_PORTWIDTH-1:0]),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
               .trace_stm                  (trace_stm[`DEBUG_STM_PORTWIDTH-1:0]),
`endif
`ifdef OPTIMSOC_CDC_DYNAMIC
               .cdc_conf                (cdc_conf[2:0]),
               .cdc_enable              (cdc_enable),
`endif
               /*AUTOINST*/
               // Outputs
               .ht1_in_ready            (ext_link_in_ready),     // Templated
               .ht1_out_flit            (ext_link_out_flit),     // Templated
               .ht1_out_valid           (ext_link_out_valid),    // Templated
               .wb_mt2_adr_o            (wb_mt_adr_i),           // Templated
               .wb_mt2_cyc_o            (wb_mt_cyc_i),           // Templated
               .wb_mt2_dat_o            (wb_mt_dat_i),           // Templated
               .wb_mt2_sel_o            (wb_mt_sel_i),           // Templated
               .wb_mt2_stb_o            (wb_mt_stb_i),           // Templated
               .wb_mt2_we_o             (wb_mt_we_i),            // Templated
               .wb_mt2_cti_o            (wb_mt_cti_i),           // Templated
               .wb_mt2_bte_o            (wb_mt_bte_i),           // Templated
               // Inputs
               .clk_ct                  (clk_ct),
               .clk_noc                 (clk_noc),
               .rst_cpu                 (rst_cpu),               // Templated
               .rst_sys                 (rst_sys),               // Templated
               .ht1_in_flit             (ext_link_in_flit),      // Templated
               .ht1_in_valid            (ext_link_in_valid),     // Templated
               .ht1_out_ready           (ext_link_out_ready),    // Templated
               .wb_mt2_ack_i            (wb_mt_ack_o),           // Templated
               .wb_mt2_rty_i            (wb_mt_rty_o),           // Templated
               .wb_mt2_err_i            (wb_mt_err_o),           // Templated
               .wb_mt2_dat_i            (wb_mt_dat_o));          // Templated

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
       .DEBUG_ITM_CORE_COUNT            (1),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
       .DEBUG_STM_CORE_COUNT            (1),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_NRM
       .DEBUG_ROUTER_COUNT              (1),
       .DEBUG_ROUTER_LINKS_PER_ROUTER   (1),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_NCM
       .OPTIMSOC_DEBUG_NCM_ID (1),
`endif
       .NOC_VCHANNELS                   (`VCHANNELS),
       .NOC_USED_VCHANNEL               (`VCHANNEL_LSU_REQ),
       .SYSTEM_IDENTIFIER (16'hce75))
      u_dbg_system(.clk(clk_dbg),
                   .rst(rst_sys),

                   // System control
                   .sys_clk_disable     (sys_clk_disable),
                   .cpu_reset           (cpu_reset),
                   .cpu_stall           (cpu_stall),
                   .start_cpu           (start_cpu),
                   .sys_clk_is_halted   (sys_clk_is_halted),

`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
                   .itm_ports_flat     (trace_itm),
 `ifdef OPTIMSOC_CLOCKDOMAINS
                   .clk_itm            (clk_ct),
 `endif
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
                   .stm_ports_flat     (trace_stm),
 `ifdef OPTIMSOC_CLOCKDOMAINS
                   .clk_stm            (clk_ct),
 `endif
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_NCM
                   .noc32_out_ready (ext_link_in_ready),
                   .noc32_out_valid (ext_link_in_valid),
                   .noc32_out_flit (ext_link_in_flit),
                   .noc32_in_ready (ext_link_out_ready),
                   .noc32_in_valid (ext_link_out_valid),
                   .noc32_in_flit (ext_link_out_flit),
 `ifdef OPTIMSOC_CLOCKDOMAINS
                   .clk_ncm            (clk_noc),
 `endif
`endif
                   // Debug NoC link to USB
                   .dbgnoc_in_flit     (dbgnoc_in_flit),
                   .dbgnoc_in_valid    (dbgnoc_in_valid),
                   .dbgnoc_in_ready    (dbgnoc_in_ready),
                   .dbgnoc_out_flit    (dbgnoc_out_flit),
                   .dbgnoc_out_valid   (dbgnoc_out_valid),
                   .dbgnoc_out_ready   (dbgnoc_out_ready)
                   );


`ifdef OPTIMSOC_MAINMEM_DDR
    /* ztex_ddr2_if AUTO_TEMPLATE(
    .ddr2_clk(clk_ddr2),
    .ddr2_rst(rst_sys),
    .wbm._clk_i (clk_noc),
    .wbm._rst_i (rst_sys),
    .wbm0_\(.*\) (wb_mt_\1[]),
    .wbm1_\(.*\)_i ('b0),
    .wbm1_\(.*\)_o (),
    .wbm2_\(.*\)_i ('b0),
    .wbm2_\(.*\)_o (),
    .wbm3_\(.*\)_i ('b0),
    .wbm3_\(.*\)_o (),
    ); */
   ztex_ddr2_if
     u_gram(/*AUTOINST*/
            // Outputs
            .ddr2_calib_done            (ddr2_calib_done),
            .mcb3_dram_a                (mcb3_dram_a[12:0]),
            .mcb3_dram_ba               (mcb3_dram_ba[2:0]),
            .mcb3_dram_ras_n            (mcb3_dram_ras_n),
            .mcb3_dram_cas_n            (mcb3_dram_cas_n),
            .mcb3_dram_we_n             (mcb3_dram_we_n),
            .mcb3_dram_cke              (mcb3_dram_cke),
            .mcb3_dram_dm               (mcb3_dram_dm),
            .mcb3_dram_udm              (mcb3_dram_udm),
            .mcb3_dram_ck               (mcb3_dram_ck),
            .mcb3_dram_ck_n             (mcb3_dram_ck_n),
            .wbm0_ack_o                 (wb_mt_ack_o),           // Templated
            .wbm0_err_o                 (wb_mt_err_o),           // Templated
            .wbm0_rty_o                 (wb_mt_rty_o),           // Templated
            .wbm0_dat_o                 (wb_mt_dat_o[31:0]),     // Templated
            .wbm1_ack_o                 (),                      // Templated
            .wbm1_err_o                 (),                      // Templated
            .wbm1_rty_o                 (),                      // Templated
            .wbm1_dat_o                 (),                      // Templated
            .wbm2_ack_o                 (),                      // Templated
            .wbm2_err_o                 (),                      // Templated
            .wbm2_rty_o                 (),                      // Templated
            .wbm2_dat_o                 (),                      // Templated
            .wbm3_ack_o                 (),                      // Templated
            .wbm3_err_o                 (),                      // Templated
            .wbm3_rty_o                 (),                      // Templated
            .wbm3_dat_o                 (),                      // Templated
            // Inouts
            .mcb3_dram_dq               (mcb3_dram_dq[15:0]),
            .mcb3_dram_udqs             (mcb3_dram_udqs),
            .mcb3_dram_udqs_n           (mcb3_dram_udqs_n),
            .mcb3_rzq                   (mcb3_rzq),
            .mcb3_zio                   (mcb3_zio),
            .mcb3_dram_dqs              (mcb3_dram_dqs),
            .mcb3_dram_dqs_n            (mcb3_dram_dqs_n),
            // Inputs
            .ddr2_clk                   (clk_ddr2),              // Templated
            .ddr2_rst                   (rst_sys),               // Templated
            .wbm0_clk_i                 (clk_noc),               // Templated
            .wbm0_rst_i                 (rst_sys),               // Templated
            .wbm0_dat_i                 (wb_mt_dat_i[31:0]),     // Templated
            .wbm0_adr_i                 (wb_mt_adr_i[31:0]),     // Templated
            .wbm0_bte_i                 (wb_mt_bte_i[1:0]),      // Templated
            .wbm0_cti_i                 (wb_mt_cti_i[2:0]),      // Templated
            .wbm0_cyc_i                 (wb_mt_cyc_i),           // Templated
            .wbm0_sel_i                 (wb_mt_sel_i[3:0]),      // Templated
            .wbm0_stb_i                 (wb_mt_stb_i),           // Templated
            .wbm0_we_i                  (wb_mt_we_i),            // Templated
            .wbm1_clk_i                 (clk_noc),               // Templated
            .wbm1_rst_i                 (rst_sys),               // Templated
            .wbm1_dat_i                 ('b0),                   // Templated
            .wbm1_adr_i                 ('b0),                   // Templated
            .wbm1_bte_i                 ('b0),                   // Templated
            .wbm1_cti_i                 ('b0),                   // Templated
            .wbm1_cyc_i                 ('b0),                   // Templated
            .wbm1_sel_i                 ('b0),                   // Templated
            .wbm1_stb_i                 ('b0),                   // Templated
            .wbm1_we_i                  ('b0),                   // Templated
            .wbm2_clk_i                 (clk_noc),               // Templated
            .wbm2_rst_i                 (rst_sys),               // Templated
            .wbm2_dat_i                 ('b0),                   // Templated
            .wbm2_adr_i                 ('b0),                   // Templated
            .wbm2_bte_i                 ('b0),                   // Templated
            .wbm2_cti_i                 ('b0),                   // Templated
            .wbm2_cyc_i                 ('b0),                   // Templated
            .wbm2_sel_i                 ('b0),                   // Templated
            .wbm2_stb_i                 ('b0),                   // Templated
            .wbm2_we_i                  ('b0),                   // Templated
            .wbm3_dat_i                 ('b0),                   // Templated
            .wbm3_adr_i                 ('b0),                   // Templated
            .wbm3_bte_i                 ('b0),                   // Templated
            .wbm3_cti_i                 ('b0),                   // Templated
            .wbm3_cyc_i                 ('b0),                   // Templated
            .wbm3_sel_i                 ('b0),                   // Templated
            .wbm3_stb_i                 ('b0),                   // Templated
            .wbm3_we_i                  ('b0),                   // Templated
            .wbm3_clk_i                 (clk_noc),               // Templated
            .wbm3_rst_i                 (rst_sys));              // Templated
   
`else
`ifdef OPTIMSOC_MAINMEM_BRAM
   /* wb_sram_sp AUTO_TEMPLATE(
    .wb_clk_i (clk_noc),
    .wb_rst_i (rst_sys),
    .wb_adr_i (wb_mt_adr_i[(32-PGAS_ADDRW)-1:0]),
    .wb_\(.*\) (wb_mt_\1),
    ); */
   wb_sram_sp
      #(.MEM_SIZE(MEMORY_SIZE))
   u_gram(/*AUTOINST*/
          // Outputs
          .wb_ack_o                     (wb_mt_ack_o),           // Templated
          .wb_err_o                     (wb_mt_err_o),           // Templated
          .wb_rty_o                     (wb_mt_rty_o),           // Templated
          .wb_dat_o                     (wb_mt_dat_o),           // Templated
          // Inputs
          .wb_adr_i                     (wb_mt_adr_i[(32-PGAS_ADDRW)-1:0]), // Templated
          .wb_bte_i                     (wb_mt_bte_i),           // Templated
          .wb_cti_i                     (wb_mt_cti_i),           // Templated
          .wb_cyc_i                     (wb_mt_cyc_i),           // Templated
          .wb_dat_i                     (wb_mt_dat_i),           // Templated
          .wb_sel_i                     (wb_mt_sel_i),           // Templated
          .wb_stb_i                     (wb_mt_stb_i),           // Templated
          .wb_we_i                      (wb_mt_we_i),            // Templated
          .wb_clk_i                     (clk_noc),               // Templated
          .wb_rst_i                     (rst_sys));              // Templated
`else // !`ifdef OPTIMSOC_MAINMEN_BRAM
   $fatal("You need to select the main memory implementation technology");
`endif
`endif

   `include "optimsoc_functions.vh"

endmodule

// Local Variables:
// verilog-library-directories:("./ztex_ddr2_if/verilog/" "../../../src/rtl/*/verilog/" "../../../" "." )
// verilog-auto-inst-param-value: t
// End:
