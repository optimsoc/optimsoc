/* Copyright (c) 2017 by the author(s)
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
 * Mux virtual channels
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module noc_vchannel_mux
  #(parameter FLIT_WIDTH = 32,
    parameter CHANNELS = 2)
   (
    input                                clk,
    input                                rst,

    input [CHANNELS-1:0][FLIT_WIDTH-1:0] in_flit,
    input [CHANNELS-1:0]                 in_last,
    input [CHANNELS-1:0]                 in_valid,
    output [CHANNELS-1:0]                in_ready,

    output reg [FLIT_WIDTH-1:0]          out_flit,
    output reg                           out_last,
    output [CHANNELS-1:0]                out_valid,
    input [CHANNELS-1:0]                 out_ready
    );

   reg [CHANNELS-1:0]                    select;
   logic [CHANNELS-1:0]                  nxt_select;

   assign out_valid = in_valid & select;
   assign in_ready  = out_ready & select;

   always @(*) begin
      out_flit = 'x;
      out_last = 'x;
      for (int c = 0; c < CHANNELS; c++) begin
         if (select[c]) begin
            out_flit = in_flit[c];
            out_last = in_last[c];
         end
      end
   end

   arb_rr
     #(.N (CHANNELS))
   u_arbiter
     (.req (in_valid & out_ready),
      .en  (1),
      .gnt (select),
      .nxt_gnt (nxt_select));

   always_ff @(posedge clk) begin
      if (rst) begin
         select <= {{CHANNELS-1{1'b0}},1'b1};
      end else begin
         select <= nxt_select;
      end
   end

endmodule // noc_vchannel_mux
