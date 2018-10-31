/* Copyright (c) 2013-2017 by the author(s)
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
 * This is the compute tile for distributed memory systems.
 *
 *
 * The assignment of the 32 bit IRQ vector connected to core 0 is as follows:
 *  - Bit [1:0]:  N/A
 *  - Bit 2:      UART
 *  - Bit 3:      NA - message passing
 *  - Bit 4:      NA - DMA
 *  - Bit [31:5]: N/A
 *
 * The address ranges of the bus slaves are as follows:
 *  - Slave 0 - DM:     0x00000000-0x7fffffff
 *  - Slave 1 - PGAS:   not used
 *  - Slave 2 - NA:     0xe0000000-0xefffffff
 *  - Slave 3 - BOOT:   0xf0000000-0xffffffff
 *  - Slave 4 - UART:   0x90000000-0x9000000f
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module compute_tile_dm
  import dii_package::dii_flit;
   import opensocdebug::mor1kx_trace_exec;
   import optimsoc_config::*;
  #(
    parameter config_t CONFIG = 'x,

    parameter ID       = 'x,
    parameter COREBASE = 'x,

    parameter DEBUG_BASEID = 'x,

    parameter MEM_FILE = 'x,

    localparam CHANNELS = CONFIG.NOC_CHANNELS,
    localparam FLIT_WIDTH = CONFIG.NOC_FLIT_WIDTH
    )
   (
   input                                 dii_flit [1:0] debug_ring_in,
   output [1:0]                          debug_ring_in_ready,
   output                                dii_flit [1:0] debug_ring_out,
   input [1:0]                           debug_ring_out_ready,

   output [31:0]                         wb_ext_adr_i,
   output                                wb_ext_cyc_i,
   output [31:0]                         wb_ext_dat_i,
   output [3:0]                          wb_ext_sel_i,
   output                                wb_ext_stb_i,
   output                                wb_ext_we_i,
   output                                wb_ext_cab_i,
   output [2:0]                          wb_ext_cti_i,
   output [1:0]                          wb_ext_bte_i,
   input                                 wb_ext_ack_o,
   input                                 wb_ext_rty_o,
   input                                 wb_ext_err_o,
   input [31:0]                          wb_ext_dat_o,

   input                                 clk,
   input                                 rst_cpu, rst_sys, rst_dbg,

   input [CHANNELS-1:0][FLIT_WIDTH-1:0]  noc_in_flit,
   input [CHANNELS-1:0]                  noc_in_last,
   input [CHANNELS-1:0]                  noc_in_valid,
   output [CHANNELS-1:0]                 noc_in_ready,
   output [CHANNELS-1:0][FLIT_WIDTH-1:0] noc_out_flit,
   output [CHANNELS-1:0]                 noc_out_last,
   output [CHANNELS-1:0]                 noc_out_valid,
   input [CHANNELS-1:0]                  noc_out_ready
   );

   import optimsoc_functions::*;

   localparam NR_MASTERS = CONFIG.CORES_PER_TILE * 2 + 1;
   localparam NR_SLAVES = 5;
   localparam SLAVE_DM   = 0;
   localparam SLAVE_PGAS = 1;
   localparam SLAVE_NA   = 2;
   localparam SLAVE_BOOT = 3;
   localparam SLAVE_UART = 4;

   mor1kx_trace_exec [CONFIG.CORES_PER_TILE-1:0] trace;

   logic        wb_mem_clk_i;
   logic        wb_mem_rst_i;
   logic [31:0] wb_mem_adr_i;
   logic        wb_mem_cyc_i;
   logic [31:0] wb_mem_dat_i;
   logic [3:0]  wb_mem_sel_i;
   logic        wb_mem_stb_i;
   logic        wb_mem_we_i;
   logic        wb_mem_cab_i;
   logic [2:0]  wb_mem_cti_i;
   logic [1:0]  wb_mem_bte_i;
   logic         wb_mem_ack_o;
   logic         wb_mem_rty_o;
   logic         wb_mem_err_o;
   logic [31:0]  wb_mem_dat_o;

   // create DI ring segment with routers
   localparam DEBUG_MODS_PER_TILE_NONZERO = (CONFIG.DEBUG_MODS_PER_TILE == 0) ? 1 : CONFIG.DEBUG_MODS_PER_TILE;

   dii_flit [DEBUG_MODS_PER_TILE_NONZERO-1:0] dii_in;
   logic [DEBUG_MODS_PER_TILE_NONZERO-1:0] dii_in_ready;
   dii_flit [DEBUG_MODS_PER_TILE_NONZERO-1:0] dii_out;
   logic [DEBUG_MODS_PER_TILE_NONZERO-1:0] dii_out_ready;

   generate
      if (CONFIG.USE_DEBUG == 1) begin : gen_debug_ring
         genvar i;
         logic [CONFIG.DEBUG_MODS_PER_TILE-1:0][15:0] id_map;
         for (i = 0; i < CONFIG.DEBUG_MODS_PER_TILE; i = i+1) begin
            assign id_map[i][15:0] = 16'(DEBUG_BASEID+i);
         end

         debug_ring_expand
           #(.BUFFER_SIZE(CONFIG.DEBUG_ROUTER_BUFFER_SIZE),
             .PORTS(CONFIG.DEBUG_MODS_PER_TILE))
         u_debug_ring_segment
           (.clk           (clk),
            .rst           (rst_dbg),
            .id_map        (id_map),
            .dii_in        (dii_in),
            .dii_in_ready  (dii_in_ready),
            .dii_out       (dii_out),
            .dii_out_ready (dii_out_ready),
            .ext_in        (debug_ring_in),
            .ext_in_ready  (debug_ring_in_ready),
            .ext_out       (debug_ring_out),
            .ext_out_ready (debug_ring_out_ready));
      end // if (USE_DEBUG)
   endgenerate

   wire [31:0]   busms_adr_o[0:NR_MASTERS-1];
   wire          busms_cyc_o[0:NR_MASTERS-1];
   wire [31:0]   busms_dat_o[0:NR_MASTERS-1];
   wire [3:0]    busms_sel_o[0:NR_MASTERS-1];
   wire          busms_stb_o[0:NR_MASTERS-1];
   wire          busms_we_o[0:NR_MASTERS-1];
   wire          busms_cab_o[0:NR_MASTERS-1];
   wire [2:0]    busms_cti_o[0:NR_MASTERS-1];
   wire [1:0]    busms_bte_o[0:NR_MASTERS-1];
   wire          busms_ack_i[0:NR_MASTERS-1];
   wire          busms_rty_i[0:NR_MASTERS-1];
   wire          busms_err_i[0:NR_MASTERS-1];
   wire [31:0]   busms_dat_i[0:NR_MASTERS-1];

   wire [31:0]   bussl_adr_i[0:NR_SLAVES-1];
   wire          bussl_cyc_i[0:NR_SLAVES-1];
   wire [31:0]   bussl_dat_i[0:NR_SLAVES-1];
   wire [3:0]    bussl_sel_i[0:NR_SLAVES-1];
   wire          bussl_stb_i[0:NR_SLAVES-1];
   wire          bussl_we_i[0:NR_SLAVES-1];
   wire          bussl_cab_i[0:NR_SLAVES-1];
   wire [2:0]    bussl_cti_i[0:NR_SLAVES-1];
   wire [1:0]    bussl_bte_i[0:NR_SLAVES-1];
   wire          bussl_ack_o[0:NR_SLAVES-1];
   wire          bussl_rty_o[0:NR_SLAVES-1];
   wire          bussl_err_o[0:NR_SLAVES-1];
   wire [31:0]   bussl_dat_o[0:NR_SLAVES-1];

   wire          snoop_enable;
   wire [31:0]   snoop_adr;

   wire [31:0]   pic_ints_i [0:CONFIG.CORES_PER_TILE-1];
   assign pic_ints_i[0][31:5] = 27'h0;
   assign pic_ints_i[0][1:0] = 2'b00;

   genvar        c, m, s;

   wire [32*NR_MASTERS-1:0] busms_adr_o_flat;
   wire [NR_MASTERS-1:0]    busms_cyc_o_flat;
   wire [32*NR_MASTERS-1:0] busms_dat_o_flat;
   wire [4*NR_MASTERS-1:0]  busms_sel_o_flat;
   wire [NR_MASTERS-1:0]    busms_stb_o_flat;
   wire [NR_MASTERS-1:0]    busms_we_o_flat;
   wire [NR_MASTERS-1:0]    busms_cab_o_flat;
   wire [3*NR_MASTERS-1:0]  busms_cti_o_flat;
   wire [2*NR_MASTERS-1:0]  busms_bte_o_flat;
   wire [NR_MASTERS-1:0]    busms_ack_i_flat;
   wire [NR_MASTERS-1:0]    busms_rty_i_flat;
   wire [NR_MASTERS-1:0]    busms_err_i_flat;
   wire [32*NR_MASTERS-1:0] busms_dat_i_flat;

   wire [32*NR_SLAVES-1:0] bussl_adr_i_flat;
   wire [NR_SLAVES-1:0]    bussl_cyc_i_flat;
   wire [32*NR_SLAVES-1:0] bussl_dat_i_flat;
   wire [4*NR_SLAVES-1:0]  bussl_sel_i_flat;
   wire [NR_SLAVES-1:0]    bussl_stb_i_flat;
   wire [NR_SLAVES-1:0]    bussl_we_i_flat;
   wire [NR_SLAVES-1:0]    bussl_cab_i_flat;
   wire [3*NR_SLAVES-1:0]  bussl_cti_i_flat;
   wire [2*NR_SLAVES-1:0]  bussl_bte_i_flat;
   wire [NR_SLAVES-1:0]    bussl_ack_o_flat;
   wire [NR_SLAVES-1:0]    bussl_rty_o_flat;
   wire [NR_SLAVES-1:0]    bussl_err_o_flat;
   wire [32*NR_SLAVES-1:0] bussl_dat_o_flat;

   generate
      for (m = 0; m < NR_MASTERS; m = m + 1) begin : gen_busms_flat
         assign busms_adr_o_flat[32*(m+1)-1:32*m] = busms_adr_o[m];
         assign busms_cyc_o_flat[m] = busms_cyc_o[m];
         assign busms_dat_o_flat[32*(m+1)-1:32*m] = busms_dat_o[m];
         assign busms_sel_o_flat[4*(m+1)-1:4*m] = busms_sel_o[m];
         assign busms_stb_o_flat[m] = busms_stb_o[m];
         assign busms_we_o_flat[m] = busms_we_o[m];
         assign busms_cab_o_flat[m] = busms_cab_o[m];
         assign busms_cti_o_flat[3*(m+1)-1:3*m] = busms_cti_o[m];
         assign busms_bte_o_flat[2*(m+1)-1:2*m] = busms_bte_o[m];
         assign busms_ack_i[m] = busms_ack_i_flat[m];
         assign busms_rty_i[m] = busms_rty_i_flat[m];
         assign busms_err_i[m] = busms_err_i_flat[m];
         assign busms_dat_i[m] = busms_dat_i_flat[32*(m+1)-1:32*m];
      end

      for (s = 0; s < NR_SLAVES; s = s + 1) begin : gen_bussl_flat
         assign bussl_adr_i[s] = bussl_adr_i_flat[32*(s+1)-1:32*s];
         assign bussl_cyc_i[s] = bussl_cyc_i_flat[s];
         assign bussl_dat_i[s] = bussl_dat_i_flat[32*(s+1)-1:32*s];
         assign bussl_sel_i[s] = bussl_sel_i_flat[4*(s+1)-1:4*s];
         assign bussl_stb_i[s] = bussl_stb_i_flat[s];
         assign bussl_we_i[s] = bussl_we_i_flat[s];
         assign bussl_cab_i[s] = bussl_cab_i_flat[s];
         assign bussl_cti_i[s] = bussl_cti_i_flat[3*(s+1)-1:3*s];
         assign bussl_bte_i[s] = bussl_bte_i_flat[2*(s+1)-1:2*s];
         assign bussl_ack_o_flat[s] = bussl_ack_o[s];
         assign bussl_rty_o_flat[s] = bussl_rty_o[s];
         assign bussl_err_o_flat[s] = bussl_err_o[s];
         assign bussl_dat_o_flat[32*(s+1)-1:32*s] = bussl_dat_o[s];
      end
   endgenerate

   generate
      for (c = 1; c < CONFIG.CORES_PER_TILE; c = c + 1) begin
         assign pic_ints_i[c] = 32'h0;
      end
   endgenerate


   localparam MOR1KX_FEATURE_FPU = (CONFIG.CORE_ENABLE_FPU ? "ENABLED" : "NONE");
   localparam MOR1KX_FEATURE_PERFCOUNTERS = (CONFIG.CORE_ENABLE_PERFCOUNTERS ? "ENABLED" : "NONE");
   localparam MOR1KX_FEATURE_DEBUGUNIT = "NONE"; // XXX: Enable debug unit with OSD CDM module (once it's ready)

   generate
      for (c = 0; c < CONFIG.CORES_PER_TILE; c = c + 1) begin : gen_cores
         /* mor1kx_module AUTO_TEMPLATE(
          .clk_i          (clk),
          .rst_i          (rst_cpu),
          .bus_clk_i      (clk),
          .bus_rst_i      (rst_cpu),
          .dbg_.*_o       (),
          .dbg_stall_i    (1'b0),
          .dbg_ewt_i      (1'b0),
          .dbg_stb_i      (1'b0),
          .dbg_we_i       (1'b0),
          .dbg_adr_i      (32'h00000000),
          .dbg_dat_i      (32'h00000000),
          .iwb_\(.*\)     (busms_\1[c*2][]),
          .dwb_\(.*\)     (busms_\1[c*2+1][]),
          .pic_ints_i     (pic_ints_i[c]),
          .snoop_enable_i (snoop_enable),
          .snoop_adr_i    (snoop_adr),
          .trace_exec     (trace[c]),
          ); */
         mor1kx_module
               #(.ID(c),
                 .NUMCORES(CONFIG.CORES_PER_TILE),
                 .FEATURE_FPU(MOR1KX_FEATURE_FPU),
                 .FEATURE_PERFCOUNTERS(MOR1KX_FEATURE_PERFCOUNTERS),
                 .FEATURE_DEBUGUNIT(MOR1KX_FEATURE_DEBUGUNIT))
         u_core (
                 /*AUTOINST*/
                 // Interfaces
                 .trace_exec            (trace[c]),              // Templated
                 // Outputs
                 .dbg_lss_o             (),                      // Templated
                 .dbg_is_o              (),                      // Templated
                 .dbg_wp_o              (),                      // Templated
                 .dbg_bp_o              (),                      // Templated
                 .dbg_dat_o             (),                      // Templated
                 .dbg_ack_o             (),                      // Templated
                 .iwb_cyc_o             (busms_cyc_o[c*2]),      // Templated
                 .iwb_adr_o             (busms_adr_o[c*2][31:0]), // Templated
                 .iwb_stb_o             (busms_stb_o[c*2]),      // Templated
                 .iwb_we_o              (busms_we_o[c*2]),       // Templated
                 .iwb_sel_o             (busms_sel_o[c*2][3:0]), // Templated
                 .iwb_dat_o             (busms_dat_o[c*2][31:0]), // Templated
                 .iwb_bte_o             (busms_bte_o[c*2][1:0]), // Templated
                 .iwb_cti_o             (busms_cti_o[c*2][2:0]), // Templated
                 .dwb_cyc_o             (busms_cyc_o[c*2+1]),    // Templated
                 .dwb_adr_o             (busms_adr_o[c*2+1][31:0]), // Templated
                 .dwb_stb_o             (busms_stb_o[c*2+1]),    // Templated
                 .dwb_we_o              (busms_we_o[c*2+1]),     // Templated
                 .dwb_sel_o             (busms_sel_o[c*2+1][3:0]), // Templated
                 .dwb_dat_o             (busms_dat_o[c*2+1][31:0]), // Templated
                 .dwb_bte_o             (busms_bte_o[c*2+1][1:0]), // Templated
                 .dwb_cti_o             (busms_cti_o[c*2+1][2:0]), // Templated
                 // Inputs
                 .clk_i                 (clk),                   // Templated
                 .bus_clk_i             (clk),                   // Templated
                 .rst_i                 (rst_cpu),               // Templated
                 .bus_rst_i             (rst_cpu),               // Templated
                 .dbg_stall_i           (1'b0),                  // Templated
                 .dbg_ewt_i             (1'b0),                  // Templated
                 .dbg_stb_i             (1'b0),                  // Templated
                 .dbg_we_i              (1'b0),                  // Templated
                 .dbg_adr_i             (32'h00000000),          // Templated
                 .dbg_dat_i             (32'h00000000),          // Templated
                 .pic_ints_i            (pic_ints_i[c]),         // Templated
                 .iwb_ack_i             (busms_ack_i[c*2]),      // Templated
                 .iwb_err_i             (busms_err_i[c*2]),      // Templated
                 .iwb_rty_i             (busms_rty_i[c*2]),      // Templated
                 .iwb_dat_i             (busms_dat_i[c*2][31:0]), // Templated
                 .dwb_ack_i             (busms_ack_i[c*2+1]),    // Templated
                 .dwb_err_i             (busms_err_i[c*2+1]),    // Templated
                 .dwb_rty_i             (busms_rty_i[c*2+1]),    // Templated
                 .dwb_dat_i             (busms_dat_i[c*2+1][31:0]), // Templated
                 .snoop_enable_i        (snoop_enable),          // Templated
                 .snoop_adr_i           (snoop_adr));            // Templated


         assign busms_cab_o[c*2] = 1'b0;
         assign busms_cab_o[c*2+1] = 1'b0;

         if (CONFIG.USE_DEBUG == 1) begin : gen_ctm_stm
            osd_stm_mor1kx
              #(.MAX_PKT_LEN(CONFIG.DEBUG_MAX_PKT_LEN))
              u_stm
                (.clk  (clk),
                 .rst  (rst_dbg),
                 .id   (16'(DEBUG_BASEID + 1 + c*CONFIG.DEBUG_MODS_PER_CORE)),
                 .debug_in (dii_out[1+c*CONFIG.DEBUG_MODS_PER_CORE]),
                 .debug_in_ready (dii_out_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE]),
                 .debug_out (dii_in[1+c*CONFIG.DEBUG_MODS_PER_CORE]),
                 .debug_out_ready (dii_in_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE]),
                 .trace_port (trace[c]));

            osd_ctm_mor1kx
              #(.MAX_PKT_LEN(CONFIG.DEBUG_MAX_PKT_LEN))
              u_ctm
                (.clk  (clk),
                 .rst  (rst_dbg),
                 .id   (16'(DEBUG_BASEID + 1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1)),
                 .debug_in (dii_out[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1]),
                 .debug_in_ready (dii_out_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1]),
                 .debug_out (dii_in[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1]),
                 .debug_out_ready (dii_in_ready[1 + c*CONFIG.DEBUG_MODS_PER_CORE + 1]),
                 .trace_port (trace[c]));
         end
      end
   endgenerate

   generate
      if (CONFIG.USE_DEBUG != 0 && CONFIG.DEBUG_DEM_UART != 0) begin : gen_dem_uart
         osd_dem_uart_wb
         u_dem_uart(
            .clk              (clk),
            .rst              (rst_sys),
            .id               (16'(DEBUG_BASEID + CONFIG.DEBUG_MODS_PER_TILE - 1)),
            .irq              (pic_ints_i[0][2]),
            .debug_in         (dii_out[CONFIG.DEBUG_MODS_PER_TILE - 1]),
            .debug_in_ready   (dii_out_ready[CONFIG.DEBUG_MODS_PER_TILE - 1]),
            .debug_out        (dii_in[CONFIG.DEBUG_MODS_PER_TILE - 1]),
            .debug_out_ready  (dii_in_ready[CONFIG.DEBUG_MODS_PER_TILE - 1]),
            .wb_adr_i         (bussl_adr_i[SLAVE_UART][3:0]),
            .wb_cyc_i         (bussl_cyc_i[SLAVE_UART]),
            .wb_dat_i         (bussl_dat_i[SLAVE_UART]),
            .wb_sel_i         (bussl_sel_i[SLAVE_UART]),
            .wb_stb_i         (bussl_stb_i[SLAVE_UART]),
            .wb_we_i          (bussl_we_i[SLAVE_UART]),
            .wb_cti_i         (bussl_cti_i[SLAVE_UART]),
            .wb_bte_i         (bussl_bte_i[SLAVE_UART]),
            .wb_ack_o         (bussl_ack_o[SLAVE_UART]),
            .wb_err_o         (bussl_err_o[SLAVE_UART]),
            .wb_rty_o         (bussl_rty_o[SLAVE_UART]),
            .wb_dat_o         (bussl_dat_o[SLAVE_UART]));
      end
   endgenerate

   /* wb_bus_b3 AUTO_TEMPLATE(
    .clk_i      (clk),
    .rst_i      (rst_sys),
    .m_\(.*\)_o (busms_\1_i_flat),
    .m_\(.*\)_i (busms_\1_o_flat),
    .s_\(.*\)_o (bussl_\1_i_flat),
    .s_\(.*\)_i (bussl_\1_o_flat),
    .snoop_en_o (snoop_enable),
    .snoop_adr_o (snoop_adr),
    .bus_hold (1'b0),
    .bus_hold_ack (),
    ); */
   wb_bus_b3
     #(.MASTERS(NR_MASTERS),.SLAVES(NR_SLAVES),
       .S0_ENABLE(CONFIG.ENABLE_DM),
       .S0_RANGE_WIDTH(CONFIG.DM_RANGE_WIDTH),.S0_RANGE_MATCH(CONFIG.DM_RANGE_MATCH),
       .S1_ENABLE(CONFIG.ENABLE_PGAS),
       .S1_RANGE_WIDTH(CONFIG.PGAS_RANGE_WIDTH),.S1_RANGE_MATCH(CONFIG.PGAS_RANGE_MATCH),
       .S2_RANGE_WIDTH(4),.S2_RANGE_MATCH(4'he),
       .S3_ENABLE(CONFIG.ENABLE_BOOTROM),
       .S3_RANGE_WIDTH(4),.S3_RANGE_MATCH(4'hf),
       .S4_ENABLE(CONFIG.DEBUG_DEM_UART),
       .S4_RANGE_WIDTH(28),.S4_RANGE_MATCH(28'h9000000))
   u_bus(/*AUTOINST*/
         // Outputs
         .m_dat_o                       (busms_dat_i_flat),      // Templated
         .m_ack_o                       (busms_ack_i_flat),      // Templated
         .m_err_o                       (busms_err_i_flat),      // Templated
         .m_rty_o                       (busms_rty_i_flat),      // Templated
         .s_adr_o                       (bussl_adr_i_flat),      // Templated
         .s_dat_o                       (bussl_dat_i_flat),      // Templated
         .s_cyc_o                       (bussl_cyc_i_flat),      // Templated
         .s_stb_o                       (bussl_stb_i_flat),      // Templated
         .s_sel_o                       (bussl_sel_i_flat),      // Templated
         .s_we_o                        (bussl_we_i_flat),       // Templated
         .s_cti_o                       (bussl_cti_i_flat),      // Templated
         .s_bte_o                       (bussl_bte_i_flat),      // Templated
         .snoop_adr_o                   (snoop_adr),             // Templated
         .snoop_en_o                    (snoop_enable),          // Templated
         .bus_hold_ack                  (),                      // Templated
         // Inputs
         .clk_i                         (clk),                   // Templated
         .rst_i                         (rst_sys),               // Templated
         .m_adr_i                       (busms_adr_o_flat),      // Templated
         .m_dat_i                       (busms_dat_o_flat),      // Templated
         .m_cyc_i                       (busms_cyc_o_flat),      // Templated
         .m_stb_i                       (busms_stb_o_flat),      // Templated
         .m_sel_i                       (busms_sel_o_flat),      // Templated
         .m_we_i                        (busms_we_o_flat),       // Templated
         .m_cti_i                       (busms_cti_o_flat),      // Templated
         .m_bte_i                       (busms_bte_o_flat),      // Templated
         .s_dat_i                       (bussl_dat_o_flat),      // Templated
         .s_ack_i                       (bussl_ack_o_flat),      // Templated
         .s_err_i                       (bussl_err_o_flat),      // Templated
         .s_rty_i                       (bussl_rty_o_flat),      // Templated
         .bus_hold                      (1'b0));                         // Templated

   // Unused leftover from an older Wishbone spec version
   assign bussl_cab_i_flat = NR_SLAVES'(1'b0);

   //MAM - WB adapter signals
   logic          mam_dm_stb_o;
   logic          mam_dm_cyc_o;
   logic          mam_dm_ack_i;
   logic          mam_dm_err_i;
   logic          mam_dm_rty_i;
   logic          mam_dm_we_o;
   logic [31:0]   mam_dm_addr_o;
   logic [31:0]   mam_dm_dat_o;
   logic [31:0]   mam_dm_dat_i;
   logic [2:0]    mam_dm_cti_o;
   logic [1:0]    mam_dm_bte_o;
   logic [3:0]    mam_dm_sel_o;

   if (CONFIG.USE_DEBUG == 1) begin : gen_mam_dm_wb
      //MAM
      osd_mam_wb #(
           .DATA_WIDTH(32),
           .MAX_PKT_LEN(CONFIG.DEBUG_MAX_PKT_LEN),
           .MEM_SIZE0(CONFIG.LMEM_SIZE),
           .BASE_ADDR0(0))
      u_mam_dm_wb(
           .clk_i(clk),
           .rst_i(rst_dbg),
           .debug_in(dii_out[0]),
           .debug_in_ready(dii_out_ready[0]),
           .debug_out(dii_in[0]),
           .debug_out_ready(dii_in_ready[0]),
           .id (16'(DEBUG_BASEID)),
           .stb_o(mam_dm_stb_o),
           .cyc_o(mam_dm_cyc_o),
           .ack_i(mam_dm_ack_i),
           .we_o(mam_dm_we_o),
           .addr_o(mam_dm_addr_o),
           .dat_o(mam_dm_dat_o),
           .dat_i(mam_dm_dat_i),
           .cti_o(mam_dm_cti_o),
           .bte_o(mam_dm_bte_o),
           .sel_o(mam_dm_sel_o));
   end //if (USE_DEBUG == 1)

   if (CONFIG.ENABLE_DM) begin : gen_mam_wb_adapter
      /* mam_wb_adapter AUTO_TEMPLATE(
       .wb_in_clk_i  (clk),
       .wb_in_rst_i  (rst_sys),
       .wb_in_\(.*\) (bussl_\1[SLAVE_DM]),
       .wb_out_\(.*\) (wb_mem_\1),
       ); */
      mam_wb_adapter
        #(.DW(32),
          .AW(32))
      u_mam_wb_adapter_dm
        (
         .wb_mam_adr_o    (mam_dm_addr_o),
         .wb_mam_cyc_o    (mam_dm_cyc_o),
         .wb_mam_dat_o    (mam_dm_dat_o),
         .wb_mam_sel_o    (mam_dm_sel_o),
         .wb_mam_stb_o    (mam_dm_stb_o),
         .wb_mam_we_o     (mam_dm_we_o),
         .wb_mam_cab_o    (1'b0),
         .wb_mam_cti_o    (mam_dm_cti_o),
         .wb_mam_bte_o    (mam_dm_bte_o),
         .wb_mam_ack_i    (mam_dm_ack_i),
         .wb_mam_rty_i    (mam_dm_rty_i),
         .wb_mam_err_i    (mam_dm_err_i),
         .wb_mam_dat_i    (mam_dm_dat_i),
         /*AUTOINST*/
         // Outputs
         .wb_in_ack_o                   (bussl_ack_o[SLAVE_DM]), // Templated
         .wb_in_err_o                   (bussl_err_o[SLAVE_DM]), // Templated
         .wb_in_rty_o                   (bussl_rty_o[SLAVE_DM]), // Templated
         .wb_in_dat_o                   (bussl_dat_o[SLAVE_DM]), // Templated
         .wb_out_adr_i                  (wb_mem_adr_i),          // Templated
         .wb_out_bte_i                  (wb_mem_bte_i),          // Templated
         .wb_out_cti_i                  (wb_mem_cti_i),          // Templated
         .wb_out_cyc_i                  (wb_mem_cyc_i),          // Templated
         .wb_out_dat_i                  (wb_mem_dat_i),          // Templated
         .wb_out_sel_i                  (wb_mem_sel_i),          // Templated
         .wb_out_stb_i                  (wb_mem_stb_i),          // Templated
         .wb_out_we_i                   (wb_mem_we_i),           // Templated
         .wb_out_clk_i                  (wb_mem_clk_i),          // Templated
         .wb_out_rst_i                  (wb_mem_rst_i),          // Templated
         // Inputs
         .wb_in_adr_i                   (bussl_adr_i[SLAVE_DM]), // Templated
         .wb_in_bte_i                   (bussl_bte_i[SLAVE_DM]), // Templated
         .wb_in_cti_i                   (bussl_cti_i[SLAVE_DM]), // Templated
         .wb_in_cyc_i                   (bussl_cyc_i[SLAVE_DM]), // Templated
         .wb_in_dat_i                   (bussl_dat_i[SLAVE_DM]), // Templated
         .wb_in_sel_i                   (bussl_sel_i[SLAVE_DM]), // Templated
         .wb_in_stb_i                   (bussl_stb_i[SLAVE_DM]), // Templated
         .wb_in_we_i                    (bussl_we_i[SLAVE_DM]),  // Templated
         .wb_in_clk_i                   (clk),                   // Templated
         .wb_in_rst_i                   (rst_sys),               // Templated
         .wb_out_ack_o                  (wb_mem_ack_o),          // Templated
         .wb_out_err_o                  (wb_mem_err_o),          // Templated
         .wb_out_rty_o                  (wb_mem_rty_o),          // Templated
         .wb_out_dat_o                  (wb_mem_dat_o));                 // Templated
   end else begin // if (CONFIG.ENABLE_DM)
      assign mam_dm_dat_i = 32'hx;
      assign {mam_dm_ack_i, mam_dm_err_i, mam_dm_rty_i} = 3'b000;
      assign bussl_dat_o[SLAVE_DM] = 32'hx;
      assign bussl_ack_o[SLAVE_DM] = 1'b0;
      assign bussl_err_o[SLAVE_DM] = 1'b0;
      assign bussl_rty_o[SLAVE_DM] = 1'b0;
   end

   if (!CONFIG.ENABLE_PGAS) begin : gen_tieoff_pgas
      assign bussl_dat_o[SLAVE_PGAS] = 32'h0;
      assign bussl_ack_o[SLAVE_PGAS] = 1'b0;
      assign bussl_err_o[SLAVE_PGAS] = 1'b0;
      assign bussl_rty_o[SLAVE_PGAS] = 1'b0;
   end

   generate
      if ((CONFIG.ENABLE_DM) &&
       (CONFIG.LMEM_STYLE == PLAIN)) begin : gen_sram
         /* wb_sram_sp AUTO_TEMPLATE(
          .wb_\(.*\) (wb_mem_\1),
          ); */
         wb_sram_sp
           #(.DW(32),
             .AW(clog2_width(CONFIG.LMEM_SIZE)),
             .MEM_SIZE_BYTE(CONFIG.LMEM_SIZE),
             .MEM_FILE(MEM_FILE),
             .MEM_IMPL_TYPE("PLAIN")
             )
         u_ram(/*AUTOINST*/
               // Outputs
               .wb_ack_o                (wb_mem_ack_o),
               .wb_err_o                (wb_mem_err_o),
               .wb_rty_o                (wb_mem_rty_o),
               .wb_dat_o                (wb_mem_dat_o),
               // Inputs
               .wb_adr_i                (wb_mem_adr_i[clog2_width(CONFIG.LMEM_SIZE)-1:0]),
               .wb_bte_i                (wb_mem_bte_i),
               .wb_cti_i                (wb_mem_cti_i),
               .wb_cyc_i                (wb_mem_cyc_i),
               .wb_dat_i                (wb_mem_dat_i),
               .wb_sel_i                (wb_mem_sel_i),
               .wb_stb_i                (wb_mem_stb_i),
               .wb_we_i                 (wb_mem_we_i),
               .wb_clk_i                (wb_mem_clk_i),
               .wb_rst_i                (wb_mem_rst_i));
      end else begin // block: gen_sram
         assign wb_ext_adr_i = wb_mem_adr_i;
         assign wb_ext_bte_i = wb_mem_bte_i;
         assign wb_ext_cti_i = wb_mem_cti_i;
         assign wb_ext_cyc_i = wb_mem_cyc_i;
         assign wb_ext_dat_i = wb_mem_dat_i;
         assign wb_ext_sel_i = wb_mem_sel_i;
         assign wb_ext_stb_i = wb_mem_stb_i;
         assign wb_ext_we_i = wb_mem_we_i;
         assign wb_mem_ack_o = wb_ext_ack_o;
         assign wb_mem_rty_o = wb_ext_rty_o;
         assign wb_mem_err_o = wb_ext_err_o;
         assign wb_mem_dat_o = wb_ext_dat_o;
      end // else: !if((CONFIG.ENABLE_DM) &&...
   endgenerate

   networkadapter_ct
      #(.CONFIG(CONFIG),
        .TILEID(ID),
        .COREBASE(COREBASE))
      u_na(
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
           .cdc_conf                     (cdc_conf[2:0]),
           .cdc_enable                   (cdc_enable),
 `endif
`endif
           // Outputs
           .noc_in_ready                (noc_in_ready),
           .noc_out_flit                (noc_out_flit),
           .noc_out_last                (noc_out_last),
           .noc_out_valid               (noc_out_valid),
           .wbm_adr_o                   (busms_adr_o[NR_MASTERS-1]),
           .wbm_cyc_o                   (busms_cyc_o[NR_MASTERS-1]),
           .wbm_dat_o                   (busms_dat_o[NR_MASTERS-1]),
           .wbm_sel_o                   (busms_sel_o[NR_MASTERS-1]),
           .wbm_stb_o                   (busms_stb_o[NR_MASTERS-1]),
           .wbm_we_o                    (busms_we_o[NR_MASTERS-1]),
           .wbm_cab_o                   (busms_cab_o[NR_MASTERS-1]),
           .wbm_cti_o                   (busms_cti_o[NR_MASTERS-1]),
           .wbm_bte_o                   (busms_bte_o[NR_MASTERS-1]),
           .wbs_ack_o                   (bussl_ack_o[SLAVE_NA]),
           .wbs_rty_o                   (bussl_rty_o[SLAVE_NA]),
           .wbs_err_o                   (bussl_err_o[SLAVE_NA]),
           .wbs_dat_o                   (bussl_dat_o[SLAVE_NA]),
           .irq                         (pic_ints_i[0][4:3]),
           // Inputs
           .clk                         (clk),
           .rst                         (rst_sys),
           .noc_in_flit                 (noc_in_flit),
           .noc_in_last                 (noc_in_last),
           .noc_in_valid                (noc_in_valid),
           .noc_out_ready               (noc_out_ready),
           .wbm_ack_i                   (busms_ack_i[NR_MASTERS-1]),
           .wbm_rty_i                   (busms_rty_i[NR_MASTERS-1]),
           .wbm_err_i                   (busms_err_i[NR_MASTERS-1]),
           .wbm_dat_i                   (busms_dat_i[NR_MASTERS-1]),
           .wbs_adr_i                   (bussl_adr_i[SLAVE_NA]),
           .wbs_cyc_i                   (bussl_cyc_i[SLAVE_NA]),
           .wbs_dat_i                   (bussl_dat_i[SLAVE_NA]),
           .wbs_sel_i                   (bussl_sel_i[SLAVE_NA]),
           .wbs_stb_i                   (bussl_stb_i[SLAVE_NA]),
           .wbs_we_i                    (bussl_we_i[SLAVE_NA]),
           .wbs_cab_i                   (bussl_cab_i[SLAVE_NA]),
           .wbs_cti_i                   (bussl_cti_i[SLAVE_NA]),
           .wbs_bte_i                   (bussl_bte_i[SLAVE_NA]));

   generate
      if (CONFIG.ENABLE_BOOTROM) begin : gen_bootrom
         /* bootrom AUTO_TEMPLATE(
          .clk(clk),
          .rst(rst_sys),
          .wb_\(.*\) (bussl_\1[SLAVE_BOOT]),
          ); */
         bootrom
           u_bootrom
             (/*AUTOINST*/
              // Outputs
              .wb_dat_o                 (bussl_dat_o[SLAVE_BOOT]), // Templated
              .wb_ack_o                 (bussl_ack_o[SLAVE_BOOT]), // Templated
              .wb_err_o                 (bussl_err_o[SLAVE_BOOT]), // Templated
              .wb_rty_o                 (bussl_rty_o[SLAVE_BOOT]), // Templated
              // Inputs
              .clk                      (clk),                   // Templated
              .rst                      (rst_sys),               // Templated
              .wb_adr_i                 (bussl_adr_i[SLAVE_BOOT]), // Templated
              .wb_dat_i                 (bussl_dat_i[SLAVE_BOOT]), // Templated
              .wb_cyc_i                 (bussl_cyc_i[SLAVE_BOOT]), // Templated
              .wb_stb_i                 (bussl_stb_i[SLAVE_BOOT]), // Templated
              .wb_sel_i                 (bussl_sel_i[SLAVE_BOOT])); // Templated
      end else begin // if (CONFIG.ENABLE_BOOTROM)
         assign bussl_dat_o[SLAVE_BOOT] = 32'hx;
         assign bussl_ack_o[SLAVE_BOOT] = 1'b0;
         assign bussl_err_o[SLAVE_BOOT] = 1'b0;
         assign bussl_rty_o[SLAVE_BOOT] = 1'b0;
      end // else: !if(CONFIG.ENABLE_BOOTROM)
   endgenerate
endmodule
