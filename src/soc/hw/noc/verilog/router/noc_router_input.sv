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
 * This is the input stage of the router. It buffers the flits and
 * routes them to the proper output.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module noc_router_input
  #(parameter FLIT_WIDTH = 'x,
    parameter VCHANNELS = 1,
    parameter DESTS = 1,
    parameter OUTPUTS = 1,
    parameter [OUTPUTS*DESTS-1:0] ROUTES = {DESTS*OUTPUTS{1'b0}},
    parameter BUFFER_DEPTH = 4
    )
   (
    input                                  clk,
    input                                  rst,

    input [FLIT_WIDTH-1:0]                 in_flit,
    input                                  in_last,
    input [VCHANNELS-1:0]                  in_valid,
    output [VCHANNELS-1:0]                 in_ready,

    output [VCHANNELS-1:0][OUTPUTS-1:0]    out_valid,
    output [VCHANNELS-1:0]                 out_last,
    output [VCHANNELS-1:0][FLIT_WIDTH-1:0] out_flit,
    input [VCHANNELS-1:0][OUTPUTS-1:0]     out_ready
    );

   generate
      genvar                               v;

      for (v = 0; v < VCHANNELS; v++) begin : vc
         wire [FLIT_WIDTH-1:0] buffer_flit;
         wire                  buffer_last;
         wire                  buffer_valid;
         wire                  buffer_ready;

         noc_buffer
           #(.FLIT_WIDTH (FLIT_WIDTH),
             .DEPTH  (BUFFER_DEPTH))
         u_buffer
           (.*,
            .in_valid    (in_valid[v]),
            .in_ready    (in_ready[v]),
            .out_flit    (buffer_flit),
            .out_last    (buffer_last),
            .out_valid   (buffer_valid),
            .out_ready   (buffer_ready),
            .packet_size ()
            );

         noc_router_lookup
           #(.FLIT_WIDTH (FLIT_WIDTH), .DESTS (DESTS),
             .OUTPUTS (OUTPUTS), .ROUTES (ROUTES))
         u_lookup
           (.*,
            .in_flit   (buffer_flit),
            .in_last   (buffer_last),
            .in_valid  (buffer_valid),
            .in_ready  (buffer_ready),
            .out_flit  (out_flit[v]),
            .out_last  (out_last[v]),
            .out_valid (out_valid[v]),
            .out_ready (out_ready[v])
            );
      end
   endgenerate

endmodule // noc_router_input
