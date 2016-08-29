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

module osd_stm
  #(
    parameter REG_ADDR_WIDTH = 5, // the address width of the core register file
    parameter XLEN = 64
    )
   (
    input                       clk, rst,

    input [9:0]                 id,

    input dii_flit              debug_in,
    output                      debug_in_ready,
    output dii_flit             debug_out,
    input                       debug_out_ready,

    input                       trace_valid,
    input [15:0]                trace_id,
    input [XLEN-1:0]            trace_value,

    output                      trace_reg_enable,
    output [REG_ADDR_WIDTH-1:0] trace_reg_addr
    );

   logic        reg_request;
   logic        reg_write;
   logic [15:0] reg_addr;
   logic [1:0]  reg_size;
   logic [15:0] reg_wdata;
   logic        reg_ack;
   logic        reg_err;
   logic [15:0] reg_rdata;

   logic                   stall;

   dii_flit dp_out, dp_in;
   logic        dp_out_ready, dp_in_ready;

   osd_regaccess_layer
     #(.MODID(16'h4), .MODVERSION(16'h0),
       .MAX_REG_SIZE(16), .CAN_STALL(1))
   u_regaccess(.*,
               .module_in (dp_out),
               .module_in_ready (dp_out_ready),
               .module_out (dp_in),
               .module_out_ready (dp_in_ready));

   always @(*) begin
      reg_ack = 1;
      reg_rdata = 'x;
      reg_err = 0;

      case (reg_addr)
        16'h200: reg_rdata = 16'(XLEN);
        default: reg_err = reg_request;
      endcase // case (reg_addr)
   end // always @ (*)

   // Event width
   localparam EW = 32 + 16 + XLEN;

   logic [EW-1:0] sample_data;
   logic          sample_valid;
   logic [31:0]   timestamp;
   logic [EW-1:0] fifo_data;
   logic          fifo_overflow;
   logic          fifo_valid;
   logic          fifo_ready;
   logic [EW-1:0] packet_data;
   logic          packet_overflow;
   logic          packet_valid;
   logic          packet_ready;

   assign sample_valid = trace_valid;
   assign sample_data = {trace_value, trace_id, timestamp};

   osd_timestamp
     #(.WIDTH(32))
   u_timestamp(.clk  (clk),
               .rst  (rst),
               .enable (1),
               .timestamp (timestamp));

   osd_tracesample
     #(.WIDTH(EW))
   u_sample(.clk            (clk),
            .rst            (rst),
            .sample_data    (sample_data),
            .sample_valid   (sample_valid & !stall),
            .fifo_data      (fifo_data),
            .fifo_overflow  (fifo_overflow),
            .fifo_valid     (fifo_valid),
            .fifo_ready     (fifo_ready));

   osd_fifo
     #(.WIDTH(EW+1), .DEPTH(8))
   u_buffer(.clk     (clk),
            .rst     (rst),
            .in_data ({fifo_overflow, fifo_data}),
            .in_valid (fifo_valid),
            .in_ready (fifo_ready),
            .out_data ({packet_overflow, packet_data}),
            .out_valid (packet_valid),
            .out_ready (packet_ready));

   osd_trace_packetization
     #(.WIDTH(EW))
   u_packetization(.clk             (clk),
                   .rst             (rst),
                   .id              (id),
                   .trace_data      (packet_data),
                   .trace_overflow  (packet_overflow),
                   .trace_valid     (packet_valid),
                   .trace_ready     (packet_ready),
                   .debug_out       (dp_out),
                   .debug_out_ready (dp_out_ready));

endmodule // osd_stm
