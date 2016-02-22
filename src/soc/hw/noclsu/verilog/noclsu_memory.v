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
 * REVIEW: Module description missing
 *
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */
module noclsu_memory(/*AUTOARG*/
   // Outputs
   noc_out_flit, noc_out_valid, noc_in_ready, wb_adr_o, wb_bte_o,
   wb_cti_o, wb_cyc_o, wb_dat_o, wb_sel_o, wb_stb_o, wb_we_o,
   // Inputs
   clk, rst, noc_out_ready, noc_in_flit, noc_in_valid, wb_ack_i,
   wb_err_i, wb_rty_i, wb_dat_i
   );

   parameter ID = 0;
   parameter CACHELINESIZE = 4;

   `include "noclsu_nocparms.vh"

   input clk;
   input rst;

   output  [33:0] noc_out_flit;
   output         noc_out_valid;
   input          noc_out_ready;

   input [33:0]      noc_in_flit;
   input             noc_in_valid;
   output reg        noc_in_ready;

   output reg [31:0] wb_adr_o;
   output reg [1:0]  wb_bte_o;
   output reg [2:0]  wb_cti_o;
   output reg        wb_cyc_o;
   output reg [31:0] wb_dat_o;
   output reg [3:0]  wb_sel_o;
   output reg        wb_stb_o;
   output reg        wb_we_o;

   input             wb_ack_i;
   input             wb_err_i;
   input             wb_rty_i;
   input [31:0]      wb_dat_i;

   reg [3:0]     state;
   reg [3:0]     nxt_state;

   reg [31:0]    req_addr;
   reg [31:0]    nxt_req_addr;
   reg           req_size;
   reg           nxt_req_size;
   reg [ph_dest_width-1:0] req_src;
   reg [ph_dest_width-1:0] nxt_req_src;
   reg                     req_type;
   reg                     nxt_req_type;
   reg [3:0]               req_sel;
   reg [3:0]               nxt_req_sel;

   // This intermediate fifo hands the generated flits to the NoC
   // during a read reply. A read request is not served until the
   // packet has completely left the FIFO. As it can hold the complete
   // response the bus interface does not have to care about it later.
   reg                     fifo_valid;
   reg [33:0]              fifo_flit;
   wire                    fifo_ready;

   /* lisnoc_fifo AUTO_TEMPLATE(
    .out_\(.*\) (noc_out_\1),
    .in_\(.*\)  (fifo_\1),
    ); */
   lisnoc_fifo
     #(.LENGTH(CACHELINESIZE+1))
     u_fifo(/*AUTOINST*/
            // Outputs
            .in_ready                   (fifo_ready),            // Templated
            .out_flit                   (noc_out_flit),          // Templated
            .out_valid                  (noc_out_valid),         // Templated
            // Inputs
            .clk                        (clk),
            .rst                        (rst),
            .in_flit                    (fifo_flit),             // Templated
            .in_valid                   (fifo_valid),            // Templated
            .out_ready                  (noc_out_ready));        // Templated

   wire                    fifo_empty;
   assign fifo_empty = ~noc_out_valid;

   reg                     wb_busy;
   reg                     nxt_wb_busy;
   reg [31:0]              wb_wrdata;
   reg [31:0]              nxt_wb_wrdata;

   localparam STATE_IDLE       = 4'd0;
   localparam STATE_RD_GETADDR = 4'd1;
   localparam STATE_RD_GENHDR  = 4'd2;
   localparam STATE_RD_DATA    = 4'd3;
   localparam STATE_WR_GETADDR = 4'd4;
   localparam STATE_WR_DATA    = 4'd5;

   always @(*) begin
      wb_cyc_o = 1'b0;
      wb_stb_o = 1'b0;
      wb_adr_o = 'bx;
      wb_bte_o = 2'b01;
      wb_cti_o = 'bx;
      wb_dat_o = 'bx;
      wb_sel_o = 'bx;
      wb_we_o = 1'b0;

      noc_in_ready = 1'b0;
      fifo_valid = 1'b0;
      fifo_flit = 'bx;

      nxt_state = state;
      nxt_req_addr = req_addr;
      nxt_req_size = req_size;
      nxt_req_src = req_src;
      nxt_req_type = req_type;
      nxt_wb_busy = wb_busy;
      nxt_wb_wrdata = wb_wrdata;
      nxt_req_sel = req_sel;

      case(state)
        STATE_IDLE: begin
           noc_in_ready = 1'b1;
           if (noc_in_flit[32]) begin
              if (noc_in_valid &&
                  (noc_in_flit[ph_class_msb:ph_class_lsb] == ph_lsu_class)) begin
                 // A new packet arrives
                 if (noc_in_flit[ph_msgtype_msb:ph_msgtype_lsb] == msgtype_readreq) begin
                    // This is a read request
                    nxt_state = STATE_RD_GETADDR;
                    nxt_req_size = noc_in_flit[ph_size_msb:ph_size_lsb];
                    nxt_req_src = noc_in_flit[ph_src_msb:ph_src_lsb];
                    nxt_req_type = noc_in_flit[ph_type_msb:ph_type_lsb];
                 end else if (noc_in_flit[ph_msgtype_msb:ph_msgtype_lsb] == msgtype_writereq) begin
                    nxt_state = STATE_WR_GETADDR;
                    nxt_req_size = noc_in_flit[ph_size_msb:ph_size_lsb];
                    nxt_req_src = noc_in_flit[ph_src_msb:ph_src_lsb];
                    nxt_req_type = noc_in_flit[ph_type_msb:ph_type_lsb];
                    nxt_req_sel = noc_in_flit[ph_bsel_msb:ph_bsel_lsb];
                 end
              end
           end
        end // case: STATE_IDLE
        STATE_RD_GETADDR: begin
           noc_in_ready = fifo_empty;
           if (noc_in_valid && noc_in_ready) begin
              nxt_req_addr = noc_in_flit[31:0];
              nxt_state = STATE_RD_GENHDR;
           end
        end
        STATE_RD_GENHDR: begin
           // We are sure this will be directly accepted (fifo empty)
           fifo_valid = 1'b1;
           fifo_flit[33:32] = 2'b01;
           fifo_flit[ph_dest_msb:ph_dest_lsb] = req_src;
           fifo_flit[ph_class_msb:ph_class_lsb] = ph_lsu_class;
           fifo_flit[ph_src_msb:ph_src_lsb] = ID;
           fifo_flit[ph_msgtype_msb:ph_msgtype_lsb] = msgtype_readresp;
           fifo_flit[ph_type_msb:ph_type_lsb] = req_type;
           fifo_flit[ph_size_msb:ph_size_lsb] = req_size;
           // We already set up the first request, the remain will be
           // handled in the later state
           wb_adr_o = req_addr;
           wb_cyc_o = 1'b1;
           wb_stb_o = 1'b1;
           wb_sel_o = 4'hf;
           wb_we_o = 1'b0;
           wb_cti_o = (req_size==size_single) ? 3'b111 : 3'b010;
           nxt_state = STATE_RD_DATA;
        end // case: STATE_RD_GENHDR
        STATE_RD_DATA: begin
           wb_adr_o = req_addr;
           wb_cyc_o = 1'b1;
           wb_stb_o = 1'b1;
           wb_sel_o = 4'hf;
           wb_we_o = 1'b0;
           wb_cti_o = (req_size==size_single) ? 3'b111 : 3'b010;
           if (wb_ack_i) begin
              if (req_size==size_single) begin
                 fifo_valid = 1'b1;
                 fifo_flit = {2'b10,wb_dat_i};
                 nxt_state = STATE_IDLE;
              end else begin
                 // TODO: Burst
              end
           end
        end // case: STATE_RD_DATA
        STATE_WR_GETADDR: begin
           noc_in_ready = 1'b1;
           if (noc_in_valid) begin
              nxt_req_addr = noc_in_flit[31:0];
              nxt_state = STATE_WR_DATA;
           end
        end
        STATE_WR_DATA: begin
           wb_adr_o = req_addr;
           wb_we_o = 1'b1;
           wb_cti_o = 3'b111;
           wb_sel_o = req_sel;
           if (wb_busy) begin
              wb_dat_o = wb_wrdata;
              wb_cyc_o = 1'b1;
              wb_stb_o = 1'b1;
              if (wb_ack_i) begin
                 nxt_wb_busy = 1'b0;
                 nxt_state = STATE_IDLE;
              end
           end else begin
              noc_in_ready = 1'b1;
              if (noc_in_valid) begin
                 wb_cyc_o = 1'b1;
                 wb_stb_o = 1'b1;
                 wb_dat_o = noc_in_flit[31:0];
                 nxt_wb_wrdata = noc_in_flit[31:0];
                 nxt_wb_busy = 1'b1;
              end
           end
        end
        default: begin
           nxt_state = STATE_IDLE;
        end
      endcase
   end

   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         req_size <= 'bx;
         req_addr <= 'bx;
         req_src <= 'bx;
         req_type <= 'bx;
         req_sel <= 'bx;
         wb_busy <= 1'b0;
         wb_wrdata <= 'bx;
      end else begin
         state <= nxt_state;
         req_size <= nxt_req_size;
         req_addr <= nxt_req_addr;
         req_src <= nxt_req_src;
         req_type <= nxt_req_type;
         req_sel <= nxt_req_sel;
         wb_busy <= nxt_wb_busy;
         wb_wrdata <= nxt_wb_wrdata;
      end
   end

endmodule // noclsu_memory

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/router")
// verilog-auto-inst-param-value: t
// End:
