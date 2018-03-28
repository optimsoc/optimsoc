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
 * This is a NoC router. It has a configurable number of input ports
 * and output ports, that are not necessarily equal. It supports
 * virtual channels, meaning that the flit signal between routers is
 * shared logically.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module noc_router
  #(parameter FLIT_WIDTH = 32,
    parameter VCHANNELS = 1,
    parameter INPUTS = 'x,
    parameter OUTPUTS = 'x,
    parameter BUFFER_SIZE_IN = 4,
    parameter BUFFER_SIZE_OUT = 4,
    parameter DESTS = 'x,
    parameter [OUTPUTS*DESTS-1:0] ROUTES = {DESTS*OUTPUTS{1'b0}}
    )
   (
    input                                clk, rst,

    output [OUTPUTS-1:0][FLIT_WIDTH-1:0] out_flit,
    output [OUTPUTS-1:0]                 out_last,
    output [OUTPUTS-1:0][VCHANNELS-1:0]  out_valid,
    input [OUTPUTS-1:0][VCHANNELS-1:0]   out_ready,

    input [INPUTS-1:0][FLIT_WIDTH-1:0]   in_flit,
    input [INPUTS-1:0]                   in_last,
    input [INPUTS-1:0][VCHANNELS-1:0]    in_valid,
    output [INPUTS-1:0][VCHANNELS-1:0]   in_ready
    );

   // The "switch" is just wiring (all logic is in input and
   // output). All inputs generate their requests for the outputs and
   // the output arbitrate between the input requests.

   // The input valid signals are one (or zero) hot and hence share
   // the flit signal.
   wire [INPUTS-1:0][VCHANNELS-1:0][FLIT_WIDTH-1:0] switch_in_flit;
   wire [INPUTS-1:0][VCHANNELS-1:0]                 switch_in_last;
   wire [INPUTS-1:0][VCHANNELS-1:0][OUTPUTS-1:0]    switch_in_valid;
   wire [INPUTS-1:0][VCHANNELS-1:0][OUTPUTS-1:0]    switch_in_ready;

   // Outputs are fully wired to receive all input requests.
   wire [OUTPUTS-1:0][VCHANNELS-1:0][INPUTS-1:0][FLIT_WIDTH-1:0] switch_out_flit;
   wire [OUTPUTS-1:0][VCHANNELS-1:0][INPUTS-1:0]                 switch_out_last;
   wire [OUTPUTS-1:0][VCHANNELS-1:0][INPUTS-1:0]                 switch_out_valid;
   wire [OUTPUTS-1:0][VCHANNELS-1:0][INPUTS-1:0]                 switch_out_ready;

   genvar                                                        i, v, o;
   generate
      for (i = 0; i < INPUTS; i++) begin : inputs
         // The input stages
         noc_router_input
               #(.FLIT_WIDTH(FLIT_WIDTH), .VCHANNELS(VCHANNELS),
                 .DESTS(DESTS), .OUTPUTS(OUTPUTS), .ROUTES(ROUTES),
                 .BUFFER_DEPTH (BUFFER_SIZE_IN))
         u_input
               (.*,
                .in_flit   (in_flit[i]),
                .in_last   (in_last[i]),
                .in_valid  (in_valid[i]),
                .in_ready  (in_ready[i]),
                .out_flit  (switch_in_flit[i]),
                .out_last  (switch_in_last[i]),
                .out_valid (switch_in_valid[i]),
                .out_ready (switch_in_ready[i])
                );
      end // block: inputs

      // The switching logic
      for (o = 0; o < OUTPUTS; o++) begin
         for (v = 0; v < VCHANNELS; v++) begin
            for (i = 0; i < INPUTS; i++) begin
               assign switch_out_flit[o][v][i] = switch_in_flit[i][v];
               assign switch_out_last[o][v][i] = switch_in_last[i][v];
               assign switch_out_valid[o][v][i] = switch_in_valid[i][v][o];
               assign switch_in_ready[i][v][o] = switch_out_ready[o][v][i];
            end
         end
      end

      for (o = 0; o < OUTPUTS; o++) begin :  outputs
         // The output stages
         noc_router_output
               #(.FLIT_WIDTH(FLIT_WIDTH), .VCHANNELS(VCHANNELS),
                 .INPUTS(INPUTS), .BUFFER_DEPTH(BUFFER_SIZE_OUT))
         u_output
               (.*,
                .in_flit   (switch_out_flit[o]),
                .in_last   (switch_out_last[o]),
                .in_valid  (switch_out_valid[o]),
                .in_ready  (switch_out_ready[o]),
                .out_flit  (out_flit[o]),
                .out_last  (out_last[o]),
                .out_valid (out_valid[o]),
                .out_ready (out_ready[o]));
      end
   endgenerate
endmodule // noc_router
