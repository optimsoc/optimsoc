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
 *
 * This is the output port. It muxes between concurrent input packets
 * and buffers them before forwarding to the next router. If virtual
 * channels are used, it muxes between the different virtual channels
 * in this output port.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module noc_router_output
  #(parameter FLIT_WIDTH = 'x,
    parameter VCHANNELS = 1,
    parameter INPUTS = 1,
    parameter BUFFER_DEPTH = 4
    )
   (
    input                                             clk,
    input                                             rst,

    input [VCHANNELS-1:0][INPUTS-1:0][FLIT_WIDTH-1:0] in_flit,
    input [VCHANNELS-1:0][INPUTS-1:0]                 in_last,
    input [VCHANNELS-1:0][INPUTS-1:0]                 in_valid,
    output [VCHANNELS-1:0][INPUTS-1:0]                in_ready,

    output [FLIT_WIDTH-1:0]                           out_flit,
    output                                            out_last,
    output [VCHANNELS-1:0]                            out_valid,
    input [VCHANNELS-1:0]                             out_ready
    );

   genvar                                             v;

   wire [VCHANNELS-1:0][FLIT_WIDTH-1:0]               channel_flit;
   wire [VCHANNELS-1:0]                               channel_last;
   wire [VCHANNELS-1:0]                               channel_valid;
   wire [VCHANNELS-1:0]                               channel_ready;

   generate
      for (v = 0; v < VCHANNELS; v++) begin
         wire [FLIT_WIDTH-1:0] buffer_flit;
         wire                  buffer_last;
         wire                  buffer_valid;
         wire                  buffer_ready;

         noc_mux
           #(.FLIT_WIDTH (FLIT_WIDTH), .CHANNELS (INPUTS))
         u_mux
           (.*,
            .in_flit   (in_flit[v]),
            .in_last   (in_last[v]),
            .in_valid  (in_valid[v]),
            .in_ready  (in_ready[v]),
            .out_flit  (buffer_flit),
            .out_last  (buffer_last),
            .out_valid (buffer_valid),
            .out_ready (buffer_ready));

         noc_buffer
           #(.FLIT_WIDTH (FLIT_WIDTH), .DEPTH(BUFFER_DEPTH))
         u_buffer
           (.*,
            .in_flit     (buffer_flit),
            .in_last     (buffer_last),
            .in_valid    (buffer_valid),
            .in_ready    (buffer_ready),
            .out_flit    (channel_flit[v]),
            .out_last    (channel_last[v]),
            .out_valid   (channel_valid[v]),
            .out_ready   (channel_ready[v]),
            .packet_size ()
            );
      end // for (v = 0; v < VCHANNELS; v++)

      if (VCHANNELS > 1) begin : vc_mux
         noc_vchannel_mux
           #(.FLIT_WIDTH (FLIT_WIDTH), .CHANNELS(VCHANNELS))
         u_mux
           (.*,
            .in_flit  (channel_flit),
            .in_last  (channel_last),
            .in_valid (channel_valid),
            .in_ready (channel_ready)
            );
      end else begin // block: vc_mux
         assign out_flit = channel_flit[0];
         assign out_last = channel_last[0];
         assign out_valid = channel_valid[0];
         assign channel_ready[0] = out_ready;
      end
   endgenerate

endmodule // noc_router_output
