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

module osd_regaccess_layer
  #(parameter MODID = 'x,
    parameter MODVERSION = 'x,
    parameter CAN_STALL = 0,
    parameter MAX_REG_SIZE = 16)
   (input clk, rst,

    input [9:0]   id,

    input         dii_flit debug_in, output logic debug_in_ready,
    output        dii_flit debug_out, input debug_out_ready,

    output        dii_flit module_out, input module_out_ready,
    input         dii_flit module_in, output module_in_ready,

    output reg    reg_request,
    output        reg_write,
    output [15:0] reg_addr,
    output [1:0]  reg_size,
    output [15:0] reg_wdata,
    input         reg_ack,
    input         reg_err,
    input [15:0]  reg_rdata,

    output        stall);

   dii_flit       regaccess_in, regaccess_out;
   logic          regaccess_in_ready, regaccess_out_ready;

   osd_regaccess
     #(.MODID(MODID), .MODVERSION(MODVERSION), .CAN_STALL(CAN_STALL),
       .MAX_REG_SIZE(MAX_REG_SIZE))
   u_regaccess
     (.*,
      .debug_in (regaccess_in),
      .debug_in_ready (regaccess_in_ready),
      .debug_out (regaccess_out),
      .debug_out_ready (regaccess_out_ready));

   // Ingress path demux
   osd_regaccess_demux
     u_demux(.*,
             .in (debug_in),
             .in_ready (debug_in_ready),
             .out_reg (regaccess_in),
             .out_reg_ready (regaccess_in_ready),
             .out_bypass (module_out),
             .out_bypass_ready (module_out_ready));

   // Egress path mux
   ring_router_mux
     u_mux(.*,
           .in_local (module_in),
           .in_local_ready (module_in_ready),
           .in_ring (regaccess_out),
           .in_ring_ready (regaccess_out_ready),
           .out_mux    (debug_out),
           .out_mux_ready    (debug_out_ready));

endmodule
