/* Copyright (c) 2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * =============================================================================
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "uart_defines.v"

//`define IN_CLK  48000000
//`define UART_BAUD_RATE  115200 // TODO: Make this a parameter!
`define UART_BASE  32'h90000000
`define DATA_WIDTH     32
`define STATE_SETUP_RESET  4'd0
`define STATE_SETUP_FC  4'd1
`define STATE_SETUP_IE  4'd2
`define STATE_SETUP_LC1  4'd3
`define STATE_SETUP_DL2  4'd4
`define STATE_SETUP_DL1  4'd5
`define STATE_SETUP_LC2  4'd6
`define STATE_SETUP_WAIT 4'd7
`define STATE_WAIT_THRE  4'd8
`define STATE_SETUP_TRANSMIT  4'd9
`define STATE_RECEIVE_READ  4'd10
`define STATE_SETUP_RECEIVE  4'd11

module char2uart(/*AUTOARG*/
   // Outputs
   in_char_ready, out_char_data, out_char_valid, wb_we_i, wb_stb_i,
   wb_cyc_i, wb_adr_i, wb_sel_i, wb_dat_i,
   // Inputs
   clk, rst, in_char_data, in_char_valid, out_char_ready, wb_dat_o,
   wb_ack_o,wb_int_o);

   parameter uart_data_width = `UART_DATA_WIDTH;
   parameter uart_addr_width = `UART_ADDR_WIDTH;

   input       clk;
   input       rst;
   input [7:0] in_char_data;
   input       in_char_valid;
   output reg  in_char_ready;
   output reg [7:0] out_char_data;
   output reg       out_char_valid;
   input            out_char_ready;

   reg [7:0]   in_character;
   reg [7:0]   nxt_in_character;
   reg [7:0]   out_character;
   reg [7:0]   nxt_out_character;

   // WISHBONE interface
   input [uart_data_width-1:0]      wb_dat_o;
   input                            wb_ack_o;
   output reg                       wb_we_i;
   output reg                       wb_stb_i;
   output reg                       wb_cyc_i;
   output reg [uart_addr_width-1:0] wb_adr_i;
   output reg [3:0]                 wb_sel_i;
   output reg [uart_data_width-1:0] wb_dat_i;
   input                            wb_int_o;

   parameter UART_BAUD_RATE = 115200;
   parameter divisor = (`OPTIMSOC_CLOCK/(16*UART_BAUD_RATE));
//   parameter divisor = 14;


   //UART FSM
   reg [3:0]     uart_state;
   reg [3:0]     nxt_uart_state;

   reg uart_waitstate; // Insert extra wait state as UART waits for cyc&stb posedge
   reg nxt_uart_waitstate;

   always @(posedge clk) begin
      if (rst) begin
         uart_state <= 0;
         uart_waitstate <= 0;
         in_character <= 8'h0;
         out_character <= 8'h0;
      end else begin
         in_character <= nxt_in_character;
         out_character <= nxt_out_character;
         uart_state  <= nxt_uart_state;
         uart_waitstate <= nxt_uart_waitstate;
      end
   end

   always @(*) begin
      nxt_uart_state = uart_state;
      nxt_uart_waitstate = uart_waitstate;
      nxt_in_character = in_character;
      nxt_out_character = out_character;
      out_char_data = 'bx;
      out_char_valid = 1'b0;
      in_char_ready = 1'b0;
      wb_we_i = 'bx;
      wb_adr_i = 'bx;
      wb_dat_i = 'bx;
      wb_sel_i = 'bx;
      wb_cyc_i = 1'b0;
      wb_stb_i = 1'b0;

      case (uart_state)
        `STATE_SETUP_RESET: begin //reset state
           nxt_uart_state = `STATE_SETUP_FC;
        end

        `STATE_SETUP_FC:   begin // UART Initialization
           wb_we_i = 1'b1;
           wb_adr_i = `UART_BASE + `UART_REG_FC;
           wb_dat_i = {16'b0, 8'b00000110,8'h0};
           wb_sel_i= 4'b0010;
           wb_cyc_i = 1'b1;
           wb_stb_i = 1'b1;
           if(wb_ack_o)
             begin
                nxt_uart_state = `STATE_SETUP_IE;
                nxt_uart_waitstate = 1;
             end
        end
        `STATE_SETUP_IE:   begin
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;
           end else begin
              wb_we_i = 1'b1;
              wb_adr_i =`UART_BASE + `UART_REG_IE;
              wb_dat_i = {8'b0, 8'b00000001, 16'h0};
              wb_sel_i= 4'b0100;
              wb_cyc_i = 1'b1;
              wb_stb_i = 1'b1;
              if(wb_ack_o)
                begin
                   nxt_uart_state = `STATE_SETUP_LC1;
                   nxt_uart_waitstate = 1;
                end
           end
        end

        `STATE_SETUP_LC1:  begin
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;
           end else begin
              wb_we_i = 1'b1;
              wb_adr_i = `UART_BASE + `UART_REG_LC;
              wb_dat_i = {24'b0, 8'b10000011};
              wb_sel_i= 4'b0001;
              wb_cyc_i = 1'b1;
              wb_stb_i = 1'b1;
              if(wb_ack_o)
                begin
                   nxt_uart_state = `STATE_SETUP_DL2;
                   nxt_uart_waitstate = 1;
                end
           end
        end

        `STATE_SETUP_DL2:   begin
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;
           end else begin
              wb_we_i = 1'b1;
              wb_adr_i = `UART_BASE + `UART_REG_DL2;
              wb_dat_i = (((divisor>>8) & 32'h000000ff)<<16);
              wb_sel_i= 4'b0100;
              wb_cyc_i = 1'b1;
              wb_stb_i = 1'b1;
              if(wb_ack_o)
                begin
                   nxt_uart_state = `STATE_SETUP_DL1;
                   nxt_uart_waitstate = 1;
                end
           end
        end

        `STATE_SETUP_DL1:   begin
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;
           end else begin
              wb_we_i = 1'b1;
              wb_adr_i = `UART_BASE + `UART_REG_DL1;
              wb_dat_i = (((divisor) & 32'h000000ff)<<24);
              wb_sel_i= 4'b1000;
              wb_cyc_i = 1'b1;
              wb_stb_i = 1'b1;
              if(wb_ack_o)
                begin
                   nxt_uart_state = `STATE_SETUP_LC2;
                   nxt_uart_waitstate = 1;
                end
           end
        end

        `STATE_SETUP_LC2:   begin
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;
           end else begin
              wb_we_i = 1'b1;
              wb_adr_i = `UART_BASE + `UART_REG_LC;
              wb_dat_i = {24'b0, 8'b00000011};
              wb_sel_i= 4'b0001;
              wb_cyc_i = 1'b1;
              wb_stb_i = 1'b1;
              if(wb_ack_o)
                begin
                   nxt_uart_state = `STATE_SETUP_WAIT;
                   nxt_uart_waitstate = 1;
                end
           end
        end

        `STATE_SETUP_WAIT:    begin // UART Wait for NOC
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;
           end else begin

              in_char_ready = 1'b1;
              nxt_in_character = in_char_data;

        if (wb_int_o) begin
                 nxt_uart_state = `STATE_RECEIVE_READ;
                 nxt_uart_waitstate = 1;
              end else if(in_char_valid)    begin
                 nxt_uart_state = `STATE_WAIT_THRE;
                 nxt_uart_waitstate = 1;
              end else begin
                 nxt_uart_state = `STATE_SETUP_WAIT;
                 nxt_uart_waitstate = 0;
              end

           end
        end // case: `STATE_SETUP_WAIT

        `STATE_WAIT_THRE: begin
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;
           end else begin
              wb_we_i = 1'b0;
              wb_cyc_i = 1'b1;
              wb_stb_i = 1'b1;
              wb_adr_i = `UART_BASE + `UART_REG_LS;
              wb_sel_i = 4'b0100;
              //in_char_ready = 1'b1;
              if (wb_ack_o) begin
                 nxt_uart_waitstate = 1;
                 if (wb_dat_o[21]) begin
                    nxt_uart_state = `STATE_SETUP_TRANSMIT;
                 end else begin
                    // If we want to do this, we need to ensure
                    // that the FSM will return here after receiving and
                    // continue the operation
//                  if (wb_int_o) begin
 //                      nxt_uart_state = `STATE_RECEIVE_READ;
//                  end else begin
                       nxt_uart_state = `STATE_WAIT_THRE;
//                  end
                 end
              end
           end
        end

        `STATE_SETUP_TRANSMIT: begin // UART Tramsmitting
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;

           end else begin
              wb_we_i = 1'b1;
              wb_adr_i =`UART_BASE + `UART_REG_TR;
              wb_dat_i = {in_character, 24'h0};
              wb_sel_i= 4'b1000;
              wb_cyc_i = 1'b1;
              wb_stb_i = 1'b1;
              if(wb_ack_o) begin
                 nxt_uart_state = `STATE_SETUP_WAIT;
                 nxt_uart_waitstate = 1;
              end
           end
        end // case: `STATE_SETUP_TRANSMIT

         `STATE_RECEIVE_READ: begin // UART Receiving
           if (uart_waitstate) begin
              nxt_uart_waitstate = 0;
              wb_cyc_i = 1'b0;
              wb_stb_i = 1'b0;
           end else begin
              wb_we_i = 1'b0;
              wb_adr_i =`UART_BASE + `UART_REG_RB;
              wb_sel_i= 4'b1000;
              wb_cyc_i = 1'b1;
              wb_stb_i = 1'b1;
              nxt_out_character = wb_dat_o[31:24];
              if(wb_ack_o) begin
                 nxt_uart_state = `STATE_SETUP_RECEIVE;
              end
           end // else: !if(uart_waitstate)
         end // case: `STATE_SETUP_RECEIVE_READ

        `STATE_SETUP_RECEIVE: begin // UART Receiving
           out_char_valid = 1'b1;
           out_char_data = out_character;
           if (out_char_ready) begin
              nxt_uart_state = `STATE_SETUP_WAIT;
           end
        end
      endcase
   end

endmodule
