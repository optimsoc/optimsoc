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

   input            dii_flit debug_in, output debug_in_ready,
   output           dii_flit debug_out, input debug_out_ready,

   input [15:0]     id,

   output           drop,

   input [7:0]      out_char,
   input            out_valid,
   output reg       out_ready,

   output reg [7:0] in_char,
   output reg       in_valid,
   input            in_ready
   );

   localparam TYPE_EVENT          = 2'b10;
   localparam TYPE_SUB_EVENT_LAST = 4'b0000;

   logic         stall;
   assign drop = stall;

   dii_flit     c_uart_out, c_uart_in;
   logic        c_uart_out_ready, c_uart_in_ready;

   reg [15:0]   event_dest;
   reg [7:0]    out_char_buf;

   osd_regaccess_layer
     #(.MOD_VENDOR(16'h1), .MOD_TYPE(16'h2), .MOD_VERSION(16'h0),
       .MAX_REG_SIZE(16), .CAN_STALL(1), .MOD_EVENT_DEST_DEFAULT(16'h0))
   u_regaccess(.clk (clk), .rst (rst), .id (id),
               .debug_in (debug_in),
               .debug_in_ready (debug_in_ready),
               .debug_out (debug_out),
               .debug_out_ready (debug_out_ready),
               .module_in (c_uart_out),
               .module_in_ready (c_uart_out_ready),
               .module_out (c_uart_in),
               .module_out_ready (c_uart_in_ready),
               .stall (stall),
               .event_dest(event_dest),
               .reg_request (),
               .reg_write (),
               .reg_addr (),
               .reg_size (),
               .reg_wdata (),
               .reg_ack (1'b0),
               .reg_err (1'b0),
               .reg_rdata (16'h0));

   enum         { STATE_IDLE, STATE_HDR_DEST, STATE_HDR_SRC, STATE_HDR_FLAGS,
                  STATE_XFER } state_tx, state_rx;

   always @(posedge clk) begin
      if (rst) begin
         state_tx <= STATE_IDLE;
         state_rx <= STATE_IDLE;
      end else begin
         case (state_tx)
           STATE_IDLE: begin
              if (out_valid & !stall) begin
                 state_tx <= STATE_HDR_DEST;
                 out_char_buf <= out_char;
              end
           end
           STATE_HDR_DEST: begin
              if (c_uart_out_ready) begin
                 state_tx <= STATE_HDR_SRC;
              end
           end
           STATE_HDR_SRC: begin
              if (c_uart_out_ready) begin
                 state_tx <= STATE_HDR_FLAGS;
              end
           end
           STATE_HDR_FLAGS: begin
              if (c_uart_out_ready) begin
                 state_tx <= STATE_XFER;
              end
           end
           STATE_XFER: begin
              if (c_uart_out_ready) begin
                 state_tx <= STATE_IDLE;
              end
           end
         endcase

         case (state_rx)
           STATE_IDLE: begin
              if (c_uart_in.valid) begin
                 state_rx <= STATE_HDR_SRC;
              end
           end
           STATE_HDR_SRC: begin
              if (c_uart_in.valid) begin
                 state_rx <= STATE_HDR_FLAGS;
              end
           end
           STATE_HDR_FLAGS: begin
              if (c_uart_in.valid) begin
                 state_rx <= STATE_XFER;
              end
           end
           STATE_XFER: begin
              if (c_uart_in.valid & in_ready) begin
                 state_rx <= STATE_IDLE;
              end
           end
         endcase
      end
   end

   always_comb begin
      c_uart_out.valid = 0;
      c_uart_out.last = 0;
      c_uart_out.data = 16'h0;
      out_ready = 0;

      case (state_tx)
        STATE_IDLE: begin
           out_ready = !stall;
        end
        STATE_HDR_DEST: begin
           c_uart_out.valid = 1;
           c_uart_out.data = event_dest;
        end
        STATE_HDR_SRC: begin
           c_uart_out.valid = 1;
           c_uart_out.data = id;
        end
        STATE_HDR_FLAGS: begin
           c_uart_out.valid = 1;
           c_uart_out.data = {TYPE_EVENT, TYPE_SUB_EVENT_LAST, 10'h0};
        end
        STATE_XFER: begin
           c_uart_out.valid = 1;
           c_uart_out.data = {8'h0, out_char_buf};
           c_uart_out.last = 1;
        end
      endcase

      c_uart_in_ready = 0;
      in_valid = 0;
      in_char = 8'h0;

      case (state_rx)
        STATE_IDLE: begin
           c_uart_in_ready = 1;
        end
        STATE_HDR_SRC: begin
           c_uart_in_ready = 1;
        end
        STATE_HDR_FLAGS: begin
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
