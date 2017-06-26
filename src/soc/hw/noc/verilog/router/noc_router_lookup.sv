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
 * This is the route lookup. It uses a given table of routes. It first
 * extracts the destination of the incoming NoC packets and then looks
 * the output port up. The output valid and ready signals contain bits
 * for each output and the flit and last are shared by the outputs.
 *
 * The output of this module is registered, minimizing the critical
 * path to the lookup function.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module noc_router_lookup
  #(parameter FLIT_WIDTH = 32,
    parameter DEST_WIDTH = 5,
    parameter DESTS = 1,
    parameter OUTPUTS = 1,
    parameter [DESTS*OUTPUTS-1:0] ROUTES = {DESTS*OUTPUTS{1'b0}}
    )
   (
    input                   clk,
    input                   rst,

    input [FLIT_WIDTH-1:0]  in_flit,
    input                   in_last,
    input                   in_valid,
    output                  in_ready,

    output [OUTPUTS-1:0]    out_valid,
    output                  out_last,
    output [FLIT_WIDTH-1:0] out_flit,
    input [OUTPUTS-1:0]     out_ready
    );

   // We need to track worms and directly encode the output of the
   // current worm.
   reg [OUTPUTS-1:0]        worm;
   logic [OUTPUTS-1:0]      nxt_worm;
   // This is high if we are in a worm
   logic                    wormhole;
   assign wormhole = |worm;

   // Extract destination from flit
   logic [DEST_WIDTH-1:0]   dest;
   assign dest = in_flit[FLIT_WIDTH-1 -: DEST_WIDTH];

   // This is the selection signal of the slave, one hot so that it
   // directly serves as flow control valid
   logic [OUTPUTS-1:0]      valid;

   // Register slice at the output.
   noc_router_lookup_slice
     #(.FLIT_WIDTH (FLIT_WIDTH),
       .OUTPUTS    (OUTPUTS))
   u_slice
     (.*,
      .in_valid (valid));

   always_comb begin
      nxt_worm = worm;
      valid = 0;

      if (!wormhole) begin
         // We are waiting for a flit
         if (in_valid) begin
            // This is a header. Lookup output
            valid = ROUTES[dest*OUTPUTS +: OUTPUTS];
            if (in_ready & !in_last) begin
               // If we can push it further and it is not the only
               // flit, enter a worm and store the output
               nxt_worm = ROUTES[dest*OUTPUTS +: OUTPUTS];
            end
         end
      end else begin // if (!wormhole)
         // We are in a worm
         // The valid is set on the currently select output
         valid = worm & {OUTPUTS{in_valid}};
          if (in_ready & in_last) begin
             // End of worm
             nxt_worm = 0;
          end
       end
   end

   always_ff @(posedge clk) begin
      if (rst) begin
         worm <= 0;
      end else begin
         worm <= nxt_worm;
      end
   end

endmodule
