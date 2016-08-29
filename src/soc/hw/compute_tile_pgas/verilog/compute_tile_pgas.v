/* Copyright (c) 2013 by the author(s)
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
 * A compute tile for partitioned global address space (PGAS) systems
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"
`include "dbg_config.vh"

module compute_tile_pgas(
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   trace_itm,
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   trace_stm,
`endif
`ifdef OPTIMSOC_CLOCKDOMAINS
   clk_noc,
 `ifdef OPTIMSOC_CDC_DYNAMIC
   cdc_conf, cdc_enable,
 `endif
`endif
   /*AUTOARG*/
   // Outputs
   noc_in_ready, noc_out_flit, noc_out_valid,
   // Inputs
   clk, rst_cpu, rst_sys, noc_in_flit, noc_in_valid, noc_out_ready,
   cpu_stall
   );

   input clk;

`ifdef OPTIMSOC_CLOCKDOMAINS
   input clk_noc;
 `ifdef OPTIMSOC_CDC_DYNAMIC
   output [2:0] cdc_conf;
   output cdc_enable;
 `endif
`endif

   input rst_cpu;
   input rst_sys;

   parameter NOC_FLIT_DATA_WIDTH = 32;
   parameter NOC_FLIT_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_FLIT_DATA_WIDTH+NOC_FLIT_TYPE_WIDTH;
   parameter VCHANNELS = `VCHANNELS;

   localparam PGAS_ENABLE = 1;
   parameter PGAS_ADDRW = 15;
   parameter PGAS_BASE = 0;

   parameter DMA_ENTRIES = 4;

   input [NOC_FLIT_WIDTH-1:0]      noc_in_flit;
   input [VCHANNELS-1:0]           noc_in_valid;
   output [VCHANNELS-1:0]          noc_in_ready;
   output reg [NOC_FLIT_WIDTH-1:0] noc_out_flit;
   output [VCHANNELS-1:0]          noc_out_valid;
   input [VCHANNELS-1:0]           noc_out_ready;

   input                       cpu_stall;

`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   output [`DEBUG_ITM_PORTWIDTH-1:0] trace_itm;
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   output [`DEBUG_STM_PORTWIDTH-1:0] trace_stm;
`endif

   parameter VC_LSU_REQ  = `VCHANNEL_LSU_REQ;
   parameter VC_LSU_RESP = `VCHANNEL_LSU_RESP;
   parameter VC_MPSIMPLE = `VCHANNEL_MPSIMPLE;

   parameter ID       = 0;
   parameter MEMORYID = `OPTIMSOC_MEMORYID;
   parameter CORES    = 1;

   wire [31:0]   ic_adr_o;
   wire          ic_cyc_o;
   wire [31:0]   ic_dat_o;
   wire [3:0]    ic_sel_o;
   wire          ic_stb_o;
   wire          ic_we_o;
   wire [2:0]    ic_cti_o;
   wire [1:0]    ic_bte_o;
   wire          ic_ack_i;
   wire          ic_rty_i;
   wire          ic_err_i;
   wire [31:0]   ic_dat_i;

   wire [31:0]   dc_adr_o;
   wire          dc_cyc_o;
   wire [31:0]   dc_dat_o;
   wire [3:0]    dc_sel_o;
   wire          dc_stb_o;
   wire          dc_we_o;
   wire [2:0]    dc_cti_o;
   wire [1:0]    dc_bte_o;
   wire          dc_ack_i;
   wire          dc_rty_i;
   wire          dc_err_i;
   wire [31:0]   dc_dat_i;

   wire [31:0]   dc_lsu_adr_o;
   wire          dc_lsu_cyc_o;
   wire [31:0]   dc_lsu_dat_o;
   wire [3:0]    dc_lsu_sel_o;
   wire          dc_lsu_stb_o;
   wire          dc_lsu_we_o;
   wire [2:0]    dc_lsu_cti_o;
   wire [1:0]    dc_lsu_bte_o;
   wire          dc_lsu_ack_i;
   wire          dc_lsu_rty_i;
   wire          dc_lsu_err_i;
   wire [31:0]   dc_lsu_dat_i;

   wire [31:0]   dc_na_adr_o;
   wire          dc_na_cyc_o;
   wire [31:0]   dc_na_dat_o;
   wire [3:0]    dc_na_sel_o;
   wire          dc_na_stb_o;
   wire          dc_na_we_o;
   wire [2:0]    dc_na_cti_o;
   wire [1:0]    dc_na_bte_o;
   wire          dc_na_ack_i;
   wire          dc_na_rty_i;
   wire          dc_na_err_i;
   wire [31:0]   dc_na_dat_i;

   wire [19:0]   pic_ints_i;
   assign pic_ints_i[19:4] = 16'h0;
   assign pic_ints_i[1:0] = 2'b00;

   /* or1200_module AUTO_TEMPLATE(
    .clk_i          (clk),
    .rst_i          (rst_cpu),
    .bus_clk_i          (clk),
    .bus_rst_i          (rst_cpu),
    .dbg_.*_o       (),
    .dbg_stall_i    (cpu_stall),
    .dbg_ewt_i      (1'b0),
    .dbg_stb_i      (1'b0),
    .dbg_we_i       (1'b0),
    .dbg_adr_i      (32'h00000000),
    .dbg_dat_i      (32'h00000000),
    .iwb_\(.*\)     (ic_\1[]),
    .dwb_\(.*\)     (dc_\1[]),
    ); */
   or1200_module
      #(.ID(ID))
   u_core0 (
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
            .trace_itm                  (trace_itm[`DEBUG_ITM_PORTWIDTH-1:0]),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
            .trace_stm                  (trace_stm[`DEBUG_STM_PORTWIDTH-1:0]),
`endif
            /*AUTOINST*/
            // Outputs
            .dbg_lss_o                  (),                      // Templated
            .dbg_is_o                   (),                      // Templated
            .dbg_wp_o                   (),                      // Templated
            .dbg_bp_o                   (),                      // Templated
            .dbg_dat_o                  (),                      // Templated
            .dbg_ack_o                  (),                      // Templated
            .iwb_cyc_o                  (ic_cyc_o),              // Templated
            .iwb_adr_o                  (ic_adr_o[31:0]),        // Templated
            .iwb_stb_o                  (ic_stb_o),              // Templated
            .iwb_we_o                   (ic_we_o),               // Templated
            .iwb_sel_o                  (ic_sel_o[3:0]),         // Templated
            .iwb_dat_o                  (ic_dat_o[31:0]),        // Templated
            .iwb_bte_o                  (ic_bte_o[1:0]),         // Templated
            .iwb_cti_o                  (ic_cti_o[2:0]),         // Templated
            .dwb_cyc_o                  (dc_cyc_o),              // Templated
            .dwb_adr_o                  (dc_adr_o[31:0]),        // Templated
            .dwb_stb_o                  (dc_stb_o),              // Templated
            .dwb_we_o                   (dc_we_o),               // Templated
            .dwb_sel_o                  (dc_sel_o[3:0]),         // Templated
            .dwb_dat_o                  (dc_dat_o[31:0]),        // Templated
            .dwb_bte_o                  (dc_bte_o[1:0]),         // Templated
            .dwb_cti_o                  (dc_cti_o[2:0]),         // Templated
            // Inputs
            .clk_i                      (clk),                   // Templated
            .bus_clk_i                  (clk),                   // Templated
            .rst_i                      (rst_cpu),               // Templated
            .bus_rst_i                  (rst_cpu),               // Templated
            .dbg_stall_i                (cpu_stall),             // Templated
            .dbg_ewt_i                  (1'b0),                  // Templated
            .dbg_stb_i                  (1'b0),                  // Templated
            .dbg_we_i                   (1'b0),                  // Templated
            .dbg_adr_i                  (32'h00000000),          // Templated
            .dbg_dat_i                  (32'h00000000),          // Templated
            .pic_ints_i                 (pic_ints_i[19:0]),
            .iwb_ack_i                  (ic_ack_i),              // Templated
            .iwb_err_i                  (ic_err_i),              // Templated
            .iwb_rty_i                  (ic_rty_i),              // Templated
            .iwb_dat_i                  (ic_dat_i[31:0]),        // Templated
            .dwb_ack_i                  (dc_ack_i),              // Templated
            .dwb_err_i                  (dc_err_i),              // Templated
            .dwb_rty_i                  (dc_rty_i),              // Templated
            .dwb_dat_i                  (dc_dat_i[31:0]));       // Templated

   /* ct_select AUTO_TEMPLATE(
    .m_\(.*\)_o (dc_\1_i[]),
    .m_\(.*\)_i (dc_\1_o[]),
    .s_0_\(.*\) (dc_lsu_\1[]),
    .s_1_\(.*\) (dc_na_\1[]),
    ); */
   ct_select
      u_select(/*AUTOINST*/
               // Outputs
               .m_ack_o                 (dc_ack_i),              // Templated
               .m_err_o                 (dc_err_i),              // Templated
               .m_rty_o                 (dc_rty_i),              // Templated
               .m_dat_o                 (dc_dat_i[31:0]),        // Templated
               .s_0_dat_o               (dc_lsu_dat_o[31:0]),    // Templated
               .s_0_adr_o               (dc_lsu_adr_o[31:0]),    // Templated
               .s_0_sel_o               (dc_lsu_sel_o[3:0]),     // Templated
               .s_0_we_o                (dc_lsu_we_o),           // Templated
               .s_0_cyc_o               (dc_lsu_cyc_o),          // Templated
               .s_0_stb_o               (dc_lsu_stb_o),          // Templated
               .s_0_cti_o               (dc_lsu_cti_o[2:0]),     // Templated
               .s_0_bte_o               (dc_lsu_bte_o[1:0]),     // Templated
               .s_1_dat_o               (dc_na_dat_o[31:0]),     // Templated
               .s_1_adr_o               (dc_na_adr_o[31:0]),     // Templated
               .s_1_sel_o               (dc_na_sel_o[3:0]),      // Templated
               .s_1_we_o                (dc_na_we_o),            // Templated
               .s_1_cyc_o               (dc_na_cyc_o),           // Templated
               .s_1_stb_o               (dc_na_stb_o),           // Templated
               .s_1_cti_o               (dc_na_cti_o[2:0]),      // Templated
               .s_1_bte_o               (dc_na_bte_o[1:0]),      // Templated
               // Inputs
               .m_dat_i                 (dc_dat_o[31:0]),        // Templated
               .m_adr_i                 (dc_adr_o[31:0]),        // Templated
               .m_sel_i                 (dc_sel_o[3:0]),         // Templated
               .m_we_i                  (dc_we_o),               // Templated
               .m_cyc_i                 (dc_cyc_o),              // Templated
               .m_stb_i                 (dc_stb_o),              // Templated
               .m_cti_i                 (dc_cti_o[2:0]),         // Templated
               .m_bte_i                 (dc_bte_o[1:0]),         // Templated
               .s_0_ack_i               (dc_lsu_ack_i),          // Templated
               .s_0_err_i               (dc_lsu_err_i),          // Templated
               .s_0_rty_i               (dc_lsu_rty_i),          // Templated
               .s_0_dat_i               (dc_lsu_dat_i[31:0]),    // Templated
               .s_1_ack_i               (dc_na_ack_i),           // Templated
               .s_1_err_i               (dc_na_err_i),           // Templated
               .s_1_rty_i               (dc_na_rty_i),           // Templated
               .s_1_dat_i               (dc_na_dat_i[31:0]));    // Templated

   genvar i;
   generate
      for (i=0;i<VCHANNELS;i=i+1) begin
         if ((i != VC_LSU_REQ) &&
             (i != VC_LSU_RESP) &&
             (i != VC_MPSIMPLE)) begin
            assign noc_out_valid[i] = 1'b0;
            assign noc_in_ready[i] = 1'b0;
         end
      end
   endgenerate

   wire [NOC_FLIT_WIDTH-1:0] noc_out_lsu_flit_cdc;
   wire                      noc_out_lsu_valid_cdc;
   reg                       noc_out_lsu_ready_cdc;
   wire [NOC_FLIT_WIDTH-1:0] noc_out_na_flit_cdc;
   wire                      noc_out_na_valid_cdc;
   reg                       noc_out_na_ready_cdc;


   wire [NOC_FLIT_WIDTH-1:0] noc_out_lsu_flit;
   wire                      noc_out_lsu_valid;
   wire                      noc_out_lsu_ready;
   wire [NOC_FLIT_WIDTH-1:0] noc_out_na_flit;
   wire                      noc_out_na_valid;
   wire                      noc_out_na_ready;

`ifdef OPTIMSOC_CLOCKDOMAINS
   wire                      lsu_fifo_empty;
   assign noc_out_lsu_valid_cdc = ~lsu_fifo_empty;
   wire                      lsu_fifo_full;
   assign noc_out_lsu_ready = ~lsu_fifo_full;

   cdc_fifo
      #( // Parameters
       .DW                              (NOC_FLIT_WIDTH),
       .ADDRSIZE                        (2))
      u_fifo_out_lsu(// Outputs
                     .wr_full            (lsu_fifo_full),
                     .rd_empty           (lsu_fifo_empty),
                     .rd_data            (noc_out_lsu_flit_cdc),
                     // Inputs
                     .wr_clk             (clk),
                     .rd_clk             (clk_noc),
                     .wr_rst             (~rst_sys),
                     .rd_rst             (~rst_sys),
                     .rd_en              (noc_out_lsu_ready_cdc),
                     .wr_en              (noc_out_lsu_valid),
                     .wr_data            (noc_out_lsu_flit));

   wire                      na_fifo_empty;
   assign noc_out_na_valid_cdc = ~na_fifo_empty;
   wire                      na_fifo_full;
   assign noc_out_na_ready = ~na_fifo_full;

   cdc_fifo
      #(.DW                              (NOC_FLIT_WIDTH),
        .ADDRSIZE                        (2))
      u_fifo_out_na(// Outputs
                    .wr_full            (na_fifo_full),
                    .rd_empty           (na_fifo_empty),
                    .rd_data            (noc_out_na_flit_cdc),
                    // Inputs
                    .wr_clk             (clk),
                    .rd_clk             (clk_noc),
                    .wr_rst             (~rst_sys),
                    .rd_rst             (~rst_sys),
                    .rd_en              (noc_out_na_ready_cdc),
                    .wr_en              (noc_out_na_valid),
                    .wr_data            (noc_out_na_flit));

`else
   assign noc_out_lsu_valid_cdc = noc_out_lsu_valid;
   assign noc_out_lsu_flit_cdc  = noc_out_lsu_flit;
   assign noc_out_lsu_ready     = noc_out_lsu_ready_cdc;
   assign noc_out_na_valid_cdc = noc_out_na_valid;
   assign noc_out_na_flit_cdc  = noc_out_na_flit;
   assign noc_out_na_ready     = noc_out_na_ready_cdc;
`endif // `ifdef OPTIMSOC_CLOCKDOMAINS

   reg noc_out_na_active;
   reg noc_out_lsu_active;

   assign noc_out_valid[VC_MPSIMPLE] = noc_out_na_active;
   assign noc_out_valid[VC_LSU_REQ] = noc_out_lsu_active;
   assign noc_out_valid[VC_LSU_RESP] = 1'b0;
   assign noc_in_ready[VC_LSU_REQ] = 1'b0;

   always @(*) begin
      if (noc_out_lsu_valid_cdc) begin
         // If there is a request
         noc_out_lsu_ready_cdc = noc_out_ready[VC_LSU_REQ];
         noc_out_na_ready_cdc = 1'b0;
         noc_out_lsu_active = 1'b1;
         noc_out_na_active = 1'b0;
         noc_out_flit = noc_out_lsu_flit_cdc;
      end else if (noc_out_na_valid_cdc) begin
         noc_out_lsu_ready_cdc = 1'b0;
         noc_out_na_ready_cdc  = noc_out_ready[VC_MPSIMPLE];
         noc_out_lsu_active = 1'b0;
         noc_out_na_active = 2'b1;
         noc_out_flit = noc_out_na_flit_cdc;
      end else begin
         noc_out_lsu_ready_cdc = 1'b0;
         noc_out_na_ready_cdc = 1'b0;
         noc_out_lsu_active = 1'b0;
         noc_out_na_active = 1'b0;
         noc_out_flit = {NOC_FLIT_WIDTH{1'bx}};
      end
   end

   wire [NOC_FLIT_WIDTH-1:0]      noc_in_lsu_flit_cdc;
   wire                           noc_in_lsu_valid_cdc;
   wire                           noc_in_lsu_ready_cdc;
   wire [NOC_FLIT_WIDTH-1:0]      noc_in_na_flit_cdc;
   wire                           noc_in_na_valid_cdc;
   wire                           noc_in_na_ready_cdc;

`ifdef OPTIMSOC_CLOCKDOMAINS
   wire                           in_lsu_fifo_empty;
   assign noc_in_lsu_valid_cdc = ~in_lsu_fifo_empty;
   wire                           in_lsu_fifo_full;
   assign noc_in_ready[VC_LSU_RESP] = ~in_lsu_fifo_full;

   cdc_fifo
      #(.DW                              (NOC_FLIT_WIDTH),
        .ADDRSIZE                        (2))
      u_fifo_in_lsu(// Outputs
                    .wr_full            (in_lsu_fifo_full),
                    .rd_empty           (in_lsu_fifo_empty),
                    .rd_data            (noc_in_lsu_flit_cdc),
                    // Inputs
                    .wr_clk             (clk_noc),
                    .rd_clk             (clk),
                    .wr_rst             (~rst_sys),
                    .rd_rst             (~rst_sys),
                    .rd_en              (noc_in_lsu_ready_cdc),
                    .wr_en              (noc_in_valid[VC_LSU_RESP]),
                    .wr_data            (noc_in_flit));

   wire                           in_na_fifo_empty;
   assign noc_in_na_valid_cdc = ~in_na_fifo_empty;
   wire                           in_na_fifo_full;
   assign noc_in_ready[VC_MPSIMPLE] = ~in_na_fifo_full;

   cdc_fifo
      #(.DW                             (NOC_FLIT_WIDTH),
        .ADDRSIZE                       (2))
      u_fifo_in_na(// Outputs
                   .wr_full            (in_na_fifo_full),
                   .rd_empty           (in_na_fifo_empty),
                   .rd_data            (noc_in_na_flit_cdc),
                   // Inputs
                   .wr_clk             (clk_noc),
                   .rd_clk             (clk),
                   .wr_rst             (~rst_sys),
                   .rd_rst             (~rst_sys),
                   .rd_en              (noc_in_na_ready_cdc),
                   .wr_en              (noc_in_valid[VC_MPSIMPLE]),
                   .wr_data            (noc_in_flit));
`else
   assign noc_in_lsu_flit_cdc       = noc_in_flit;
   assign noc_in_na_flit_cdc        = noc_in_flit;
   assign noc_in_lsu_valid_cdc      = noc_in_valid[VC_LSU_RESP];
   assign noc_in_na_valid_cdc       = noc_in_valid[VC_MPSIMPLE];
   assign noc_in_ready[VC_LSU_RESP] = noc_in_lsu_ready_cdc;
   assign noc_in_ready[VC_MPSIMPLE] = noc_in_na_ready_cdc;
`endif

   assign ic_rty_i = 1'b0;
   assign dc_lsu_rty_i = 1'b0;

   /* noclsu_tile AUTO_TEMPLATE(
    .rst (rst_sys),
    .dc_\(.*\)_o (dc_lsu_\1_i),
    .dc_\(.*\)_i (dc_lsu_\1_o),
    .ic_\(.*\)_o (ic_\1_i),
    .ic_\(.*\)_i (ic_\1_o),
    .noc_out_valid (noc_out_lsu_valid),
    .noc_out_ready (noc_out_lsu_ready),
    .noc_out_flit  (noc_out_lsu_flit[]),
    .noc_in_valid (noc_in_lsu_valid_cdc),
    .noc_in_ready (noc_in_lsu_ready_cdc),
    .noc_in_flit  (noc_in_lsu_flit_cdc),
    ); */
   noclsu_tile
      #(.TILEID(ID),
        .MEMORYID(MEMORYID),
        .PGAS_ENABLE(PGAS_ENABLE),
        .PGAS_ADDRW(PGAS_ADDRW),
        .PGAS_BASE(PGAS_BASE))
      u_lsu(/*AUTOINST*/
            // Outputs
            .noc_out_flit               (noc_out_lsu_flit[33:0]), // Templated
            .noc_out_valid              (noc_out_lsu_valid),     // Templated
            .noc_in_ready               (noc_in_lsu_ready_cdc),  // Templated
            .dc_dat_o                   (dc_lsu_dat_i),          // Templated
            .dc_ack_o                   (dc_lsu_ack_i),          // Templated
            .dc_err_o                   (dc_lsu_err_i),          // Templated
            .dc_rty_o                   (dc_lsu_rty_i),          // Templated
            .ic_dat_o                   (ic_dat_i),              // Templated
            .ic_ack_o                   (ic_ack_i),              // Templated
            .ic_err_o                   (ic_err_i),              // Templated
            .ic_rty_o                   (ic_rty_i),              // Templated
            // Inputs
            .clk                        (clk),
            .rst                        (rst_sys),               // Templated
            .noc_out_ready              (noc_out_lsu_ready),     // Templated
            .noc_in_flit                (noc_in_lsu_flit_cdc),   // Templated
            .noc_in_valid               (noc_in_lsu_valid_cdc),  // Templated
            .dc_dat_i                   (dc_lsu_dat_o),          // Templated
            .dc_adr_i                   (dc_lsu_adr_o),          // Templated
            .dc_cyc_i                   (dc_lsu_cyc_o),          // Templated
            .dc_stb_i                   (dc_lsu_stb_o),          // Templated
            .dc_we_i                    (dc_lsu_we_o),           // Templated
            .dc_sel_i                   (dc_lsu_sel_o),          // Templated
            .dc_bte_i                   (dc_lsu_bte_o),          // Templated
            .dc_cti_i                   (dc_lsu_cti_o),          // Templated
            .ic_dat_i                   (ic_dat_o),              // Templated
            .ic_adr_i                   (ic_adr_o),              // Templated
            .ic_cyc_i                   (ic_cyc_o),              // Templated
            .ic_stb_i                   (ic_stb_o),              // Templated
            .ic_we_i                    (ic_we_o),               // Templated
            .ic_sel_i                   (ic_sel_o),              // Templated
            .ic_bte_i                   (ic_bte_o),              // Templated
            .ic_cti_i                   (ic_cti_o));             // Templated

   wire [DMA_ENTRIES:0] na_irq;

   /*
    *  +---+-..-+----+
    *  |   dma  | mp |
    *  +---+-..-+----+
    * dma_entries 1  (0)
    *
    * map to irq lines of cpu
    *
    *  +----+-----+
    *  | mp | dma |
    *  +----+-----+
    *    3     2
    */
   assign pic_ints_i[3:2] = {na_irq[0],|na_irq[DMA_ENTRIES:1]};

   /* networkadapter_ct AUTO_TEMPLATE(
    .noc_in_ready (noc_in_na_ready_cdc),
    .noc_in_valid (noc_in_na_valid_cdc),
    .noc_in_flit (noc_in_na_flit_cdc),
    .noc_out_ready (noc_out_na_ready),
    .noc_out_valid (noc_out_na_valid),
    .noc_out_flit (noc_out_na_flit),
    .wbm_\(.*\)_o (),
    .wbm_dat_i (32'b0),
    .wbm_.*_i (1'b0),
    .wbm_\(.*\)_i ('0),
    .wbs_\(.*\)_o (dc_na_\1_i),
    .wbs_\(.*\)_i (dc_na_\1_o),
    .wbs_cab_i (1'bx),
    .irq (na_irq),
    .rst (rst_sys),
    ); */
   networkadapter_ct
      #(.tileid(ID),
        .vchannels(1), /* REVIEW: Why is this hardcoded to 1 and not VCHANNELS? */
        .vc_mp_simple(0),
        .noc_xdim(2), /* REVIEW: hardcoded? */
        .noc_ydim(2),
        .conf_mp_simple(1),
        .conf_dma(0))
      u_na(
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
           .cdc_conf                     (cdc_conf[2:0]),
           .cdc_enable                   (cdc_enable),
 `endif
`endif
           /*AUTOINST*/
           // Outputs
           .noc_in_ready                (noc_in_na_ready_cdc),   // Templated
           .noc_out_flit                (noc_out_na_flit),       // Templated
           .noc_out_valid               (noc_out_na_valid),      // Templated
           .wbm_adr_o                   (),                      // Templated
           .wbm_cyc_o                   (),                      // Templated
           .wbm_dat_o                   (),                      // Templated
           .wbm_sel_o                   (),                      // Templated
           .wbm_stb_o                   (),                      // Templated
           .wbm_we_o                    (),                      // Templated
           .wbm_cab_o                   (),                      // Templated
           .wbm_cti_o                   (),                      // Templated
           .wbm_bte_o                   (),                      // Templated
           .wbs_ack_o                   (dc_na_ack_i),           // Templated
           .wbs_rty_o                   (dc_na_rty_i),           // Templated
           .wbs_err_o                   (dc_na_err_i),           // Templated
           .wbs_dat_o                   (dc_na_dat_i),           // Templated
           .irq                         (na_irq),                // Templated
           // Inputs
           .clk                         (clk),
           .rst                         (rst_sys),               // Templated
           .noc_in_flit                 (noc_in_na_flit_cdc),    // Templated
           .noc_in_valid                (noc_in_na_valid_cdc),   // Templated
           .noc_out_ready               (noc_out_na_ready),      // Templated
           .wbm_ack_i                   (1'b0),                  // Templated
           .wbm_rty_i                   (1'b0),                  // Templated
           .wbm_err_i                   (1'b0),                  // Templated
           .wbm_dat_i                   (32'b0),                 // Templated
           .wbs_adr_i                   (dc_na_adr_o),           // Templated
           .wbs_cyc_i                   (dc_na_cyc_o),           // Templated
           .wbs_dat_i                   (dc_na_dat_o),           // Templated
           .wbs_sel_i                   (dc_na_sel_o),           // Templated
           .wbs_stb_i                   (dc_na_stb_o),           // Templated
           .wbs_we_i                    (dc_na_we_o),            // Templated
           .wbs_cab_i                   (1'bx),                  // Templated
           .wbs_cti_i                   (dc_na_cti_o),           // Templated
           .wbs_bte_i                   (dc_na_bte_o));          // Templated


endmodule

// Local Variables:
// verilog-library-directories:("../../*/verilog/")
// verilog-auto-inst-param-value: t
// End:
