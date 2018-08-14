// Copyright 2016-2018 by the authors
//
// Copyright and related rights are licensed under the Solderpad
// Hardware License, Version 0.51 (the "License"); you may not use
// this file except in compliance with the License. You may obtain a
// copy of the License at http://solderpad.org/licenses/SHL-0.51.
// Unless required by applicable law or agreed to in writing,
// software, hardware and materials distributed under this License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the
// License.
//
// Authors:
//    Philipp Wagner <philipp.wagner@tum.de>
//    Stefan Wallentowitz <stefan@wallentowitz.de>
//    Max Koenen <max.koenen@tum.de>

import dii_package::dii_flit;

/**
 * Module: osd_regaccess
 *
 * Handle register accesses to a debug modules. Accesses to the base registers
 * (0x0000 - 0x01ff) are answered directly by this module, all other accesses
 * are available on the reg_* ports to be handled by the debug module itself.
 */
module osd_regaccess
  #(parameter MOD_VENDOR = 'x,     // module vendor
    parameter MOD_TYPE = 'x,       // module type
    parameter MOD_VERSION = 'x,    // module version
    parameter MOD_EVENT_DEST_DEFAULT = 'x, // default event destination
    parameter CAN_STALL = 0,
    parameter MAX_REG_SIZE = 16)
   (input clk, rst,

    input [15:0]   id,

    input dii_flit debug_in, output logic debug_in_ready,
    output dii_flit debug_out, input debug_out_ready,

    output reg                   reg_request,
    output                       reg_write,
    output [15:0]                reg_addr,
    output [1:0]                 reg_size,
    output [MAX_REG_SIZE-1:0]    reg_wdata,
    input                        reg_ack,
    input                        reg_err,
    input [MAX_REG_SIZE-1:0]     reg_rdata,

    output [15:0]                event_dest,
    output                       stall);

   localparam ACCESS_SIZE_16  = 2'b00;
   localparam ACCESS_SIZE_32  = 2'b01;
   localparam ACCESS_SIZE_64  = 2'b10;
   localparam ACCESS_SIZE_128 = 2'b11;

   // ensure that parameters are set to allowed values
   initial begin
      if (MAX_REG_SIZE != 16 && MAX_REG_SIZE != 32 && MAX_REG_SIZE != 64 && MAX_REG_SIZE != 128) begin
         $fatal("osd_regaccess: MAX_REG_SIZE must be set to '16', '32', '64', or '128'!");
      end
   end

   localparam MAX_REQ_SIZE = MAX_REG_SIZE == 16 ? ACCESS_SIZE_16 :
      MAX_REG_SIZE == 32 ? ACCESS_SIZE_32 :
      MAX_REG_SIZE == 64 ? ACCESS_SIZE_64 : ACCESS_SIZE_128;

   // base register addresses
   localparam REG_MOD_VENDOR     = 16'h0;
   localparam REG_MOD_TYPE       = 16'h1;
   localparam REG_MOD_VERSION    = 16'h2;
   localparam REG_MOD_CS         = 16'h3;
   localparam REG_MOD_CS_ACTIVE  = 0;
   localparam REG_MOD_EVENT_DEST = 16'h4;

   // Registers
   reg          mod_cs_active;
   logic        nxt_mod_cs_active;
   reg [15:0]   mod_event_dest;
   reg [15:0]   nxt_mod_event_dest;

   assign stall = CAN_STALL ? ~mod_cs_active : 1'b0;
   assign event_dest = mod_event_dest;

   // State machine
   enum {
         STATE_IDLE, STATE_REQ_HDR_SRC, STATE_REQ_HDR_FLAGS, STATE_ADDR,
         STATE_WRITE, STATE_RESP_HDR_DEST, STATE_RESP_HDR_SRC,
         STATE_RESP_HDR_FLAGS, STATE_RESP_VALUE, STATE_DROP, STATE_EXT_START,
         STATE_EXT_WAIT
         } state, nxt_state;

   // Local request/response data
   reg                      req_write;
   reg [1:0]                req_size;
   reg [2:0]                word_it;
   reg [15:0]               req_addr;
   reg [MAX_REG_SIZE-1:0]   reqresp_value;
   reg [15:0]               resp_dest;
   reg                      resp_error;
   logic                    nxt_req_write;
   logic [1:0]              nxt_req_size;
   logic [2:0]              nxt_word_it;
   logic [15:0]             nxt_req_addr;
   logic [MAX_REG_SIZE-1:0] nxt_reqresp_value;
   logic [15:0]             nxt_resp_dest;
   logic                    nxt_resp_error;

   logic                    reg_addr_is_ext;
   logic [8:0]              reg_addr_internal;
   // handle the base addresses 0x0000 - 0x01ff as "internal"
   assign reg_addr_is_ext = (debug_in.data[15:9] != 0);
   assign reg_addr_internal = debug_in.data[8:0];

   assign reg_write = req_write;
   assign reg_addr = req_addr;
   assign reg_size = req_size;
   assign reg_wdata = reqresp_value;

   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         mod_cs_active <= 0;
         mod_event_dest <= 16'(MOD_EVENT_DEST_DEFAULT);
         req_write <= 0;
         req_addr <= 0;
         req_size <= 0;
         reqresp_value <= 0;
      end else begin
         state <= nxt_state;
         mod_cs_active <= nxt_mod_cs_active;
         mod_event_dest <= nxt_mod_event_dest;
         req_write <= nxt_req_write;
         req_addr <= nxt_req_addr;
         req_size <= nxt_req_size;
         reqresp_value <= nxt_reqresp_value;
      end
      resp_dest <= nxt_resp_dest;
      resp_error <= nxt_resp_error;
      word_it <= nxt_word_it;
   end

   always @(*) begin
      nxt_state = state;

      nxt_req_write = req_write;
      nxt_req_size = req_size;
      nxt_word_it = word_it;
      nxt_req_addr = req_addr;
      nxt_resp_dest = resp_dest;
      nxt_reqresp_value = reqresp_value;
      nxt_resp_error = resp_error;

      nxt_mod_cs_active = mod_cs_active;
      nxt_mod_event_dest = mod_event_dest;

      debug_in_ready = 0;
      debug_out = 0;

      reg_request = 0;

      case (state)
        STATE_IDLE: begin
           debug_in_ready = 1;
           if (debug_in.valid) begin
              nxt_state = STATE_REQ_HDR_SRC;
           end
        end
        STATE_REQ_HDR_SRC: begin
           debug_in_ready = 1;
           nxt_resp_dest = debug_in.data[15:0];
           nxt_resp_error = 0;
           nxt_state = STATE_REQ_HDR_FLAGS;
        end
        STATE_REQ_HDR_FLAGS: begin
           debug_in_ready = 1;
           nxt_req_write = debug_in.data[12];
           nxt_req_size = debug_in.data[11:10];
           if (MAX_REQ_SIZE < debug_in.data[11:10]) begin
              nxt_resp_error = 1;
              nxt_state = STATE_DROP;
           end else begin
              case (debug_in.data[11:10])
                 ACCESS_SIZE_16: nxt_word_it = 0;
                 ACCESS_SIZE_32: nxt_word_it = 1;
                 ACCESS_SIZE_64: nxt_word_it = 3;
                 ACCESS_SIZE_128: nxt_word_it = 7;
              endcase

              if (debug_in.valid) begin
                 if (|debug_in.data[15:14]) begin
                    nxt_state = STATE_DROP;
                 end else begin
                    nxt_state = STATE_ADDR;
                 end
              end
           end
        end
        STATE_ADDR: begin
           debug_in_ready = 1;

           if (reg_addr_is_ext) begin
              nxt_req_addr = debug_in.data;
              if (debug_in.valid) begin
                 if (req_write) begin
                    nxt_reqresp_value = 0;
                    nxt_state = STATE_WRITE;
                 end else begin
                    nxt_state = STATE_EXT_START;
                 end
              end
           end else begin
              if (req_write) begin
                 // LOCAL WRITE
                 if (req_size != ACCESS_SIZE_16) begin
                    // only 16 bit writes are supported for local writes
                    nxt_resp_error = 1;
                 end else begin
                    nxt_req_addr = debug_in.data;
                    case (debug_in.data)
                      REG_MOD_EVENT_DEST: nxt_resp_error = 0;
                      REG_MOD_CS: nxt_resp_error = 0;
                      default: nxt_resp_error = 1;
                    endcase // case (debug_in.data)
                 end
              end else begin // if (nxt_req_write)
                 // LOCAL READ
                 case (debug_in.data)
                    REG_MOD_VENDOR: nxt_reqresp_value = 16'(MOD_VENDOR);
                    REG_MOD_TYPE: nxt_reqresp_value = 16'(MOD_TYPE);
                    REG_MOD_VERSION: nxt_reqresp_value = 16'(MOD_VERSION);
                    REG_MOD_CS: nxt_reqresp_value = {15'h0, ~stall};
                    REG_MOD_EVENT_DEST: nxt_reqresp_value = mod_event_dest;
                    default: nxt_resp_error = 1;
                 endcase // case (debug_in.data)
              end

              if (debug_in.valid) begin
                 if (req_write) begin
                    if (debug_in.last) begin
                       nxt_resp_error = 1;
                       nxt_state = STATE_RESP_HDR_DEST;
                    end else if (nxt_resp_error) begin
                       nxt_state = STATE_DROP;
                    end else begin
                       nxt_reqresp_value = 0;
                       nxt_state = STATE_WRITE;
                    end
                 end else begin
                    if (debug_in.last) begin
                       nxt_state = STATE_RESP_HDR_DEST;
                    end else begin
                       nxt_state = STATE_DROP;
                    end
                 end
              end
           end
        end // case: STATE_ADDR
        STATE_WRITE: begin
           debug_in_ready = 1;

           if (debug_in.valid) begin
              if (req_addr[15:9] != 0) begin
                 nxt_reqresp_value = (reqresp_value & ~(16'hffff << word_it*16)) | (debug_in.data << word_it*16);
                 if (word_it == 0) begin
                    if (debug_in.last)
                      nxt_state = STATE_EXT_START;
                    else
                      nxt_state = STATE_DROP;
                 end else begin
                    if (debug_in.last) begin
                       nxt_resp_error = 1;
                       nxt_state = STATE_RESP_HDR_DEST;
                    end else
                       nxt_word_it = word_it - 1;
                 end
              end else begin
                 nxt_reqresp_value = debug_in.data;
                 case (req_addr)
                    REG_MOD_CS: begin
                       nxt_mod_cs_active = debug_in.data[REG_MOD_CS_ACTIVE];
                       nxt_resp_error = 0;
                    end
                    REG_MOD_EVENT_DEST: begin
                       nxt_mod_event_dest = debug_in.data;
                       nxt_resp_error = 0;
                    end
                 endcase // case (req_addr)

                 if (debug_in.last) begin
                    nxt_state = STATE_RESP_HDR_DEST;
                 end else begin
                    nxt_state = STATE_DROP;
                 end
              end
           end
        end
        STATE_RESP_HDR_DEST: begin
           debug_out.valid = 1;
           debug_out.data = resp_dest;

           if (debug_out_ready) begin
              nxt_state = STATE_RESP_HDR_SRC;
           end
        end
        STATE_RESP_HDR_SRC: begin
           debug_out.valid = 1;
           debug_out.data = id;

           if (debug_out_ready) begin
              nxt_state = STATE_RESP_HDR_FLAGS;
           end
        end
        STATE_RESP_HDR_FLAGS: begin
           debug_out.valid = 1;
           debug_out.data[9:0] = 10'h0; // reserved

           debug_out.data[15:14] = 2'b00; // TYPE == REG

           // TYPE_SUB
           if (req_write) begin
              if (resp_error) begin
                 debug_out.data[13:10] = 4'b1111; // RESP_WRITE_REG_ERROR
              end else begin
                 debug_out.data[13:10] = 4'b1110; // RESP_WRITE_REG_SUCCESS
              end
           end else begin
              if (resp_error) begin
                 debug_out.data[13:10] = 4'b1100; // RESP_READ_REG_ERROR
              end else begin
                 debug_out.data[13:10] = {2'b10, req_size}; // RESP_READ_REG_SUCCESS_*
              end
           end

           debug_out.last = resp_error | req_write;

           if (debug_out_ready) begin
              if (resp_error | req_write) begin
                 nxt_state = STATE_IDLE;
              end else begin
                 nxt_state = STATE_RESP_VALUE;
              end
           end
        end
        STATE_RESP_VALUE: begin
           debug_out.valid = 1;
           debug_out.data = reqresp_value >> word_it*16;
           if (debug_out_ready) begin
              if (word_it == 0) begin
                 debug_out.last = 1;
                 nxt_state = STATE_IDLE;
              end else
                 nxt_word_it = word_it - 1;
           end
        end

        STATE_EXT_START: begin
           reg_request = 1;
           if (reg_ack | reg_err) begin
              nxt_reqresp_value = reg_rdata;
              nxt_resp_error = reg_err;
              nxt_state = STATE_RESP_HDR_DEST;
           end
        end

        STATE_DROP: begin
           debug_in_ready = 1;
           if (debug_in.valid & debug_in.last) begin
              nxt_state = STATE_RESP_HDR_DEST;
           end
        end
      endcase // case (state)
   end

endmodule
