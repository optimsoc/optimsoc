/* Copyright (c) 2015-2017 by the author(s)
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
 * Arbitrate between different channels, don't break worms.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Andreas Lankes <andreas.lankes@tum.de>
 */

module noc_mux
  import optimsoc_config::*;
  #(
    parameter FLIT_WIDTH = 32,
    parameter CHANNELS   = 2
    )
   (
    input                                clk, rst,

    input [CHANNELS-1:0][FLIT_WIDTH-1:0] in_flit,
    input [CHANNELS-1:0]                 in_last,
    input [CHANNELS-1:0]                 in_valid,
    output reg [CHANNELS-1:0]            in_ready,

    output reg [FLIT_WIDTH-1:0]          out_flit,
    output reg                           out_last,
    output reg                           out_valid,
    input                                out_ready
    );

   wire [CHANNELS-1:0]                               select;
   reg [CHANNELS-1:0]                                active;

   reg                                               activeroute, nxt_activeroute;

   wire [CHANNELS-1:0]                               req_masked;
   assign req_masked = {CHANNELS{~activeroute & out_ready}} & in_valid;

   always @(*) begin
      out_flit = {FLIT_WIDTH{1'b0}};
      out_last = 1'b0;
      for (int c = 0; c < CHANNELS; c = c + 1) begin
         if (select[c]) begin
            out_flit = in_flit[c];
            out_last = in_last[c];
         end
      end
   end

   always @(*) begin
      nxt_activeroute = activeroute;
      in_ready = {CHANNELS{1'b0}};

      if (activeroute) begin
         if (|(in_valid & active) && out_ready) begin
            in_ready = active;
            out_valid = 1;
            if (out_last)
              nxt_activeroute = 0;
         end else begin
            out_valid = 1'b0;
            in_ready = 0;
         end
      end else begin
         out_valid = 0;
         if (|in_valid) begin
            out_valid = 1'b1;
            nxt_activeroute = ~out_last;
            in_ready = select & {CHANNELS{out_ready}};
         end
      end
   end // always @ (*)

   always @(posedge clk) begin
      if (rst) begin
         activeroute <= 0;
         active <= {{CHANNELS-1{1'b0}},1'b1};
      end else begin
         activeroute <= nxt_activeroute;
         active <= select;
      end
   end

   arb_rr
     #(.N(CHANNELS))
   u_arb
     (.nxt_gnt (select),
      .req     (req_masked),
      .gnt     (active),
      .en      (1'b1));

endmodule // noc_mux
