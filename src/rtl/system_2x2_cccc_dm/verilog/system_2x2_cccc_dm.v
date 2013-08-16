/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * A 2x2 distributed memory system with four compute tiles (CCCC)
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

`include "lisnoc_def.vh"
`include "optimsoc_def.vh"

module system_2x2_cccc_dm(
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   trace_itm,
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   trace_stm,
`endif
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
   // Inputs
   clk, rst_sys, rst_cpu
   );

   // NoC parameters
   parameter NOC_DATA_WIDTH = 32;
   parameter NOC_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_DATA_WIDTH + NOC_TYPE_WIDTH;
   parameter VCHANNELS = `VCHANNELS;

   // compute tile parameters
   parameter MEM_FILE = "ct.vmem";
   parameter MEM_SIZE = 1*1024*1024; // 1 MByte


   input clk, rst_sys, rst_cpu;

`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   output [`DEBUG_ITM_PORTWIDTH*4-1:0] trace_itm;
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   output [`DEBUG_STM_PORTWIDTH*4-1:0] trace_stm;
`endif
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
   output [4*32-1:0] wb_mem_adr_i;
   output [4*1-1:0]  wb_mem_cyc_i;
   output [4*32-1:0] wb_mem_dat_i;
   output [4*4-1:0]  wb_mem_sel_i;
   output [4*1-1:0]  wb_mem_stb_i;
   output [4*1-1:0]  wb_mem_we_i;
   output [4*1-1:0]  wb_mem_cab_i;
   output [4*3-1:0]  wb_mem_cti_i;
   output [4*2-1:0]  wb_mem_bte_i;
   input [4*1-1:0]   wb_mem_ack_o;
   input [4*1-1:0]   wb_mem_rty_o;
   input [4*1-1:0]   wb_mem_err_o;
   input [4*32-1:0]  wb_mem_dat_o;
`endif

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
             .link3_out_ready_i         (link_in_ready[3]));      // Templated

   genvar i;
   generate
      for (i=0; i<4; i=i+1) begin : gen_ct
         compute_tile_dm
            #(.ID(i),
              .MEM_SIZE(MEM_SIZE),
              .MEM_FILE(MEM_FILE))
            u_ct(// Outputs
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
                 .trace_itm                  (trace_itm[(`DEBUG_ITM_PORTWIDTH*(i+1))-1:`DEBUG_ITM_PORTWIDTH*i]),
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
                 .trace_stm                  (trace_stm[(`DEBUG_STM_PORTWIDTH*(i+1))-1:`DEBUG_STM_PORTWIDTH*i]),
`endif
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

endmodule

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/meshs/" "../../*/verilog")
// verilog-auto-inst-param-value: t
// End:
