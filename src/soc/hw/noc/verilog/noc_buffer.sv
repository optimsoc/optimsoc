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
 * Buffer for NoC packets
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 *   Wei Song <wsong83@gmail.com>
 */

module noc_buffer
  #(parameter FLIT_WIDTH = 32,
    parameter DEPTH = 16,
    parameter FULLPACKET = 0,

    localparam ID_W = $clog2(DEPTH) // the width of the index
   )(
    input                       clk,
    input                       rst,

    // FIFO input side
    input [FLIT_WIDTH-1:0]      in_flit,
    input                       in_last,
    input                       in_valid,
    output                      in_ready,

    //FIFO output side
    output reg [FLIT_WIDTH-1:0] out_flit,
    output reg                  out_last,
    output reg                  out_valid,
    input                       out_ready,

    output [ID_W-1:0]           packet_size
    );

   // internal shift register
   reg [DEPTH-1:0][FLIT_WIDTH:0] data;
   reg [ID_W:0]                     rp; // read pointer
   logic                            reg_out_valid;  // local output valid
   logic                            in_fire, out_fire;

   assign in_ready = (rp != DEPTH - 1) || !reg_out_valid;
   assign in_fire = in_valid && in_ready;
   assign out_fire = out_valid && out_ready;

   always_ff @(posedge clk)
     if(rst)
       reg_out_valid <= 0;
     else if(in_valid)
       reg_out_valid <= 1;
     else if(out_fire && rp == 0)
       reg_out_valid <= 0;

   always_ff @(posedge clk)
     if(rst)
       rp <= 0;
     else if(in_fire && !out_fire && reg_out_valid)
       rp <= rp + 1;
     else if(out_fire && !in_fire && rp != 0)
       rp <= rp - 1;

   always @(posedge clk)
     if(in_fire)
       data <= {data, {in_last, in_flit}};

   generate                     // SRL does not allow parallel read
      if(FULLPACKET != 0) begin
         logic [DEPTH-1:0] data_last_buf, data_last_shifted;

         always @(posedge clk)
           if(rst)
             data_last_buf <= 0;
           else if(in_fire)
             data_last_buf <= {data_last_buf, in_last && in_valid};

         // extra logic to get the packet size in a stable manner
         assign data_last_shifted = data_last_buf << DEPTH - 1 - rp;

         function logic [ID_W:0] find_first_one(input logic [DEPTH-1:0] data);
            automatic int i;
            for(i=DEPTH-1; i>=0; i--)
              if(data[i]) return i;
            return DEPTH;
         endfunction // size_count

         assign packet_size = DEPTH - find_first_one(data_last_shifted);

         always_comb begin
            out_flit = data[rp][FLIT_WIDTH-1:0];
            out_last = data[rp][FLIT_WIDTH];
            out_valid = reg_out_valid && |data_last_shifted;
         end
      end else begin // if (FULLPACKET)
         assign packet_size = 0;
         always_comb begin
            out_flit = data[rp][FLIT_WIDTH-1:0];
            out_last = data[rp][FLIT_WIDTH];
            out_valid = reg_out_valid;
         end
      end
   endgenerate
endmodule // noc_buffer
