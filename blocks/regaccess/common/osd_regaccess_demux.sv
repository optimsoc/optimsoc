// Copyright 2016-2017 by the authors
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

import dii_package::dii_flit;

/**
 * Debug Interconnect Register Access Demultiplexer
 *
 * This module splits an incoming DI link depending on the packet type.
 * Register access packets (FLAGS.TYPE = REG) are forwarded to the out_reg
 * output ports, all other packets are forwarded to the out_bypass ports.
 *
 * The latency between the in and the out port is 3 cycles.
 */
module osd_regaccess_demux
  (input clk, input rst,

   input  dii_flit in, output logic in_ready,

   output dii_flit out_reg, input out_reg_ready,
   output dii_flit out_bypass, input out_bypass_ready);

   typedef struct packed {
      logic [15:0] data;
      logic last;
      logic valid;
   } bufdata;

   bufdata buf_reg[3];
   logic [2:0] buf_reg_is_regaccess;
   logic [2:0] buf_reg_is_bypass;

   logic do_tag, mark_bypass, mark_regaccess;
   assign do_tag = buf_reg[2].valid & buf_reg[1].valid & buf_reg[0].valid &
      (!buf_reg_is_regaccess[2] & !buf_reg_is_bypass[2]) &
      (!buf_reg_is_regaccess[1] & !buf_reg_is_bypass[1]) &
      (!buf_reg_is_regaccess[0] & !buf_reg_is_bypass[0]);

   assign mark_bypass = do_tag & (buf_reg[0].data[15:14] != 2'b00);
   assign mark_regaccess = do_tag & (buf_reg[0].data[15:14] == 2'b00);

   logic pkg_is_bypass, pkg_is_regaccess;
   always_ff @(posedge clk) begin
      if (rst) begin
         pkg_is_bypass <= 0;
         pkg_is_regaccess <= 0;
      end else begin
         pkg_is_bypass <= (pkg_is_bypass | mark_bypass)
            & !(in.last & in.valid & in_ready)
            & !(buf_reg[0].last & buf_reg[0].valid);
         pkg_is_regaccess <= (pkg_is_regaccess | mark_regaccess)
            & !(in.last & in.valid & in_ready)
            & !(buf_reg[0].last & buf_reg[0].valid);
      end
   end

   logic keep_1, keep_2;
   assign keep_1 = !do_tag & buf_reg[1].valid
      & !(buf_reg_is_bypass[1] | buf_reg_is_regaccess[1]) & keep_2;
   assign keep_2 = !do_tag & buf_reg[2].valid
      & !(buf_reg_is_bypass[2] | buf_reg_is_regaccess[2]);

   always_ff @(posedge clk) begin
      if (rst) begin
         buf_reg[0].valid <= 0;
         buf_reg_is_regaccess[0] <= 0;
         buf_reg_is_bypass[0] <= 0;

         buf_reg[1].valid <= 0;
         buf_reg_is_regaccess[1] <= 0;
         buf_reg_is_bypass[1] <= 0;

         buf_reg[2].valid <= 0;
         buf_reg_is_regaccess[2] <= 0;
         buf_reg_is_bypass[2] <= 0;
      end else begin
         if (in_ready) begin
            buf_reg[0].data <= in.data;
            buf_reg[0].last <= in.last;
            buf_reg[0].valid <= in.valid & in_ready;
            if (buf_reg[0].valid & !buf_reg[0].last) begin
               buf_reg_is_regaccess[0] <= pkg_is_regaccess | mark_regaccess;
               buf_reg_is_bypass[0] <= pkg_is_bypass | mark_bypass;
            end else begin
               buf_reg_is_regaccess[0] <= pkg_is_regaccess;
               buf_reg_is_bypass[0] <= pkg_is_bypass;
            end

            if (!keep_1) begin
               buf_reg[1] <= buf_reg[0];
               buf_reg_is_regaccess[1] <= buf_reg_is_regaccess[0] | mark_regaccess;
               buf_reg_is_bypass[1] <= buf_reg_is_bypass[0] | mark_bypass;
            end else begin
               buf_reg_is_regaccess[1] <= buf_reg_is_regaccess[1] | mark_regaccess;
               buf_reg_is_bypass[1] <= buf_reg_is_bypass[1] | mark_bypass;
            end

            if (!keep_2) begin
               buf_reg[2] <= buf_reg[1];
               buf_reg_is_regaccess[2] <= buf_reg_is_regaccess[1] | mark_regaccess;
               buf_reg_is_bypass[2] <= buf_reg_is_bypass[1] | mark_bypass;
            end else begin
               buf_reg_is_regaccess[2] <= buf_reg_is_regaccess[2] | mark_regaccess;
               buf_reg_is_bypass[2] <= buf_reg_is_bypass[2] | mark_bypass;
            end
         end
      end
   end

   // Output data
   assign out_reg.data = buf_reg[2].data;
   assign out_reg.last = buf_reg[2].last;
   assign out_reg.valid = buf_reg[2].valid
      & (buf_reg_is_regaccess[2] | mark_regaccess);

   assign out_bypass.data = buf_reg[2].data;
   assign out_bypass.last = buf_reg[2].last;
   assign out_bypass.valid = buf_reg[2].valid
      & (buf_reg_is_bypass[2] | mark_bypass);

   logic no_buf_entry_is_tagged;
   assign no_buf_entry_is_tagged = ~do_tag & ~((|buf_reg_is_regaccess) | (|buf_reg_is_bypass));

   assign in_ready = (out_bypass_ready & out_reg_ready) |
      (out_bypass_ready & (buf_reg_is_bypass[2] | mark_bypass)) |
      (out_reg_ready & (buf_reg_is_regaccess[2] | mark_regaccess)) |
      no_buf_entry_is_tagged;

endmodule // osd_regaccess_demux
