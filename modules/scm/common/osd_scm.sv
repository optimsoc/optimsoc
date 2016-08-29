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

module osd_scm
  #(parameter SYSTEMID='x,
    parameter NUM_MOD='x,
    parameter MAX_PKT_LEN=8)
   (input clk, rst,

    input [9:0] id,

    input dii_flit debug_in, output debug_in_ready,
    output dii_flit debug_out, input debug_out_ready,

    output sys_rst,
    output cpu_rst);

   logic        reg_request;
   logic        reg_write;
   logic [15:0] reg_addr;
   logic [1:0]  reg_size;
   logic [15:0] reg_wdata;
   logic        reg_ack;
   logic        reg_err;
   logic [15:0] reg_rdata;

   logic [1:0]  rst_vector;
   assign sys_rst = rst_vector[0] | rst;
   assign cpu_rst = rst_vector[1] | rst;

   osd_regaccess
     #(.MODID(16'h1), .MODVERSION(16'h0),
       .MAX_REG_SIZE(16))
   u_regaccess(.*,
               .stall ());

   always @(*) begin
      reg_ack = 1;
      reg_rdata = 'x;
      reg_err = 0;

      case (reg_addr)
        16'h200: reg_rdata = 16'(SYSTEMID);
        16'h201: reg_rdata = 16'(NUM_MOD);
        16'h202: reg_rdata = 16'(MAX_PKT_LEN);
        16'h203: reg_rdata = {14'h0, rst_vector};
        default: reg_err = reg_request;
      endcase // case (reg_addr)
   end // always @ (*)

   always @(posedge clk) begin
      if (rst) begin
         rst_vector <= 2'b00;
      end else begin
         if (reg_request & reg_write & (reg_addr == 16'h203))
            rst_vector <= reg_wdata[1:0];
      end
   end
endmodule
