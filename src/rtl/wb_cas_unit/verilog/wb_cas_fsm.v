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

module wb_cas_fsm(
    input clk_i,
    input rst_i,
    input [31:0] core_adr_i,
    input [31:0] core_dat_i,
    input [3:0] core_sel_i,
    input core_we_i,
    input core_cyc_i,
    input core_stb_i,
    output [31:0] core_dat_o,
    output core_ack_o,
    output core_err_o,
    output core_rty_o,
    output [31:0] bus_adr_o,
    output [31:0] bus_dat_o,
    output [3:0] bus_sel_o,
    output bus_we_o,
    output bus_cyc_o,
    output bus_stb_o,
    input bus_ack_i,
    input [31:0] bus_dat_i,
    input bus_err_i,
    input bus_rty_i
    );

assign core_err_o = 0;
assign core_rty_o = 0;

assign bus_sel_o = 4'b1111;

parameter state_00 = 11'b00000000001;
parameter state_01 = 11'b00000000010;
parameter state_02 = 11'b00000000100;
parameter state_03 = 11'b00000001000;
parameter state_04 = 11'b00000010000;
parameter state_05 = 11'b00000100000;
parameter state_06 = 11'b00001000000;
parameter state_07 = 11'b00010000000;
parameter state_08 = 11'b00100000000;
parameter state_09 = 11'b01000000000;
parameter state_10 = 11'b10000000000;

reg [31:0] address = 32'h00000000;
reg [31:0] compare = 32'h00000000;
reg [31:0] value   = 32'h00000000;
reg [31:0] old_value = 32'h00000000;

reg cycle = 0;
reg strobe = 0;
reg we = 0;

(* FSM_ENCODING="ONE-HOT", SAFE_IMPLEMENTATION="YES", SAFE_RECOVERY_STATE="11'b00000000001" *) reg [10:0] state = state_00;

always @ (posedge clk_i)
begin
        if (rst_i)
        begin
                state <= state_00;
                cycle <= 0;
                strobe <= 0;
        end
        else 
          case (state)
                        state_00: begin
                                if ( core_cyc_i & core_stb_i & core_we_i )
                                        state <= state_01;
                        end
                        state_01: begin
                                state <= state_02;
                                address <= core_dat_i;
                        end
                        state_02: begin
                                if ( core_cyc_i & core_stb_i & core_we_i )
                                        state <= state_03;
                        end
                        state_03: begin
                                state <= state_04;
                                compare <= core_dat_i;
                        end
                        state_04: begin
                                if ( core_cyc_i & core_stb_i & core_we_i )
                                        state <= state_05;
                        end
                        state_05: begin
                                state <= state_06;
                                value <= core_dat_i;
                        end
                        state_06: begin
                                if ( core_cyc_i & core_stb_i & ~core_we_i )
                                begin
                                        state <= state_07;
                                        cycle <= 1;
                                        strobe <= 1;
                                end
                        end
                        state_07: begin
                                if ( bus_ack_i )
                                begin
                                        strobe <= 0;
                                        old_value <= bus_dat_i;
                                        if ( bus_dat_i == compare )
                                        begin
                                                state <= state_08;
                                        end
                                        else
                                        begin
                                                state <= state_10;
                                                cycle <= 0;
                                        end
                                end
                        end
                        state_08: begin
                                we <= 1;
                                strobe <= 1;
                                state <= state_09;
                        end
                        state_09: begin
                                if (bus_ack_i)
                                begin
                                        strobe <= 0;
                                        cycle <= 0;
                                        we <= 0;
                                        state <= state_10;
                                end
                        end
                        state_10: begin
                                state <= state_00;                              
                        end
            default:
              state <= state_00;
                endcase
end

assign core_ack_o = (state == state_01) | (state == state_03) | (state == state_05) | (state == state_10);

assign bus_cyc_o = cycle;
assign bus_stb_o = strobe;
assign bus_adr_o = address;
assign bus_dat_o = value;
assign bus_we_o = we;

assign core_dat_o = old_value;
endmodule
