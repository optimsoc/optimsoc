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
 * Route packets to an output port depending on the class
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module noc_demux
  #(
    parameter FLIT_DATA_WIDTH = 32,
    parameter FLIT_TYPE_WIDTH = 34,
    parameter CHANNELS = 2,
    parameter [0:63] MAPPING = 'x,
    parameter CLASS_MSB = 26,
    parameter CLASS_LSB = 24
    )
   (
    input 				  clk, rst,

    input [FLIT_WIDTH-1:0] 		  in_flit,
    input 				  in_valid,
    output reg 				  in_ready,

    output [CHANNELS-1:0][FLIT_WIDTH-1:0] out_flit,
    output reg [CHANNELS-1:0] 		  out_valid,
    input [CHANNELS-1:0] 		  out_ready
    );

   localparam FLIT_WIDTH = FLIT_DATA_WIDTH + FLIT_TYPE_WIDTH;

   reg [CHANNELS-1:0]                         active;
   reg [CHANNELS-1:0]                         nxt_active;

   wire [2:0]                                 packet_class;
   wire [CHANNELS-1:0] 			      select;
   
   assign packet_class = in_flit[CLASS_MSB:CLASS_LSB];
   
   always @(*) begin : gen_select
      select = MAPPING[8*packet_class +: CHANNELS];
      if (select == 0) begin
         select = { {CHANNELS-1{1'b0}}, 1'b1};
      end
   end

   assign out_flit = {CHANNELS{in_flit}};
   
   always @(*) begin
      nxt_active = active;

      out_valid = 0;
      in_ready = 0;

      if (active == 0) begin
         in_ready = select & out_ready;
         out_valid = select & {CHANNELS{in_valid}};	 
	 
         if (in_valid & ~in_flit[FLIT_DATA_WIDTH]) begin
	    nxt_active = select;
         end
      end else begin
         in_ready = active & out_ready;
         out_valid = active & {CHANNELS{in_valid}};

         if (in_valid & in_flit[FLIT_DATA_WIDTH]) begin
            nxt_active = 0;
         end
      end
   end
   
   always @(posedge clk)
     if (rst) begin
        active <= '0;
     end else begin
        active <= nxt_active;
     end
   
endmodule // noc_demux


