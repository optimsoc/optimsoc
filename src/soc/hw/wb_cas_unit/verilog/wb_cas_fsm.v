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
 * This is the FSM performing the CAS operation based on the memory
 * mapped configuration.
 * 
 * (c) 2009-2013 by the author(s)
 * 
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module wb_cas_fsm(/*AUTOARG*/
   // Outputs
   core_dat_o, core_ack_o, core_err_o, core_rty_o, bus_adr_o,
   bus_dat_o, bus_sel_o, bus_we_o, bus_cyc_o, bus_stb_o,
   // Inputs
   clk_i, rst_i, core_adr_i, core_dat_i, core_sel_i, core_we_i,
   core_cyc_i, core_stb_i, bus_ack_i, bus_dat_i, bus_err_i, bus_rty_i
   );

   input             clk_i;
   input             rst_i;
   input [31:0]      core_adr_i;
   input [31:0]      core_dat_i;
   input [3:0]       core_sel_i;
   input             core_we_i;
   input             core_cyc_i;
   input             core_stb_i;
   output reg [31:0] core_dat_o;
   output reg        core_ack_o;
   output            core_err_o;
   output            core_rty_o;
   output [31:0]     bus_adr_o;
   output [31:0]     bus_dat_o;
   output [3:0]      bus_sel_o;
   output reg        bus_we_o;
   output reg        bus_cyc_o;
   output reg        bus_stb_o;
   input             bus_ack_i;
   input [31:0]      bus_dat_i;
   input             bus_err_i;
   input             bus_rty_i;

   localparam STATE_IDLE = 0;
   localparam STATE_COMPARE = 1;
   localparam STATE_SWAP = 2;
   
   reg [1:0]     state;
   reg [1:0]     nxt_state;

   reg [31:0]    address;
   reg [31:0]    nxt_address;
   reg [31:0]    compare;
   reg [31:0]    nxt_compare;
   reg [31:0]    swap;
   reg [31:0]    nxt_swap;
   reg [31:0]    old;
   reg [31:0]    nxt_old;
   
   assign {core_err_o, core_rty_o} = 2'b00;

   assign bus_adr_o = address;
   assign bus_dat_o = swap;
   assign bus_sel_o = 4'hf;
   
   always @(*) begin
      core_ack_o = 1'b0;
      core_dat_o = 32'hx;
      bus_cyc_o = 1'b0;
      bus_stb_o = 1'b0;
      bus_we_o = 1'b0;

      nxt_address = address;
      nxt_compare = compare;
      nxt_swap = swap;
      nxt_old = old;

      nxt_state = state;
      
      case(state)
        STATE_IDLE: begin
           if (core_cyc_i && core_stb_i) begin
              if (core_we_i) begin
                 core_ack_o = 1'b1;
                 if (core_adr_i[3:0] == 4'h0) begin
                    nxt_address = core_dat_i;
                 end else if (core_adr_i[3:0] == 4'h4) begin
                    nxt_compare = core_dat_i;
                 end else if (core_adr_i[3:0] == 4'h8) begin
                    nxt_swap = core_dat_i;
                 end else begin
                    nxt_state = STATE_IDLE;
                 end
              end else begin // if (core_we_i)
                 if (core_adr_i[3:0] == 4'h0) begin
                    core_ack_o = 1'b1;
                    core_dat_o = address;
                 end else if (core_adr_i[3:0] == 4'h4) begin
                    core_ack_o = 1'b1;
                    core_dat_o = compare;
                 end else if (core_adr_i[3:0] == 4'h8) begin
                    core_ack_o = 1'b1;
                    core_dat_o = swap;
                 end else if (core_adr_i[3:0] == 4'hc) begin
                    nxt_state = STATE_COMPARE;
                 end else begin
                    nxt_state = STATE_IDLE;
                 end
              end
           end
        end // case: STATE_IDLE
        STATE_COMPARE: begin
           bus_cyc_o = 1'b1;
           bus_stb_o = 1'b1;
           bus_we_o = 1'b0;
           if (bus_ack_i) begin
              nxt_old = bus_dat_i;
              if (bus_dat_i == compare) begin
                 nxt_state = STATE_SWAP;
              end else begin
                 core_ack_o = 1'b1;
                 core_dat_o = bus_dat_i;
                 nxt_state = STATE_IDLE;
              end
           end
        end // case: STATE_COMPARE
        STATE_SWAP: begin
           bus_cyc_o = 1'b1;
           bus_stb_o = 1'b1;
           bus_we_o = 1'b1;
           if (bus_ack_i) begin
              nxt_state = STATE_IDLE;
              core_ack_o = 1'b1;
              core_dat_o = old;
           end
        end
        default: begin
           nxt_state = STATE_IDLE;
        end
      endcase
   end 
   
   always @(posedge clk_i) begin
      if (rst_i) begin
         state <= STATE_IDLE;
         address <= 32'hx;
         compare <= 32'hx;
         swap <= 32'hx;
         old <= 32'hx;
      end else begin
         state <= nxt_state;
         address <= nxt_address;
         compare <= nxt_compare;
         swap <= nxt_swap;
         old <= nxt_old;
      end
   end

endmodule
