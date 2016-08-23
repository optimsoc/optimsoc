/* Copyright (c) 2012-2016 by the author(s)
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
 * A 2x2 distributed memory system with four compute tiles (CCCC)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"
`include "optimsoc_def.vh"
`include "dbg_config.vh"

module system_2x2_ccc_cam_dm
  #(// NoC parameters
    parameter NOC_DATA_WIDTH = 32,
    parameter NOC_TYPE_WIDTH = 2,
    parameter VCHANNELS = `VCHANNELS,

    // compute tile parameters
    parameter MEM_FILE = "ct.vmem",
    parameter MEM_SIZE = 1*1024*1024, // 1 MByte

    parameter CORES = 1,

    parameter USE_DEBUG = 1
    )
   (
	input clk, rst,

	// external inputs for camera module
	input PCLK, HREF, VSYNC,
	input [7:0] D,

    // external outputs to camera module
	output SIOC,
	output SIOD,
	output RESET,
	output PWDN,
	output XVCLK,

    glip_channel c_glip_in,
    glip_channel c_glip_out

`ifdef OPTIMSOC_CTRAM_WIRES
    , output [4*32-1:0] wb_mem_adr_i,
    output [4*1-1:0]  wb_mem_cyc_i,
    output [4*32-1:0] wb_mem_dat_i,
    output [4*4-1:0]  wb_mem_sel_i,
    output [4*1-1:0]  wb_mem_stb_i,
    output [4*1-1:0]  wb_mem_we_i,
    output [4*1-1:0]  wb_mem_cab_i,
    output [4*3-1:0]  wb_mem_cti_i,
    output [4*2-1:0]  wb_mem_bte_i,
    input [4*1-1:0]   wb_mem_ack_o,
    input [4*1-1:0]   wb_mem_rty_o,
    input [4*1-1:0]   wb_mem_err_o,
    input [4*32-1:0]  wb_mem_dat_o
`endif

	


    );

   localparam NOC_FLIT_WIDTH = NOC_DATA_WIDTH + NOC_TYPE_WIDTH;

   localparam DEBUG_MODS_PER_CORE = 2;
   localparam DEBUG_MODS_PER_TILE = CORES * DEBUG_MODS_PER_CORE + 1;
   localparam DEBUG_MODS = 3 * DEBUG_MODS_PER_TILE;

   localparam NUMCTS = 32'h3;
   localparam [NUMCTS*16-1:0] CTLIST = {16'h0, 16'h1, 16'h2};

   dii_flit [1:0] debug_ring_in [0:2];
   dii_flit [1:0] debug_ring_out [0:2];
   logic [1:0] debug_ring_in_ready [0:2];
   logic [1:0] debug_ring_out_ready [0:2];

   logic       rst_sys, rst_cpu;

   debug_interface
      #(
         .SYSTEMID    (1),
         .NUM_MODULES (DEBUG_MODS)
      )
      u_debuginterface
        (
         .clk            (clk),
         .rst            (rst),
         .sys_rst        (rst_sys),
         .cpu_rst        (rst_cpu),
         .glip_in        (c_glip_in),
         .glip_out       (c_glip_out),
         .ring_out       (debug_ring_in[0]),
         .ring_out_ready (debug_ring_in_ready[0]),
         .ring_in        (debug_ring_out[2]),
         .ring_in_ready  (debug_ring_out_ready[2])
      );

   // We are routing the debug in a meander
   assign debug_ring_in[1] = debug_ring_out[0];
   assign debug_ring_out_ready[0] = debug_ring_in_ready[1];
   assign debug_ring_in[2] = debug_ring_out[1];
   assign debug_ring_out_ready[1] = debug_ring_in_ready[2];

   // Flits from NoC->tiles
   wire [NOC_FLIT_WIDTH-1:0] link_in_flit[0:3];
   wire [VCHANNELS-1:0]      link_in_valid[0:3];
   wire [VCHANNELS-1:0]      link_in_ready[0:3];

   // Flits from tiles->NoC
   wire [NOC_FLIT_WIDTH-1:0] link_out_flit[0:3];
   wire [VCHANNELS-1:0]      link_out_valid[0:3];
   wire [VCHANNELS-1:0]      link_out_ready[0:3];

   /* lisnoc_mesh2x2 AUTO_TEMPLATE(
    .link\(.*\)_in_\(.*\)_.* (link_out_\2[\1]),
    .link\(.*\)_out_\(.*\)_.* (link_in_\2[\1]),
    .clk(clk),
    .rst(rst_sys),
    ); */
   lisnoc_mesh2x2
      #(.vchannels(VCHANNELS),.in_fifo_length(2),.out_fifo_length(2))
      u_mesh(/*AUTOINST*/
             // Outputs
             .link0_in_ready_o          (link_out_ready[0]),     // Templated
             .link0_out_flit_o          (link_in_flit[0]),       // Templated
             .link0_out_valid_o         (link_in_valid[0]),      // Templated
             .link1_in_ready_o          (link_out_ready[1]),     // Templated
             .link1_out_flit_o          (link_in_flit[1]),       // Templated
             .link1_out_valid_o         (link_in_valid[1]),      // Templated
             .link2_in_ready_o          (link_out_ready[2]),     // Templated
             .link2_out_flit_o          (link_in_flit[2]),       // Templated
             .link2_out_valid_o         (link_in_valid[2]),      // Templated
             .link3_in_ready_o          (link_out_ready[3]),     // Templated
             .link3_out_flit_o          (link_in_flit[3]),       // Templated
             .link3_out_valid_o         (link_in_valid[3]),      // Templated
             // Inputs
             .clk                       (clk),                   // Templated
             .rst                       (rst_sys),               // Templated
             .link0_in_flit_i           (link_out_flit[0]),      // Templated
             .link0_in_valid_i          (link_out_valid[0]),     // Templated
             .link0_out_ready_i         (link_in_ready[0]),      // Templated
             .link1_in_flit_i           (link_out_flit[1]),      // Templated
             .link1_in_valid_i          (link_out_valid[1]),     // Templated
             .link1_out_ready_i         (link_in_ready[1]),      // Templated
             .link2_in_flit_i           (link_out_flit[2]),      // Templated
             .link2_in_valid_i          (link_out_valid[2]),     // Templated
             .link2_out_ready_i         (link_in_ready[2]),      // Templated
             .link3_in_flit_i           (link_out_flit[3]),      // Templated
             .link3_in_valid_i          (link_out_valid[3]),     // Templated
             .link3_out_ready_i         (link_in_ready[3]));     // Templated

   genvar i;
   generate
      for (i=0; i<3; i=i+1) begin : gen_ct
         compute_tile_dm
            #(.ID(i),
              .CORES(CORES),
              .COREBASE(i*CORES),
              .MEM_SIZE(MEM_SIZE),
              .MEM_FILE(MEM_FILE),
              .USE_DEBUG(USE_DEBUG),
              .DEBUG_BASEID(2+i*DEBUG_MODS_PER_TILE),
              .NUMCTS(NUMCTS),
              .CTLIST(CTLIST))
         u_ct(.clk                        (clk),
              .rst_cpu                    (rst_cpu),
              .rst_sys                    (rst_sys),
              .rst_dbg                    (rst),
              .debug_ring_in              (debug_ring_in[i]),
              .debug_ring_in_ready        (debug_ring_in_ready[i]),
              .debug_ring_out             (debug_ring_out[i]),
              .debug_ring_out_ready       (debug_ring_out_ready[i]),

`ifdef OPTIMSOC_CTRAM_WIRES
              .wb_mem_ack_o               (wb_mem_ack_o[i]),
              .wb_mem_rty_o               (wb_mem_rty_o[i]),
              .wb_mem_err_o               (wb_mem_err_o[i]),
              .wb_mem_dat_o               (wb_mem_dat_o[(i+1)*32-1:i*32]),
              .wb_mem_adr_i               (wb_mem_adr_i[(i+1)*32-1:i*32]),
              .wb_mem_cyc_i               (wb_mem_cyc_i[i]),
              .wb_mem_dat_i               (wb_mem_dat_i[(i+1)*32-1:i*32]),
              .wb_mem_sel_i               (wb_mem_sel_i[(i+1)*4-1:i*4]),
              .wb_mem_stb_i               (wb_mem_stb_i[i]),
              .wb_mem_we_i                (wb_mem_we_i[i]),
              .wb_mem_cab_i               (wb_mem_cab_i[i]),
              .wb_mem_cti_i               (wb_mem_cti_i[(i+1)*3-1:i*3]),
              .wb_mem_bte_i               (wb_mem_bte_i[(i+1)*2-1:i*2]),
`endif //  `ifdef OPTIMSOC_CTRAM_WIRES

              .noc_in_ready               (link_in_ready[i][VCHANNELS-1:0]),
              .noc_out_flit               (link_out_flit[i][NOC_FLIT_WIDTH-1:0]),
              .noc_out_valid              (link_out_valid[i][VCHANNELS-1:0]),

              .noc_in_flit                (link_in_flit[i][NOC_FLIT_WIDTH-1:0]),
              .noc_in_valid               (link_in_valid[i][VCHANNELS-1:0]),
              .noc_out_ready              (link_out_ready[i][VCHANNELS-1:0]));
      end
   endgenerate

	camera_tile
	#(.ID(3))
		u_cam_tile (
			// Inputs
			.clk(clk),
			.rst(rst_sys),
			.noc_in_flit(link_in_flit[3][NOC_FLIT_WIDTH-1:0]),
			.noc_in_valid(link_in_valid[3][VCHANNELS-1:0]),
			.noc_out_ready(link_out_ready[3][VCHANNELS-1:0]),
			// External Inputs
			.PCLK(PCLK),
			.VSYNC(VSYNC),
			.HREF(HREF),
			.D(D),
			// Outputs
			.noc_in_ready(link_in_ready[3][VCHANNELS-1:0]),
			.noc_out_flit(link_out_flit[3][NOC_FLIT_WIDTH-1:0]),
			.noc_out_valid(link_out_valid[3][VCHANNELS-1:0]),
			// External Outputs
			.SIOC(SIOC),
			.SIOD(SIOD),
			.RESET(RESET),
			.PWDN(PWDN),
			.XVCLK(XVCLK)
		);

endmodule

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/meshs/" "../../*/verilog")
// verilog-auto-inst-param-value: t
// End:
