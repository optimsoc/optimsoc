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
 * This is a simple ROM used at boot time in distributed memory systems
 * or similar.
 * 
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module bootrom(/*AUTOARG*/
   // Outputs
   wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o,
   // Inputs
   clk, rst, wb_adr_i, wb_dat_i, wb_cyc_i, wb_stb_i, wb_sel_i
   );

   input clk;
   input rst;

   input [31:0] wb_adr_i;
   input [31:0] wb_dat_i;
   input        wb_cyc_i;
   input        wb_stb_i;
   input [3:0]  wb_sel_i;
   output reg [31:0] wb_dat_o;
   output reg    wb_ack_o;
   output        wb_err_o;
   output        wb_rty_o;

   always @(posedge clk) begin
      wb_ack_o <= wb_stb_i & ~wb_ack_o;
   end

   assign wb_err_o = 1'b0;
   assign wb_rty_o = 1'b0;

   always @(*) begin
      case(wb_adr_i[7:2])
`include "bootrom_code.v"
        default: wb_dat_o = 32'hx;
      endcase
   end

endmodule // bootrom
