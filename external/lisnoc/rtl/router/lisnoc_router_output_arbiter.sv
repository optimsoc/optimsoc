/* Copyright (c) 2015 by the author(s)
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
 * This is the arbiter for the link, that chooses one virtual channel
 * to transfer to the next hop.
 *
 * Author(s):
 *   Andreas Lankes <andreas.lankes@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 * TODO:
 *  - check for one-hot coding of channel
 */

`include "lisnoc_def.vh"

module lisnoc_router_output_arbiter(/*AUTOARG*/
   // Outputs
   fifo_ready_o, link_valid_o, link_flit_o,
   // Inputs
   clk, rst, fifo_valid_i, fifo_flit_i, link_ready_i
   );

   parameter flit_data_width = 32;
   parameter flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;


   parameter vchannels = 1;

   localparam CHANNEL_WIDTH = $clog2(vchannels);
// localparam CHANNEL_WIDTH = 1;

   input                  clk, rst;


   // fifo side
   input [vchannels-1:0]            fifo_valid_i;
   input [vchannels*flit_width-1:0] fifo_flit_i;
   output reg [vchannels-1:0]        fifo_ready_o;


   output reg [vchannels-1:0]        link_valid_o;
   output [flit_width-1:0]           link_flit_o;
   input [vchannels-1:0]             link_ready_i;


   // channel that was last served in the round robin process
   reg [CHANNEL_WIDTH-1:0]           prev_channel;

   wire [vchannels-1:0]              serviceable;
   assign serviceable = (fifo_valid_i) & link_ready_i;

   reg [CHANNEL_WIDTH-1:0]           channel;


   reg [CHANNEL_WIDTH-1:0]           sel_channel;
   reg                               channel_selected;

   wire [flit_width-1:0]             fifo_flit_i_array [0:vchannels-1];
   genvar                            v;
   for (v=0;v<vchannels;v=v+1) begin
      assign fifo_flit_i_array[v] = fifo_flit_i[(v+1)*flit_width-1:v*flit_width];
   end

   assign link_flit_o = fifo_flit_i_array[channel];

   always @ (*) begin
      if (rst) begin
         link_valid_o = {vchannels{1'b0}};
         channel  = 3'b000;
         fifo_ready_o  = {vchannels{1'b0}};
      end else begin
         channel  = prev_channel;
         link_valid_o = {vchannels{1'b0}};
         fifo_ready_o  = {vchannels{1'b0}};

         sel_channel = channel;
         channel_selected = 0;

         repeat (vchannels) begin
            sel_channel = sel_channel + 1;
            if (sel_channel == vchannels)
               sel_channel = 0;

            // check if we can serve this channel
            if (serviceable[sel_channel]) begin
               channel = sel_channel;
               channel_selected = 1;
            end
         end // repeat

         if (channel_selected) begin
            link_valid_o[channel] = 1'b1;
            fifo_ready_o[channel] = 1'b1;
         end
      end // if
   end // always

   always @(posedge clk) begin
      prev_channel <= channel;
   end

endmodule // lisnoc_router_output_arbiter

`include "lisnoc_undef.vh"
