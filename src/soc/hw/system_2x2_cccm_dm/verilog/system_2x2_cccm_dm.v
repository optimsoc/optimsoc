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
 * A 2x2 distributed memory system with three compute and a memory tile (CCCM)
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"
`include "dbg_config.vh"

module system_2x2_cccm_dm(
`ifdef OPTIMSOC_DEBUG_ENABLE_MAM
   wb_mam_adr_o, wb_mam_cyc_o, wb_mam_dat_o, wb_mam_sel_o, wb_mam_stb_o,
   wb_mam_we_o, wb_mam_cab_o, wb_mam_cti_o, wb_mam_bte_o, wb_mam_ack_i,
   wb_mam_rty_i, wb_mam_err_i, wb_mam_dat_i,
`endif
`ifdef OPTIMSOC_CTRAM_WIRES
   wb_mem_adr_i, wb_mem_cyc_i, wb_mem_dat_i, wb_mem_sel_i,
   wb_mem_stb_i, wb_mem_we_i, wb_mem_cab_i, wb_mem_cti_i,
   wb_mem_bte_i, wb_mem_ack_o, wb_mem_rty_o, wb_mem_err_o,
   wb_mem_dat_o,
`endif
   /*AUTOARG*/
   // Outputs
   trace, mt3_adr_o, mt3_cyc_o, mt3_dat_o, mt3_sel_o, mt3_stb_o,
   mt3_we_o, mt3_cti_o, mt3_bte_o,
   // Inputs
   clk, rst_sys, rst_cpu, mt3_ack_i, mt3_rty_i, mt3_err_i, mt3_dat_i
   );

   // NoC parameters
   parameter NOC_DATA_WIDTH = 32;
   parameter NOC_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_DATA_WIDTH + NOC_TYPE_WIDTH;
   parameter VCHANNELS = `VCHANNELS;

   // compute tile parameters
   parameter CT_MEM_FILE = "ct.vmem";
   parameter CT_MEM_SIZE = 1*1024*1024; // 1 MByte

   parameter MT_MEM_FILE = "ct.vmem";
   parameter MT_MEM_SIZE = 1*1024*1024; // 1 MByte

   parameter CORES = 1;

   input clk, rst_sys, rst_cpu;

   output [`DEBUG_TRACE_EXEC_WIDTH*3*CORES-1:0] trace;

`ifdef OPTIMSOC_DEBUG_ENABLE_MAM
   input [4*32-1:0]   wb_mam_adr_o;
   input [4*1-1:0]    wb_mam_cyc_o;
   input [4*32-1:0]   wb_mam_dat_o;
   input [4*4-1:0]    wb_mam_sel_o;
   input [4*1-1:0]    wb_mam_stb_o;
   input [4*1-1:0]    wb_mam_we_o;
   input [4*1-1:0]    wb_mam_cab_o;
   input [4*3-1:0]    wb_mam_cti_o;
   input [4*2-1:0]    wb_mam_bte_o;
   output [4*1-1:0]   wb_mam_ack_i;
   output [4*1-1:0]   wb_mam_rty_i;
   output [4*1-1:0]   wb_mam_err_i;
   output [4*32-1:0]  wb_mam_dat_i;
`endif
`ifdef OPTIMSOC_CTRAM_WIRES
   output [3*32-1:0] wb_mem_adr_i;
   output [3*1-1:0]  wb_mem_cyc_i;
   output [3*32-1:0] wb_mem_dat_i;
   output [3*4-1:0]  wb_mem_sel_i;
   output [3*1-1:0]  wb_mem_stb_i;
   output [3*1-1:0]  wb_mem_we_i;
   output [3*1-1:0]  wb_mem_cab_i;
   output [3*3-1:0]  wb_mem_cti_i;
   output [3*2-1:0]  wb_mem_bte_i;
   input [3*1-1:0]   wb_mem_ack_o;
   input [3*1-1:0]   wb_mem_rty_o;
   input [3*1-1:0]   wb_mem_err_o;
   input [3*32-1:0]  wb_mem_dat_o;
`endif

   output [31:0]     mt3_adr_o;
   output            mt3_cyc_o;
   output [31:0]     mt3_dat_o;
   output [3:0]      mt3_sel_o;
   output            mt3_stb_o;
   output            mt3_we_o;
   output [2:0]      mt3_cti_o;
   output [1:0]      mt3_bte_o;
   input             mt3_ack_i;
   input             mt3_rty_i;
   input             mt3_err_i;
   input [31:0]      mt3_dat_i;
   
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
              .MEM_SIZE(CT_MEM_SIZE),
              .MEM_FILE(CT_MEM_FILE),
              .GLOBAL_MEMORY_SIZE(MT_MEM_SIZE),
              .GLOBAL_MEMORY_TILE(3))
            u_ct(// Outputs
                 .trace                      (trace[(`DEBUG_TRACE_EXEC_WIDTH*CORES*(i+1))-1:`DEBUG_TRACE_EXEC_WIDTH*CORES*i]),
`ifdef OPTIMSOC_DEBUG_ENABLE_MAM
                 .wb_mam_ack_i               (wb_mam_ack_i[i]),
                 .wb_mam_rty_i               (wb_mam_rty_i[i]),
                 .wb_mam_err_i               (wb_mam_err_i[i]),
                 .wb_mam_dat_i               (wb_mam_dat_i[(i+1)*32-1:i*32]),
                 .wb_mam_adr_o               (wb_mam_adr_o[(i+1)*32-1:i*32]),
                 .wb_mam_cyc_o               (wb_mam_cyc_o[i]),
                 .wb_mam_dat_o               (wb_mam_dat_o[(i+1)*32-1:i*32]),
                 .wb_mam_sel_o               (wb_mam_sel_o[(i+1)*4-1:i*4]),
                 .wb_mam_stb_o               (wb_mam_stb_o[i]),
                 .wb_mam_we_o                (wb_mam_we_o[i]),
                 .wb_mam_cab_o               (wb_mam_cab_o[i]),
                 .wb_mam_cti_o               (wb_mam_cti_o[(i+1)*3-1:i*3]),
                 .wb_mam_bte_o               (wb_mam_bte_o[(i+1)*2-1:i*2]),
`endif
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
`endif
                 .noc_in_ready               (link_in_ready[i][VCHANNELS-1:0]),
                 .noc_out_flit               (link_out_flit[i][NOC_FLIT_WIDTH-1:0]),
                 .noc_out_valid              (link_out_valid[i][VCHANNELS-1:0]),
                 // Inputs
                 .clk                        (clk),
                 .rst_cpu                    (rst_cpu),
                 .rst_sys                    (rst_sys),
                 .noc_in_flit                (link_in_flit[i][NOC_FLIT_WIDTH-1:0]),
                 .noc_in_valid               (link_in_valid[i][VCHANNELS-1:0]),
                 .noc_out_ready              (link_out_ready[i][VCHANNELS-1:0]),
                 .cpu_stall                  (1'b0));
      end
   endgenerate

   
   memory_tile_dm
     u_mt3(// Outputs
           .noc_in_ready                (link_in_ready[3]),
           .noc_out_flit                (link_out_flit[3]),
           .noc_out_valid               (link_out_valid[3]),
           .wb_adr_o                    (mt3_adr_o[31:0]),
           .wb_cyc_o                    (mt3_cyc_o),
           .wb_dat_o                    (mt3_dat_o[31:0]),
           .wb_sel_o                    (mt3_sel_o[3:0]),
           .wb_stb_o                    (mt3_stb_o),
           .wb_we_o                     (mt3_we_o),
           .wb_cti_o                    (mt3_cti_o[2:0]),
           .wb_bte_o                    (mt3_bte_o[1:0]),
           // Inputs
           .clk                         (clk),
           .rst                         (rst_sys),
           .noc_in_flit                 (link_in_flit[3]),
           .noc_in_valid                (link_in_valid[3]),
           .noc_out_ready               (link_out_ready[3]),
           .wb_ack_i                    (mt3_ack_i),
           .wb_rty_i                    (mt3_rty_i),
           .wb_err_i                    (mt3_err_i),
           .wb_dat_i                    (mt3_dat_i[31:0]));
   

endmodule

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/meshs/" "../../*/verilog")
// verilog-auto-inst-param-value: t
// End:
