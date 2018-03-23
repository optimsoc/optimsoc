/* Copyright (c) 2017-2018 by the author(s)
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
 * Toplevel: system with compute tiles only on a Xilinx VCU 108 board
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module system_allct_vcu108
   #(parameter ENABLE_VCHANNELS = 1*1,
     parameter integer NUM_CORES = 1,
     parameter XDIM = 2,
     parameter YDIM = 2,
     parameter LMEM_EXTERNAL = 0,
     parameter integer LMEM_SIZE_BYTES = 128*1024*1024,

     // off-chip host interface
     // uart: use a UART connection (see UART0_SOURCE for connectivity options)
     // usb3: use a USB 3 connection (through a Cypress FX3 chip)
     parameter HOST_IF = "usb3",

     // source of the UART connection
     // onboard: Use the UART chip on the VCU108 board
     // pmod: Connect a pmodusbuart module to J52 (bottom row)
     parameter UART0_SOURCE = "pmod",

     // onboard: 921600, max. for CP2105
     // pmod: 3 MBaud, max. for FT232R
     parameter UART0_BAUD = (UART0_SOURCE == "pmod" ? 3000000 : 921600)
     )
  (
   // 300 MHz system clock
   input                 sysclk1_300_p,
   input                 sysclk1_300_n,

   // CPU reset button
   input                 cpu_reset,

   // all following UART signals are from a DTE (the PC) point-of-view
   // USB UART (onboard)
   output                usb_uart_rx,
   input                 usb_uart_tx,
   output                usb_uart_cts, // active low (despite the name)
   input                 usb_uart_rts, // active low (despite the name)

   // UART over PMOD (bottom row of J52)
   output                pmod_uart_rx,
   input                 pmod_uart_tx,
   output                pmod_uart_cts, // active low (despite the name)
   input                 pmod_uart_rts, // active low (despite the name)

   // DDR
   output                c0_ddr4_act_n,
   output [16:0]         c0_ddr4_adr,
   output [1:0]          c0_ddr4_ba,
   output [0:0]          c0_ddr4_bg,
   output [0:0]          c0_ddr4_cke,
   output [0:0]          c0_ddr4_odt,
   output [0:0]          c0_ddr4_cs_n,
   output [0:0]          c0_ddr4_ck_t,
   output [0:0]          c0_ddr4_ck_c,
   output                c0_ddr4_reset_n,
   inout  [7:0]          c0_ddr4_dm_dbi_n,
   inout  [63:0]         c0_ddr4_dq,
   inout  [7:0]          c0_ddr4_dqs_t,
   inout  [7:0]          c0_ddr4_dqs_c,


   // Cypress FX3 connected to FMC HPC1 (right, next to the Ethernet port)
   output                fx3_pclk,
   inout [15:0]          fx3_dq,
   output                fx3_slcs_n,
   output                fx3_sloe_n,
   output                fx3_slrd_n,
   output                fx3_slwr_n,
   output                fx3_pktend_n,
   output [1:0]          fx3_a,
   input                 fx3_flaga_n,
   input                 fx3_flagb_n,
   input                 fx3_flagc_n,
   input                 fx3_flagd_n,
   input                 fx3_com_rst,
   input                 fx3_logic_rst,
   output [2:0]          fx3_pmode,

   // signals for fan control
   input                 sm_fan_tach,
   output                sm_fan_pwm
   );

   import dii_package::dii_flit;
   import optimsoc_config::*;

   localparam AXI_ID_WIDTH = 4;
   localparam DDR_ADDR_WIDTH = 30;
   localparam DDR_DATA_WIDTH = 32;
   localparam TILE_ADDR_WIDTH = DDR_ADDR_WIDTH - 2;

   localparam TILES = XDIM*YDIM;

   initial begin
      if (LMEM_EXTERNAL && (TILES != 4))
        $error("This design only allows external LMEM for 4 tiles");

      if ((HOST_IF != "uart") && (HOST_IF != "usb3"))
        $error("Invalid HOST_IF, valid values: uart, usb3");
   end

   localparam base_config_t
      BASE_CONFIG = '{ NUMTILES: 4,
         NUMCTS: 4,
         CTLIST: 1024'hf,
         CORES_PER_TILE: NUM_CORES,
         GMEM_SIZE: 0,
         GMEM_TILE: 'x,
         NOC_ENABLE_VCHANNELS: ENABLE_VCHANNELS,
         LMEM_SIZE: LMEM_SIZE_BYTES,
         LMEM_STYLE: (LMEM_EXTERNAL ? EXTERNAL : PLAIN),
         ENABLE_BOOTROM: 0,
         BOOTROM_SIZE: 0,
         ENABLE_DM: 1,
         DM_BASE: 32'h0,
         DM_SIZE: LMEM_SIZE_BYTES,
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
      #(.ID_WIDTH   (0),
         .ADDR_WIDTH (TILE_ADDR_WIDTH),
         .DATA_WIDTH (DDR_DATA_WIDTH))
   c_axi_tile[TILES-1:0](), c_axi_ddr[TILES-1:0]();

   nasti_channel
      #(.ID_WIDTH   (AXI_ID_WIDTH),
         .ADDR_WIDTH (DDR_ADDR_WIDTH),
         .DATA_WIDTH (DDR_DATA_WIDTH))
   c_axi_ddrmem();

   wb_channel
      #(.ADDR_WIDTH (32),
         .DATA_WIDTH (DDR_DATA_WIDTH))
   c_wb_ddr[TILES-1:0]();

   // clocks and reset
   // sysclk1_300_p/n is the 300 MHz board clock
   // cpu_reset is a push button on the board labeled "CPU RESET"

   logic glip_com_rst, glip_ctrl_logic_rst;

   // system clock: 100 MHz
   logic sys_clk_100;
   // system clock: 50 MHz
   logic sys_clk_50;

   // reset from the board and the memory subsystem. Held low until the MIGs
   // are ready.
   logic sys_rst_board;

   // system reset: triggered either from the board, or from the user through
   // GLIP's glip_logic_reset() function
   // XXX: Currently the reset logic of HIM does not take the glip_com_rst
   // properly into account in order to support "hot attach", i.e. connecting
   // to a already running system without fully resetting it. Until this is
   // properly being worked out, we take the glip_com_reset also as system
   // reset to reset the full system (i.e. all CPUs and the debug system).
   logic sys_rst;
   assign sys_rst = sys_rst_board | glip_ctrl_logic_rst | glip_com_rst;

   // UART signals (naming from our point of view, i.e. from the DCE)
   logic uart_rx, uart_tx, uart_cts_n, uart_rts_n;

   // Debug system
   glip_channel c_glip_in(.clk(sys_clk_50));
   glip_channel c_glip_out(.clk(sys_clk_50));

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
                  .rst(sys_rst_board),
                  .com_rst(glip_com_rst),
                  .ctrl_logic_rst(glip_ctrl_logic_rst),

                  .error(/* XXX: connect this to a LED */),

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
                  .rst(sys_rst_board),
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

   logic [TILES*32-1:0] wb_ext_adr_i;
   logic [TILES-1:0] wb_ext_cyc_i;
   logic [TILES*32-1:0] wb_ext_dat_i;
   logic [TILES*4-1:0] wb_ext_sel_i;
   logic [TILES-1:0] wb_ext_stb_i;
   logic [TILES-1:0] wb_ext_we_i;
   logic [TILES*3-1:0] wb_ext_cti_i;
   logic [TILES*2-1:0] wb_ext_bte_i;
   logic [TILES-1:0] wb_ext_ack_o;
   logic [TILES-1:0] wb_ext_err_o;
   logic [TILES-1:0] wb_ext_rty_o;
   logic [TILES*32-1:0] wb_ext_dat_o;

   genvar               t;
   generate
      for (t = 0; t < TILES; t=t+1) begin
         if (LMEM_EXTERNAL) begin
            assign wb_ext_adr_i[32*t +: 32] = c_wb_ddr[t].adr_o;
            assign wb_ext_cyc_i[t] = c_wb_ddr[t].cyc_o;
            assign wb_ext_dat_i[32*t +: 32] = c_wb_ddr[t].dat_o;
            assign wb_ext_sel_i[4*t +: 4] = c_wb_ddr[t].sel_o;
            assign wb_ext_stb_i[t] = c_wb_ddr[t].stb_o;
            assign wb_ext_we_i[t] = c_wb_ddr[t].we_o;
            assign wb_ext_cti_i[3*t +: 3] = c_wb_ddr[t].cti_o;
            assign wb_ext_bte_i[2*t +: 2] = c_wb_ddr[t].bte_o;
            assign c_wb_ddr[t].ack_i = wb_ext_ack_o[t];
            assign c_wb_ddr[t].rty_i = wb_ext_rty_o[t];
            assign c_wb_ddr[t].err_i = wb_ext_err_o[t];
            assign c_wb_ddr[t].dat_i = wb_ext_dat_o[t];
         end else begin
            assign wb_ext_adr_i = 'x;
            assign wb_ext_cyc_i = {TILES{1'b0}};
            assign wb_ext_dat_i = 'x;
            assign wb_ext_sel_i = 'x;
            assign wb_ext_stb_i = {TILES{1'b0}};
            assign wb_ext_we_i = 'x;
            assign wb_ext_cti_i = 'x;
            assign wb_ext_bte_i = 'x;
            assign c_wb_ddr[t].ack_i = {TILES{1'b0}};
            assign c_wb_ddr[t].err_i = {TILES{1'b0}};
            assign c_wb_ddr[t].rty_i = {TILES{1'b0}};
            assign c_wb_ddr[t].dat_i = 'x;
         end // else: !if(LMEM_EXTERNAL)
      end
   endgenerate

   system_allct
      #(.CONFIG(CONFIG), .XDIM(XDIM), .YDIM(YDIM))
      u_system
      (
       .clk           (sys_clk_50),
       .rst           (sys_rst),

       .c_glip_in     (c_glip_in),
       .c_glip_out    (c_glip_out),

       .wb_ext_adr_i  (wb_ext_adr_i),
       .wb_ext_cyc_i  (wb_ext_cyc_i),
       .wb_ext_dat_i  (wb_ext_dat_i),
       .wb_ext_sel_i  (wb_ext_sel_i),
       .wb_ext_stb_i  (wb_ext_stb_i),
       .wb_ext_we_i   (wb_ext_we_i),
       .wb_ext_cab_i  (), // XXX: this is an old signal not present in WB B3 any more!?
       .wb_ext_cti_i  (wb_ext_cti_i),
       .wb_ext_bte_i  (wb_ext_bte_i),
       .wb_ext_ack_o  (wb_ext_ack_o),
       .wb_ext_rty_o  (wb_ext_rty_o),
       .wb_ext_err_o  (wb_ext_err_o),
       .wb_ext_dat_o  (wb_ext_dat_o)
      );

   // board wrapper
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
         .sys_clk_100 (sys_clk_100),
         .sys_clk_50  (sys_clk_50),
         .sys_rst     (sys_rst_board),

         .uart_rx     (uart_rx),
         .uart_tx     (uart_tx),
         .uart_rts_n  (uart_rts_n),
         .uart_cts_n  (uart_cts_n),

         .ddr_awid    (c_axi_ddrmem.aw_id),
         .ddr_awaddr  (c_axi_ddrmem.aw_addr),
         .ddr_awlen   (c_axi_ddrmem.aw_len),
         .ddr_awsize  (c_axi_ddrmem.aw_size),
         .ddr_awburst (c_axi_ddrmem.aw_burst),
         .ddr_awlock  (1'b0), // unused
         .ddr_awcache (c_axi_ddrmem.aw_cache),
         .ddr_awprot  (c_axi_ddrmem.aw_prot),
         .ddr_awqos   (c_axi_ddrmem.aw_qos),
         .ddr_awvalid (c_axi_ddrmem.aw_valid),
         .ddr_awready (c_axi_ddrmem.aw_ready),
         .ddr_wdata   (c_axi_ddrmem.w_data),
         .ddr_wstrb   (c_axi_ddrmem.w_strb),
         .ddr_wlast   (c_axi_ddrmem.w_last),
         .ddr_wvalid  (c_axi_ddrmem.w_valid),
         .ddr_wready  (c_axi_ddrmem.w_ready),
         .ddr_bid     (c_axi_ddrmem.b_id),
         .ddr_bresp   (c_axi_ddrmem.b_resp),
         .ddr_bvalid  (c_axi_ddrmem.b_valid),
         .ddr_bready  (c_axi_ddrmem.b_ready),
         .ddr_arid    (c_axi_ddrmem.ar_id),
         .ddr_araddr  (c_axi_ddrmem.ar_addr),
         .ddr_arlen   (c_axi_ddrmem.ar_len),
         .ddr_arsize  (c_axi_ddrmem.ar_size),
         .ddr_arburst (c_axi_ddrmem.ar_burst),
         .ddr_arlock  (1'b0), // unused
         .ddr_arcache (c_axi_ddrmem.ar_cache),
         .ddr_arprot  (c_axi_ddrmem.ar_prot),
         .ddr_arqos   (c_axi_ddrmem.ar_qos),
         .ddr_arvalid (c_axi_ddrmem.ar_valid),
         .ddr_arready (c_axi_ddrmem.ar_ready),
         .ddr_rid     (c_axi_ddrmem.r_id),
         .ddr_rresp   (c_axi_ddrmem.r_resp),
         .ddr_rdata   (c_axi_ddrmem.r_data),
         .ddr_rlast   (c_axi_ddrmem.r_last),
         .ddr_rvalid  (c_axi_ddrmem.r_valid),
         .ddr_rready  (c_axi_ddrmem.r_ready),

         // signals for fan control
         .sm_fan_tach (sm_fan_tach),
         .sm_fan_pwm  (sm_fan_pwm)
      );

   generate
      if (LMEM_EXTERNAL) begin
         for (t = 0; t < TILES; t=t+1) begin
            // Memory interface: convert WishBone signals from system to AXI for DRAM
            wb2axi
                  #(.ADDR_WIDTH (TILE_ADDR_WIDTH),
                    .DATA_WIDTH (DDR_DATA_WIDTH),
                    .AXI_ID_WIDTH (0))
            u_wb2axi_ddr
                  (.clk             (sys_clk_50),
                   .rst             (sys_rst),
                   .wb_cyc_i        (c_wb_ddr[t].cyc_o),
                   .wb_stb_i        (c_wb_ddr[t].stb_o),
                   .wb_we_i         (c_wb_ddr[t].we_o),
                   .wb_adr_i        (c_wb_ddr[t].adr_o[TILE_ADDR_WIDTH-1:0]),
                   .wb_dat_i        (c_wb_ddr[t].dat_o),
                   .wb_sel_i        (c_wb_ddr[t].sel_o),
                   .wb_cti_i        (c_wb_ddr[t].cti_o),
                   .wb_bte_i        (c_wb_ddr[t].bte_o),
                   .wb_ack_o        (c_wb_ddr[t].ack_i),
                   .wb_err_o        (c_wb_ddr[t].err_i),
                   .wb_rty_o        (c_wb_ddr[t].rty_i),
                   .wb_dat_o        (c_wb_ddr[t].dat_i),
                   .m_axi_awid      (c_axi_tile[t].aw_id),
                   .m_axi_awaddr    (c_axi_tile[t].aw_addr),
                   .m_axi_awlen     (c_axi_tile[t].aw_len),
                   .m_axi_awsize    (c_axi_tile[t].aw_size),
                   .m_axi_awburst   (c_axi_tile[t].aw_burst),
                   .m_axi_awcache   (c_axi_tile[t].aw_cache),
                   .m_axi_awprot    (c_axi_tile[t].aw_prot),
                   .m_axi_awqos     (c_axi_tile[t].aw_qos),
                   .m_axi_awvalid   (c_axi_tile[t].aw_valid),
                   .m_axi_awready   (c_axi_tile[t].aw_ready),
                   .m_axi_wdata     (c_axi_tile[t].w_data),
                   .m_axi_wstrb     (c_axi_tile[t].w_strb),
                   .m_axi_wlast     (c_axi_tile[t].w_last),
                   .m_axi_wvalid    (c_axi_tile[t].w_valid),
                   .m_axi_wready    (c_axi_tile[t].w_ready),
                   .m_axi_bid       (c_axi_tile[t].b_id),
                   .m_axi_bresp     (c_axi_tile[t].b_resp),
                   .m_axi_bvalid    (c_axi_tile[t].b_valid),
                   .m_axi_bready    (c_axi_tile[t].b_ready),
                   .m_axi_arid      (c_axi_tile[t].ar_id),
                   .m_axi_araddr    (c_axi_tile[t].ar_addr),
                   .m_axi_arlen     (c_axi_tile[t].ar_len),
                   .m_axi_arsize    (c_axi_tile[t].ar_size),
                   .m_axi_arburst   (c_axi_tile[t].ar_burst),
                   .m_axi_arcache   (c_axi_tile[t].ar_cache),
                   .m_axi_arprot    (c_axi_tile[t].ar_prot),
                   .m_axi_arqos     (c_axi_tile[t].ar_qos),
                   .m_axi_arvalid   (c_axi_tile[t].ar_valid),
                   .m_axi_arready   (c_axi_tile[t].ar_ready),
                   .m_axi_rid       (c_axi_tile[t].r_id),
                   .m_axi_rdata     (c_axi_tile[t].r_data),
                   .m_axi_rresp     (c_axi_tile[t].r_resp),
                   .m_axi_rlast     (c_axi_tile[t].r_last),
                   .m_axi_rvalid    (c_axi_tile[t].r_valid),
                   .m_axi_rready    (c_axi_tile[t].r_ready)
                   );

            assign c_axi_tile[t].aw_lock = 1'h0;
            assign c_axi_tile[t].aw_region = 4'h0;
            assign c_axi_tile[t].ar_lock = 1'h0;
            assign c_axi_tile[t].ar_region = 4'h0;

            xilinx_axi_register_slice
              u_slice0
                (.aclk(sys_clk_50),
                 .aresetn(!sys_rst),
                 .s_axi_awaddr(c_axi_tile[t].aw_addr),
                 .s_axi_awlen(c_axi_tile[t].aw_len),
                 .s_axi_awsize(c_axi_tile[t].aw_size),
                 .s_axi_awburst(c_axi_tile[t].aw_burst),
                 .s_axi_awlock(c_axi_tile[t].aw_lock),
                 .s_axi_awcache(c_axi_tile[t].aw_cache),
                 .s_axi_awprot(c_axi_tile[t].aw_prot),
                 .s_axi_awregion(c_axi_tile[t].aw_region),
                 .s_axi_awqos(c_axi_tile[t].aw_qos),
                 .s_axi_awvalid(c_axi_tile[t].aw_valid),
                 .s_axi_awready(c_axi_tile[t].aw_ready),
                 .s_axi_wdata(c_axi_tile[t].w_data),
                 .s_axi_wstrb(c_axi_tile[t].w_strb),
                 .s_axi_wlast(c_axi_tile[t].w_last),
                 .s_axi_wvalid(c_axi_tile[t].w_valid),
                 .s_axi_wready(c_axi_tile[t].w_ready),
                 .s_axi_bresp(c_axi_tile[t].b_resp),
                 .s_axi_bvalid(c_axi_tile[t].b_valid),
                 .s_axi_bready(c_axi_tile[t].b_ready),
                 .s_axi_araddr(c_axi_tile[t].ar_addr),
                 .s_axi_arlen(c_axi_tile[t].ar_len),
                 .s_axi_arsize(c_axi_tile[t].ar_size),
                 .s_axi_arburst(c_axi_tile[t].ar_burst),
                 .s_axi_arlock(c_axi_tile[t].ar_lock),
                 .s_axi_arcache(c_axi_tile[t].ar_cache),
                 .s_axi_arprot(c_axi_tile[t].ar_prot),
                 .s_axi_arregion(c_axi_tile[t].ar_region),
                 .s_axi_arqos(c_axi_tile[t].ar_qos),
                 .s_axi_arvalid(c_axi_tile[t].ar_valid),
                 .s_axi_arready(c_axi_tile[t].ar_ready),
                 .s_axi_rdata(c_axi_tile[t].r_data),
                 .s_axi_rresp(c_axi_tile[t].r_resp),
                 .s_axi_rlast(c_axi_tile[t].r_last),
                 .s_axi_rvalid(c_axi_tile[t].r_valid),
                 .s_axi_rready(c_axi_tile[t].r_ready),
                 .m_axi_awaddr(c_axi_ddr[t].aw_addr),
                 .m_axi_awlen(c_axi_ddr[t].aw_len),
                 .m_axi_awsize(c_axi_ddr[t].aw_size),
                 .m_axi_awburst(c_axi_ddr[t].aw_burst),
                 .m_axi_awlock(c_axi_ddr[t].aw_lock),
                 .m_axi_awcache(c_axi_ddr[t].aw_cache),
                 .m_axi_awprot(c_axi_ddr[t].aw_prot),
                 .m_axi_awregion(c_axi_ddr[t].aw_region),
                 .m_axi_awqos(c_axi_ddr[t].aw_qos),
                 .m_axi_awvalid(c_axi_ddr[t].aw_valid),
                 .m_axi_awready(c_axi_ddr[t].aw_ready),
                 .m_axi_wdata(c_axi_ddr[t].w_data),
                 .m_axi_wstrb(c_axi_ddr[t].w_strb),
                 .m_axi_wlast(c_axi_ddr[t].w_last),
                 .m_axi_wvalid(c_axi_ddr[t].w_valid),
                 .m_axi_wready(c_axi_ddr[t].w_ready),
                 .m_axi_bresp(c_axi_ddr[t].b_resp),
                 .m_axi_bvalid(c_axi_ddr[t].b_valid),
                 .m_axi_bready(c_axi_ddr[t].b_ready),
                 .m_axi_araddr(c_axi_ddr[t].ar_addr),
                 .m_axi_arlen(c_axi_ddr[t].ar_len),
                 .m_axi_arsize(c_axi_ddr[t].ar_size),
                 .m_axi_arburst(c_axi_ddr[t].ar_burst),
                 .m_axi_arlock(c_axi_ddr[t].ar_lock),
                 .m_axi_arcache(c_axi_ddr[t].ar_cache),
                 .m_axi_arprot(c_axi_ddr[t].ar_prot),
                 .m_axi_arregion(c_axi_ddr[t].ar_region),
                 .m_axi_arqos(c_axi_ddr[t].ar_qos),
                 .m_axi_arvalid(c_axi_ddr[t].ar_valid),
                 .m_axi_arready(c_axi_ddr[t].ar_ready),
                 .m_axi_rdata(c_axi_ddr[t].r_data),
                 .m_axi_rresp(c_axi_ddr[t].r_resp),
                 .m_axi_rlast(c_axi_ddr[t].r_last),
                 .m_axi_rvalid(c_axi_ddr[t].r_valid),
                 .m_axi_rready(c_axi_ddr[t].r_ready)
                 );
         end // for (t = 0; t < TILES; t=t+1)

         xilinx_axi_interconnect_4to1
           u_axi_interconnect
             (
              .INTERCONNECT_ACLK    (sys_clk_50),
              .INTERCONNECT_ARESETN (!sys_rst),

              .S00_AXI_ARESET_OUT_N (),
              .S00_AXI_ACLK         (sys_clk_50),
              .S00_AXI_AWID         (0),
              .S00_AXI_AWADDR       ({2'b00, c_axi_ddr[0].aw_addr}),
              .S00_AXI_AWLEN        (c_axi_ddr[0].aw_len),
              .S00_AXI_AWSIZE       (c_axi_ddr[0].aw_size),
              .S00_AXI_AWBURST      (c_axi_ddr[0].aw_burst),
              .S00_AXI_AWLOCK       (0),
              .S00_AXI_AWCACHE      (c_axi_ddr[0].aw_cache),
              .S00_AXI_AWPROT       (c_axi_ddr[0].aw_prot),
              .S00_AXI_AWQOS        (c_axi_ddr[0].aw_qos),
              .S00_AXI_AWVALID      (c_axi_ddr[0].aw_valid),
              .S00_AXI_AWREADY      (c_axi_ddr[0].aw_ready),
              .S00_AXI_WDATA        (c_axi_ddr[0].w_data),
              .S00_AXI_WSTRB        (c_axi_ddr[0].w_strb),
              .S00_AXI_WLAST        (c_axi_ddr[0].w_last),
              .S00_AXI_WVALID       (c_axi_ddr[0].w_valid),
              .S00_AXI_WREADY       (c_axi_ddr[0].w_ready),
              .S00_AXI_BID          (),
              .S00_AXI_BRESP        (c_axi_ddr[0].b_resp),
              .S00_AXI_BVALID       (c_axi_ddr[0].b_valid),
              .S00_AXI_BREADY       (c_axi_ddr[0].b_ready),
              .S00_AXI_ARID         (0),
              .S00_AXI_ARADDR       ({2'b00, c_axi_ddr[0].ar_addr}),
              .S00_AXI_ARLEN        (c_axi_ddr[0].ar_len),
              .S00_AXI_ARSIZE       (c_axi_ddr[0].ar_size),
              .S00_AXI_ARBURST      (c_axi_ddr[0].ar_burst),
              .S00_AXI_ARLOCK       (0),
              .S00_AXI_ARCACHE      (c_axi_ddr[0].ar_cache),
              .S00_AXI_ARPROT       (c_axi_ddr[0].ar_prot),
              .S00_AXI_ARQOS        (c_axi_ddr[0].ar_qos),
              .S00_AXI_ARVALID      (c_axi_ddr[0].ar_valid),
              .S00_AXI_ARREADY      (c_axi_ddr[0].ar_ready),
              .S00_AXI_RID          (),
              .S00_AXI_RDATA        (c_axi_ddr[0].r_data),
              .S00_AXI_RRESP        (c_axi_ddr[0].r_resp),
              .S00_AXI_RLAST        (c_axi_ddr[0].r_last),
              .S00_AXI_RVALID       (c_axi_ddr[0].r_valid),
              .S00_AXI_RREADY       (c_axi_ddr[0].r_ready),

              .S01_AXI_ARESET_OUT_N (),
              .S01_AXI_ACLK         (sys_clk_50),
              .S01_AXI_AWID         (0),
              .S01_AXI_AWADDR       ({2'b01, c_axi_ddr[1].aw_addr}),
              .S01_AXI_AWLEN        (c_axi_ddr[1].aw_len),
              .S01_AXI_AWSIZE       (c_axi_ddr[1].aw_size),
              .S01_AXI_AWBURST      (c_axi_ddr[1].aw_burst),
              .S01_AXI_AWLOCK       (0),
              .S01_AXI_AWCACHE      (c_axi_ddr[1].aw_cache),
              .S01_AXI_AWPROT       (c_axi_ddr[1].aw_prot),
              .S01_AXI_AWQOS        (c_axi_ddr[1].aw_qos),
              .S01_AXI_AWVALID      (c_axi_ddr[1].aw_valid),
              .S01_AXI_AWREADY      (c_axi_ddr[1].aw_ready),
              .S01_AXI_WDATA        (c_axi_ddr[1].w_data),
              .S01_AXI_WSTRB        (c_axi_ddr[1].w_strb),
              .S01_AXI_WLAST        (c_axi_ddr[1].w_last),
              .S01_AXI_WVALID       (c_axi_ddr[1].w_valid),
              .S01_AXI_WREADY       (c_axi_ddr[1].w_ready),
              .S01_AXI_BID          (),
              .S01_AXI_BRESP        (c_axi_ddr[1].b_resp),
              .S01_AXI_BVALID       (c_axi_ddr[1].b_valid),
              .S01_AXI_BREADY       (c_axi_ddr[1].b_ready),
              .S01_AXI_ARID         (0),
              .S01_AXI_ARADDR       ({2'b01, c_axi_ddr[1].ar_addr}),
              .S01_AXI_ARLEN        (c_axi_ddr[1].ar_len),
              .S01_AXI_ARSIZE       (c_axi_ddr[1].ar_size),
              .S01_AXI_ARBURST      (c_axi_ddr[1].ar_burst),
              .S01_AXI_ARLOCK       (0),
              .S01_AXI_ARCACHE      (c_axi_ddr[1].ar_cache),
              .S01_AXI_ARPROT       (c_axi_ddr[1].ar_prot),
              .S01_AXI_ARQOS        (c_axi_ddr[1].ar_qos),
              .S01_AXI_ARVALID      (c_axi_ddr[1].ar_valid),
              .S01_AXI_ARREADY      (c_axi_ddr[1].ar_ready),
              .S01_AXI_RID          (),
              .S01_AXI_RDATA        (c_axi_ddr[1].r_data),
              .S01_AXI_RRESP        (c_axi_ddr[1].r_resp),
              .S01_AXI_RLAST        (c_axi_ddr[1].r_last),
              .S01_AXI_RVALID       (c_axi_ddr[1].r_valid),
              .S01_AXI_RREADY       (c_axi_ddr[1].r_ready),

              .S02_AXI_ARESET_OUT_N (),
              .S02_AXI_ACLK         (sys_clk_50),
              .S02_AXI_AWID         (0),
              .S02_AXI_AWADDR       ({2'b10, c_axi_ddr[2].aw_addr}),
              .S02_AXI_AWLEN        (c_axi_ddr[2].aw_len),
              .S02_AXI_AWSIZE       (c_axi_ddr[2].aw_size),
              .S02_AXI_AWBURST      (c_axi_ddr[2].aw_burst),
              .S02_AXI_AWLOCK       (0),
              .S02_AXI_AWCACHE      (c_axi_ddr[2].aw_cache),
              .S02_AXI_AWPROT       (c_axi_ddr[2].aw_prot),
              .S02_AXI_AWQOS        (c_axi_ddr[2].aw_qos),
              .S02_AXI_AWVALID      (c_axi_ddr[2].aw_valid),
              .S02_AXI_AWREADY      (c_axi_ddr[2].aw_ready),
              .S02_AXI_WDATA        (c_axi_ddr[2].w_data),
              .S02_AXI_WSTRB        (c_axi_ddr[2].w_strb),
              .S02_AXI_WLAST        (c_axi_ddr[2].w_last),
              .S02_AXI_WVALID       (c_axi_ddr[2].w_valid),
              .S02_AXI_WREADY       (c_axi_ddr[2].w_ready),
              .S02_AXI_BID          (),
              .S02_AXI_BRESP        (c_axi_ddr[2].b_resp),
              .S02_AXI_BVALID       (c_axi_ddr[2].b_valid),
              .S02_AXI_BREADY       (c_axi_ddr[2].b_ready),
              .S02_AXI_ARID         (0),
              .S02_AXI_ARADDR       ({2'b10, c_axi_ddr[2].ar_addr}),
              .S02_AXI_ARLEN        (c_axi_ddr[2].ar_len),
              .S02_AXI_ARSIZE       (c_axi_ddr[2].ar_size),
              .S02_AXI_ARBURST      (c_axi_ddr[2].ar_burst),
              .S02_AXI_ARLOCK       (0),
              .S02_AXI_ARCACHE      (c_axi_ddr[2].ar_cache),
              .S02_AXI_ARPROT       (c_axi_ddr[2].ar_prot),
              .S02_AXI_ARQOS        (c_axi_ddr[2].ar_qos),
              .S02_AXI_ARVALID      (c_axi_ddr[2].ar_valid),
              .S02_AXI_ARREADY      (c_axi_ddr[2].ar_ready),
              .S02_AXI_RID          (),
              .S02_AXI_RDATA        (c_axi_ddr[2].r_data),
              .S02_AXI_RRESP        (c_axi_ddr[2].r_resp),
              .S02_AXI_RLAST        (c_axi_ddr[2].r_last),
              .S02_AXI_RVALID       (c_axi_ddr[2].r_valid),
              .S02_AXI_RREADY       (c_axi_ddr[2].r_ready),

              .S03_AXI_ARESET_OUT_N (),
              .S03_AXI_ACLK         (sys_clk_50),
              .S03_AXI_AWID         (0),
              .S03_AXI_AWADDR       ({2'b11, c_axi_ddr[3].aw_addr}),
              .S03_AXI_AWLEN        (c_axi_ddr[3].aw_len),
              .S03_AXI_AWSIZE       (c_axi_ddr[3].aw_size),
              .S03_AXI_AWBURST      (c_axi_ddr[3].aw_burst),
              .S03_AXI_AWLOCK       (0),
              .S03_AXI_AWCACHE      (c_axi_ddr[3].aw_cache),
              .S03_AXI_AWPROT       (c_axi_ddr[3].aw_prot),
              .S03_AXI_AWQOS        (c_axi_ddr[3].aw_qos),
              .S03_AXI_AWVALID      (c_axi_ddr[3].aw_valid),
              .S03_AXI_AWREADY      (c_axi_ddr[3].aw_ready),
              .S03_AXI_WDATA        (c_axi_ddr[3].w_data),
              .S03_AXI_WSTRB        (c_axi_ddr[3].w_strb),
              .S03_AXI_WLAST        (c_axi_ddr[3].w_last),
              .S03_AXI_WVALID       (c_axi_ddr[3].w_valid),
              .S03_AXI_WREADY       (c_axi_ddr[3].w_ready),
              .S03_AXI_BID          (),
              .S03_AXI_BRESP        (c_axi_ddr[3].b_resp),
              .S03_AXI_BVALID       (c_axi_ddr[3].b_valid),
              .S03_AXI_BREADY       (c_axi_ddr[3].b_ready),
              .S03_AXI_ARID         (0),
              .S03_AXI_ARADDR       ({2'b11, c_axi_ddr[3].ar_addr}),
              .S03_AXI_ARLEN        (c_axi_ddr[3].ar_len),
              .S03_AXI_ARSIZE       (c_axi_ddr[3].ar_size),
              .S03_AXI_ARBURST      (c_axi_ddr[3].ar_burst),
              .S03_AXI_ARLOCK       (0),
              .S03_AXI_ARCACHE      (c_axi_ddr[3].ar_cache),
              .S03_AXI_ARPROT       (c_axi_ddr[3].ar_prot),
              .S03_AXI_ARQOS        (c_axi_ddr[3].ar_qos),
              .S03_AXI_ARVALID      (c_axi_ddr[3].ar_valid),
              .S03_AXI_ARREADY      (c_axi_ddr[3].ar_ready),
              .S03_AXI_RID          (),
              .S03_AXI_RDATA        (c_axi_ddr[3].r_data),
              .S03_AXI_RRESP        (c_axi_ddr[3].r_resp),
              .S03_AXI_RLAST        (c_axi_ddr[3].r_last),
              .S03_AXI_RVALID       (c_axi_ddr[3].r_valid),
              .S03_AXI_RREADY       (c_axi_ddr[3].r_ready),

              .M00_AXI_ARESET_OUT_N (),
              .M00_AXI_ACLK         (sys_clk_50),
              .M00_AXI_AWID         (c_axi_ddrmem.aw_id),
              .M00_AXI_AWADDR       (c_axi_ddrmem.aw_addr),
              .M00_AXI_AWLEN        (c_axi_ddrmem.aw_len),
              .M00_AXI_AWSIZE       (c_axi_ddrmem.aw_size),
              .M00_AXI_AWBURST      (c_axi_ddrmem.aw_burst),
              .M00_AXI_AWLOCK       (),
              .M00_AXI_AWCACHE      (c_axi_ddrmem.aw_cache),
              .M00_AXI_AWPROT       (c_axi_ddrmem.aw_prot),
              .M00_AXI_AWQOS        (c_axi_ddrmem.aw_qos),
              .M00_AXI_AWVALID      (c_axi_ddrmem.aw_valid),
              .M00_AXI_AWREADY      (c_axi_ddrmem.aw_ready),
              .M00_AXI_WDATA        (c_axi_ddrmem.w_data),
              .M00_AXI_WSTRB        (c_axi_ddrmem.w_strb),
              .M00_AXI_WLAST        (c_axi_ddrmem.w_last),
              .M00_AXI_WVALID       (c_axi_ddrmem.w_valid),
              .M00_AXI_WREADY       (c_axi_ddrmem.w_ready),
              .M00_AXI_BID          (c_axi_ddrmem.b_id),
              .M00_AXI_BRESP        (c_axi_ddrmem.b_resp),
              .M00_AXI_BVALID       (c_axi_ddrmem.b_valid),
              .M00_AXI_BREADY       (c_axi_ddrmem.b_ready),
              .M00_AXI_ARID         (c_axi_ddrmem.ar_id),
              .M00_AXI_ARADDR       (c_axi_ddrmem.ar_addr),
              .M00_AXI_ARLEN        (c_axi_ddrmem.ar_len),
              .M00_AXI_ARSIZE       (c_axi_ddrmem.ar_size),
              .M00_AXI_ARBURST      (c_axi_ddrmem.ar_burst),
              .M00_AXI_ARLOCK       (),
              .M00_AXI_ARCACHE      (c_axi_ddrmem.ar_cache),
              .M00_AXI_ARPROT       (c_axi_ddrmem.ar_prot),
              .M00_AXI_ARQOS        (c_axi_ddrmem.ar_qos),
              .M00_AXI_ARVALID      (c_axi_ddrmem.ar_valid),
              .M00_AXI_ARREADY      (c_axi_ddrmem.ar_ready),
              .M00_AXI_RID          (c_axi_ddrmem.r_id),
              .M00_AXI_RDATA        (c_axi_ddrmem.r_data),
              .M00_AXI_RRESP        (c_axi_ddrmem.r_resp),
              .M00_AXI_RLAST        (c_axi_ddrmem.r_last),
              .M00_AXI_RVALID       (c_axi_ddrmem.r_valid),
              .M00_AXI_RREADY       (c_axi_ddrmem.r_ready)
              );
      end
   endgenerate

endmodule // system_allct_vcu108
