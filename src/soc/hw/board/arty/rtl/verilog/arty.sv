module arty
  #(parameter NUM_UART = 1)
  (
   // FPGA IO
   input                 clk,
   input [3:0]           sw,
   output [3:0]          led,

   inout [7:0]           ja,
   
   input                 ck_rst,
   
   output [13:0]         ddr3_addr,
   output [2:0]          ddr3_ba,
   output                ddr3_cas_n,
   output                ddr3_ck_n,
   output                ddr3_ck_p,
   output                ddr3_cke,
   output                ddr3_ras_n,
   output                ddr3_reset_n,
   output                ddr3_we_n,
   inout [15:0]          ddr3_dq,
   inout [1:0]           ddr3_dqs_n,
   inout [1:0]           ddr3_dqs_p,
   output                ddr3_cs_n,
   output [1:0]          ddr3_dm,
   output                ddr3_odt,

   output                uart_rxd_out,
   input                 uart_txd_in,
   
   // System Interface
   output                sys_clk,
   output                sys_rst,

   input [3:0]           ddr_awid,
   input [27:0]          ddr_awaddr,
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
   input [27:0]          ddr_araddr,
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

   output                jtag_capture_dr,
   output                jtag_select,
   output                jtag_shift_dr,
   output                jtag_update_dr,
   output                jtag_tck,
   output                jtag_tms,
   output                jtag_tdi,
   input                 jtag_tdo,

   output [NUM_UART-1:0] uart_rx,
   input [NUM_UART-1:0]  uart_tx
   );

   logic         rst;
   assign rst = !ck_rst;
   
   logic        clk_ddr_ref; // 200 MHz clock
   logic        clk_ddr_sys; // 166.667 MHz clock
   logic        clk_ddr_locked;
   logic        ddr_calib_done;
   logic        ddr_mmcm_locked;
   logic        mig_ui_clk; // clock from MIG UI (83.333 MHz)
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
   assign sys_rst = mig_ui_rst;   

   assign uart_rx[0] = uart_txd_in;
   assign uart_rxd_out = uart_tx[0];
   
   BSCANE2
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
      );
   
   mig_7series
     u_mig_7series
       (.*,
        .init_calib_complete            (ddr_calib_done),
        .sys_clk_i                      (clk_ddr_sys),
        .clk_ref_i                      (clk_ddr_ref),
        .sys_rst                        (clk_ddr_locked | rst),
         
        // Application interface ports
        .ui_clk                         (mig_ui_clk),
        .ui_clk_sync_rst                (mig_ui_rst),
        .mmcm_locked                    (ddr_mmcm_locked),
        .aresetn                        (0),
        .app_sr_req                     (0),
        .app_ref_req                    (0),
        .app_zq_req                     (0),
        .app_sr_active                  (),
        .app_ref_ack                    (),
        .app_zq_ack                     (),
        // Slave Interface Write Address Ports
        .s_axi_awid                     (ddr_awid),
        .s_axi_awaddr                   (ddr_awaddr),
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
        .s_axi_araddr                   (ddr_araddr),
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

endmodule // arty
