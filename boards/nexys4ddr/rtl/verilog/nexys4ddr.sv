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
 * Nexys 4 DDR board abstraction
 *
 * Parameters:
 * - NUM_UART: Number of UART devices supported.
 *     Currently 1 is the only valid choice.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */
module nexys4ddr
  #(parameter NUM_UART = 1)
  (
   // FPGA IO
   input                 clk,
   input                 cpu_resetn,

   // all following UART signals are from a DTE (the PC) point-of-view
   output                uart_rxd_out,
   input                 uart_txd_in,
   output                uart_cts, // active low (despite the name)
   input                 uart_rts, // active low (despite the name)

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

   // System Interface
   output                sys_clk,
   output                sys_rst,

   input [3:0]           ddr_awid,
   input [31:0]          ddr_awaddr,
   input [7:0]           ddr_awlen,
   input [2:0]           ddr_awsize,
   input [1:0]           ddr_awburst,
   input [3:0]           ddr_awcache,
   input [2:0]           ddr_awprot,
   input [3:0]           ddr_awqos,
   input                 ddr_awvalid,
   output                ddr_awready,
   input [31:0]          ddr_wdata,
   input [3:0]           ddr_wstrb,
   input                 ddr_wlast,
   input                 ddr_wvalid,
   output                ddr_wready,
   output [3:0]          ddr_bid,
   output [1:0]          ddr_bresp,
   output                ddr_bvalid,
   input                 ddr_bready,
   input [3:0]           ddr_arid,
   input [31:0]          ddr_araddr,
   input [7:0]           ddr_arlen,
   input [2:0]           ddr_arsize,
   input [1:0]           ddr_arburst,
   input [3:0]           ddr_arcache,
   input [2:0]           ddr_arprot,
   input [3:0]           ddr_arqos,
   input                 ddr_arvalid,
   output                ddr_arready,
   output [31:0]         ddr_rdata,
   output                ddr_rlast,
   output [3:0]          ddr_rid,
   output [1:0]          ddr_rresp,
   output                ddr_rvalid,
   input                 ddr_rready,

/*   output                jtag_capture_dr,
   output                jtag_select,
   output                jtag_shift_dr,
   output                jtag_update_dr,
   output                jtag_tck,
   output                jtag_tms,
   output                jtag_tdi,
   input                 jtag_tdo,*/

   // all following UART signals are from a DCE (FPGA) point of view!
   // i.e. "crossed" from the input signals
   output [NUM_UART-1:0] uart_rx,
   input [NUM_UART-1:0]  uart_tx,
   output [NUM_UART-1:0] uart_cts_n,
   input [NUM_UART-1:0]  uart_rts_n

/*   output [23*8-1:0]     gpio_in,
   input [23*8-1:0]      gpio_out,
   input [23*8-1:0]      gpio_oe*/
   );

   logic         rst;
   assign rst = !cpu_resetn;

   logic        clk_ddr_ref; // 200 MHz clock
   logic        clk_ddr_sys; // 200 MHz clock
   logic        clk_ddr_locked;
   logic        ddr_calib_done;
   logic        ddr_mmcm_locked;
   logic        mig_ui_clk; // clock from MIG UI (1/4 of clk_ddr_ref = 50 MHz)
   logic        mig_ui_rst; // Synchronized reset

   clk_gen_ddr
     u_clk_gen_ddr
       (.clk_in      (clk),
        .clk_ddr_sys (clk_ddr_sys),
        .clk_ddr_ref (clk_ddr_ref),
        .locked      (clk_ddr_locked),
        .reset       (rst)
        );

/*   clk_gen_sys
     u_clk_gen_sys
       (.clk_in  (mig_ui_clk),
        .clk_sys (sys_clk),
        .reset   (!ddr_mmcm_locked | rst),
        .locked  (clk_sys_locked));*/
   assign sys_clk = mig_ui_clk;
   assign sys_rst = !(ddr_mmcm_locked & ddr_calib_done);



   // UART
   initial begin
      if (NUM_UART != 1) begin
         $display("%m: NUM_UART must be 1");
         $stop;
      end
   end
   // Important note: We change to a different view of naming here.
   // The signals from/to the board are seen from a DTE (host PC) point of view,
   // all internally used signals are seen from a DCE (FPGA) point of view
   // (i.e. from our view).
   assign uart_rx[0] = uart_txd_in;
   assign uart_rxd_out = uart_tx[0];
   assign uart_cts = uart_rts_n[0];
   assign uart_cts_n[0] = uart_rts;

/*   BSCANE2
     #(.JTAG_CHAIN(2)) // Use ID 2 as 1 is used by chipscope
   xilinx_jtag_tap0
     (
      .CAPTURE   (jtag_capture_dr),
      .DRCK      (),
      .RESET     (),
      .RUNTEST   (),
      .SEL       (jtag_select),
      .SHIFT     (jtag_shift_dr),
      .TCK       (jtag_tck),
      .TDI       (jtag_tdo),
      .TMS       (jtag_tms),
      .UPDATE    (jtag_update_dr),
      .TDO       (jtag_tdi)
      );*/

   mig_7series
     u_mig_7series
       (.init_calib_complete            (ddr_calib_done),
        .sys_clk_i                      (clk_ddr_sys),
        .sys_rst                        (clk_ddr_locked | rst),

        // off-chip connection
        .ddr2_dq                        (ddr2_dq),
        .ddr2_dqs_n                     (ddr2_dqs_n),
        .ddr2_dqs_p                     (ddr2_dqs_p),
        .ddr2_addr                      (ddr2_addr),
        .ddr2_ba                        (ddr2_ba),
        .ddr2_ras_n                     (ddr2_ras_n),
        .ddr2_cas_n                     (ddr2_cas_n),
        .ddr2_we_n                      (ddr2_we_n),
        .ddr2_ck_p                      (ddr2_ck_p),
        .ddr2_ck_n                      (ddr2_ck_n),
        .ddr2_cke                       (ddr2_cke),
        .ddr2_cs_n                      (ddr2_cs_n),
        .ddr2_dm                        (ddr2_dm),
        .ddr2_odt                       (ddr2_odt),

        // Application interface ports
        .ui_clk                         (mig_ui_clk),
        .ui_clk_sync_rst                (mig_ui_rst),
        .mmcm_locked                    (ddr_mmcm_locked),
        .aresetn                        (!sys_rst),
        .app_sr_req                     (0),
        .app_ref_req                    (0),
        .app_zq_req                     (0),
        .app_sr_active                  (),
        .app_ref_ack                    (),
        .app_zq_ack                     (),

        .ui_addn_clk_0                  (),
        .ui_addn_clk_1                  (),
        .ui_addn_clk_2                  (),
        .ui_addn_clk_3                  (),
        .ui_addn_clk_4                  (),

        .device_temp_i                  (0),

        // Slave Interface Write Address Ports
        .s_axi_awid                     (ddr_awid),
        .s_axi_awaddr                   (ddr_awaddr[27:0]),
        .s_axi_awlen                    (ddr_awlen),
        .s_axi_awsize                   (ddr_awsize),
        .s_axi_awburst                  (ddr_awburst),
        .s_axi_awlock                   (0),
        .s_axi_awcache                  (ddr_awcache),
        .s_axi_awprot                   (ddr_awprot),
        .s_axi_awqos                    (ddr_awqos),
        .s_axi_awvalid                  (ddr_awvalid),
        .s_axi_awready                  (ddr_awready),
        // Slave Interface Write Data Ports
        .s_axi_wdata                    (ddr_wdata),
        .s_axi_wstrb                    (ddr_wstrb),
        .s_axi_wlast                    (ddr_wlast),
        .s_axi_wvalid                   (ddr_wvalid),
        .s_axi_wready                   (ddr_wready),
        // Slave Interface Write Response Ports
        .s_axi_bid                      (ddr_bid),
        .s_axi_bresp                    (ddr_bresp),
        .s_axi_bvalid                   (ddr_bvalid),
        .s_axi_bready                   (ddr_bready),
        // Slave Interface Read Address Ports
        .s_axi_arid                     (ddr_arid),
        .s_axi_araddr                   (ddr_araddr[27:0]),
        .s_axi_arlen                    (ddr_arlen),
        .s_axi_arsize                   (ddr_arsize),
        .s_axi_arburst                  (ddr_arburst),
        .s_axi_arlock                   (0),
        .s_axi_arcache                  (ddr_arcache),
        .s_axi_arprot                   (ddr_arprot),
        .s_axi_arqos                    (ddr_arqos),
        .s_axi_arvalid                  (ddr_arvalid),
        .s_axi_arready                  (ddr_arready),
        // Slave Interface Read Data Ports
        .s_axi_rid                      (ddr_rid),
        .s_axi_rdata                    (ddr_rdata),
        .s_axi_rresp                    (ddr_rresp),
        .s_axi_rlast                    (ddr_rlast),
        .s_axi_rvalid                   (ddr_rvalid),
        .s_axi_rready                   (ddr_rready)
        );

endmodule // nexys4ddr
