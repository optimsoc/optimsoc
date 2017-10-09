/* Copyright (c) 2016 by the author(s)
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
 * Toplevel: compute_tile_dm on a Nexys 4 DDR board with support for Ethernet
 *
 * Author(s):
 *   Annika Fuchs <annika.fuchs@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

import dii_package::dii_flit;
import optimsoc::*;

module compute_tile_dm_ethernet_nexys4
  (
   // 100 MHz system clock from board
   input                 clk,
   // Button "CPU RESET" (C12)
   input                 cpu_resetn,

   // UART; RTS and CTS are seen from the host side
   output                uart_rxd_out,
   input                 uart_txd_in,
   output                uart_cts,
   input                 uart_rts,

   // DDR
   output [12:0]         ddr2_addr,
   output [2:0]          ddr2_ba,
   output                ddr2_cas_n,
   output                ddr2_ck_n,
   output                ddr2_ck_p,
   output                ddr2_cke,
   output                ddr2_cs_n,
   output [1:0]          ddr2_dm,
   inout [15:0]          ddr2_dq,
   inout [1:0]           ddr2_dqs_n,
   inout [1:0]           ddr2_dqs_p,
   output                ddr2_odt,
   output                ddr2_ras_n,
   output                ddr2_we_n,
   
   
   // Ethernet
    input                 eth_crsdv,
    input                 eth_rxerr,
    input  [1:0]          eth_rxd,
    output [1:0]          eth_txd,
    output                eth_txen,
    output                eth_mdc,
    inout                 eth_mdio,
    output                eth_rstn,
    output                eth_refclk,
   
   output [15:0] led
   
   );

   parameter integer NUM_CORES = 1;
   localparam integer LMEM_SIZE = 128*1024*1024;

   localparam AXI_ID_WIDTH = 4;
   localparam DDR_ADDR_WIDTH = 28;
   localparam DDR_DATA_WIDTH = 32;

   localparam base_config_t
     BASE_CONFIG = '{ NUMTILES: 1,
                      NUMCTS: 1,
                      CTLIST: {{63{16'hx}}, 16'h0},
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
                      DEBUG_CTM: 1
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

   wb_channel
     #(.ADDR_WIDTH (32),
       .DATA_WIDTH (32))
     c_wb_ess();

   wb_channel
     #(.ADDR_WIDTH (32),
       .DATA_WIDTH (32))
     c_wb_fifo();

     
   // clocks and reset
   // clk is the 100 MHz board clock
   // cpu_resetn is a push button on the board (active low)

   // system clock: 50 MHz
   logic sys_clk;

   // system reset
   logic sys_rst;
   
   // 50 MHz fixed clock (for eth)
   logic clk_50mhz;
   // 50 MHz fixed clock with 45 degree phase shift (relative to clk_50mhz, 
   // for eth phy)
   logic clk_50mhz_45deg;
   // 125 MHz clock
   logic clk_125mhz;

   // UART signals (naming from our point of view, i.e. from the DCE)
   logic uart_rx, uart_tx, uart_cts_n, uart_rts_n;

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
   glip_channel c_glip_in(.clk(sys_clk));
   glip_channel c_glip_out(.clk(sys_clk));

   // XXX: does the HIM support hot-attach by now?
   // See discussion in system_2x2_cccc_ztex
   logic glip_com_rst, glip_ctrl_logic_rst;

   // Off-chip UART communication interface for debug
   glip_uart_toplevel
      #(.FREQ_CLK_IO(50000000),
        .BAUD(12000000),
        .WIDTH(16),
        .BUFFER_OUT_DEPTH(256*1024))
      u_glip(
         .clk_io(sys_clk),
         .clk(sys_clk),
         .rst(sys_rst),
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

   logic dbg_sys_rst, dbg_cpu_rst;

   dii_flit [1:0] debug_ring_in;
   dii_flit [1:0] debug_ring_out;
   logic [1:0] debug_ring_in_ready;
   logic [1:0] debug_ring_out_ready;

   debug_interface
      #(
         .SYSTEMID    (1),
         .NUM_MODULES (CONFIG.DEBUG_NUM_MODS)
      )
      u_debuginterface
        (
         .clk            (sys_clk),
         .rst            (sys_rst),
         .sys_rst        (dbg_sys_rst),
         .cpu_rst        (dbg_cpu_rst),
         .glip_in        (c_glip_in),
         .glip_out       (c_glip_out),
         .ring_out       (debug_ring_in),
         .ring_out_ready (debug_ring_in_ready),
         .ring_in        (debug_ring_out),
         .ring_in_ready  (debug_ring_out_ready)
      );


    (* mark_debug = "true" *)   wire eth_irq;
     
   // Single compute tile with all memory mapped to the DRAM
   compute_tile_dm
      #(.CONFIG(CONFIG),
        .DEBUG_BASEID(2)
      )
      u_compute_tile
        (
         .clk           (sys_clk),
         .rst_cpu       (dbg_cpu_rst | sys_rst),
         .rst_sys       (dbg_sys_rst | sys_rst),
         .rst_dbg       (sys_rst),

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
         
         .eth_irq (eth_irq),

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
         .wb_ext_dat_o  (c_wb_ddr.dat_i),

         // Ethernet Subsystem Control
         .wb_ess_adr_i  (c_wb_ess.adr_o),
         .wb_ess_cyc_i  (c_wb_ess.cyc_o),
         .wb_ess_dat_i  (c_wb_ess.dat_o),
         .wb_ess_sel_i  (c_wb_ess.sel_o),
         .wb_ess_stb_i  (c_wb_ess.stb_o),
         .wb_ess_we_i   (c_wb_ess.we_o),
         .wb_ess_cti_i  (c_wb_ess.cti_o),
         .wb_ess_bte_i  (c_wb_ess.bte_o),
         .wb_ess_ack_o  (c_wb_ess.ack_i),
         .wb_ess_rty_o  (c_wb_ess.rty_i),
         .wb_ess_err_o  (c_wb_ess.err_i),
         .wb_ess_dat_o  (c_wb_ess.dat_i),

         // Ethernet Subsystem Data
         .wb_fifo_adr_i  (c_wb_fifo.adr_o),
         .wb_fifo_cyc_i  (c_wb_fifo.cyc_o),
         .wb_fifo_dat_i  (c_wb_fifo.dat_o),
         .wb_fifo_sel_i  (c_wb_fifo.sel_o),
         .wb_fifo_stb_i  (c_wb_fifo.stb_o),
         .wb_fifo_we_i   (c_wb_fifo.we_o),
         .wb_fifo_cti_i  (c_wb_fifo.cti_o),
         .wb_fifo_bte_i  (c_wb_fifo.bte_o),
         .wb_fifo_ack_o  (c_wb_fifo.ack_i),
         .wb_fifo_rty_o  (c_wb_fifo.rty_i),
         .wb_fifo_err_o  (c_wb_fifo.err_i),
         .wb_fifo_dat_o  (c_wb_fifo.dat_i)
      );
      

   // Nexys 4 board wrapper
   nexys4ddr
      #(
         .NUM_UART(1)
      )
      u_board(
         // FPGA/board interface
         .clk(clk),
         .cpu_resetn(cpu_resetn),

         .uart_rxd_out(uart_rxd_out),
         .uart_txd_in(uart_txd_in),
         .uart_rts(uart_rts),
         .uart_cts(uart_cts),

         .ddr2_addr(ddr2_addr),
         .ddr2_ba(ddr2_ba),
         .ddr2_cas_n(ddr2_cas_n),
         .ddr2_ck_n(ddr2_ck_n),
         .ddr2_ck_p(ddr2_ck_p),
         .ddr2_cke(ddr2_cke),
         .ddr2_cs_n(ddr2_cs_n),
         .ddr2_dm(ddr2_dm),
         .ddr2_dq(ddr2_dq),
         .ddr2_dqs_n(ddr2_dqs_n),
         .ddr2_dqs_p(ddr2_dqs_p),
         .ddr2_odt(ddr2_odt),
         .ddr2_ras_n(ddr2_ras_n),
         .ddr2_we_n(ddr2_we_n),

         // system interface
         .sys_clk     (sys_clk),
         .sys_rst     (sys_rst),
         
         .clk_50mhz   (clk_50mhz),
         .clk_50mhz_45deg (clk_50mhz_45deg),
         .clk_125mhz(clk_125mhz),

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
         .ddr_rready  (c_axi_ddr.r_ready)
      );

   // Memory interface: convert WishBone signals from system to AXI for DRAM
   wb2axi
     #(.ADDR_WIDTH (DDR_ADDR_WIDTH),
       .DATA_WIDTH (DDR_DATA_WIDTH),
       .AXI_ID_WIDTH (AXI_ID_WIDTH))
   u_wb2axi_ddr
     (.clk             (sys_clk),
      .rst             (sys_rst),
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

// tri-state assignment for mdio
     wire eth_mdio_t;
     wire eth_mdio_i;
     wire eth_mdio_o;
     //assign eth_mdio = eth_mdio_t ? eth_mdio_i : eth_mdio_o;
     
     IOBUF 
        mdio_io_iobuf ( 
           .I (eth_mdio_o ), 
           .IO(eth_mdio   ), 
           .O (eth_mdio_i ), 
           .T (eth_mdio_t ));
        
     // PHY needs 50 Mhz clock shifted 45 deg compared to RMII 2 MII
     // See Nexys 4 DDR board documentation for details.
     assign eth_refclk = clk_50mhz_45deg;
     
     
// AXI Ethernet Subsystem
     
     wire phy_rst_n;
     assign eth_rstn = phy_rst_n;
     
     // wires MII2RMII Converter to Ethernet Subsystem
        (* mark_debug = "true" *) wire [3:0] mii_txd;
        (* mark_debug = "true" *) wire mii_tx_en;
        (* mark_debug = "true" *) wire mii_tx_er;
        (* mark_debug = "true" *) wire mii_tx_clk;
        (* mark_debug = "true" *) wire mii_rx_clk;
        (* mark_debug = "true" *) wire [3:0] mii_rxd;
        (* mark_debug = "true" *) wire mii_rx_dv;
        (* mark_debug = "true" *) wire mii_rx_er;
                 
     // wires AXI4Stream FIFO to Ethernet Subsystem
        (* mark_debug = "true" *)   wire [31:0] s_axis_txc_tdata;
        (* mark_debug = "true" *)   wire [3:0]  s_axis_txc_tkeep; // not in use
        (* mark_debug = "true" *)   wire s_axis_txc_tlast;
        (* mark_debug = "true" *)   wire s_axis_txc_tready;
        (* mark_debug = "true" *)   wire s_axis_txc_tvalid;
        
        (* mark_debug = "true" *)   wire [31:0] s_axis_txd_tdata;
        (* mark_debug = "true" *)   wire [3:0]  s_axis_txd_tkeep; // not in use
        (* mark_debug = "true" *)   wire s_axis_txd_tlast;
        (* mark_debug = "true" *)   wire s_axis_txd_tready;
        (* mark_debug = "true" *)   wire s_axis_txd_tvalid;
        
        (* mark_debug = "true" *)   wire [31:0] m_axis_rxd_tdata;
        (* mark_debug = "true" *)   wire [3:0]  m_axis_rxd_tkeep;
        (* mark_debug = "true" *)   wire m_axis_rxd_tlast;
        (* mark_debug = "true" *)   wire m_axis_rxd_tready;
        (* mark_debug = "true" *)   wire m_axis_rxd_tvalid;         
        

       // wire AXI4 Ethernet Subsystem to WB2AXI Converter
      //wire s_axi_aclk; // noch nicht verbunden!
      //wire s_axi_aresetn;// noch nicht verbunden!
        (* mark_debug = "true" *)wire [31:0] s_axi_awaddr;
        (* mark_debug = "true" *)wire s_axi_awvalid;
        (* mark_debug = "true" *)wire s_axi_awready;
        (* mark_debug = "true" *)wire [31:0]s_axi_wdata;
        (* mark_debug = "true" *)wire [3:0]s_axi_wstrb;
        (* mark_debug = "true" *)wire s_axi_wvalid;
        (* mark_debug = "true" *) wire s_axi_wready;
        (* mark_debug = "true" *)wire [1:0]  s_axi_bresp;
        (* mark_debug = "true" *)wire s_axi_bvalid;
        (* mark_debug = "true" *)wire s_axi_bready;
        (* mark_debug = "true" *)wire [31:0] s_axi_araddr;
        (* mark_debug = "true" *)wire s_axi_arvalid;
        (* mark_debug = "true" *)wire s_axi_arready;
        (* mark_debug = "true" *)wire [31:0] s_axi_rdata;
        (* mark_debug = "true" *)wire [1:0] s_axi_rresp;
        (* mark_debug = "true" *)wire s_axi_rvalid;
        (* mark_debug = "true" *)wire s_axi_rready;
        (* mark_debug = "true" *)wire [3:0] s_axi_wstrb;
                  
   // wire AXI4 Stream FIFO to WB2AXI Converter
   //wire fifo_s_axi_aclk; // noch nicht verbunden!
   //wire fifo_s_axi_aresetn;// noch nicht verbunden!
   (* mark_debug = "true" *)   wire [31:0] fifo_s_axi_awaddr;
   (* mark_debug = "true" *)   wire fifo_s_axi_awvalid;
   (* mark_debug = "true" *)   wire fifo_s_axi_awready;
   (* mark_debug = "true" *)   wire [31:0] fifo_s_axi_wdata;
   (* mark_debug = "true" *)   wire [3:0] fifo_s_axi_wstrb;
   (* mark_debug = "true" *)   wire fifo_s_axi_wvalid;
   (* mark_debug = "true" *)   wire fifo_s_axi_wready;
   (* mark_debug = "true" *)   wire [1:0] fifo_s_axi_bresp;
   (* mark_debug = "true" *)   wire fifo_s_axi_bvalid;
   (* mark_debug = "true" *)   wire fifo_s_axi_bready;
   (* mark_debug = "true" *)   wire [31:0] fifo_s_axi_araddr;
   (* mark_debug = "true" *)    wire fifo_s_axi_arvalid;
   (* mark_debug = "true" *)   wire fifo_s_axi_arready;
   (* mark_debug = "true" *)   wire [31:0] fifo_s_axi_rdata;
   (* mark_debug = "true" *)   wire [1:0] fifo_s_axi_rresp;
   (* mark_debug = "true" *)   wire fifo_s_axi_rvalid;
   (* mark_debug = "true" *)   wire fifo_s_axi_rready;       

   // ASSUME THIS IS CORRECT!?
   assign s_axis_txd_tkeep = 4'b1111;
   assign s_axis_txc_tkeep = 4'b1111;
   
     axi_ethernet_0
        u_axi_ethernet
        (
           // AXI Lite control interface
           .s_axi_lite_clk      (sys_clk      ) ,
           .s_axi_lite_resetn   (~sys_rst   ),
           .s_axi_araddr        (s_axi_araddr      ),
           .s_axi_arready       (s_axi_arready     ),
           .s_axi_arvalid       (s_axi_arvalid     ),
           .s_axi_awaddr        (s_axi_awaddr      ),
           .s_axi_awready       (s_axi_awready     ),
           .s_axi_awvalid       (s_axi_awvalid     ),
           .s_axi_bready        (s_axi_bready      ),
           .s_axi_bresp         (s_axi_bresp       ),
           .s_axi_bvalid        (s_axi_bvalid      ),
           .s_axi_rdata         (s_axi_rdata       ),
           .s_axi_rready        (s_axi_rready      ),
           .s_axi_rresp         (s_axi_rresp       ),
           .s_axi_rvalid        (s_axi_rvalid      ),
           .s_axi_wdata         (s_axi_wdata       ),
           .s_axi_wready        (s_axi_wready      ),
           .s_axi_wvalid        (s_axi_wvalid      ),
           .s_axi_wstrb         (s_axi_wstrb       ),

           // AXI Stream TX/RX interface
           .axis_clk            (sys_clk          ),

           .axi_txc_arstn       (~sys_rst         ),
           .s_axis_txc_tdata    (s_axis_txc_tdata  ),
           .s_axis_txc_tkeep    (s_axis_txc_tkeep  ),
           .s_axis_txc_tlast    (s_axis_txc_tlast  ),
           .s_axis_txc_tready   (s_axis_txc_tready ),
           .s_axis_txc_tvalid   (s_axis_txc_tvalid ),

           .axi_txd_arstn       (~sys_rst         ),
           .s_axis_txd_tdata    (s_axis_txd_tdata  ),
           .s_axis_txd_tkeep    (s_axis_txd_tkeep  ),
           .s_axis_txd_tlast    (s_axis_txd_tlast  ),
           .s_axis_txd_tready   (s_axis_txd_tready ),
           .s_axis_txd_tvalid   (s_axis_txd_tvalid ),

           .axi_rxd_arstn       (~sys_rst         ),
           .m_axis_rxd_tdata    (m_axis_rxd_tdata  ),
           .m_axis_rxd_tkeep    (m_axis_rxd_tkeep  ),
           .m_axis_rxd_tlast    (m_axis_rxd_tlast  ),
           .m_axis_rxd_tready   (m_axis_rxd_tready ),
           .m_axis_rxd_tvalid   (m_axis_rxd_tvalid ),
        
           .axi_rxs_arstn       (~sys_rst         ), // not in use only for DMA
           .m_axis_rxs_tdata    (  ), // not in use only for DMA
           .m_axis_rxs_tkeep    (  ), // not in use only for DMA
           .m_axis_rxs_tlast    (  ), // not in use only for DMA
           .m_axis_rxs_tready   (1'b1 ), // not in use only for DMA
           .m_axis_rxs_tvalid   ( ), // not in use only for DMA

           // MII Interface
           .mii_txd             (mii_txd           ),
           .mii_tx_en           (mii_tx_en         ),
           .mii_tx_er           (mii_tx_er         ),
           .mii_tx_clk          (mii_tx_clk        ),
           .mii_rx_clk          (mii_rx_clk        ),
           .mii_rxd             (mii_rxd           ),
           .mii_rx_dv           (mii_rx_dv         ),
           .mii_rx_er           (mii_rx_er         ),

           // Ethernet PHY connections
           .mdio_mdc            (eth_mdc           ),
           .mdio_mdio_i         (eth_mdio_i        ),
           .mdio_mdio_o         (eth_mdio_o        ),
           .mdio_mdio_t         (eth_mdio_t            ),
           .phy_rst_n           (phy_rst_n         ), 

           .gtx_clk             (clk_125mhz    )

        );

        
        
// MII to RMII Converter
      mii_to_rmii_0
         u_mii_to_rmii_0 
         (
              .ref_clk           (clk_50mhz),
              .rst_n             (phy_rst_n),
              .mac2rmii_tx_en    (mii_tx_en         ),
              .mac2rmii_txd      (mii_txd           ),
              .mac2rmii_tx_er    (mii_tx_er         ),
              .rmii2mac_tx_clk   (mii_tx_clk        ),
              .rmii2mac_rx_clk   (mii_rx_clk        ),
              .rmii2mac_col      (),
              .rmii2mac_crs      (led[0]), // debug only; carrier sense led
              .rmii2mac_rx_dv    (mii_rx_dv         ),
              .rmii2mac_rx_er    (mii_rx_er),
              .rmii2mac_rxd      (mii_rxd           ),
              .phy2rmii_crs_dv   (eth_crsdv),
              .phy2rmii_rx_er    (eth_rxerr),
              .phy2rmii_rxd      (eth_rxd  ),
              .rmii2phy_txd      (eth_txd  ),
              .rmii2phy_tx_en    (eth_txen )
              
           );
// AXI4Stream FIFO
      axi_fifo_mm_s_0
         u_axi_fifo_mm_s_0
         (
            .s_axi_aclk            (sys_clk    ),
            .s_axi_aresetn         (~sys_rst),
            
            .s_axi_awaddr          (fifo_s_axi_awaddr  ),
            .s_axi_awvalid         (fifo_s_axi_awvalid ),
            .s_axi_awready         (fifo_s_axi_awready ),
            .s_axi_wdata           (fifo_s_axi_wdata   ),
            .s_axi_wstrb           (fifo_s_axi_wstrb   ),
            .s_axi_wvalid          (fifo_s_axi_wvalid  ),
            .s_axi_wready          (fifo_s_axi_wready  ),
            .s_axi_bresp           (fifo_s_axi_bresp   ),
            .s_axi_bvalid          (fifo_s_axi_bvalid  ),
            .s_axi_bready          (fifo_s_axi_bready  ),
            .s_axi_araddr          (fifo_s_axi_araddr  ),
            .s_axi_arvalid         (fifo_s_axi_arvalid ),
            .s_axi_arready         (fifo_s_axi_arready ),
            .s_axi_rdata           (fifo_s_axi_rdata   ),
            .s_axi_rresp           (fifo_s_axi_rresp   ),
            .s_axi_rvalid          (fifo_s_axi_rvalid  ),
            .s_axi_rready          (fifo_s_axi_rready  ),
            .mm2s_prmry_reset_out_n(),
            .axi_str_txd_tvalid    (s_axis_txd_tvalid  ),
            .axi_str_txd_tready    (s_axis_txd_tready  ),
            .axi_str_txd_tlast     (s_axis_txd_tlast   ),
            .axi_str_txd_tdata     (s_axis_txd_tdata   ),
            .mm2s_cntrl_reset_out_n(),
            .axi_str_txc_tvalid    (s_axis_txc_tvalid  ),
            .axi_str_txc_tready    (s_axis_txc_tready  ),
            .axi_str_txc_tlast     (s_axis_txc_tlast   ),
            .axi_str_txc_tdata     (s_axis_txc_tdata   ),
            .s2mm_prmry_reset_out_n(),
            .axi_str_rxd_tvalid    (m_axis_rxd_tvalid  ),
            .axi_str_rxd_tready    (m_axis_rxd_tready  ),
            .axi_str_rxd_tlast     (m_axis_rxd_tlast   ),
            .axi_str_rxd_tdata     (m_axis_rxd_tdata   ),
            
            .interrupt             (eth_irq)
         );
            
         // Memory interface: convert WishBone signals from system to AXI for AXI4FIFO
         wb2axi
            #(.ADDR_WIDTH (32), // vielleich??
               .DATA_WIDTH (32),// vielleich??
               .AXI_ID_WIDTH (AXI_ID_WIDTH))// vielleich??
            u_wb2axi_fifo
            (.clk             (sys_clk),
             .rst             (sys_rst),
             .wb_cyc_i        (c_wb_fifo.cyc_o),
             .wb_stb_i        (c_wb_fifo.stb_o),
             .wb_we_i         (c_wb_fifo.we_o),
             .wb_adr_i        (c_wb_fifo.adr_o),
             .wb_dat_i        (c_wb_fifo.dat_o),
             .wb_sel_i        (c_wb_fifo.sel_o),
             .wb_cti_i        (c_wb_fifo.cti_o),
             .wb_bte_i        (c_wb_fifo.bte_o),
             .wb_ack_o        (c_wb_fifo.ack_i),
             .wb_err_o        (c_wb_fifo.err_i),
             .wb_rty_o        (c_wb_fifo.rty_i),
             .wb_dat_o        (c_wb_fifo.dat_i),
             .m_axi_awid      (),
             .m_axi_awaddr    (fifo_s_axi_awaddr),
             .m_axi_awlen     (),
             .m_axi_awsize    (),
             .m_axi_awburst   (),
             .m_axi_awcache   (),
             .m_axi_awprot    (),
             .m_axi_awqos     (),
             .m_axi_awvalid   (fifo_s_axi_awvalid),
             .m_axi_awready   (fifo_s_axi_awready),
             .m_axi_wdata     (fifo_s_axi_wdata),
             .m_axi_wstrb     (fifo_s_axi_wstrb),
             .m_axi_wlast     (),
             .m_axi_wvalid    (fifo_s_axi_wvalid),
             .m_axi_wready    (fifo_s_axi_wready),
             .m_axi_bid       (),
             .m_axi_bresp     (fifo_s_axi_bresp),
             .m_axi_bvalid    (fifo_s_axi_bvalid),
             .m_axi_bready    (fifo_s_axi_bready),
             .m_axi_arid      (),
             .m_axi_araddr    (fifo_s_axi_araddr),
             .m_axi_arlen     (),
             .m_axi_arsize    (),
             .m_axi_arburst   (),
             .m_axi_arcache   (),
             .m_axi_arprot    (),
             .m_axi_arqos     (),
             .m_axi_arvalid   (fifo_s_axi_arvalid),
             .m_axi_arready   (fifo_s_axi_arready),
             .m_axi_rid       (),
             .m_axi_rdata     (fifo_s_axi_rdata),
             .m_axi_rresp     (fifo_s_axi_rresp),
             .m_axi_rlast     (),
             .m_axi_rvalid    (fifo_s_axi_rvalid),
             .m_axi_rready    (fifo_s_axi_rready)
            );

            
// Memory interface: convert WishBone signals from system to AXI for AXI Ethernet Subsystem
            wb2axi
               #(.ADDR_WIDTH (32), // vielleich??
                  .DATA_WIDTH (32),// vielleich??
                  .AXI_ID_WIDTH (AXI_ID_WIDTH))// vielleich??
               u_wb2axi_ess
               (.clk             (sys_clk),
                  .rst             (sys_rst),
                  .wb_cyc_i        (c_wb_ess.cyc_o),
                  .wb_stb_i        (c_wb_ess.stb_o),
                  .wb_we_i         (c_wb_ess.we_o),
                  .wb_adr_i        (c_wb_ess.adr_o),
                  .wb_dat_i        (c_wb_ess.dat_o),
                  .wb_sel_i        (c_wb_ess.sel_o),
                  .wb_cti_i        (c_wb_ess.cti_o),
                  .wb_bte_i        (c_wb_ess.bte_o),
                  .wb_ack_o        (c_wb_ess.ack_i),
                  .wb_err_o        (c_wb_ess.err_i),
                  .wb_rty_o        (c_wb_ess.rty_i),
                  .wb_dat_o        (c_wb_ess.dat_i),
                  .m_axi_awid      (),
                  .m_axi_awaddr    (s_axi_awaddr),
                  .m_axi_awlen     (),
                  .m_axi_awsize    (),
                  .m_axi_awburst   (),
                  .m_axi_awcache   (),
                  .m_axi_awprot    (),
                  .m_axi_awqos     (),
                  .m_axi_awvalid   (s_axi_awvalid),
                  .m_axi_awready   (s_axi_awready),
                  .m_axi_wdata     (s_axi_wdata),
                  .m_axi_wstrb     (s_axi_wstrb),
                  .m_axi_wlast     (),
                  .m_axi_wvalid    (s_axi_wvalid),
                  .m_axi_wready    (s_axi_wready),
                  .m_axi_bid       (),
                  .m_axi_bresp     (s_axi_bresp),
                  .m_axi_bvalid    (s_axi_bvalid),
                  .m_axi_bready    (s_axi_bready),
                  .m_axi_arid      (),
                  .m_axi_araddr    (s_axi_araddr),
                  .m_axi_arlen     (),
                  .m_axi_arsize    (),
                  .m_axi_arburst   (),
                  .m_axi_arcache   (),
                  .m_axi_arprot    (),
                  .m_axi_arqos     (),
                  .m_axi_arvalid   (s_axi_arvalid),
                  .m_axi_arready   (s_axi_arready),
                  .m_axi_rid       (),
                  .m_axi_rdata     (s_axi_rdata),
                  .m_axi_rresp     (s_axi_rresp),
                  .m_axi_rlast     (),
                  .m_axi_rvalid    (s_axi_rvalid),
                  .m_axi_rready    (s_axi_rready)
               );            
     
     
endmodule
