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
 * Single-Port RAM with Wishbone Interface
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Markus Goehrle <markus.goehrle@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module wb_sram_sp(/*AUTOARG*/
   // Outputs
   wb_ack_o, wb_err_o, wb_rty_o, wb_dat_o,
   // Inputs
   wb_adr_i, wb_bte_i, wb_cti_i, wb_cyc_i, wb_dat_i, wb_sel_i,
   wb_stb_i, wb_we_i, wb_clk_i, wb_rst_i
   );

   import optimsoc_functions::*;

   // Memory size in bytes
   parameter MEM_SIZE_BYTE = 'hx;

   // VMEM file used to initialize the memory in simulation
   parameter MEM_FILE = "sram.vmem";

   // address width
   parameter AW = $clog2(MEM_SIZE_BYTE);

   // data width (must be multiple of 8 for byte selects to work)
   // Valid values: 32, 16 and 8
   parameter DW = 32;

   // byte select width
   localparam SW = (DW == 32) ? 4 :
                   (DW == 16) ? 2 :
                   (DW ==  8) ? 1 : 'hx;

   // Allowed values:
   //   * PLAIN
   parameter MEM_IMPL_TYPE = "PLAIN";

   /*
    * +--------------+--------------+
    * | word address | byte in word |
    * +--------------+--------------+
    *     WORD_AW         BYTE_AW
    *        +----- AW -----+
    */
   localparam BYTE_AW = SW >> 1;
   localparam WORD_AW = AW - BYTE_AW;

   // Wishbone SLAVE interface
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
   wire [WORD_AW-1:0]   sram_waddr;             // From wb_ram of wb2sram.v
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
            .sram_waddr                 (sram_waddr),
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
    .clk   (wb_clk_i),
    .rst   (wb_rst_i),
    .ce    (sram_ce),
    .we    (sram_we),
    .oe    (1'b1),
    .waddr (sram_waddr),
    .sel   (sram_sel),
    .din   (sram_din),
    .dout  (sram_dout[]),
    ); */
   sram_sp
      #(.DW            (DW),
        .MEM_SIZE_BYTE (MEM_SIZE_BYTE),
        .AW            (AW),
        .WORD_AW       (WORD_AW),
        .MEM_IMPL_TYPE (MEM_IMPL_TYPE),
        .MEM_FILE      (MEM_FILE))
      sp_ram(/*AUTOINST*/
             // Outputs
             .dout                      (sram_dout[DW-1:0]),     // Templated
             // Inputs
             .clk                       (wb_clk_i),              // Templated
             .rst                       (wb_rst_i),              // Templated
             .ce                        (sram_ce),               // Templated
             .we                        (sram_we),               // Templated
             .oe                        (1'b1),                  // Templated
             .waddr                     (sram_waddr),            // Templated
             .din                       (sram_din),              // Templated
             .sel                       (sram_sel));             // Templated

endmodule
