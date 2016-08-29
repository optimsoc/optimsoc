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
 * A PGAS memory tile
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module memory_tile_pgas(/*AUTOARG*/
   // Outputs
   noc_in_ready, noc_out_flit, noc_out_valid, wb_adr_o, wb_cyc_o,
   wb_dat_o, wb_sel_o, wb_stb_o, wb_we_o, wb_cti_o, wb_bte_o,
   // Inputs
   clk, rst, noc_in_flit, noc_in_valid, noc_out_ready, wb_ack_i,
   wb_rty_i, wb_err_i, wb_dat_i
   );

   parameter ID = 'bx;

   parameter MEM_FILE  = "sram.vmem";

   parameter VCHANNELS = `VCHANNELS;
   parameter VC_LSU_REQ = `VCHANNEL_LSU_REQ;
   parameter VC_LSU_RESP = `VCHANNEL_LSU_RESP;

   parameter NOC_DATA_WIDTH = 32;
   parameter NOC_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_DATA_WIDTH + NOC_TYPE_WIDTH;

   input clk;
   input rst;

   input [NOC_FLIT_WIDTH-1:0]  noc_in_flit;
   input [VCHANNELS-1:0]       noc_in_valid;
   output [VCHANNELS-1:0]      noc_in_ready;
   output [NOC_FLIT_WIDTH-1:0] noc_out_flit;
   output [VCHANNELS-1:0]      noc_out_valid;
   input [VCHANNELS-1:0]       noc_out_ready;

   // WB-Interface from memtile-bus to mem (gets accessed externally)
   output [31:0]          wb_adr_o;
   output                 wb_cyc_o;
   output [31:0]          wb_dat_o;
   output [3:0]           wb_sel_o;
   output                 wb_stb_o;
   output                 wb_we_o;
   output [2:0]           wb_cti_o;
   output [1:0]           wb_bte_o;
   input                  wb_ack_i;
   input                  wb_rty_i;
   input                  wb_err_i;
   input [31:0]           wb_dat_i;

   genvar i;
   generate
      for (i=0;i<VCHANNELS;i=i+1) begin
         if ((i!=VC_LSU_REQ) && (i!=VC_LSU_RESP)) begin
            assign noc_in_ready[i]  = 1'b0;
            assign noc_out_valid[i] = 1'b0;
         end
      end
   endgenerate

   // This tile will not generate any request
   assign noc_out_valid[VC_LSU_REQ] = 1'b0;
   // and it will therefore not receive a response
   assign noc_in_ready[VC_LSU_RESP] = 1'b0;

   /* noclsu_memory AUTO_TEMPLATE(
    .noc_out_valid (noc_out_valid[VC_LSU_RESP]),
    .noc_out_ready (noc_out_ready[VC_LSU_RESP]),
    .noc_in_valid (noc_in_valid[VC_LSU_REQ]),
    .noc_in_ready (noc_in_ready[VC_LSU_REQ]),
    .noc_out_flit (noc_out_flit[]),
    .noc_in_flit (noc_in_flit[]),
    ); */
   noclsu_memory
      #(.ID(ID))
      u_lsu(/*AUTOINST*/
            // Outputs
            .noc_out_flit               (noc_out_flit[33:0]),    // Templated
            .noc_out_valid              (noc_out_valid[VC_LSU_RESP]), // Templated
            .noc_in_ready               (noc_in_ready[VC_LSU_REQ]), // Templated
            .wb_adr_o                   (wb_adr_o[31:0]),
            .wb_bte_o                   (wb_bte_o[1:0]),
            .wb_cti_o                   (wb_cti_o[2:0]),
            .wb_cyc_o                   (wb_cyc_o),
            .wb_dat_o                   (wb_dat_o[31:0]),
            .wb_sel_o                   (wb_sel_o[3:0]),
            .wb_stb_o                   (wb_stb_o),
            .wb_we_o                    (wb_we_o),
            // Inputs
            .clk                        (clk),
            .rst                        (rst),
            .noc_out_ready              (noc_out_ready[VC_LSU_RESP]), // Templated
            .noc_in_flit                (noc_in_flit[33:0]),     // Templated
            .noc_in_valid               (noc_in_valid[VC_LSU_REQ]), // Templated
            .wb_ack_i                   (wb_ack_i),
            .wb_err_i                   (wb_err_i),
            .wb_rty_i                   (wb_rty_i),
            .wb_dat_i                   (wb_dat_i[31:0]));

endmodule

// Local Variables:
// verilog-library-directories:("../../*/verilog" "../../../../../lisnoc/rtl/*/")
// verilog-auto-inst-param-value: t
// End:
