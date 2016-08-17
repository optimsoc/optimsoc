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
 * Toplevel: compute_tile_dm on a Nexys 4 DDR board
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

`include "optimsoc_def.vh"

import dii_package::dii_flit;

module compute_tile_dm_nexys4
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
   output                ddr2_we_n
   );

   parameter USE_DEBUG = 1;

   localparam AXI_ID_WIDTH = 4;
   localparam DDR_ADDR_WIDTH = 28;
   localparam DDR_DATA_WIDTH = 32;

   localparam NOC_FLIT_DATA_WIDTH = 32;
   localparam NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH+NOC_FLIT_TYPE_WIDTH;
   localparam VCHANNELS = `VCHANNELS;

   dii_flit ring_2_him_in;
   logic ring_2_him_in_ready;
   dii_flit ring_2_him_out;
   logic ring_2_him_out_ready;

   dii_flit ring_2_scm_in;
   logic ring_2_scm_in_ready;
   dii_flit ring_2_scm_out;
   logic ring_2_scm_out_ready;

   dii_flit ring_2_mam_in;
   logic ring_2_mam_in_ready;
   dii_flit ring_2_mam_out;
   logic ring_2_mam_out_ready;

   localparam NUM_PORTS = 3;
   localparam NUM_MODS = 1;
   logic [2:0][9:0]    id_map = {{10'h2}, {10'h1}, {10'h0}};
   dii_flit [2:0]      ring_in = {ring_2_mam_out, ring_2_scm_out, ring_2_him_out};
   logic [2:0]         ring_in_ready = {ring_2_mam_out_ready, ring_2_scm_out_ready, ring_2_him_out_ready};
   dii_flit [2:0]      ring_out = {ring_2_mam_in, ring_2_scm_in, ring_2_him_in};
   logic [2:0]         ring_out_ready = {ring_2_mam_in_ready, ring_2_scm_in_ready, ring_2_him_in_ready};

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
   // clk is the 100 MHz board clock
   // cpu_resetn is a push button on the board (active low)

   // system clock: 50 MHz
   logic sys_clk;

   // system reset
   logic sys_rst;

   // UART signals (naming from our point of view, i.e. from the DCE)
   logic uart_rx, uart_tx, uart_cts_n, uart_rts_n;

   // terminate NoC connection
   logic [NOC_FLIT_WIDTH-1:0] noc_in_flit;
   logic [VCHANNELS-1:0] noc_in_valid;
   logic [VCHANNELS-1:0] noc_in_ready;
   logic [NOC_FLIT_WIDTH-1:0] noc_out_flit;
   logic [VCHANNELS-1:0] noc_out_valid;
   logic [VCHANNELS-1:0] noc_out_ready;

   assign noc_in_valid = 0;
   assign noc_out_ready = 0;

   // Debug system
   glip_channel c_glip_in(.clk(sys_clk));
   glip_channel c_glip_out(.clk(sys_clk));

   dii_flit debug_ring_in;
   logic debug_ring_in_ready;
   dii_flit debug_ring_out;
   logic debug_ring_out_ready;

   // XXX: does the HIM support hot-attach by now?
   // See discussion in system_2x2_cccc_ztex
   logic glip_com_rst, glip_ctrl_logic_rst;

   // Off-chip UART communication interface for debug
   glip_uart_toplevel
      #(.FREQ_CLK_IO(50000000),
        .BAUD(115200),
        .WIDTH(16),
        .XILINX_TARGET_DEVICE("7SERIES"))
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

   if (USE_DEBUG == 1) begin
      // Debug Ring
      // Port 0 connects to HIM
      // Port 1 connects to SCM
      // Port 2 connects to MAM
      debug_ring
         #(
            .PORTS(NUM_PORTS)
         )
         u_dbg_ring(
         .clk(sys_clk),
         .rst(sys_rst),
         .dii_in(ring_in),
         .dii_in_ready(ring_in_ready),
         .dii_out(ring_out),
         .dii_out_ready(ring_out_ready),
         .id_map(id_map)
         );
   
      debug_interface
         #(
            .SYSTEMID    (1),
            .NUM_MODULES (NUM_MODS)
         )
         u_debuginterface(
            .*,
            .clk           (sys_clk),
            .rst           (sys_rst),
            .sys_rst       (),
            .cpu_rst       (),
            .glip_in       (c_glip_in),
            .glip_out      (c_glip_out),
            .him_out       (ring_in[0]),
            .him_out_ready (ring_in_ready[0]),
            .him_in        (ring_out[0]),
            .him_in_ready  (ring_out_ready[0]),
            .scm_out       (ring_in[1]),
            .scm_out_ready (ring_in_ready[1]),
            .scm_in        (ring_out[1]),
            .scm_in_ready  (ring_out_ready[1])
         );
   end // if(USE_DEBUG == 1)

   // XXX: Add system trace and other debug modules to compute tile

   // Single compute tile with all memory mapped to the DRAM
   compute_tile_dm
      #(
         .VCHANNELS(VCHANNELS),
         .NOC_FLIT_DATA_WIDTH(NOC_FLIT_DATA_WIDTH),
         .NOC_FLIT_TYPE_WIDTH(NOC_FLIT_TYPE_WIDTH)
      )
      u_compute_tile(
         .clk           (sys_clk),
         .rst_cpu       (sys_rst),
         .rst_sys       (sys_rst),

         .noc_in_flit   (noc_in_flit),
         .noc_in_ready  (noc_in_ready),
         .noc_in_valid  (noc_in_valid),
         .noc_out_flit  (noc_out_flit),
         .noc_out_ready (noc_out_ready),
         .noc_out_valid (noc_out_valid),

         .debug_in(ring_out[2]),
         .debug_in_ready(ring_out_ready[2]),
         .debug_out(ring_in[2]),
         .debug_out_ready(ring_in_ready[2])
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

endmodule // compute_tile_dm_nexys4
