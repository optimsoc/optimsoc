// Copyright 2016 by the authors
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
//    Stefan Wallentowitz <stefan@wallentowitz.de>

import dii_package::dii_flit;

module osd_regaccess
  #(parameter MODID = 'x,
    parameter MODVERSION = 'x,
    parameter CAN_STALL = 0,
    parameter MAX_REG_SIZE = 16)
   (input clk, rst,

    input [9:0]   id,

    input dii_flit debug_in, output logic debug_in_ready,
    output dii_flit debug_out, input debug_out_ready,

    output reg    reg_request,
    output        reg_write,
    output [15:0] reg_addr,
    output [1:0]  reg_size,
    output [15:0] reg_wdata,
    input         reg_ack,
    input         reg_err,
    input [15:0]  reg_rdata,

    output        stall);

   localparam REQ_SIZE_16  = 2'b00;
   localparam REQ_SIZE_32  = 2'b01;
   localparam REQ_SIZE_64  = 2'b10;
   localparam REQ_SIZE_128 = 2'b11;

   localparam REG_MODID   = 0;
   localparam REG_VERSION = 1;
   localparam REG_CS      = 3;

   localparam CS_STALL = 5'h1;

   // Registers
   reg          mod_cs_stall;
   logic        nxt_mod_cs_stall;

   assign stall = CAN_STALL ? mod_cs_stall : 0;

   // State machine
   enum {
         STATE_IDLE, STATE_START, STATE_ADDR, STATE_WRITE,
         STATE_RESP_START, STATE_RESP_SRC, STATE_RESP_VALUE,
         STATE_DROP, STATE_EXT_START, STATE_EXT_WAIT
         } state, nxt_state;

   // Local request/response data
   reg                      req_write;
   reg [1:0]                req_size;
   reg [15:0]               req_addr;
   reg [MAX_REG_SIZE-1:0]   reqresp_value;
   reg [9:0]                resp_dest;
   reg                      resp_error;
   logic                    nxt_req_write;
   logic [1:0]              nxt_req_size;
   logic [15:0]             nxt_req_addr;
   logic [MAX_REG_SIZE-1:0] nxt_reqresp_value;
   logic [9:0]              nxt_resp_dest;
   logic                    nxt_resp_error;

   logic                    addr_is_ext;
   logic [8:0]  addr_internal;
   assign addr_is_ext = (debug_in.data[15:9] != 0);
   assign addr_internal = debug_in.data[8:0];

   assign reg_write = req_write;
   assign reg_addr = req_addr;
   assign reg_size = req_size;
   assign reg_wdata = reqresp_value;

   always @(posedge clk) begin
      if (rst) begin
         state <= STATE_IDLE;
         mod_cs_stall <= 1;
      end else begin
         state <= nxt_state;
         mod_cs_stall <= nxt_mod_cs_stall;
      end
      resp_dest <= nxt_resp_dest;
      reqresp_value <= nxt_reqresp_value;
      resp_error <= nxt_resp_error;
      req_write <= nxt_req_write;
      req_size <= nxt_req_size;
      req_addr <= nxt_req_addr;
   end

   always @(*) begin
      nxt_state = state;

      nxt_req_write = req_write;
      nxt_req_size = req_size;
      nxt_req_addr = req_addr;
      nxt_resp_dest = resp_dest;
      nxt_reqresp_value = reqresp_value;
      nxt_resp_error = resp_error;

      nxt_mod_cs_stall = mod_cs_stall;

      debug_in_ready = 0;
      debug_out = 0;

      reg_request = 0;

      case (state)
        STATE_IDLE: begin
           debug_in_ready = 1;
           if (debug_in.valid) begin
              nxt_state = STATE_START;
           end
        end
        STATE_START: begin
           debug_in_ready = 1;
           nxt_req_write = (debug_in.data[12]);
           nxt_req_size = debug_in.data[11:10];
           nxt_resp_dest = debug_in.data[9:0];
           nxt_resp_error = 0;

           if (debug_in.valid) begin
              if (|debug_in.data[15:14]) begin
                 nxt_state = STATE_DROP;
              end else begin
                 nxt_state = STATE_ADDR;
              end
           end
        end // case: STATE_START
        STATE_ADDR: begin
           debug_in_ready = 1;

           if (addr_is_ext) begin
              nxt_req_addr = debug_in.data;
              if (debug_in.valid) begin
                 if (req_write)
                   nxt_state = STATE_WRITE;
                 else
                   nxt_state = STATE_EXT_START;
              end
           end else begin
              if (req_write) begin
                 // LOCAL WRITE
                 if (req_size != REQ_SIZE_16) begin
                    nxt_resp_error = 1;
                 end else begin
                    nxt_req_addr = debug_in.data;
                    case (debug_in.data)
                      REG_CS: nxt_resp_error = 0;
                      default: nxt_resp_error = 1;
                    endcase // case (debug_in.data)
                 end
              end else begin // if (nxt_req_write)
                 // LOCAL READ
                 case (debug_in.data)
                   REG_MODID: nxt_reqresp_value = 16'(MODID);
                   REG_VERSION: nxt_reqresp_value = 16'(MODVERSION);
                   default: nxt_resp_error = 1;
                 endcase // case (debug_in.data)
              end

              if (debug_in.valid) begin
                 if (req_write) begin
                    if (debug_in.last) begin
                       nxt_resp_error = 1;
                       nxt_state = STATE_RESP_START;
                    end else if (nxt_resp_error) begin
                       nxt_state = STATE_RESP_START;
                    end else begin
                       nxt_state = STATE_WRITE;
                    end
                 end else begin
                    if (debug_in.last) begin
                       nxt_state = STATE_RESP_START;
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
              nxt_reqresp_value = debug_in.data;
              if (req_addr[15:9] != 0) begin
                 if (debug_in.last)
                   nxt_state = STATE_EXT_START;
                 else
                   nxt_state = STATE_DROP;
              end else begin
                 case (req_addr)
                   REG_CS: begin
                      if (debug_in.data[15:11] === CS_STALL) begin
                         if (!CAN_STALL) begin
                            nxt_resp_error = 1;
                         end else begin
                            nxt_mod_cs_stall = debug_in.data[0];
                         end
                      end else begin
                         nxt_resp_error = 1;
                      end
                   end
                 endcase // case (req_addr)

                 if (debug_in.last) begin
                    nxt_state = STATE_RESP_START;
                 end else begin
                    nxt_state = STATE_DROP;
                 end
              end
           end
        end
        STATE_RESP_START: begin
           debug_out.valid = 1;
           debug_out.data = {6'h0, resp_dest};

           if (debug_out_ready) begin
              nxt_state = STATE_RESP_SRC;
           end
        end
        STATE_RESP_SRC: begin
           debug_out.valid = 1;
           debug_out.data = {4'h0, req_write, resp_error, 10'(id)};

           debug_out.last = resp_error | req_write;

           if (debug_out_ready) begin
              if (req_write) begin
                 nxt_state = STATE_IDLE;
              end else begin
                 nxt_state = STATE_RESP_VALUE;
              end
           end
        end
        STATE_RESP_VALUE: begin
           debug_out.valid = 1;
           debug_out.data = reqresp_value[15:0];
           debug_out.last = 1;
           if (debug_out_ready) begin
              nxt_state = STATE_IDLE;
           end
        end

        STATE_EXT_START: begin
           reg_request = 1;
           if (reg_ack | reg_err) begin
              nxt_reqresp_value = reg_rdata;
              nxt_resp_error = reg_err;
              nxt_state = STATE_RESP_START;
           end
        end

        STATE_DROP: begin
           debug_in_ready = 1;
           if (debug_in.valid & debug_in.last) begin
              nxt_state = STATE_IDLE;
           end
        end
      endcase // case (state)
   end

endmodule
