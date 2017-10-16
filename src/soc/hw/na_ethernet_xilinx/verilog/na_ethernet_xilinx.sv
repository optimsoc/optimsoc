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
 * Toplevel: network adapter na_etherent_xilinx for Ethernet
 *
 * Author(s):
 *   Annika Fuchs <annika.fuchs@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

import dii_package::dii_flit;
import optimsoc::*;

module na_etherent_xilinx
   (
   // clocks and resets
   input                 sys_rst,
   input                 sys_clk,
   input                 clk_125mhz,

   // interrupt
   output                 eth_irq,
      
   // WB bus (TX/RX) - to AXI Stream FIFO 
   // (Bennenung aus Slave Sicht)
   input [31:0]          wb_data_adr_i,
   input                 wb_data_cyc_i,
   input [31:0]          wb_data_dat_i,
   input [3:0]           wb_data_sel_i,
   input                 wb_data_stb_i,
   input                 wb_data_we_i ,
   input [2:0]           wb_data_cti_i,
   input [1:0]           wb_data_bte_i,
   output                wb_data_ack_o,
   output                wb_data_rty_o,
   output                wb_data_err_o,
   output [31:0]         wb_data_dat_o,
      
   // WB bus (Control - AXI4_Lite System) - to AXI4 Ethernet Subsystem 
   // (Bennenung aus Slave Sicht)
   input [31:0]          wb_ctrl_adr_i,
   input                 wb_ctrl_cyc_i,
   input [31:0]          wb_ctrl_dat_i,
   input [3:0]           wb_ctrl_sel_i,
   input                 wb_ctrl_stb_i,
   input                 wb_ctrl_we_i ,
   input [2:0]           wb_ctrl_cti_i,
   input [1:0]           wb_ctrl_bte_i,
   output                wb_ctrl_ack_o,
   output                wb_ctrl_rty_o,
   output                wb_ctrl_err_o,
   output [31:0]         wb_ctrl_dat_o,   
   // MII Output
   output [3:0]          phy_mii_txd,
   output                phy_mii_tx_en,
   output                phy_mii_tx_er,
   input                 phy_mii_tx_clk,
   input                 phy_mii_rx_clk,
   input  [3:0]          phy_mii_rxd,
   input                 phy_mii_rx_dv,
   input                 phy_mii_rx_er,   
      
   // MDIO Interface - Ethernet
   output                phy_mdc,        
   inout                 eth_mdio,   
   output                phy_rst_n
   );

   localparam AXI_ID_WIDTH = 4;
  
   // tri-state assignment for mdio
   wire eth_mdio_t;
   wire eth_mdio_i;
   wire eth_mdio_o;
   
   IOBUF 
      mdio_io_iobuf ( 
         .I (eth_mdio_o   ), 
         .IO(eth_mdio      ), 
         .O (eth_mdio_i   ), 
         .T (eth_mdio_t   ));      
                 
   // wires AXI4Stream FIFO to Ethernet Subsystem
   wire [31:0] s_axis_txc_tdata;
   wire [3:0]  s_axis_txc_tkeep; // not in use
   wire        s_axis_txc_tlast;
   wire        s_axis_txc_tready;
   wire        s_axis_txc_tvalid;
      
   wire [31:0] s_axis_txd_tdata;
   wire [3:0]  s_axis_txd_tkeep; // not in use
   wire        s_axis_txd_tlast;
   wire        s_axis_txd_tready;
   wire        s_axis_txd_tvalid;
        
   wire [31:0] m_axis_rxd_tdata;
   wire [3:0]  m_axis_rxd_tkeep;
   wire        m_axis_rxd_tlast;
   wire        m_axis_rxd_tready;
   wire        m_axis_rxd_tvalid;         
        

   // wire AXI4 Ethernet Subsystem to WB2AXI Converter
   wire [31:0] s_axi_awaddr;
   wire        s_axi_awvalid;
   wire        s_axi_awready;
   wire [31:0] s_axi_wdata;
   wire [3:0]  s_axi_wstrb;
   wire        s_axi_wvalid;
   wire        s_axi_wready;
   wire [1:0]  s_axi_bresp;
   wire        s_axi_bvalid;
   wire        s_axi_bready;
   wire [31:0] s_axi_araddr;
   wire        s_axi_arvalid;
   wire        s_axi_arready;
   wire [31:0] s_axi_rdata;
   wire [1:0]  s_axi_rresp;
   wire        s_axi_rvalid;
   wire        s_axi_rready;
                  
   // wire AXI4 Stream FIFO to WB2AXI Converter
   wire [31:0] axi_data_awaddr;
   wire        axi_data_awvalid;
   wire        axi_data_awready;
   wire [31:0] axi_data_wdata;
   wire [3:0]  axi_data_wstrb;
   wire        axi_data_wvalid;
   wire        axi_data_wready;
   wire [1:0]  axi_data_bresp;
   wire        axi_data_bvalid;
   wire        axi_data_bready;
   wire [31:0] axi_data_araddr;
   wire        axi_data_arvalid;
   wire        axi_data_arready;
   wire [31:0] axi_data_rdata;
   wire [1:0]  axi_data_rresp;
   wire        axi_data_rvalid;
   wire        axi_data_rready;       

   assign s_axis_txd_tkeep = 4'b1111;
   assign s_axis_txc_tkeep = 4'b1111;
     
   
   // wire AXI4 Ethernet Subsystem to WB2AXI Converter - for assignements
   wire [31:0] axi_ctrl_awaddr;
   wire        axi_ctrl_awvalid;
   wire        axi_ctrl_awready;
   wire [31:0] axi_ctrl_wdata;
   wire [3:0]  axi_ctrl_wstrb;
   wire        axi_ctrl_wvalid;
   wire        axi_ctrl_wready;
   wire [1:0]  axi_ctrl_bresp;
   wire        axi_ctrl_bvalid;
   wire        axi_ctrl_bready;
   wire [31:0] axi_ctrl_araddr;
   wire        axi_ctrl_arvalid;
   wire        axi_ctrl_arready;
   wire [31:0] axi_ctrl_rdata;
   wire [1:0]  axi_ctrl_rresp;
   wire        axi_ctrl_rvalid;
   wire        axi_ctrl_rready;   
   
   // wires AXI4Stream FIFO to Ethernet Subsystem
   wire [31:0] axis_data_txc_tdata;
   wire [3:0]  axis_data_txc_tkeep; // not in use
   wire        axis_data_txc_tlast;
   wire        axis_data_txc_tready;
   wire        axis_data_txc_tvalid;
      
   wire [31:0] axis_data_txd_tdata;
   wire [3:0]  axis_data_txd_tkeep; // not in use
   wire        axis_data_txd_tlast;
   wire        axis_data_txd_tready;
   wire        axis_data_txd_tvalid;
        
   wire [31:0] axis_data_rxd_tdata;
   wire [3:0]  axis_data_rxd_tkeep;
   wire        axis_data_rxd_tlast;
   wire        axis_data_rxd_tready;
   wire        axis_data_rxd_tvalid;         
   
   axi_ethernet_0
      u_axi_ethernet
      (// AXI Lite control interface
      .s_axi_lite_clk      (sys_clk      ) , // input to ess
      .s_axi_lite_resetn   (~sys_rst   ), // input to ess
      .s_axi_araddr        (s_axi_araddr      ), // input to ess
      .s_axi_arready       (s_axi_arready     ), // output to ess
      .s_axi_arvalid       (s_axi_arvalid     ), // input to ess
      .s_axi_awaddr        (s_axi_awaddr      ), // input to ess
      .s_axi_awready       (s_axi_awready     ), // output to ess
      .s_axi_awvalid       (s_axi_awvalid     ), // input to ess
      .s_axi_bready        (s_axi_bready      ), // input to ess
      .s_axi_bresp         (s_axi_bresp       ), // output to ess
      .s_axi_bvalid        (s_axi_bvalid      ), // output to ess
      .s_axi_rdata         (s_axi_rdata       ), // output to ess
      .s_axi_rready        (s_axi_rready      ), // input to ess
      .s_axi_rresp         (s_axi_rresp       ), // output to ess
      .s_axi_rvalid        (s_axi_rvalid      ), // output to ess
      .s_axi_wdata         (s_axi_wdata       ), // input to ess
      .s_axi_wready        (s_axi_wready      ), // output to ess
      .s_axi_wvalid        (s_axi_wvalid      ), // input to ess
      .s_axi_wstrb         (s_axi_wstrb       ), // input to ess

      // AXI Stream TX/RX interface
      .axis_clk            (sys_clk          ), // input to ess

      .axi_txc_arstn       (~sys_rst         ), // input to ess
      .s_axis_txc_tdata    (s_axis_txc_tdata  ), // input to ess
      .s_axis_txc_tkeep    (s_axis_txc_tkeep  ), // input to ess
      .s_axis_txc_tlast    (s_axis_txc_tlast  ), // input to ess
      .s_axis_txc_tready   (s_axis_txc_tready ), // output to ess
      .s_axis_txc_tvalid   (s_axis_txc_tvalid ), // input to ess

      .axi_txd_arstn       (~sys_rst         ), // input to ess
      .s_axis_txd_tdata    (s_axis_txd_tdata  ), // input to ess
      .s_axis_txd_tkeep    (s_axis_txd_tkeep  ), // input to ess
      .s_axis_txd_tlast    (s_axis_txd_tlast  ), // input to ess
      .s_axis_txd_tready   (s_axis_txd_tready ), // output to ess
      .s_axis_txd_tvalid   (s_axis_txd_tvalid ), // input to ess

      .axi_rxd_arstn       (~sys_rst         ), // input to ess
      .m_axis_rxd_tdata    (m_axis_rxd_tdata  ), // output to ess
      .m_axis_rxd_tkeep    (m_axis_rxd_tkeep  ), // output to ess
      .m_axis_rxd_tlast    (m_axis_rxd_tlast  ), // output to ess
      .m_axis_rxd_tready   (m_axis_rxd_tready ), // input to ess
      .m_axis_rxd_tvalid   (m_axis_rxd_tvalid ), // output to ess
        
      .axi_rxs_arstn       (~sys_rst         ), // not in use only for DMA
      .m_axis_rxs_tdata    (  ), // not in use only for DMA
      .m_axis_rxs_tkeep    (  ), // not in use only for DMA
      .m_axis_rxs_tlast    (  ), // not in use only for DMA
      .m_axis_rxs_tready   (1'b1 ), // not in use only for DMA
      .m_axis_rxs_tvalid   ( ), // not in use only for DMA

      // MII Interface
      .mii_txd             (phy_mii_txd           ), // output to ess
      .mii_tx_en           (phy_mii_tx_en         ), // output to ess
      .mii_tx_er           (phy_mii_tx_er         ), // output to ess
      .mii_tx_clk          (phy_mii_tx_clk        ), // input to ess
      .mii_rx_clk          (phy_mii_rx_clk        ), // input to ess
      .mii_rxd             (phy_mii_rxd           ), // input to ess
      .mii_rx_dv           (phy_mii_rx_dv         ), // input to ess
      .mii_rx_er           (phy_mii_rx_er         ), // input to ess

         // AXI Ethernet Output
      .mdio_mdc            (phy_mdc          ), // output to ess
      .mdio_mdio_i         (eth_mdio_i       ), 
      .mdio_mdio_o         (eth_mdio_o       ),
      .mdio_mdio_t         (eth_mdio_t       ),
      .phy_rst_n           (phy_rst_n         ), // output to ess

      .gtx_clk             (clk_125mhz    ), // input to ess
      .mac_irq             (), // output to ess
      .interrupt           () // output to ess
      );

      // AXI Ethernet Subsystem assignements
      assign s_axi_araddr = axi_ctrl_araddr;
      assign axi_ctrl_arready = s_axi_arready;
      assign s_axi_arvalid = axi_ctrl_arvalid;
      assign s_axi_awaddr = axi_ctrl_awaddr;
      assign axi_ctrl_awready = s_axi_awready ;
      assign s_axi_awvalid = axi_ctrl_awvalid;
      assign s_axi_bready = axi_ctrl_bready;
      assign axi_ctrl_bresp = s_axi_bresp;
      assign axi_ctrl_bvalid = s_axi_bvalid;
      assign axi_ctrl_rdata = s_axi_rdata;
      assign s_axi_rready = axi_ctrl_rready;
      assign axi_ctrl_rresp = s_axi_rresp;
      assign axi_ctrl_rvalid = s_axi_rvalid;
      assign s_axi_wdata = axi_ctrl_wdata;
      assign axi_ctrl_wready = s_axi_wready ;
      assign s_axi_wvalid = axi_ctrl_wvalid;
      assign s_axi_wstrb = axi_ctrl_wstrb;      
        
      // AXI Ethernet Data Stream Assignements
      assign s_axis_txc_tdata = axis_data_txc_tdata;
      assign s_axis_txc_tkeep = axis_data_txc_tkeep;
      assign s_axis_txc_tlast = axis_data_txc_tlast;
      assign axis_data_txc_tready = s_axis_txc_tready;
      assign s_axis_txc_tvalid = axis_data_txc_tvalid;
   
      assign s_axis_txd_tdata = axis_data_txd_tdata;  
      assign s_axis_txd_tkeep = axis_data_txd_tkeep;
      assign s_axis_txd_tlast = axis_data_txd_tlast;
      assign axis_data_txd_tready = s_axis_txd_tready;  
      assign s_axis_txd_tvalid = axis_data_txd_tvalid;
   
      assign axis_data_rxd_tdata = m_axis_rxd_tdata;
      assign axis_data_rxd_tkeep = m_axis_rxd_tkeep;  
      assign axis_data_rxd_tlast = m_axis_rxd_tlast;
      assign m_axis_rxd_tready = axis_data_rxd_tready;
      assign axis_data_rxd_tvalid = m_axis_rxd_tvalid;         
      
      
   // AXI4Stream FIFO
   axi_fifo_mm_s_0
      u_axi_fifo_mm_s_0
      (
      .s_axi_aclk            (sys_clk    ), // input to fifo 1
      .s_axi_aresetn         (~sys_rst), // input to fifo 1
            
      .s_axi_awaddr          (axi_data_awaddr), // input to fifo
      .s_axi_awvalid         (axi_data_awvalid ), // input to fifo
      .s_axi_awready         (axi_data_awready ), // output to fifo
      .s_axi_wdata           (axi_data_wdata   ), // input to fifo
      .s_axi_wstrb           (axi_data_wstrb   ), // input to fifo
      .s_axi_wvalid          (axi_data_wvalid  ), // input to fifo
      .s_axi_wready          (axi_data_wready  ), // output to fifo
      .s_axi_bresp           (axi_data_bresp   ), // output to fifo
      .s_axi_bvalid          (axi_data_bvalid  ), // output to fifo
      .s_axi_bready          (axi_data_bready  ), // input to fifo
      .s_axi_araddr          (axi_data_araddr), // input to fifo
      .s_axi_arvalid         (axi_data_arvalid ), // input to fifo
      .s_axi_arready         (axi_data_arready ), // output to fifo
      .s_axi_rdata           (axi_data_rdata   ), // output to fifo
      .s_axi_rresp           (axi_data_rresp   ), // output to fifo
      .s_axi_rvalid          (axi_data_rvalid  ), // output to fifo
      .s_axi_rready          (axi_data_rready  ), // input to fifo
      .mm2s_prmry_reset_out_n(),
      .axi_str_txd_tvalid    (axis_data_txd_tvalid  ), // input to fifo
      .axi_str_txd_tready    (axis_data_txd_tready  ), // input to fifo
      .axi_str_txd_tlast     (axis_data_txd_tlast   ), // output to fifo
      .axi_str_txd_tdata     (axis_data_txd_tdata   ), // output to fifo
      .mm2s_cntrl_reset_out_n(),
      .axi_str_txc_tvalid    (axis_data_txc_tvalid  ), // output to fifo
      .axi_str_txc_tready    (axis_data_txc_tready  ), // input to fifo
      .axi_str_txc_tlast     (axis_data_txc_tlast   ), // output to fifo
      .axi_str_txc_tdata     (axis_data_txc_tdata   ), // output to fifo
      .s2mm_prmry_reset_out_n(),
      .axi_str_rxd_tvalid    (axis_data_rxd_tvalid  ), // input to fifo
      .axi_str_rxd_tready    (axis_data_rxd_tready  ), // output to fifo
      .axi_str_rxd_tlast     (axis_data_rxd_tlast   ), // input to fifo
      .axi_str_rxd_tdata     (axis_data_rxd_tdata   ), // input to fifo
            
      .interrupt             (eth_irq) // output to fifo
      );
            
   // Memory interface: convert WishBone signals from system to AXI for AXI4FIFO
   wb2axi
      #( .ADDR_WIDTH (32), 
         .DATA_WIDTH (32),
         .AXI_ID_WIDTH (AXI_ID_WIDTH))
      u_wb2axi_fifo
      ( 
      .clk             (sys_clk),
      .rst             (sys_rst),
      .wb_cyc_i        (wb_data_cyc_i),
      .wb_stb_i        (wb_data_stb_i),
      .wb_we_i         (wb_data_we_i ),
      .wb_adr_i        (wb_data_adr_i),
      .wb_dat_i        (wb_data_dat_i),
      .wb_sel_i        (wb_data_sel_i),
      .wb_cti_i        (wb_data_cti_i),
      .wb_bte_i        (wb_data_bte_i),
      .wb_ack_o        (wb_data_ack_o),
      .wb_err_o        (wb_data_err_o),
      .wb_rty_o        (wb_data_rty_o),
      .wb_dat_o        (wb_data_dat_o),
      .m_axi_awid      (),
      .m_axi_awaddr    (axi_data_awaddr),
      .m_axi_awlen     (),
      .m_axi_awsize    (),
      .m_axi_awburst   (),
      .m_axi_awcache   (),
      .m_axi_awprot    (),
      .m_axi_awqos     (),
      .m_axi_awvalid   (axi_data_awvalid),
      .m_axi_awready   (axi_data_awready),
      .m_axi_wdata     (axi_data_wdata),
      .m_axi_wstrb     (axi_data_wstrb),
      .m_axi_wlast     (),
      .m_axi_wvalid    (axi_data_wvalid),
      .m_axi_wready    (axi_data_wready),
      .m_axi_bid       (),
      .m_axi_bresp     (axi_data_bresp),
      .m_axi_bvalid    (axi_data_bvalid),
      .m_axi_bready    (axi_data_bready),
      .m_axi_arid      (),
      .m_axi_araddr    (axi_data_araddr),
      .m_axi_arlen     (),
      .m_axi_arsize    (),
      .m_axi_arburst   (),
      .m_axi_arcache   (),
      .m_axi_arprot    (),
      .m_axi_arqos     (),
      .m_axi_arvalid   (axi_data_arvalid),
      .m_axi_arready   (axi_data_arready),
      .m_axi_rid       (),
      .m_axi_rdata     (axi_data_rdata),
      .m_axi_rresp     (axi_data_rresp),
      .m_axi_rlast     (),
      .m_axi_rvalid    (axi_data_rvalid),
      .m_axi_rready    (axi_data_rready)
      );
            
   // Memory interface: convert WishBone signals from system to AXI for AXI Ethernet Subsystem
   wb2axi
      #(.ADDR_WIDTH (32), 
         .DATA_WIDTH (32),
         .AXI_ID_WIDTH (AXI_ID_WIDTH))
      u_wb2axi_ess
      (
      .clk             (sys_clk),
      .rst             (sys_rst),
      .wb_cyc_i        (wb_ctrl_cyc_i),  // input
      .wb_stb_i        (wb_ctrl_stb_i), // input
      .wb_we_i         (wb_ctrl_we_i ), // input
      .wb_adr_i        (wb_ctrl_adr_i), // input
      .wb_dat_i        (wb_ctrl_dat_i), // input
      .wb_sel_i        (wb_ctrl_sel_i), // input
      .wb_cti_i        (wb_ctrl_cti_i), // input
      .wb_bte_i        (wb_ctrl_bte_i), // input
      .wb_ack_o        (wb_ctrl_ack_o), // output
      .wb_err_o        (wb_ctrl_err_o),// output
      .wb_rty_o        (wb_ctrl_rty_o),// output
      .wb_dat_o        (wb_ctrl_dat_o),// output
      .m_axi_awid      (),
      .m_axi_awaddr    (axi_ctrl_awaddr), // output
      .m_axi_awlen     (),
      .m_axi_awsize    (),
      .m_axi_awburst   (),
      .m_axi_awcache   (),
      .m_axi_awprot    (),
      .m_axi_awqos     (),
      .m_axi_awvalid   (axi_ctrl_awvalid), // output
      .m_axi_awready   (axi_ctrl_awready), // input
      .m_axi_wdata     (axi_ctrl_wdata), // output
      .m_axi_wstrb     (axi_ctrl_wstrb), // output
      .m_axi_wlast     (),
      .m_axi_wvalid    (axi_ctrl_wvalid), // output
      .m_axi_wready    (axi_ctrl_wready), // input
      .m_axi_bid       (),
      .m_axi_bresp     (axi_ctrl_bresp),  // input
      .m_axi_bvalid    (axi_ctrl_bvalid), // input
      .m_axi_bready    (axi_ctrl_bready), // output
      .m_axi_arid      (),
      .m_axi_araddr    (axi_ctrl_araddr), // output
      .m_axi_arlen     (),
      .m_axi_arsize    (),
      .m_axi_arburst   (),
      .m_axi_arcache   (),
      .m_axi_arprot    (),
      .m_axi_arqos     (),
      .m_axi_arvalid   (axi_ctrl_arvalid), // output
      .m_axi_arready   (axi_ctrl_arready), // input
      .m_axi_rid       (),
      .m_axi_rdata     (axi_ctrl_rdata), // input
      .m_axi_rresp     (axi_ctrl_rresp), // input
      .m_axi_rlast     (),
      .m_axi_rvalid    (axi_ctrl_rvalid), // input
      .m_axi_rready    (axi_ctrl_rready) // output
      );            
endmodule // na_etherent_xilinx
