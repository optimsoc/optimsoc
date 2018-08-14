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

module osd_dem_uart_nasti
  #(parameter ID_WIDTH=1,
    parameter ADDR_WIDTH=3,
    parameter DATA_WIDTH=8)
   (input clk, rst,

    input [9:0]                 id,

    output                      irq,

    input [ADDR_WIDTH-1:0]      ar_addr,
    input                       ar_valid,
    output                      ar_ready,

    output [1:0]                r_resp,
    output reg [DATA_WIDTH-1:0] r_data,
    output                      r_valid,
    input                       r_ready,

    input [ADDR_WIDTH-1:0]      aw_addr,
    input                       aw_valid,
    output                      aw_ready,

    input [DATA_WIDTH-1:0]      w_data,
    input                       w_valid,
    output                      w_ready,

    output [1:0]                b_resp,
    output                      b_valid,
    input                       b_ready,

    input                       dii_flit debug_in,
    output                      debug_in_ready,
    output                      dii_flit debug_out,
    input                       debug_out_ready
    );

   reg                      bus_req;
   reg [2:0]                bus_addr;
   reg                      bus_write;
   reg [7:0]                bus_wdata;
   logic                    bus_ack;
   logic [7:0]              bus_rdata;

   logic                    drop;

   logic                    out_valid;
   logic [7:0]              out_char;
   logic                    out_ready;
   logic                    in_valid;
   logic [7:0]              in_char;
   logic                    in_ready;

   osd_dem_uart_16550
     u_16550(.*);

   osd_dem_uart
     u_uart_emul(.*);

   assign irq = in_valid;

   reg                      resp;

   assign aw_ready = !bus_req & !resp & aw_valid & w_valid;
   assign w_ready = !bus_req & !resp & aw_valid & w_valid;
   assign ar_ready = !bus_req & !resp & ar_valid & !aw_ready;
   assign b_valid = resp & bus_write;
   assign r_valid = resp & !bus_write;

   assign r_resp = 2'b00;
   assign b_resp = 2'b00;

   always @(posedge clk) begin
      if (rst) begin
         bus_req <= 0;
         bus_addr <= 'x;
         bus_write <= 'x;
         bus_wdata <= 'x;
         resp <= 0;
      end else begin
         if (!bus_req & !resp) begin
            if (aw_valid & w_valid) begin
               bus_req <= 1;
               bus_addr <= aw_addr[2:0];
               bus_write <= 1;
               bus_wdata <= w_data;
            end else if (ar_valid) begin
               bus_req <= 1;
               bus_addr <= ar_addr[2:0];
               bus_write <= 0;
            end
         end else if (!resp) begin // if (!bus_req)
            if (bus_ack) begin
               r_data <= bus_rdata;
               resp <= 1;
               bus_req <= 0;
            end
         end else begin
            if (bus_write & b_ready) begin
               resp <= 0;
            end else if (!bus_write & r_ready) begin
               resp <= 0;
            end
         end
      end
   end
endmodule // osd_dem_uart_nasti
