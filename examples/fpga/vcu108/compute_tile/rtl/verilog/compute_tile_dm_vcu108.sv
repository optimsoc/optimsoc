/* Copyright (c) 2017 by the author(s)
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
 * Toplevel: compute_tile_dm on a Xilinx VCU 108 board
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module compute_tile_dm_vcu108
  (

   // 300 MHz system clock
   input         sysclk1_300_p,
   input         sysclk1_300_n,

   // CPU reset button
   input         cpu_reset,

   // all following UART signals are from a DTE (the PC) point-of-view
   // USB UART (onboard)
   output        usb_uart_rx,
   input         usb_uart_tx,
   output        usb_uart_cts, // active low (despite the name)
   input         usb_uart_rts, // active low (despite the name)

   // UART over PMOD (bottom row of J52)
   output        pmod_uart_rx,
   input         pmod_uart_tx,
   output        pmod_uart_cts, // active low (despite the name)
   input         pmod_uart_rts, // active low (despite the name)

   // DDR
   output        c0_ddr4_act_n,
   output [16:0] c0_ddr4_adr,
   output [1:0]  c0_ddr4_ba,
   output [0:0]  c0_ddr4_bg,
   output [0:0]  c0_ddr4_cke,
   output [0:0]  c0_ddr4_odt,
   output [0:0]  c0_ddr4_cs_n,
   output [0:0]  c0_ddr4_ck_t,
   output [0:0]  c0_ddr4_ck_c,
   output        c0_ddr4_reset_n,
   inout [7:0]   c0_ddr4_dm_dbi_n,
   inout [63:0]  c0_ddr4_dq,
   inout [7:0]   c0_ddr4_dqs_t,
   inout [7:0]   c0_ddr4_dqs_c,

   // Cypress FX3 connected to FMC HPC1 (right, next to the Ethernet port)
   output        fx3_pclk,
   inout [15:0]  fx3_dq,
   output        fx3_slcs_n,
   output        fx3_sloe_n,
   output        fx3_slrd_n,
   output        fx3_slwr_n,
   output        fx3_pktend_n,
   output [1:0]  fx3_a,
   input         fx3_flaga_n,
   input         fx3_flagb_n,
   input         fx3_flagc_n,
   input         fx3_flagd_n,
   input         fx3_com_rst,
   input         fx3_logic_rst,
   output [2:0]  fx3_pmode,

   // signals for fan control
   input         sm_fan_tach,
   output        sm_fan_pwm
   );

   import dii_package::dii_flit;
   import optimsoc_config::*;

   parameter integer NUM_CORES = 1;
   localparam integer LMEM_SIZE = 128*1024*1024;

   // off-chip host interface
   // uart: use a UART connection (see UART0_SOURCE for connectivity options)
   // usb3: use a USB 3 connection (through a Cypress FX3 chip)
   parameter HOST_IF = "uart";

   // source of the UART connection
   // onboard: Use the UART chip on the VCU108 board
   // pmod: Connect a pmodusbuart module to J52 (bottom row)
   parameter UART0_SOURCE = "pmod";

   // onboard: 921600, max. for CP2105
   // pmod: 3 MBaud, max. for FT232R
   parameter UART0_BAUD = (UART0_SOURCE == "pmod" ? 3000000 : 921600);

   localparam AXI_ID_WIDTH = 4;
   localparam DDR_ADDR_WIDTH = 30;
   localparam DDR_DATA_WIDTH = 32;

   localparam base_config_t
     BASE_CONFIG = '{ NUMTILES: 1,
                      NUMCTS: 1,
                      CTLIST: 1024'h1,
                      CORES_PER_TILE: NUM_CORES,
                      GMEM_SIZE: 0,
                      GMEM_TILE: 'x,
                      NOC_ENABLE_VCHANNELS: 0,
                      LMEM_SIZE: LMEM_SIZE,
                      LMEM_STYLE: EXTERNAL,
                      ENABLE_BOOTROM: 0,
                      BOOTROM_SIZE: 0,
                      ENABLE_DM: 1,
                      DM_BASE: 32'h0,
                      DM_SIZE: LMEM_SIZE,
                      ENABLE_PGAS: 0,
                      PGAS_BASE: 0,
                      PGAS_SIZE: 0,
                      NA_ENABLE_MPSIMPLE: 1,
                      NA_ENABLE_DMA: 1,
                      NA_DMA_GENIRQ: 1,
                      NA_DMA_ENTRIES: 4,
                      USE_DEBUG: 1,
                      DEBUG_STM: 1,
                      DEBUG_CTM: 1,
                      DEBUG_SUBNET_BITS: 6,
                      DEBUG_LOCAL_SUBNET: 0,
                      DEBUG_ROUTER_BUFFER_SIZE: 4,
                      DEBUG_MAX_PKT_LEN: 8
                      };

   localparam config_t CONFIG = derive_config(BASE_CONFIG);

   nasti_channel
     #(.ID_WIDTH   (AXI_ID_WIDTH),
       .ADDR_WIDTH (DDR_ADDR_WIDTH),
       .DATA_WIDTH (DDR_DATA_WIDTH))
   c_axi_ddr();

   wb_channel
     #(.ADDR_WIDTH (DDR_ADDR_WIDTH),
       .DATA_WIDTH (DDR_DATA_WIDTH))
   c_wb_ddr();

   // clocks and reset
   // sysclk1_300_p/n is the 300 MHz board clock
   // cpu_reset is a push button on the board labeled "CPU RESET"

   // system clock: 50 MHz
   logic              sys_clk_50;
   // system clock: 100 MHz
   logic              sys_clk_100;

   // system reset
   logic              sys_rst;

   // UART signals (naming from our point of view, i.e. from the DCE)
   logic              uart_rx, uart_tx, uart_cts_n, uart_rts_n;

   // terminate NoC connection
   logic [CONFIG.NOC_CHANNELS-1:0][CONFIG.NOC_FLIT_WIDTH-1:0] noc_in_flit;
   logic [CONFIG.NOC_CHANNELS-1:0]                            noc_in_last;
   logic [CONFIG.NOC_CHANNELS-1:0]                            noc_in_valid;
   logic [CONFIG.NOC_CHANNELS-1:0]                            noc_in_ready;
   logic [CONFIG.NOC_CHANNELS-1:0][CONFIG.NOC_FLIT_WIDTH-1:0] noc_out_flit;
   logic [CONFIG.NOC_CHANNELS-1:0]                            noc_out_last;
   logic [CONFIG.NOC_CHANNELS-1:0]                            noc_out_valid;
   logic [CONFIG.NOC_CHANNELS-1:0]                            noc_out_ready;

   assign noc_in_flit   = {CONFIG.NOC_FLIT_WIDTH*CONFIG.NOC_CHANNELS{1'bx}};
   assign noc_in_last   = {CONFIG.NOC_CHANNELS{1'bx}};
   assign noc_in_valid  = {CONFIG.NOC_CHANNELS{1'b0}};
   assign noc_out_ready = {CONFIG.NOC_CHANNELS{1'b0}};

   // Debug system
   glip_channel c_glip_in(.clk(sys_clk_50));
   glip_channel c_glip_out(.clk(sys_clk_50));

   // XXX: does the HIM support hot-attach by now?
   // See discussion in system_2x2_cccc_ztex
   logic                                                      glip_com_rst, glip_ctrl_logic_rst;

   logic                                                      logic_rst;
   assign logic_rst = sys_rst | glip_ctrl_logic_rst;

   // Host (off-chip) interface through GLIP (mostly for debug)
   generate
      if (HOST_IF == "uart") begin
         glip_uart_toplevel
           #(.FREQ_CLK_IO(32'd50_000_000),
             .BAUD(UART0_BAUD),
             .WIDTH(16),
             .BUFFER_OUT_DEPTH(256*1024))
         u_glip(
                .clk_io(sys_clk_50),
                .clk(sys_clk_50),
                .rst(sys_rst),
                .com_rst(glip_com_rst),
                .ctrl_logic_rst(glip_ctrl_logic_rst),

                .error(),

                .fifo_out_data(c_glip_out.data),
                .fifo_out_ready(c_glip_out.ready),
                .fifo_out_valid(c_glip_out.valid),
                .fifo_in_data(c_glip_in.data),
                .fifo_in_ready(c_glip_in.ready),
                .fifo_in_valid(c_glip_in.valid),

                .uart_rx(uart_rx),
                .uart_tx(uart_tx),
                .uart_rts_n(uart_rts_n),
                .uart_cts_n(uart_cts_n)
                );
      end else if (HOST_IF == "usb3") begin
         glip_cypressfx3_toplevel
           #(.WIDTH(16))
         u_glip(
                .clk(sys_clk_50),
                .clk_io_100(sys_clk_100),
                .rst(sys_rst),
                .com_rst(glip_com_rst),
                .ctrl_logic_rst(glip_ctrl_logic_rst),

                .fifo_out_data(c_glip_out.data),
                .fifo_out_ready(c_glip_out.ready),
                .fifo_out_valid(c_glip_out.valid),
                .fifo_in_data(c_glip_in.data),
                .fifo_in_ready(c_glip_in.ready),
                .fifo_in_valid(c_glip_in.valid),

                .fx3_pclk      (fx3_pclk),
                .fx3_dq        (fx3_dq),
                .fx3_slcs_n    (fx3_slcs_n),
                .fx3_sloe_n    (fx3_sloe_n),
                .fx3_slrd_n    (fx3_slrd_n),
                .fx3_slwr_n    (fx3_slwr_n),
                .fx3_pktend_n  (fx3_pktend_n),
                .fx3_a         (fx3_a[1:0]),
                .fx3_flaga_n   (fx3_flaga_n),
                .fx3_flagb_n   (fx3_flagb_n),
                .fx3_flagc_n   (fx3_flagc_n),
                .fx3_flagd_n   (fx3_flagd_n),
                .fx3_com_rst   (fx3_com_rst),
                .fx3_logic_rst (fx3_logic_rst),
                .fx3_pmode     (fx3_pmode)
                );
      end
   endgenerate

   logic dbg_sys_rst, dbg_cpu_rst;

   dii_flit [1:0] debug_ring_in;
   dii_flit [1:0] debug_ring_out;
   logic [1:0] debug_ring_in_ready;
   logic [1:0] debug_ring_out_ready;

   debug_interface
     #(
       .SYSTEM_VENDOR_ID         (2),
       .SYSTEM_DEVICE_ID         (2),
       .NUM_MODULES              (CONFIG.DEBUG_NUM_MODS),
       .SUBNET_BITS              (CONFIG.DEBUG_SUBNET_BITS),
       .LOCAL_SUBNET             (CONFIG.DEBUG_LOCAL_SUBNET),
       .DEBUG_ROUTER_BUFFER_SIZE (CONFIG.DEBUG_ROUTER_BUFFER_SIZE)
       )
   u_debuginterface
     (
      .clk            (sys_clk_50),
      .rst            (logic_rst),
      .sys_rst        (dbg_sys_rst),
      .cpu_rst        (dbg_cpu_rst),
      .glip_in        (c_glip_in),
      .glip_out       (c_glip_out),
      .ring_out       (debug_ring_in),
      .ring_out_ready (debug_ring_in_ready),
      .ring_in        (debug_ring_out),
      .ring_in_ready  (debug_ring_out_ready)
      );

   // Single compute tile with all memory mapped to the DRAM
   compute_tile_dm
     #(.CONFIG(CONFIG),
       .DEBUG_BASEID((CONFIG.DEBUG_LOCAL_SUBNET << (16 - CONFIG.DEBUG_SUBNET_BITS)) + 1)
       )
   u_compute_tile
     (
      .clk           (sys_clk_50),
      .rst_cpu       (dbg_cpu_rst | logic_rst),
      .rst_sys       (dbg_sys_rst | logic_rst),
      .rst_dbg       (logic_rst),

      .noc_in_flit   (noc_in_flit),
      .noc_in_last   (noc_in_last),
      .noc_in_valid  (noc_in_valid),
      .noc_in_ready  (noc_in_ready),
      .noc_out_flit  (noc_out_flit),
      .noc_out_last  (noc_out_last),
      .noc_out_valid (noc_out_valid),
      .noc_out_ready (noc_out_ready),

      .debug_ring_in(debug_ring_in),
      .debug_ring_in_ready(debug_ring_in_ready),
      .debug_ring_out(debug_ring_out),
      .debug_ring_out_ready(debug_ring_out_ready),

      .wb_ext_adr_i  (c_wb_ddr.adr_o),
      .wb_ext_cyc_i  (c_wb_ddr.cyc_o),
      .wb_ext_dat_i  (c_wb_ddr.dat_o),
      .wb_ext_sel_i  (c_wb_ddr.sel_o),
      .wb_ext_stb_i  (c_wb_ddr.stb_o),
      .wb_ext_we_i   (c_wb_ddr.we_o),
      .wb_ext_cab_i  (), // XXX: this is an old signal not present in WB B3 any more!?
      .wb_ext_cti_i  (c_wb_ddr.cti_o),
      .wb_ext_bte_i  (c_wb_ddr.bte_o),
      .wb_ext_ack_o  (c_wb_ddr.ack_i),
      .wb_ext_rty_o  (c_wb_ddr.rty_i),
      .wb_ext_err_o  (c_wb_ddr.err_i),
      .wb_ext_dat_o  (c_wb_ddr.dat_i)
      );

   // VCU108 board wrapper
   vcu108
     #(
       .NUM_UART(1),
       .UART0_SOURCE(UART0_SOURCE)
       )
   u_board(
           // FPGA/board interface
           .sysclk1_300_p(sysclk1_300_p),
           .sysclk1_300_n(sysclk1_300_n),
           .cpu_reset(cpu_reset),

           .usb_uart_rx(usb_uart_rx),
           .usb_uart_tx(usb_uart_tx),
           .usb_uart_cts(usb_uart_cts),
           .usb_uart_rts(usb_uart_rts),

           .pmod_uart_rx(pmod_uart_rx),
           .pmod_uart_tx(pmod_uart_tx),
           .pmod_uart_cts(pmod_uart_cts),
           .pmod_uart_rts(pmod_uart_rts),

           .c0_ddr4_act_n          (c0_ddr4_act_n),
           .c0_ddr4_adr            (c0_ddr4_adr),
           .c0_ddr4_ba             (c0_ddr4_ba),
           .c0_ddr4_bg             (c0_ddr4_bg),
           .c0_ddr4_cke            (c0_ddr4_cke),
           .c0_ddr4_odt            (c0_ddr4_odt),
           .c0_ddr4_cs_n           (c0_ddr4_cs_n),
           .c0_ddr4_ck_t           (c0_ddr4_ck_t),
           .c0_ddr4_ck_c           (c0_ddr4_ck_c),
           .c0_ddr4_reset_n        (c0_ddr4_reset_n),

           .c0_ddr4_dm_dbi_n       (c0_ddr4_dm_dbi_n),
           .c0_ddr4_dq             (c0_ddr4_dq),
           .c0_ddr4_dqs_c          (c0_ddr4_dqs_c),
           .c0_ddr4_dqs_t          (c0_ddr4_dqs_t),

           // system interface
           .sys_clk_50  (sys_clk_50),
           .sys_clk_100 (sys_clk_100),
           .sys_rst     (sys_rst),

           .uart_rx     (uart_rx),
           .uart_tx     (uart_tx),
           .uart_rts_n  (uart_rts_n),
           .uart_cts_n  (uart_cts_n),

           .ddr_awid    (c_axi_ddr.aw_id),
           .ddr_awaddr  (c_axi_ddr.aw_addr),
           .ddr_awlen   (c_axi_ddr.aw_len),
           .ddr_awsize  (c_axi_ddr.aw_size),
           .ddr_awburst (c_axi_ddr.aw_burst),
           .ddr_awcache (c_axi_ddr.aw_cache),
           .ddr_awprot  (c_axi_ddr.aw_prot),
           .ddr_awqos   (c_axi_ddr.aw_qos),
           .ddr_awvalid (c_axi_ddr.aw_valid),
           .ddr_awready (c_axi_ddr.aw_ready),
           .ddr_wdata   (c_axi_ddr.w_data),
           .ddr_wstrb   (c_axi_ddr.w_strb),
           .ddr_wlast   (c_axi_ddr.w_last),
           .ddr_wvalid  (c_axi_ddr.w_valid),
           .ddr_wready  (c_axi_ddr.w_ready),
           .ddr_bid     (c_axi_ddr.b_id),
           .ddr_bresp   (c_axi_ddr.b_resp),
           .ddr_bvalid  (c_axi_ddr.b_valid),
           .ddr_bready  (c_axi_ddr.b_ready),
           .ddr_arid    (c_axi_ddr.ar_id),
           .ddr_araddr  (c_axi_ddr.ar_addr),
           .ddr_arlen   (c_axi_ddr.ar_len),
           .ddr_arsize  (c_axi_ddr.ar_size),
           .ddr_arburst (c_axi_ddr.ar_burst),
           .ddr_arcache (c_axi_ddr.ar_cache),
           .ddr_arprot  (c_axi_ddr.ar_prot),
           .ddr_arqos   (c_axi_ddr.ar_qos),
           .ddr_arvalid (c_axi_ddr.ar_valid),
           .ddr_arready (c_axi_ddr.ar_ready),
           .ddr_rid     (c_axi_ddr.r_id),
           .ddr_rresp   (c_axi_ddr.r_resp),
           .ddr_rdata   (c_axi_ddr.r_data),
           .ddr_rlast   (c_axi_ddr.r_last),
           .ddr_rvalid  (c_axi_ddr.r_valid),
           .ddr_rready  (c_axi_ddr.r_ready),

           // signals for fan control
           .sm_fan_tach (sm_fan_tach),
           .sm_fan_pwm  (sm_fan_pwm)
           );

   // Memory interface: convert WishBone signals from system to AXI for DRAM
   wb2axi
     #(.ADDR_WIDTH (DDR_ADDR_WIDTH),
       .DATA_WIDTH (DDR_DATA_WIDTH),
       .AXI_ID_WIDTH (AXI_ID_WIDTH))
   u_wb2axi_ddr
     (.clk             (sys_clk_50),
      .rst             (logic_rst),
      .wb_cyc_i        (c_wb_ddr.cyc_o),
      .wb_stb_i        (c_wb_ddr.stb_o),
      .wb_we_i         (c_wb_ddr.we_o),
      .wb_adr_i        (c_wb_ddr.adr_o),
      .wb_dat_i        (c_wb_ddr.dat_o),
      .wb_sel_i        (c_wb_ddr.sel_o),
      .wb_cti_i        (c_wb_ddr.cti_o),
      .wb_bte_i        (c_wb_ddr.bte_o),
      .wb_ack_o        (c_wb_ddr.ack_i),
      .wb_err_o        (c_wb_ddr.err_i),
      .wb_rty_o        (c_wb_ddr.rty_i),
      .wb_dat_o        (c_wb_ddr.dat_i),
      .m_axi_awid      (c_axi_ddr.aw_id),
      .m_axi_awaddr    (c_axi_ddr.aw_addr),
      .m_axi_awlen     (c_axi_ddr.aw_len),
      .m_axi_awsize    (c_axi_ddr.aw_size),
      .m_axi_awburst   (c_axi_ddr.aw_burst),
      .m_axi_awcache   (c_axi_ddr.aw_cache),
      .m_axi_awprot    (c_axi_ddr.aw_prot),
      .m_axi_awqos     (c_axi_ddr.aw_qos),
      .m_axi_awvalid   (c_axi_ddr.aw_valid),
      .m_axi_awready   (c_axi_ddr.aw_ready),
      .m_axi_wdata     (c_axi_ddr.w_data),
      .m_axi_wstrb     (c_axi_ddr.w_strb),
      .m_axi_wlast     (c_axi_ddr.w_last),
      .m_axi_wvalid    (c_axi_ddr.w_valid),
      .m_axi_wready    (c_axi_ddr.w_ready),
      .m_axi_bid       (c_axi_ddr.b_id),
      .m_axi_bresp     (c_axi_ddr.b_resp),
      .m_axi_bvalid    (c_axi_ddr.b_valid),
      .m_axi_bready    (c_axi_ddr.b_ready),
      .m_axi_arid      (c_axi_ddr.ar_id),
      .m_axi_araddr    (c_axi_ddr.ar_addr),
      .m_axi_arlen     (c_axi_ddr.ar_len),
      .m_axi_arsize    (c_axi_ddr.ar_size),
      .m_axi_arburst   (c_axi_ddr.ar_burst),
      .m_axi_arcache   (c_axi_ddr.ar_cache),
      .m_axi_arprot    (c_axi_ddr.ar_prot),
      .m_axi_arqos     (c_axi_ddr.ar_qos),
      .m_axi_arvalid   (c_axi_ddr.ar_valid),
      .m_axi_arready   (c_axi_ddr.ar_ready),
      .m_axi_rid       (c_axi_ddr.r_id),
      .m_axi_rdata     (c_axi_ddr.r_data),
      .m_axi_rresp     (c_axi_ddr.r_resp),
      .m_axi_rlast     (c_axi_ddr.r_last),
      .m_axi_rvalid    (c_axi_ddr.r_valid),
      .m_axi_rready    (c_axi_ddr.r_ready)
      );

endmodule
