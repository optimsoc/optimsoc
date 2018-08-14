// Copyright 2017 by the authors
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

module osd_dem_uart_wb
   (
    input         clk, rst,

    input   dii_flit debug_in,   output  debug_in_ready,
    output  dii_flit debug_out,  input   debug_out_ready,

    input [15:0]  id,

    output        irq,

    input [3:0]   wb_adr_i,
    input         wb_cyc_i,
    input [31:0]  wb_dat_i,
    input [3:0]   wb_sel_i,
    input         wb_stb_i,
    input         wb_we_i,
    input [2:0]   wb_cti_i,
    input [1:0]   wb_bte_i,
    output        wb_ack_o,
    output        wb_rty_o,
    output        wb_err_o,
    output [31:0] wb_dat_o
    );

   logic          bus_req;
   logic [2:0]    bus_addr;
   logic          bus_write;
   logic [7:0]    bus_wdata;
   logic          bus_ack;
   logic [7:0]    bus_rdata;

   logic          drop;

   logic          out_valid;
   logic [7:0]    out_char;
   logic          out_ready;

   logic          in_valid;
   logic [7:0]    in_char;
   logic          in_ready;

   osd_dem_uart
   u_uart_emul(.clk (clk), .rst (rst), .id (id),
               .debug_in (debug_in),
               .debug_in_ready (debug_in_ready),
               .debug_out (debug_out),
               .debug_out_ready (debug_out_ready),
               .out_valid (out_valid),
               .out_char (out_char),
               .out_ready (out_ready),
               .in_valid (in_valid),
               .in_char (in_char),
               .in_ready (in_ready),
               .drop (drop));

   osd_dem_uart_16550
   u_16550(.clk (clk), .rst (rst),
           .out_valid (out_valid),
           .out_char (out_char),
           .out_ready (out_ready),
           .in_valid (in_valid),
           .in_char (in_char),
           .in_ready (in_ready),
           .bus_req (bus_req),
           .bus_addr (bus_addr),
           .bus_write (bus_write),
           .bus_wdata (bus_wdata),
           .bus_ack (bus_ack),
           .bus_rdata (bus_rdata),
           .drop (drop),
           .irq (irq));

   assign bus_req = wb_cyc_i & wb_stb_i;
   assign bus_addr = { wb_adr_i[2], (wb_sel_i[0] ? 2'b11 : (wb_sel_i[1] ? 2'b10 : (wb_sel_i[2] ? 2'b01 : 2'b00))) };
   assign bus_write = wb_we_i;
   assign bus_wdata = wb_dat_i[7:0];
   assign wb_ack_o = bus_ack;
   assign wb_err_o = 1'b0;
   assign wb_rty_o = 1'b0;
   assign wb_dat_o = {4{bus_rdata}};

endmodule // osd_dem_uart_wb
