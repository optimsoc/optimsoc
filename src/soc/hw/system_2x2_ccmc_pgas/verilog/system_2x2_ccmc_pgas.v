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
 * 2x2 CCMC PGAS System with three compute tiles and one memory tiles
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"

module system_2x2_ccmc_pgas(/*AUTOARG*/
   // Outputs
   wb_mt2_adr_o, wb_mt2_cyc_o, wb_mt2_dat_o, wb_mt2_sel_o,
   wb_mt2_stb_o, wb_mt2_we_o, wb_mt2_cti_o, wb_mt2_bte_o,
   // Inputs
   clk, rst_sys, rst_cpu, wb_mt2_ack_i, wb_mt2_rty_i, wb_mt2_err_i,
   wb_mt2_dat_i
   );

   input clk, rst_sys, rst_cpu;

   parameter noc_data_width = 32;
   parameter noc_type_width = 2;
   localparam noc_flit_width = noc_data_width+noc_type_width;

   localparam flit_width = noc_flit_width;

   localparam vchannels = `VCHANNELS;

   output [31:0] wb_mt2_adr_o;
   output        wb_mt2_cyc_o;
   output [31:0] wb_mt2_dat_o;
   output [3:0]  wb_mt2_sel_o;
   output        wb_mt2_stb_o;
   output        wb_mt2_we_o;
   output [2:0]  wb_mt2_cti_o;
   output [1:0]  wb_mt2_bte_o;
   input         wb_mt2_ack_i;
   input         wb_mt2_rty_i;
   input         wb_mt2_err_i;
   input [31:0]  wb_mt2_dat_i;

   // Flits from NoC->tiles
   wire [noc_flit_width-1:0] link_in_flit[0:3];
   wire [vchannels-1:0]      link_in_valid[0:3];
   wire [vchannels-1:0]      link_in_ready[0:3];

   // Flits from tiles->NoC
   wire [noc_flit_width-1:0] link_out_flit[0:3];
   wire [vchannels-1:0]      link_out_valid[0:3];
   wire [vchannels-1:0]      link_out_ready[0:3];

   /* lisnoc_mesh2x2 AUTO_TEMPLATE(
    .link\(.*\)_in_\(.*\)_.* (link_out_\2[\1][]),
    .link\(.*\)_out_\(.*\)_.* (link_in_\2[\1][]),
    .clk(clk),
    .rst(rst_sys),
    ); */
   lisnoc_mesh2x2
     #(.vchannels(vchannels))
   u_mesh(/*AUTOINST*/
          // Outputs
          .link0_in_ready_o             (link_out_ready[0][(vchannels)-1:0]), // Templated
          .link0_out_flit_o             (link_in_flit[0][flit_width-1:0]), // Templated
          .link0_out_valid_o            (link_in_valid[0][(vchannels)-1:0]), // Templated
          .link1_in_ready_o             (link_out_ready[1][(vchannels)-1:0]), // Templated
          .link1_out_flit_o             (link_in_flit[1][flit_width-1:0]), // Templated
          .link1_out_valid_o            (link_in_valid[1][(vchannels)-1:0]), // Templated
          .link2_in_ready_o             (link_out_ready[2][(vchannels)-1:0]), // Templated
          .link2_out_flit_o             (link_in_flit[2][flit_width-1:0]), // Templated
          .link2_out_valid_o            (link_in_valid[2][(vchannels)-1:0]), // Templated
          .link3_in_ready_o             (link_out_ready[3][(vchannels)-1:0]), // Templated
          .link3_out_flit_o             (link_in_flit[3][flit_width-1:0]), // Templated
          .link3_out_valid_o            (link_in_valid[3][(vchannels)-1:0]), // Templated
          // Inputs
          .clk                          (clk),                   // Templated
          .rst                          (rst_sys),               // Templated
          .link0_in_flit_i              (link_out_flit[0][flit_width-1:0]), // Templated
          .link0_in_valid_i             (link_out_valid[0][(vchannels)-1:0]), // Templated
          .link0_out_ready_i            (link_in_ready[0][(vchannels)-1:0]), // Templated
          .link1_in_flit_i              (link_out_flit[1][flit_width-1:0]), // Templated
          .link1_in_valid_i             (link_out_valid[1][(vchannels)-1:0]), // Templated
          .link1_out_ready_i            (link_in_ready[1][(vchannels)-1:0]), // Templated
          .link2_in_flit_i              (link_out_flit[2][flit_width-1:0]), // Templated
          .link2_in_valid_i             (link_out_valid[2][(vchannels)-1:0]), // Templated
          .link2_out_ready_i            (link_in_ready[2][(vchannels)-1:0]), // Templated
          .link3_in_flit_i              (link_out_flit[3][flit_width-1:0]), // Templated
          .link3_in_valid_i             (link_out_valid[3][(vchannels)-1:0]), // Templated
          .link3_out_ready_i            (link_in_ready[3][(vchannels)-1:0])); // Templated

   /* compute_tile_pgas AUTO_TEMPLATE(
    .noc_\(.*\) (link_\1[@][]),
    .cpu_stall  (1'b0),
    );*/

   /* memory_tile_pgas AUTO_TEMPLATE(
    .noc_\(.*\) (link_\1[@][]),
    .rst(rst_sys),
    .wb_\(.*\)  (wb_mt@_\1[]),
    );*/

   compute_tile_pgas
     #(.vchannels(vchannels),.id(0),.pgas_base(0),.pgas_addrw(pgas_addrw),.memoryid(2))
   u_ct0(/*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[0][(vchannels)-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[0][noc_flit_width-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[0][(vchannels)-1:0]), // Templated
         // Inputs
         .clk                           (clk),
         .rst_cpu                       (rst_cpu),
         .rst_sys                       (rst_sys),
         .noc_in_flit                   (link_in_flit[0][noc_flit_width-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[0][(vchannels)-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[0][(vchannels)-1:0]), // Templated
         .cpu_stall                     (1'b0));                         // Templated

   compute_tile_pgas
     #(.vchannels(vchannels),.id(1),.pgas_base(1),.pgas_addrw(pgas_addrw),.memoryid(2))
   u_ct1(/*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[1][(vchannels)-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[1][noc_flit_width-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[1][(vchannels)-1:0]), // Templated
         // Inputs
         .clk                           (clk),
         .rst_cpu                       (rst_cpu),
         .rst_sys                       (rst_sys),
         .noc_in_flit                   (link_in_flit[1][noc_flit_width-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[1][(vchannels)-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[1][(vchannels)-1:0]), // Templated
         .cpu_stall                     (1'b0));                         // Templated

   memory_tile_pgas
     #(.id(2),.vchannels(vchannels))
   u_mt2(/*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[2][(vchannels)-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[2][noc_flit_width-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[2][(vchannels)-1:0]), // Templated
         .wb_adr_o                      (wb_mt2_adr_o[31:0]),    // Templated
         .wb_cyc_o                      (wb_mt2_cyc_o),          // Templated
         .wb_dat_o                      (wb_mt2_dat_o[31:0]),    // Templated
         .wb_sel_o                      (wb_mt2_sel_o[3:0]),     // Templated
         .wb_stb_o                      (wb_mt2_stb_o),          // Templated
         .wb_we_o                       (wb_mt2_we_o),           // Templated
         .wb_cti_o                      (wb_mt2_cti_o[2:0]),     // Templated
         .wb_bte_o                      (wb_mt2_bte_o[1:0]),     // Templated
         // Inputs
         .clk                           (clk),
         .rst                           (rst_sys),               // Templated
         .noc_in_flit                   (link_in_flit[2][noc_flit_width-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[2][(vchannels)-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[2][(vchannels)-1:0]), // Templated
         .wb_ack_i                      (wb_mt2_ack_i),          // Templated
         .wb_rty_i                      (wb_mt2_rty_i),          // Templated
         .wb_err_i                      (wb_mt2_err_i),          // Templated
         .wb_dat_i                      (wb_mt2_dat_i[31:0]));   // Templated

   compute_tile_pgas
     #(.vchannels(vchannels),.id(3),.pgas_base(2),.pgas_addrw(pgas_addrw),.memoryid(2))
   u_ct3(/*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[3][(vchannels)-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[3][noc_flit_width-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[3][(vchannels)-1:0]), // Templated
         // Inputs
         .clk                           (clk),
         .rst_cpu                       (rst_cpu),
         .rst_sys                       (rst_sys),
         .noc_in_flit                   (link_in_flit[3][noc_flit_width-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[3][(vchannels)-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[3][(vchannels)-1:0]), // Templated
         .cpu_stall                     (1'b0));                         // Templated

endmodule // system_2x2_ccmc

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/meshs/" "../../*/verilog")
// verilog-auto-inst-param-value: t
// End:
