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

module osd_dem_uart
  (input clk, rst,

   input           dii_flit debug_in, output debug_in_ready,
   output          dii_flit debug_out, input debug_out_ready,

   input [9:0]     id,

   output          drop,

   input [7:0]     out_char,
   input           out_valid,
   output reg      out_ready,

   output reg [7:0] in_char,
   output reg      in_valid,
   input           in_ready);

   logic        reg_request;
   logic        reg_write;
   logic [15:0] reg_addr;
   logic [1:0]  reg_size;
   logic [15:0] reg_wdata;
   logic        reg_ack;
   logic        reg_err;
   logic [15:0] reg_rdata;

   assign reg_ack = 0;
   assign reg_err = 0;
   assign reg_rdata = 0;

   logic        stall;
   assign drop = stall;

   dii_flit c_uart_out, c_uart_in;
   logic        c_uart_out_ready, c_uart_in_ready;

   osd_regaccess_layer
     #(.MODID(16'h2), .MODVERSION(16'h0),
       .MAX_REG_SIZE(16), .CAN_STALL(1))
   u_regaccess(.*,
               .module_in (c_uart_out),
               .module_in_ready (c_uart_out_ready),
               .module_out (c_uart_in),
               .module_out_ready (c_uart_in_ready));

   enum         { STATE_IDLE, STATE_HEADER, STATE_XFER } stateTx, stateRx;
   always @(posedge clk) begin
      if (rst) begin
         stateTx <= STATE_IDLE;
         stateRx <= STATE_IDLE;
      end else begin
         case (stateTx)
           STATE_IDLE: begin
              if (out_valid & !stall & c_uart_out_ready) begin
                 stateTx <= STATE_HEADER;
              end
           end
           STATE_HEADER: begin
              if (c_uart_out_ready) begin
                 stateTx <= STATE_XFER;
              end
           end
           STATE_XFER: begin
              if (c_uart_out_ready) begin
                 stateTx <= STATE_IDLE;
              end
           end
         endcase // case (stateTx)

         case (stateRx)
           STATE_IDLE: begin
              if (c_uart_in.valid) begin
                 stateRx <= STATE_HEADER;
              end
           end
           STATE_HEADER: begin
              if (c_uart_in.valid) begin
                 stateRx <= STATE_XFER;
              end
           end
           STATE_XFER: begin
              if (c_uart_in.valid & in_ready) begin
                 stateRx <= STATE_IDLE;
              end
           end
         endcase // case (stateRx)
      end // else: !if(rst)
   end

   always @(*) begin
      c_uart_out.valid = 0;
      c_uart_out.last = 0;
      c_uart_out.data = 'x;
      out_ready = 0;

      case (stateTx)
        STATE_IDLE: begin
           c_uart_out.valid = out_valid & !stall;
           c_uart_out.data = 0;
        end
        STATE_HEADER: begin
           c_uart_out.valid = 1;
           c_uart_out.data = {2'b01, 4'h01, 10'(id)};
        end
        STATE_XFER: begin
           c_uart_out.valid = 1;
           c_uart_out.data = {8'h0, out_char};
           c_uart_out.last = 1;
           out_ready = c_uart_out_ready;
        end
      endcase // case (stateTx)

      c_uart_in_ready = 0;
      in_valid = 0;
      in_char = 'x;

      case (stateRx)
        STATE_IDLE: begin
           c_uart_in_ready = 1;
        end
        STATE_HEADER: begin
           c_uart_in_ready = 1;
        end
        STATE_XFER: begin
           c_uart_in_ready = in_ready;
           in_valid = c_uart_in.valid;
           in_char = c_uart_in.data[7:0];
        end
      endcase
   end

endmodule // osd_dem_uart
