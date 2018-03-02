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
 * Xilinx VCU108 board abstraction
 *
 * Parameters:
 * - NUM_UART: Number of UART devices supported.
 *     Currently 1 is the only valid choice.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */
module vcu108
  #(parameter NUM_UART = 1,
    // Source of uart0
    // onboard: use USB UART connection on the board
    // pmod: connect a pmodusbuart module to PMOD connector J52 (bottom row)
    parameter UART0_SOURCE = "onboard")
  (
   //
   // FPGA IO
   //
   // 300 MHz system clock
   input                 sysclk1_300_p,
   input                 sysclk1_300_n,

   // CPU reset button
   input                 cpu_reset,

   // UART
   // all following UART signals are from a DTE (the PC) point-of-view
   output                usb_uart_rx,
   input                 usb_uart_tx,
   output                usb_uart_cts, // active low (despite the name)
   input                 usb_uart_rts, // active low (despite the name)

   // UART over pmod
   output                pmod_uart_rx,
   input                 pmod_uart_tx,
   output                pmod_uart_cts, // active low (despite the name)
   input                 pmod_uart_rts, // active low (despite the name)

   // DDR C0 I/O
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

   //
   // System Interface
   //
   output                sys_clk_100, // 100 MHz system clock
   output                sys_clk_50, // 50 MHz system clock
   output                sys_rst, // system reset (active high)

   // DRAM AXI interface
   input [3:0]           ddr_awid,
   input [29:0]          ddr_awaddr,
   input [7:0]           ddr_awlen,
   input [2:0]           ddr_awsize,
   input [1:0]           ddr_awburst,
   input [0:0]           ddr_awlock,
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
   input                 ddr_bready,
   output [3:0]          ddr_bid,
   output [1:0]          ddr_bresp,
   output                ddr_bvalid,
   input [3:0]           ddr_arid,
   input [29:0]          ddr_araddr,
   input [7:0]           ddr_arlen,
   input [2:0]           ddr_arsize,
   input [1:0]           ddr_arburst,
   input [0:0]           ddr_arlock,
   input [3:0]           ddr_arcache,
   input [2:0]           ddr_arprot,
   input [3:0]           ddr_arqos,
   input                 ddr_arvalid,
   output                ddr_arready,
   input                 ddr_rready,
   output [3:0]          ddr_rid,
   output [31:0]         ddr_rdata,
   output [1:0]          ddr_rresp,
   output                ddr_rlast,
   output                ddr_rvalid,


   // all following UART signals are from a DCE (FPGA) point of view!
   // i.e. "crossed" from the input signals
   output [NUM_UART-1:0] uart_rx,
   input [NUM_UART-1:0]  uart_tx,
   output [NUM_UART-1:0] uart_cts_n,
   input [NUM_UART-1:0]  uart_rts_n,

/*   output [23*8-1:0]     gpio_in,
   input [23*8-1:0]      gpio_out,
   input [23*8-1:0]      gpio_oe*/

   // signals for fan control
   input                 sm_fan_tach,
   output                sm_fan_pwm
   );

   logic        ddr_calib_done;

   // Reset logic
   logic mig_ui_rst;

   // external reset input (push button)
   logic external_rst_in;
   assign external_rst_in = cpu_reset;

   // reset output to the attached logic
   // deasserted if DRAM is ready (mig_ui_rst is part of ddr_calib_done already)
   assign sys_rst = ~ddr_calib_done;
   logic sys_rst_n;
   assign sys_rst_n = ~sys_rst;


   // System clock is generated from the MIG UI clock
   logic mig_ui_clk;

   BUFGCE_DIV #(
         .BUFGCE_DIVIDE(6.0),    // 1-8
         // Programmable Inversion Attributes: Specifies built-in programmable inversion on specific pins
         .IS_CE_INVERTED(1'b0),  // Optional inversion for CE
         .IS_CLR_INVERTED(1'b0), // Optional inversion for CLR
         .IS_I_INVERTED(1'b0)    // Optional inversion for I
      )
      BUFGCE_DIV_CLK1_inst (
         .O(sys_clk_50),   // 1-bit output: Buffer
         .CE(1'b1),        // 1-bit input: Buffer enable
         .CLR(1'b0),       // 1-bit input: Asynchronous clear
         .I(mig_ui_clk)    // 1-bit input: Buffer
      );

   BUFGCE_DIV #(
         .BUFGCE_DIVIDE(3.0),    // 1-8
         // Programmable Inversion Attributes: Specifies built-in programmable inversion on specific pins
         .IS_CE_INVERTED(1'b0),  // Optional inversion for CE
         .IS_CLR_INVERTED(1'b0), // Optional inversion for CLR
         .IS_I_INVERTED(1'b0)    // Optional inversion for I
      )
      BUFGCE_DIV_CLK2_inst (
         .O(sys_clk_100),  // 1-bit output: Buffer
         .CE(1'b1),        // 1-bit input: Buffer enable
         .CLR(1'b0),       // 1-bit input: Asynchronous clear
         .I(mig_ui_clk)    // 1-bit input: Buffer
      );

   logic mig_ui_rst_n;
   assign mig_ui_rst_n = ~mig_ui_rst;


   // connection signals between the DRAM (slave) and the AXI clock converter
   // (master)
   logic [3:0]           c0_ddr4_s_axi_awid;
   logic [29:0]          c0_ddr4_s_axi_awaddr;
   logic [7:0]           c0_ddr4_s_axi_awlen;
   logic [2:0]           c0_ddr4_s_axi_awsize;
   logic [1:0]           c0_ddr4_s_axi_awburst;
   logic [0:0]           c0_ddr4_s_axi_awlock;
   logic [3:0]           c0_ddr4_s_axi_awcache;
   logic [2:0]           c0_ddr4_s_axi_awprot;
   logic [3:0]           c0_ddr4_s_axi_awqos;
   logic                 c0_ddr4_s_axi_awvalid;
   logic                 c0_ddr4_s_axi_awready;
   logic [31:0]          c0_ddr4_s_axi_wdata;
   logic [3:0]           c0_ddr4_s_axi_wstrb;
   logic                 c0_ddr4_s_axi_wlast;
   logic                 c0_ddr4_s_axi_wvalid;
   logic                 c0_ddr4_s_axi_wready;
   logic                 c0_ddr4_s_axi_bready;
   logic [3:0]           c0_ddr4_s_axi_bid;
   logic [1:0]           c0_ddr4_s_axi_bresp;
   logic                 c0_ddr4_s_axi_bvalid;
   logic [3:0]           c0_ddr4_s_axi_arid;
   logic [29:0]          c0_ddr4_s_axi_araddr;
   logic [7:0]           c0_ddr4_s_axi_arlen;
   logic [2:0]           c0_ddr4_s_axi_arsize;
   logic [1:0]           c0_ddr4_s_axi_arburst;
   logic [0:0]           c0_ddr4_s_axi_arlock;
   logic [3:0]           c0_ddr4_s_axi_arcache;
   logic [2:0]           c0_ddr4_s_axi_arprot;
   logic [3:0]           c0_ddr4_s_axi_arqos;
   logic                 c0_ddr4_s_axi_arvalid;
   logic                 c0_ddr4_s_axi_arready;
   logic                 c0_ddr4_s_axi_rready;
   logic [3:0]           c0_ddr4_s_axi_rid;
   logic [31:0]          c0_ddr4_s_axi_rdata;
   logic [1:0]           c0_ddr4_s_axi_rresp;
   logic                 c0_ddr4_s_axi_rlast;
   logic                 c0_ddr4_s_axi_rvalid;

   mig_ultrascale u_mig_ultrascale
      (

         .c0_init_calib_complete (ddr_calib_done), // signals that the DDR is ready to be used

         .sys_rst           (external_rst_in), // reset input for the whole MIG

         // directly connect to 300 MHz sysclk
         .c0_sys_clk_p                   (sysclk1_300_p),
         .c0_sys_clk_n                   (sysclk1_300_n),

         // UI clock generated by the clock manager inside the MIG (300 MHz)
         .c0_ddr4_ui_clk                (mig_ui_clk),
         .c0_ddr4_ui_clk_sync_rst       (mig_ui_rst),

         // Additional UI clock generated by the same clock manager (50 MHz)
         .addn_ui_clkout1               (),

         // DDR4 off-chip interface
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

         .dbg_clk                                    (),


         // AXI4 slave interface
         // in the mig_ui_clk clock domain

         // Slave Interface Write Address Ports
         .c0_ddr4_aresetn                     (mig_ui_rst_n),
         .c0_ddr4_s_axi_awid                  (c0_ddr4_s_axi_awid),
         .c0_ddr4_s_axi_awaddr                (c0_ddr4_s_axi_awaddr),
         .c0_ddr4_s_axi_awlen                 (c0_ddr4_s_axi_awlen),
         .c0_ddr4_s_axi_awsize                (c0_ddr4_s_axi_awsize),
         .c0_ddr4_s_axi_awburst               (c0_ddr4_s_axi_awburst),
         .c0_ddr4_s_axi_awlock                (c0_ddr4_s_axi_awlock),
         .c0_ddr4_s_axi_awcache               (c0_ddr4_s_axi_awcache),
         .c0_ddr4_s_axi_awprot                (c0_ddr4_s_axi_awprot),
         .c0_ddr4_s_axi_awqos                 (c0_ddr4_s_axi_awqos),
         .c0_ddr4_s_axi_awvalid               (c0_ddr4_s_axi_awvalid),
         .c0_ddr4_s_axi_awready               (c0_ddr4_s_axi_awready),
         // Slave Interface Write Data Ports
         .c0_ddr4_s_axi_wdata                 (c0_ddr4_s_axi_wdata),
         .c0_ddr4_s_axi_wstrb                 (c0_ddr4_s_axi_wstrb),
         .c0_ddr4_s_axi_wlast                 (c0_ddr4_s_axi_wlast),
         .c0_ddr4_s_axi_wvalid                (c0_ddr4_s_axi_wvalid),
         .c0_ddr4_s_axi_wready                (c0_ddr4_s_axi_wready),
         // Slave Interface Write Response Ports
         .c0_ddr4_s_axi_bid                   (c0_ddr4_s_axi_bid),
         .c0_ddr4_s_axi_bresp                 (c0_ddr4_s_axi_bresp),
         .c0_ddr4_s_axi_bvalid                (c0_ddr4_s_axi_bvalid),
         .c0_ddr4_s_axi_bready                (c0_ddr4_s_axi_bready),
         // Slave Interface Read Address Ports
         .c0_ddr4_s_axi_arid                  (c0_ddr4_s_axi_arid),
         .c0_ddr4_s_axi_araddr                (c0_ddr4_s_axi_araddr),
         .c0_ddr4_s_axi_arlen                 (c0_ddr4_s_axi_arlen),
         .c0_ddr4_s_axi_arsize                (c0_ddr4_s_axi_arsize),
         .c0_ddr4_s_axi_arburst               (c0_ddr4_s_axi_arburst),
         .c0_ddr4_s_axi_arlock                (c0_ddr4_s_axi_arlock),
         .c0_ddr4_s_axi_arcache               (c0_ddr4_s_axi_arcache),
         .c0_ddr4_s_axi_arprot                (c0_ddr4_s_axi_arprot),
         .c0_ddr4_s_axi_arqos                 (c0_ddr4_s_axi_arqos),
         .c0_ddr4_s_axi_arvalid               (c0_ddr4_s_axi_arvalid),
         .c0_ddr4_s_axi_arready               (c0_ddr4_s_axi_arready),
         // Slave Interface Read Data Ports
         .c0_ddr4_s_axi_rid                   (c0_ddr4_s_axi_rid),
         .c0_ddr4_s_axi_rdata                 (c0_ddr4_s_axi_rdata),
         .c0_ddr4_s_axi_rresp                 (c0_ddr4_s_axi_rresp),
         .c0_ddr4_s_axi_rlast                 (c0_ddr4_s_axi_rlast),
         .c0_ddr4_s_axi_rvalid                (c0_ddr4_s_axi_rvalid),
         .c0_ddr4_s_axi_rready                (c0_ddr4_s_axi_rready),

         // Debug Port
         .dbg_bus         ()
      );


   // cross the memory AXI bus from 300 MHz to 50 MHz
   ddr4_c0_axi_clk_converter
      u_ddr4_c0_axi_clk_converter
      (
         // AXI slave side for the outside world
         /**************** Write Address Channel Signals ****************/
         .s_axi_awaddr(ddr_awaddr),
         .s_axi_awprot(ddr_awprot),
         .s_axi_awvalid(ddr_awvalid),
         .s_axi_awready(ddr_awready),
         .s_axi_awsize(ddr_awsize),
         .s_axi_awburst(ddr_awburst),
         .s_axi_awcache(ddr_awcache),
         .s_axi_awlen(ddr_awlen),
         .s_axi_awlock(ddr_awlock),
         .s_axi_awqos(ddr_awqos),
         .s_axi_awregion(4'b0000), // not supported by MIG
         .s_axi_awid(ddr_awid),
         /**************** Write Data Channel Signals ****************/
         .s_axi_wdata(ddr_wdata),
         .s_axi_wstrb(ddr_wstrb),
         .s_axi_wvalid(ddr_wvalid),
         .s_axi_wready(ddr_wready),
         .s_axi_wlast(ddr_wlast),
         /**************** Write Response Channel Signals ****************/
         .s_axi_bresp(ddr_bresp),
         .s_axi_bvalid(ddr_bvalid),
         .s_axi_bready(ddr_bready),
         .s_axi_bid(ddr_bid),
         /**************** Read Address Channel Signals ****************/
         .s_axi_araddr(ddr_araddr),
         .s_axi_arprot(ddr_arprot),
         .s_axi_arvalid(ddr_arvalid),
         .s_axi_arready(ddr_arready),
         .s_axi_arsize(ddr_arsize),
         .s_axi_arburst(ddr_arburst),
         .s_axi_arcache(ddr_arcache),
         .s_axi_arlock(ddr_arlock),
         .s_axi_arlen(ddr_arlen),
         .s_axi_arqos(ddr_arqos),
         .s_axi_arregion(),
         .s_axi_arid(ddr_arid),
         /**************** Read Data Channel Signals ****************/
         .s_axi_rdata(ddr_rdata),
         .s_axi_rresp(ddr_rresp),
         .s_axi_rvalid(ddr_rvalid),
         .s_axi_rready(ddr_rready),
         .s_axi_rlast(ddr_rlast),
         .s_axi_rid(ddr_rid),
         /**************** System Signals ****************/
         .s_axi_aclk(sys_clk_50),
         .s_axi_aresetn(sys_rst_n),


         // AXI master interface: connect to MIG
         /**************** Write Address Channel Signals ****************/
         .m_axi_awaddr(c0_ddr4_s_axi_awaddr),
         .m_axi_awprot(c0_ddr4_s_axi_awprot),
         .m_axi_awvalid(c0_ddr4_s_axi_awvalid),
         .m_axi_awready(c0_ddr4_s_axi_awready),
         .m_axi_awsize(c0_ddr4_s_axi_awsize),
         .m_axi_awburst(c0_ddr4_s_axi_awburst),
         .m_axi_awcache(c0_ddr4_s_axi_awcache),
         .m_axi_awlen(c0_ddr4_s_axi_awlen),
         .m_axi_awlock(c0_ddr4_s_axi_awlock),
         .m_axi_awqos(c0_ddr4_s_axi_awqos),
         .m_axi_awregion(), // not supported by MIG
         .m_axi_awid(c0_ddr4_s_axi_awid),
         /**************** Write Data Channel Signals ****************/
         .m_axi_wdata(c0_ddr4_s_axi_wdata),
         .m_axi_wstrb(c0_ddr4_s_axi_wstrb),
         .m_axi_wvalid(c0_ddr4_s_axi_wvalid),
         .m_axi_wready(c0_ddr4_s_axi_wready),
         .m_axi_wlast(c0_ddr4_s_axi_wlast),
         /**************** Write Response Channel Signals ****************/
         .m_axi_bresp(c0_ddr4_s_axi_bresp),
         .m_axi_bvalid(c0_ddr4_s_axi_bvalid),
         .m_axi_bready(c0_ddr4_s_axi_bready),
         .m_axi_bid(c0_ddr4_s_axi_bid),
         /**************** Read Address Channel Signals ****************/
         .m_axi_araddr(c0_ddr4_s_axi_araddr),
         .m_axi_arprot(c0_ddr4_s_axi_arprot),
         .m_axi_arvalid(c0_ddr4_s_axi_arvalid),
         .m_axi_arready(c0_ddr4_s_axi_arready),
         .m_axi_arsize(c0_ddr4_s_axi_arsize),
         .m_axi_arburst(c0_ddr4_s_axi_arburst),
         .m_axi_arcache(c0_ddr4_s_axi_arcache),
         .m_axi_arlock(c0_ddr4_s_axi_arlock),
         .m_axi_arlen(c0_ddr4_s_axi_arlen),
         .m_axi_arqos(c0_ddr4_s_axi_arqos),
         .m_axi_arregion(),
         .m_axi_arid(c0_ddr4_s_axi_arid),
         /**************** Read Data Channel Signals ****************/
         .m_axi_rdata(c0_ddr4_s_axi_rdata),
         .m_axi_rresp(c0_ddr4_s_axi_rresp),
         .m_axi_rvalid(c0_ddr4_s_axi_rvalid),
         .m_axi_rready(c0_ddr4_s_axi_rready),
         .m_axi_rlast(c0_ddr4_s_axi_rlast),
         .m_axi_rid(c0_ddr4_s_axi_rid),
         /**************** System Signals ****************/
         .m_axi_aclk(mig_ui_clk),
         .m_axi_aresetn(mig_ui_rst_n)
      );



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
   generate
      if (UART0_SOURCE == "onboard") begin
         assign uart_rx[0] = usb_uart_tx;
         assign usb_uart_rx = uart_tx[0];
         assign usb_uart_cts = uart_rts_n[0];
         assign uart_cts_n[0] = usb_uart_rts;
      end else if (UART0_SOURCE == "pmod") begin
         assign uart_rx[0] = pmod_uart_tx;
         assign pmod_uart_rx = uart_tx[0];
         assign pmod_uart_cts = uart_rts_n[0];
         assign uart_cts_n[0] = pmod_uart_rts;
      end
   endgenerate

   // system monitor and fan control
   wire system_monitor_user_temp_alarm;
   wire system_monitor_ot;

   system_management u_system_management (
      .dclk_in(sys_clk_50),
      .reset_in(sys_rst),
      .user_temp_alarm_out(system_monitor_user_temp_alarm),
      .ot_out(system_monitor_ot),
      .channel_out(),
      .eoc_out(),
      .alarm_out(),
      .eos_out(),
      .busy_out()
   );

   fancontrol u_fancontrol (
      .clk(sys_clk_50),
      .reset(sys_rst),
      .user_temp_alarm(system_monitor_user_temp_alarm),
      .ot(system_monitor_ot),
      .temp(),
      .sm_fan_tach(sm_fan_tach),
      .sm_fan_pwm(sm_fan_pwm)
   );

endmodule
