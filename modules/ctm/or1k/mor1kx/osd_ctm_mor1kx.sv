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
import opensocdebug::mor1kx_trace_exec;

module osd_ctm_mor1kx
   (
    input                        clk, rst,

    input [9:0]                  id,

    input  dii_flit              debug_in,
    output                       debug_in_ready,
    output dii_flit              debug_out,
    input                        debug_out_ready,

    input mor1kx_trace_exec      trace_port
    );

   localparam ADDR_WIDTH = 32;
   localparam DATA_WIDTH = 32;

   logic                         trace_valid;
   logic [ADDR_WIDTH-1:0]        trace_pc;
   logic [ADDR_WIDTH-1:0]        trace_npc;
   logic                         trace_jal;
   logic                         trace_jalr;
   logic                         trace_branch;
   logic                         trace_load;
   logic                         trace_store;
   logic                         trace_trap;
   logic                         trace_xcpt;
   logic                         trace_mem;
   logic                         trace_csr;
   logic                         trace_br_taken;
   logic [1:0]                   trace_prv;
   logic [ADDR_WIDTH-1:0]        trace_addr;
   logic [DATA_WIDTH-1:0]        trace_rdata;
   logic [DATA_WIDTH-1:0]        trace_wdata;
   logic [DATA_WIDTH-1:0]        trace_time;

   osd_ctm
     #(.ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH))
   u_ctm
     (.*);

   assign trace_valid = trace_port.valid;
   assign trace_pc = trace_port.pc;
   assign trace_npc = trace_port.jbtarget;
   assign trace_jal = trace_port.jal;
   assign trace_jalr = trace_port.jr;


endmodule // osd_stm_mor1kx
