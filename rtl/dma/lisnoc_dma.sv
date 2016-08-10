/* Copyright (c) 2015 by the author(s)
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
 * This is the toplevel file of the DMA controller.
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_dma_def.vh"

module lisnoc_dma(/*AUTOARG*/
   // Outputs
   noc_in_req_ready, noc_in_resp_ready, noc_out_req_flit,
   noc_out_req_valid, noc_out_resp_flit, noc_out_resp_valid,
   wb_if_dat_o, wb_if_ack_o, wb_if_err_o, wb_if_rty_o, wb_adr_o,
   wb_dat_o, wb_cyc_o, wb_stb_o, wb_sel_o, wb_we_o, wb_cab_o,
   wb_cti_o, wb_bte_o, irq,
   // Inputs
   clk, rst, noc_in_req_flit, noc_in_req_valid, noc_in_resp_flit,
   noc_in_resp_valid, noc_out_req_ready, noc_out_resp_ready,
   wb_if_adr_i, wb_if_dat_i, wb_if_cyc_i, wb_if_stb_i, wb_if_we_i,
   wb_dat_i, wb_ack_i
   );

   parameter table_entries = 4;
   localparam table_entries_ptrwidth = $clog2(table_entries);

   parameter tileid = 0;

   parameter noc_packet_size = 16;

   parameter generate_interrupt = 1;

   input clk;
   input rst;

   input [`FLIT_WIDTH-1:0] noc_in_req_flit;
   input                   noc_in_req_valid;
   output                  noc_in_req_ready;

   input [`FLIT_WIDTH-1:0] noc_in_resp_flit;
   input                   noc_in_resp_valid;
   output                  noc_in_resp_ready;

   output [`FLIT_WIDTH-1:0] noc_out_req_flit;
   output                   noc_out_req_valid;
   input                    noc_out_req_ready;

   output [`FLIT_WIDTH-1:0] noc_out_resp_flit;
   output                   noc_out_resp_valid;
   input                    noc_out_resp_ready;

   input [31:0]             wb_if_adr_i;
   input [31:0]             wb_if_dat_i;
   input                    wb_if_cyc_i;
   input                    wb_if_stb_i;
   input                    wb_if_we_i;
   output [31:0]            wb_if_dat_o;
   output                   wb_if_ack_o;
   output                   wb_if_err_o;
   output                   wb_if_rty_o;

   output reg [31:0]        wb_adr_o;
   output reg [31:0]        wb_dat_o;
   output reg               wb_cyc_o;
   output reg               wb_stb_o;
   output reg [3:0]         wb_sel_o;
   output reg               wb_we_o;
   output                   wb_cab_o;
   output reg [2:0]         wb_cti_o;
   output reg [1:0]         wb_bte_o;
   input [31:0]             wb_dat_i;
   input                    wb_ack_i;

   output [table_entries-1:0] irq;

   assign wb_if_err_o = 1'b0;
   assign wb_if_rty_o = 1'b0;

   wire [31:0]              wb_req_adr_o;
   wire [31:0]              wb_req_dat_o;
   wire                     wb_req_cyc_o;
   wire                     wb_req_stb_o;
   wire                     wb_req_we_o;
   wire [3:0]               wb_req_sel_o;
   wire [2:0]               wb_req_cti_o;
   wire [1:0]               wb_req_bte_o;
   reg [31:0]               wb_req_dat_i;
   reg                      wb_req_ack_i;

   wire [31:0]              wb_resp_adr_o;
   wire [31:0]              wb_resp_dat_o;
   wire                     wb_resp_cyc_o;
   wire                     wb_resp_stb_o;
   wire                     wb_resp_we_o;
   wire [3:0]               wb_resp_sel_o;
   wire [2:0]               wb_resp_cti_o;
   wire [1:0]               wb_resp_bte_o;
   reg [31:0]               wb_resp_dat_i;
   reg                      wb_resp_ack_i;

   wire [31:0]              wb_target_adr_o;
   wire [31:0]              wb_target_dat_o;
   wire                     wb_target_cyc_o;
   wire                     wb_target_stb_o;
   wire                     wb_target_we_o;
   wire [2:0]               wb_target_cti_o;
   wire [1:0]               wb_target_bte_o;
   reg [31:0]               wb_target_dat_i;
   reg                      wb_target_ack_i;

   /*AUTOWIRE*/
   // Beginning of automatic wires (for undeclared instantiated-module outputs)
   wire                 ctrl_done_en;           // From ctrl_initiator of lisnoc_dma_initiator.v
   wire [table_entries_ptrwidth-1:0] ctrl_done_pos;// From ctrl_initiator of lisnoc_dma_initiator.v
   wire [table_entries_ptrwidth-1:0] ctrl_read_pos;// From ctrl_initiator of lisnoc_dma_initiator.v
   wire [`DMA_REQUEST_WIDTH-1:0] ctrl_read_req; // From request_table of lisnoc_dma_request_table.v
   wire [table_entries-1:0] done;               // From request_table of lisnoc_dma_request_table.v
   wire                 if_valid_en;            // From wbinterface of lisnoc_dma_wbinterface.v
   wire [table_entries_ptrwidth-1:0] if_valid_pos;// From wbinterface of lisnoc_dma_wbinterface.v
   wire                 if_valid_set;           // From wbinterface of lisnoc_dma_wbinterface.v
   wire                 if_validrd_en;          // From wbinterface of lisnoc_dma_wbinterface.v
   wire                 if_write_en;            // From wbinterface of lisnoc_dma_wbinterface.v
   wire [table_entries_ptrwidth-1:0] if_write_pos;// From wbinterface of lisnoc_dma_wbinterface.v
   wire [`DMA_REQUEST_WIDTH-1:0] if_write_req;  // From wbinterface of lisnoc_dma_wbinterface.v
   wire [`DMA_REQMASK_WIDTH-1:0] if_write_select;// From wbinterface of lisnoc_dma_wbinterface.v
   wire [table_entries-1:0] valid;              // From request_table of lisnoc_dma_request_table.v
   wire [3:0]           wb_target_sel_o;        // From target of lisnoc_dma_target.v
   // End of automatics

   wire [table_entries_ptrwidth-1:0] ctrl_out_read_pos;
   wire [table_entries_ptrwidth-1:0] ctrl_in_read_pos;
   wire [table_entries_ptrwidth-1:0] ctrl_write_pos;

   assign ctrl_out_read_pos = 0;
   assign ctrl_in_read_pos = 0;
   assign ctrl_write_pos = 0;

   lisnoc_dma_wbinterface
     #(.tileid(tileid))
   wbinterface(/*AUTOINST*/
               // Outputs
               .wb_if_dat_o             (wb_if_dat_o[31:0]),
               .wb_if_ack_o             (wb_if_ack_o),
               .if_write_req            (if_write_req[`DMA_REQUEST_WIDTH-1:0]),
               .if_write_pos            (if_write_pos[table_entries_ptrwidth-1:0]),
               .if_write_select         (if_write_select[`DMA_REQMASK_WIDTH-1:0]),
               .if_write_en             (if_write_en),
               .if_valid_pos            (if_valid_pos[table_entries_ptrwidth-1:0]),
               .if_valid_set            (if_valid_set),
               .if_valid_en             (if_valid_en),
               .if_validrd_en           (if_validrd_en),
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .wb_if_adr_i             (wb_if_adr_i[31:0]),
               .wb_if_dat_i             (wb_if_dat_i[31:0]),
               .wb_if_cyc_i             (wb_if_cyc_i),
               .wb_if_stb_i             (wb_if_stb_i),
               .wb_if_we_i              (wb_if_we_i),
               .done                    (done[table_entries-1:0]));

   lisnoc_dma_request_table
     #(.generate_interrupt(generate_interrupt))
   request_table(/*AUTOINST*/
                 // Outputs
                 .ctrl_read_req         (ctrl_read_req[`DMA_REQUEST_WIDTH-1:0]),
                 .valid                 (valid[table_entries-1:0]),
                 .done                  (done[table_entries-1:0]),
                 .irq                   (irq[table_entries-1:0]),
                 // Inputs
                 .clk                   (clk),
                 .rst                   (rst),
                 .if_write_req          (if_write_req[`DMA_REQUEST_WIDTH-1:0]),
                 .if_write_pos          (if_write_pos[table_entries_ptrwidth-1:0]),
                 .if_write_select       (if_write_select[`DMA_REQMASK_WIDTH-1:0]),
                 .if_write_en           (if_write_en),
                 .if_valid_pos          (if_valid_pos[table_entries_ptrwidth-1:0]),
                 .if_valid_set          (if_valid_set),
                 .if_valid_en           (if_valid_en),
                 .if_validrd_en         (if_validrd_en),
                 .ctrl_read_pos         (ctrl_read_pos[table_entries_ptrwidth-1:0]),
                 .ctrl_done_pos         (ctrl_done_pos[table_entries_ptrwidth-1:0]),
                 .ctrl_done_en          (ctrl_done_en));


   /* lisnoc_dma_initiator AUTO_TEMPLATE(
    .noc_out_\(.*\) (noc_out_req_\1[]),
    .noc_in_\(.*\)  (noc_in_resp_\1[]),
    ); */
   lisnoc_dma_initiator
     #(.tileid(tileid))
   ctrl_initiator(/*AUTOINST*/
                  // Outputs
                  .ctrl_read_pos        (ctrl_read_pos[table_entries_ptrwidth-1:0]),
                  .ctrl_done_pos        (ctrl_done_pos[table_entries_ptrwidth-1:0]),
                  .ctrl_done_en         (ctrl_done_en),
                  .noc_out_flit         (noc_out_req_flit[`FLIT_WIDTH-1:0]), // Templated
                  .noc_out_valid        (noc_out_req_valid),     // Templated
                  .noc_in_ready         (noc_in_resp_ready),     // Templated
                  .wb_req_cyc_o         (wb_req_cyc_o),
                  .wb_req_stb_o         (wb_req_stb_o),
                  .wb_req_we_o          (wb_req_we_o),
                  .wb_req_dat_o         (wb_req_dat_o[31:0]),
                  .wb_req_adr_o         (wb_req_adr_o[31:0]),
                  .wb_req_cti_o         (wb_req_cti_o[2:0]),
                  .wb_req_bte_o         (wb_req_bte_o[1:0]),
                  .wb_req_sel_o         (wb_req_sel_o[3:0]),
                  .wb_resp_cyc_o        (wb_resp_cyc_o),
                  .wb_resp_stb_o        (wb_resp_stb_o),
                  .wb_resp_we_o         (wb_resp_we_o),
                  .wb_resp_dat_o        (wb_resp_dat_o[31:0]),
                  .wb_resp_adr_o        (wb_resp_adr_o[31:0]),
                  .wb_resp_cti_o        (wb_resp_cti_o[2:0]),
                  .wb_resp_bte_o        (wb_resp_bte_o[1:0]),
                  .wb_resp_sel_o        (wb_resp_sel_o[3:0]),
                  // Inputs
                  .clk                  (clk),
                  .rst                  (rst),
                  .ctrl_read_req        (ctrl_read_req[`DMA_REQUEST_WIDTH-1:0]),
                  .valid                (valid[table_entries-1:0]),
                  .noc_out_ready        (noc_out_req_ready),     // Templated
                  .noc_in_flit          (noc_in_resp_flit[`FLIT_WIDTH-1:0]), // Templated
                  .noc_in_valid         (noc_in_resp_valid),     // Templated
                  .wb_req_ack_i         (wb_req_ack_i),
                  .wb_req_dat_i         (wb_req_dat_i[31:0]),
                  .wb_resp_ack_i        (wb_resp_ack_i),
                  .wb_resp_dat_i        (wb_resp_dat_i[31:0]));

   /* lisnoc_dma_target AUTO_TEMPLATE(
    .noc_out_\(.*\) (noc_out_resp_\1[]),
    .noc_in_\(.*\)  (noc_in_req_\1[]),
    .wb_\(.*\)      (wb_target_\1[]),
    ); */
   lisnoc_dma_target
     #(.tileid(tileid),.noc_packet_size(noc_packet_size))
   target(/*AUTOINST*/
          // Outputs
          .noc_out_flit                 (noc_out_resp_flit[`FLIT_WIDTH-1:0]), // Templated
          .noc_out_valid                (noc_out_resp_valid),    // Templated
          .noc_in_ready                 (noc_in_req_ready),      // Templated
          .wb_cyc_o                     (wb_target_cyc_o),       // Templated
          .wb_stb_o                     (wb_target_stb_o),       // Templated
          .wb_we_o                      (wb_target_we_o),        // Templated
          .wb_dat_o                     (wb_target_dat_o[31:0]), // Templated
          .wb_adr_o                     (wb_target_adr_o[31:0]), // Templated
          .wb_sel_o                     (wb_target_sel_o[3:0]),  // Templated
          .wb_cti_o                     (wb_target_cti_o[2:0]),  // Templated
          .wb_bte_o                     (wb_target_bte_o[1:0]),  // Templated
          // Inputs
          .clk                          (clk),
          .rst                          (rst),
          .noc_out_ready                (noc_out_resp_ready),    // Templated
          .noc_in_flit                  (noc_in_req_flit[`FLIT_WIDTH-1:0]), // Templated
          .noc_in_valid                 (noc_in_req_valid),      // Templated
          .wb_ack_i                     (wb_target_ack_i),       // Templated
          .wb_dat_i                     (wb_target_dat_i[31:0])); // Templated


   localparam wb_arb_req = 2'b00, wb_arb_resp = 2'b01, wb_arb_target = 2'b10;

   reg [1:0]                         wb_arb;
   reg [1:0]                         nxt_wb_arb;

   always @(posedge clk) begin
      if (rst) begin
         wb_arb <= wb_arb_target;
      end else begin
         wb_arb <= nxt_wb_arb;
      end
   end

   wire wb_arb_active;
   assign wb_arb_active = ((wb_arb == wb_arb_req) & wb_req_cyc_o) |
                          ((wb_arb == wb_arb_resp) & wb_resp_cyc_o) |
                          ((wb_arb == wb_arb_target) & wb_target_cyc_o);

   always @(*) begin
      if (wb_arb_active) begin
         nxt_wb_arb = wb_arb;
      end else begin
         if (wb_target_cyc_o) begin
            nxt_wb_arb = wb_arb_target;
         end else if (wb_resp_cyc_o) begin
            nxt_wb_arb = wb_arb_resp;
         end else if (wb_req_cyc_o) begin
            nxt_wb_arb = wb_arb_req;
         end else begin
            nxt_wb_arb = wb_arb_target;
         end
      end
   end

   assign wb_cab_o = 1'b0;
   always @(*) begin
      if (wb_arb == wb_arb_target) begin
         wb_adr_o = wb_target_adr_o;
         wb_dat_o = wb_target_dat_o;
         wb_cyc_o = wb_target_cyc_o;
         wb_stb_o = wb_target_stb_o;
         wb_sel_o = wb_target_sel_o;
         wb_we_o = wb_target_we_o;
         wb_bte_o = wb_target_bte_o;
         wb_cti_o = wb_target_cti_o;
         wb_target_ack_i = wb_ack_i;
         wb_target_dat_i = wb_dat_i;
         wb_req_ack_i = 1'b0;
         wb_req_dat_i = 32'hx;
         wb_resp_ack_i = 1'b0;
         wb_resp_dat_i = 32'hx;
      end else if (wb_arb == wb_arb_resp) begin
         wb_adr_o = wb_resp_adr_o;
         wb_dat_o = wb_resp_dat_o;
         wb_cyc_o = wb_resp_cyc_o;
         wb_stb_o = wb_resp_stb_o;
         wb_sel_o = wb_resp_sel_o;
         wb_we_o = wb_resp_we_o;
         wb_bte_o = wb_resp_bte_o;
         wb_cti_o = wb_resp_cti_o;
         wb_resp_ack_i = wb_ack_i;
         wb_resp_dat_i = wb_dat_i;
         wb_req_ack_i = 1'b0;
         wb_req_dat_i = 32'hx;
         wb_target_ack_i = 1'b0;
         wb_target_dat_i = 32'hx;
      end else if (wb_arb == wb_arb_req) begin
         wb_adr_o = wb_req_adr_o;
         wb_dat_o = wb_req_dat_o;
         wb_cyc_o = wb_req_cyc_o;
         wb_stb_o = wb_req_stb_o;
         wb_sel_o = wb_req_sel_o;
         wb_we_o = wb_req_we_o;
         wb_bte_o = wb_req_bte_o;
         wb_cti_o = wb_req_cti_o;
         wb_req_ack_i = wb_ack_i;
         wb_req_dat_i = wb_dat_i;
         wb_resp_ack_i = 1'b0;
         wb_resp_dat_i = 32'hx;
         wb_target_ack_i = 1'b0;
         wb_target_dat_i = 32'hx;
      end else begin // if (wb_arb == wb_arb_req)
         wb_adr_o = 32'h0;
         wb_dat_o = 32'h0;
         wb_cyc_o = 1'b0;
         wb_stb_o = 1'b0;
         wb_sel_o = 4'h0;
         wb_we_o = 1'b0;
         wb_bte_o = 2'b00;
         wb_cti_o = 3'b000;
         wb_req_ack_i = 1'b0;
         wb_req_dat_i = 32'hx;
         wb_resp_ack_i = 1'b0;
         wb_resp_dat_i = 32'hx;
         wb_target_ack_i = 1'b0;
         wb_target_dat_i = 32'hx;
      end
   end
endmodule // lisnoc_dma

`include "lisnoc_dma_undef.vh"
