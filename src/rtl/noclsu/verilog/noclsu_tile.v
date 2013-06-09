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
 * REVIEW: Module description missing
 *
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */
module noclsu_tile(/*AUTOARG*/
   // Outputs
   noc_out_flit, noc_out_valid, noc_in_ready, dc_dat_o, dc_ack_o,
   dc_err_o, dc_rty_o, ic_dat_o, ic_ack_o, ic_err_o, ic_rty_o,
   // Inputs
   clk, rst, noc_out_ready, noc_in_flit, noc_in_valid, dc_dat_i,
   dc_adr_i, dc_cyc_i, dc_stb_i, dc_we_i, dc_sel_i, dc_bte_i,
   dc_cti_i, ic_dat_i, ic_adr_i, ic_cyc_i, ic_stb_i, ic_we_i,
   ic_sel_i, ic_bte_i, ic_cti_i
   );

   parameter TILEID = 0;
   parameter MEMORYID = 2;

   parameter CACHELINESIZE = 4; /* REVIEW: not used? */

   parameter PGAS_ENABLE = 1;
   parameter PGAS_ADDRW = 15;
   parameter PGAS_BASE = 0;

   `include "noclsu_nocparms.vh"

   input clk;
   input rst;

   output reg [33:0] noc_out_flit;
   output reg        noc_out_valid;
   input             noc_out_ready;

   input [33:0]  noc_in_flit;
   input         noc_in_valid;
   output reg    noc_in_ready;

   input [31:0]  dc_dat_i;
   input [31:0]  dc_adr_i;
   input         dc_cyc_i;
   input         dc_stb_i;
   input         dc_we_i;
   input [3:0]   dc_sel_i;
   input [1:0]   dc_bte_i;
   input [2:0]   dc_cti_i;
   output reg [31:0] dc_dat_o;
   output reg       dc_ack_o;
   output           dc_err_o;
   output           dc_rty_o;

   input [31:0]  ic_dat_i;
   input [31:0]  ic_adr_i;
   input         ic_cyc_i;
   input         ic_stb_i;
   input         ic_we_i;
   input [3:0]   ic_sel_i;
   input [1:0]   ic_bte_i;
   input [2:0]   ic_cti_i;
   output reg [31:0] ic_dat_o;
   output reg        ic_ack_o;
   output            ic_err_o;
   output            ic_rty_o;

   assign {dc_rty_o,ic_rty_o} = {1'b0,1'b0};
   assign {dc_err_o,ic_err_o} = {1'b0,1'b0};

   wire [31:0]       dc_adr_transl;
   wire [31:0]       ic_adr_transl;

   generate
      if (PGAS_ENABLE) begin
         assign dc_adr_transl[31:32-PGAS_ADDRW] = PGAS_BASE;
         assign dc_adr_transl[31-PGAS_ADDRW:0] = dc_adr_i[31-PGAS_ADDRW:0];
         assign ic_adr_transl[31:32-PGAS_ADDRW] = PGAS_BASE;
         assign ic_adr_transl[31-PGAS_ADDRW:0] = ic_adr_i[31-PGAS_ADDRW:0];
      end else begin
         assign dc_adr_transl = dc_adr_i;
         assign ic_adr_transl = ic_adr_i;
      end
   endgenerate


   reg [3:0]     state;
   reg [3:0]     nxt_state;

   reg           status_ic;
   reg           nxt_status_ic;
   reg           status_dc;
   reg           nxt_status_dc;

   // State machine
   localparam STATE_IDLE         = 4'd0;
   localparam STATE_RESP         = 4'd1;
   localparam STATE_ICREQ_HDR    = 4'd2;
   localparam STATE_ICREQ_ADDR   = 4'd3;
   localparam STATE_READRESP     = 4'd4;
   localparam STATE_DCRDREQ_HDR  = 4'd5;
   localparam STATE_DCRDREQ_ADDR = 4'd6;
   localparam STATE_DCWRREQ_HDR  = 4'd7;
   localparam STATE_DCWRREQ_ADDR = 4'd8;
   localparam STATE_DCWRREQ_DATA = 4'd9;

   // Status
   localparam STATUS_IDLE = 1'b0;
   localparam STATUS_BUSY = 1'b1;

   reg           resp_type;
   reg           nxt_resp_type;

   always @(*) begin
      noc_in_ready  = 1'b0;
      noc_out_flit = 34'h0;
      noc_out_valid = 1'b0;

      ic_ack_o = 1'b0;
      dc_ack_o = 1'b0;
      ic_dat_o = 'bx;
      dc_dat_o = 'bx;

      nxt_resp_type = resp_type;
      nxt_state = state;
      nxt_status_ic = status_ic;
      nxt_status_dc = status_dc;

      case(state)
        STATE_IDLE: begin
           noc_in_ready = 1'b1;
           if (noc_in_valid) begin
              if (noc_in_flit[ph_msgtype_msb:ph_msgtype_lsb]==msgtype_readresp) begin
                 nxt_resp_type = noc_in_flit[ph_type_msb:ph_type_lsb];
                 nxt_state = STATE_READRESP;
              end
           end else begin
              if ((status_ic==STATUS_IDLE) &&
                  (ic_cyc_i && ic_stb_i)) begin
                 nxt_state = STATE_ICREQ_HDR;
              end else if ((status_dc==STATUS_IDLE) &&
                           (dc_cyc_i && dc_stb_i)) begin
                 if (dc_we_i) begin
                    nxt_state = STATE_DCWRREQ_HDR;
                 end else begin
                    nxt_state = STATE_DCRDREQ_HDR;
                 end
              end
           end
        end // case: IDLE
        STATE_ICREQ_HDR: begin
           noc_out_flit[33:32] = 2'b01;
           noc_out_flit[ph_dest_msb:ph_dest_lsb] = MEMORYID;
           noc_out_flit[ph_src_msb:ph_src_lsb] = TILEID;
           noc_out_flit[ph_class_msb:ph_class_lsb] = ph_lsu_class;
           noc_out_flit[ph_msgtype_msb:ph_msgtype_lsb] = msgtype_readreq;
           noc_out_flit[ph_type_msb:ph_type_lsb] = type_i;
           noc_out_flit[ph_size_msb:ph_size_lsb] = (ic_cti_i==3'b001) ? size_burst : size_single;
           noc_out_valid = 1'b1;
           if (noc_out_ready) begin
              nxt_state = STATE_ICREQ_ADDR;
           end
        end // case: STATE_ICREQ_HDR
        STATE_ICREQ_ADDR: begin
           noc_out_flit[33:32] = 2'b10;
           noc_out_flit[31:0] = ic_adr_transl;
           noc_out_valid = 1'b1;
           if (noc_out_ready) begin
              nxt_status_ic = STATUS_BUSY;
              nxt_state = STATE_IDLE;
           end
        end
        STATE_DCRDREQ_HDR: begin
           noc_out_flit[33:32] = 2'b01;
           noc_out_flit[ph_dest_msb:ph_dest_lsb] = MEMORYID;
           noc_out_flit[ph_src_msb:ph_src_lsb] = TILEID;
           noc_out_flit[ph_class_msb:ph_class_lsb] = ph_lsu_class;
           noc_out_flit[ph_msgtype_msb:ph_msgtype_lsb] = msgtype_readreq;
           noc_out_flit[ph_type_msb:ph_type_lsb] = type_d;
           noc_out_flit[ph_size_msb:ph_size_lsb] = (dc_cti_i==3'b001) ? size_burst : size_single;
           noc_out_valid = 1'b1;
           if (noc_out_ready) begin
              nxt_state = STATE_DCRDREQ_ADDR;
           end
        end
        STATE_DCRDREQ_ADDR: begin
           noc_out_flit[33:32] = 2'b10;
           noc_out_flit[31:0] = dc_adr_transl;
           noc_out_valid = 1'b1;
           if (noc_out_ready) begin
              nxt_status_dc = STATUS_BUSY;
              nxt_state = STATE_IDLE;
           end
        end
        STATE_READRESP: begin
           noc_in_ready = 1'b1;
           if (resp_type==type_i) begin
              ic_dat_o = noc_in_flit[31:0];
           end else begin
              dc_dat_o = noc_in_flit[31:0];
           end
           if (noc_in_valid) begin
              if (resp_type==type_i) begin
                 ic_ack_o = 1'b1;
                 nxt_status_ic = STATUS_IDLE;
              end else begin
                 dc_ack_o = 1'b1;
                 nxt_status_dc = STATUS_IDLE;
              end
              nxt_state = STATE_IDLE;
           end
        end
        STATE_DCWRREQ_HDR: begin
           noc_out_flit[33:32] = 2'b01;
           noc_out_flit[ph_dest_msb:ph_dest_lsb] = MEMORYID;
           noc_out_flit[ph_src_msb:ph_src_lsb] = TILEID;
           noc_out_flit[ph_class_msb:ph_class_lsb] = ph_lsu_class;
           noc_out_flit[ph_msgtype_msb:ph_msgtype_lsb] = msgtype_writereq;
           noc_out_flit[ph_type_msb:ph_type_lsb] = type_d;
           noc_out_flit[ph_size_msb:ph_size_lsb] = (dc_cti_i==3'b001) ? size_burst : size_single;
           noc_out_flit[ph_bsel_msb:ph_bsel_lsb] = dc_sel_i;
           noc_out_valid = 1'b1;
           if (noc_out_ready) begin
              nxt_state = STATE_DCWRREQ_ADDR;
           end
        end // case: STATE_DCWRREQ_HDR
        STATE_DCWRREQ_ADDR: begin
           noc_out_flit[33:32] = 2'b00;
           noc_out_flit[31:0] = dc_adr_transl;
           noc_out_valid = 1'b1;
           if (noc_out_ready) begin
              nxt_state = STATE_DCWRREQ_DATA;
           end
        end
        STATE_DCWRREQ_DATA: begin
           noc_out_flit[33:32] = 2'b10;
           noc_out_flit[31:0] = dc_dat_i;
           noc_out_valid = 1'b1;
           if (noc_out_ready) begin
              dc_ack_o = 1'b1;
              nxt_state = STATE_IDLE;
           end
        end
        default: begin
           nxt_state = STATE_IDLE;
           nxt_status_ic = STATUS_IDLE;
           nxt_status_dc = STATUS_IDLE;
           nxt_resp_type = 'bx;
        end
      endcase
   end

   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         status_ic <= STATUS_IDLE;
         status_dc <= STATUS_IDLE;
         resp_type <= 'bx;
      end else begin
         state <= nxt_state;
         status_ic <= nxt_status_ic;
         status_dc <= nxt_status_dc;
         resp_type <= nxt_resp_type;
      end
   end


endmodule // noclsu_tile
