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
 * Single-Port RAM with Withbone Interface
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 *    Markus Goehrle, markus.goehrle@tum.de
 *    Philipp Wagner, philipp.wagner@tum.de
 */

`include "optimsoc_def.vh"

module wb_sram_sp(/*AUTOARG*/
   // Outputs
   wb_ack_o, wb_err_o, wb_rty_o, wb_dat_o,
   // Inputs
   wb_adr_i, wb_bte_i, wb_cti_i, wb_cyc_i, wb_dat_i, wb_sel_i,
   wb_stb_i, wb_we_i, wb_clk_i, wb_rst_i
   );

   // Memory size in bytes
   parameter MEM_SIZE = 'x;

   // address width
   parameter AW = clog2(MEM_SIZE);
   
   // data width (must be multiple of 8 for byte selects to work)
   // Valid values: 32, 16 and 8
   parameter DW = 32;

   // byte select width
   localparam SW = (DW == 32) ? 4 :
                   (DW == 16) ? 2 :
                   (DW ==  8) ? 1 : 'hx;
   
   // Allowed valued:
   //   * PLAIN
   //   * XILINX_SPARTAN6
   parameter IMPL = `OPTIMSOC_SRAM_IMPLEMENTATION;

   /*
    * +--------------+--------------+
    * | word address | byte in word |
    * +--------------+--------------+
    *     WORD_AW         BYTE_AW
    *        +----- AW -----+
    */
   localparam BYTE_AW = SW >> 1;
   localparam WORD_AW = AW - BYTE_AW;

   // Wishbone interface
   input [AW-1:0]  wb_adr_i;
   input [1:0]     wb_bte_i;
   input [2:0]     wb_cti_i;
   input           wb_cyc_i;
   input [DW-1:0]  wb_dat_i;
   input [SW-1:0]  wb_sel_i;
   input           wb_stb_i;
   input           wb_we_i;

   output          wb_ack_o;
   output          wb_err_o;
   output          wb_rty_o;
   output [DW-1:0] wb_dat_o;

   input           wb_clk_i; // unused
   input           wb_rst_i; // unused

   /*AUTOWIRE*/
   // Beginning of automatic wires (for undeclared instantiated-module outputs)
   wire [WORD_AW-1:0]   sram_addr;              // From wb_ram of wb2sram.v
   wire                 sram_ce;                // From wb_ram of wb2sram.v
   wire [DW-1:0]        sram_din;               // From wb_ram of wb2sram.v
   wire [DW-1:0]        sram_dout;              // From sp_ram of sram_sp.v
   wire [SW-1:0]        sram_sel;               // From wb_ram of wb2sram.v
   wire                 sram_we;                // From wb_ram of wb2sram.v
   // End of automatics

   wb2sram
     #(.DW(DW),
       .AW(AW))
     wb_ram(/*AUTOINST*/
            // Outputs
            .wb_ack_o                   (wb_ack_o),
            .wb_err_o                   (wb_err_o),
            .wb_rty_o                   (wb_rty_o),
            .wb_dat_o                   (wb_dat_o[DW-1:0]),
            .sram_ce                    (sram_ce),
            .sram_we                    (sram_we),
            .sram_addr                  (sram_addr[WORD_AW-1:0]),
            .sram_din                   (sram_din[DW-1:0]),
            .sram_sel                   (sram_sel[SW-1:0]),
            // Inputs
            .wb_adr_i                   (wb_adr_i[AW-1:0]),
            .wb_bte_i                   (wb_bte_i[1:0]),
            .wb_cti_i                   (wb_cti_i[2:0]),
            .wb_cyc_i                   (wb_cyc_i),
            .wb_dat_i                   (wb_dat_i[DW-1:0]),
            .wb_sel_i                   (wb_sel_i[SW-1:0]),
            .wb_stb_i                   (wb_stb_i),
            .wb_we_i                    (wb_we_i),
            .wb_clk_i                   (wb_clk_i),
            .wb_rst_i                   (wb_rst_i),
            .sram_dout                  (sram_dout[DW-1:0]));

   /* sram_sp AUTO_TEMPLATE(
    .clk  (wb_clk_i),
    .rst  (wb_rst_i),
    .ce   (sram_ce),
    .we   (sram_we),
    .oe   (1'b1),
    .addr (sram_addr),
    .sel  (sram_sel),
    .din  (sram_din),
    .dout (sram_dout[]),
    ); */
   sram_sp
      #(.DW       (DW),
        .MEM_SIZE (MEM_SIZE),
        .AW       (WORD_AW),
        .MEM_IMPL_TYPE (IMPL))
      sp_ram(/*AUTOINST*/
             // Outputs
             .dout                      (sram_dout[DW-1:0]),     // Templated
             // Inputs
             .clk                       (wb_clk_i),              // Templated
             .rst                       (wb_rst_i),              // Templated
             .ce                        (sram_ce),               // Templated
             .we                        (sram_we),               // Templated
             .oe                        (1'b1),                  // Templated
             .addr                      (sram_addr),             // Templated
             .din                       (sram_din),              // Templated
             .sel                       (sram_sel));             // Templated

   `include "optimsoc_functions.vh"

endmodule
