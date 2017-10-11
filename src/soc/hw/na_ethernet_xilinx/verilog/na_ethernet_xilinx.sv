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

module na_etherent_xilinx
      (
      // clocks and resets
      input                 sys_rst,
      input                 sys_clk,
      input                 clk_125mhz,
      
      // interrupt
      input                 eth_irq,
      
      // WB bus (TX/RX) - to AXI Stream FIFO 
      // (Bennenung aus Slave Sicht)
      input [31:0]          wb_fifo_adr_i,
      input                 wb_fifo_cyc_i,
      input [31:0]          wb_fifo_dat_i,
      input [3:0]           wb_fifo_sel_i,
      input                 wb_fifo_stb_i,
      input                 wb_fifo_we_i ,
      input [2:0]           wb_fifo_cti_i,
      input [1:0]           wb_fifo_bte_i,
      output                wb_fifo_ack_o,
      output                wb_fifo_rty_o,
      output                wb_fifo_err_o,
      output [31:0]         wb_fifo_dat_o,
      
      // WB bus (Control - AXI4_Lite System) - to AXI4 Ethernet Subsystem 
      // (Bennenung aus Slave Sicht)
      input [31:0]          wb_ess_adr_i,
      input                 wb_ess_cyc_i,
      input [31:0]          wb_ess_dat_i,
      input [3:0]           wb_ess_sel_i,
      input                 wb_ess_stb_i,
      input                 wb_ess_we_i ,
      input [2:0]           wb_ess_cti_i,
      input [1:0]           wb_ess_bte_i,
      output                wb_ess_ack_o,
      output                wb_ess_rty_o,
      output                wb_ess_err_o,
      output [31:0]         wb_ess_dat_o,   
  
      // MII Output
      output [3:0]          mii_txd,
      output                mii_tx_en,
      output                mii_tx_er,
      input                 mii_tx_clk,
      input                 mii_rx_clk,
      input  [3:0]          mii_rxd,
      input                 mii_rx_dv,
      input                 mii_rx_er,   
      
      // MDIO Interface - Ethernet
      output                eth_mdc,        
      inout                 eth_mdio,
      
      output                phy_rst_n
      );

   localparam AXI_ID_WIDTH = 4;
  
   // tri-state assignment for mdio
   wire eth_mdio_t;
   wire eth_mdio_i;
   wire eth_mdio_o;
   //assign eth_mdio = eth_mdio_t ? eth_mdio_i : eth_mdio_o;
     
   IOBUF 
      mdio_io_iobuf ( 
         .I (eth_mdio_o   ), 
         .IO(eth_mdio      ), 
         .O (eth_mdio_i   ), 
         .T (eth_mdio_t   ));      
                 
   // wires AXI4Stream FIFO to Ethernet Subsystem
   wire [31:0] s_axis_txc_tdata;
   wire [3:0]  s_axis_txc_tkeep; // not in use
   wire s_axis_txc_tlast;
   wire s_axis_txc_tready;
   wire s_axis_txc_tvalid;
      
   wire [31:0] s_axis_txd_tdata;
   wire [3:0]  s_axis_txd_tkeep; // not in use
   wire s_axis_txd_tlast;
   wire s_axis_txd_tready;
   wire s_axis_txd_tvalid;
        
   wire [31:0] m_axis_rxd_tdata;
   wire [3:0]  m_axis_rxd_tkeep;
   wire m_axis_rxd_tlast;
   wire m_axis_rxd_tready;
   wire m_axis_rxd_tvalid;         
        

   // wire AXI4 Ethernet Subsystem to WB2AXI Converter
   wire [31:0] s_axi_awaddr;
   wire s_axi_awvalid;
   wire s_axi_awready;
   wire [31:0]s_axi_wdata;
   wire [3:0]s_axi_wstrb;
   wire s_axi_wvalid;
   wire s_axi_wready;
   wire [1:0]  s_axi_bresp;
   wire s_axi_bvalid;
   wire s_axi_bready;
   wire [31:0] s_axi_araddr;
   wire s_axi_arvalid;
   wire s_axi_arready;
   wire [31:0] s_axi_rdata;
   wire [1:0] s_axi_rresp;
   wire s_axi_rvalid;
   wire s_axi_rready;
                  
   // wire AXI4 Stream FIFO to WB2AXI Converter
   wire [31:0] fifo_s_axi_awaddr;
   wire fifo_s_axi_awvalid;
   wire fifo_s_axi_awready;
   wire [31:0] fifo_s_axi_wdata;
   wire [3:0] fifo_s_axi_wstrb;
   wire fifo_s_axi_wvalid;
   wire fifo_s_axi_wready;
   wire [1:0] fifo_s_axi_bresp;
   wire fifo_s_axi_bvalid;
   wire fifo_s_axi_bready;
   wire [31:0] fifo_s_axi_araddr;
   wire fifo_s_axi_arvalid;
   wire fifo_s_axi_arready;
   wire [31:0] fifo_s_axi_rdata;
   wire [1:0] fifo_s_axi_rresp;
   wire fifo_s_axi_rvalid;
   wire fifo_s_axi_rready;       

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

         // AXI Ethernet Output
         .mdio_mdc            (eth_mdc          ),
         .mdio_mdio_i         (eth_mdio_i       ),
         .mdio_mdio_o         (eth_mdio_o       ),
         .mdio_mdio_t         (eth_mdio_t       ),
         .phy_rst_n           (phy_rst_n         ), 

         .gtx_clk             (clk_125mhz    )

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
      #(.ADDR_WIDTH (32), 
         .DATA_WIDTH (32),
         .AXI_ID_WIDTH (AXI_ID_WIDTH))
      u_wb2axi_fifo
      (.clk             (sys_clk),
         .rst             (sys_rst),
         .wb_cyc_i        (wb_fifo_cyc_i),
         .wb_stb_i        (wb_fifo_stb_i),
         .wb_we_i         (wb_fifo_we_i ),
         .wb_adr_i        (wb_fifo_adr_i),
         .wb_dat_i        (wb_fifo_dat_i),
         .wb_sel_i        (wb_fifo_sel_i),
         .wb_cti_i        (wb_fifo_cti_i),
         .wb_bte_i        (wb_fifo_bte_i),
         .wb_ack_o        (wb_fifo_ack_o),
         .wb_err_o        (wb_fifo_err_o),
         .wb_rty_o        (wb_fifo_rty_o),
         .wb_dat_o        (wb_fifo_dat_o),
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
      #(.ADDR_WIDTH (32), 
         .DATA_WIDTH (32),
         .AXI_ID_WIDTH (AXI_ID_WIDTH))
      u_wb2axi_ess
      (.clk             (sys_clk),
         .rst             (sys_rst),
         .wb_cyc_i        (wb_ess_cyc_i),
         .wb_stb_i        (wb_ess_stb_i),
         .wb_we_i         (wb_ess_we_i ),
         .wb_adr_i        (wb_ess_adr_i),
         .wb_dat_i        (wb_ess_dat_i),
         .wb_sel_i        (wb_ess_sel_i),
         .wb_cti_i        (wb_ess_cti_i),
         .wb_bte_i        (wb_ess_bte_i),
         .wb_ack_o        (wb_ess_ack_o),
         .wb_err_o        (wb_ess_err_o),
         .wb_rty_o        (wb_ess_rty_o),
         .wb_dat_o        (wb_ess_dat_o),
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
