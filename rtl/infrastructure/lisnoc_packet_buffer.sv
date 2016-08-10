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
 * The packet buffer is similar to a FIFO but does only signal a flit
 * at the output when a complete packet is in the buffer. This relaxes
 * backpressure problems that may arise.
 *
 * (c) 2011-2013 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 */

`include "lisnoc_def.vh"

module lisnoc_packet_buffer(/*AUTOARG*/
   // Outputs
   in_ready, out_flit, out_valid, out_size,
   // Inputs
   clk, rst, in_flit, in_valid, out_ready
   );

   parameter data_width = 32;
   localparam flit_width = data_width+2;

   parameter  fifo_depth = 16;
   localparam size_width = $clog2(fifo_depth+1);

   localparam READY = 1'b0, BUSY = 1'b1;

   //inputs
   input                   clk, rst;
   input [flit_width-1:0]  in_flit;
   input                   in_valid;
   output                  in_ready;

   output [flit_width-1:0] out_flit;
   output                  out_valid;
   input                   out_ready;

   output reg [size_width-1:0] out_size;

   // Signals for fifo
   reg [flit_width-1:0] fifo_data [0:fifo_depth]; //actual fifo
   reg [fifo_depth:0]   fifo_write_ptr;

   reg [fifo_depth:0]   last_flits;

   wire                 full_packet;
   wire                 pop;
   wire                 push;

   wire [1:0] in_flit_type;
   assign in_flit_type = in_flit[flit_width-1:flit_width-2];

   wire                        in_is_last;
   assign in_is_last = (in_flit_type == `FLIT_TYPE_LAST) || (in_flit_type == `FLIT_TYPE_SINGLE);

   reg [fifo_depth-1:0]        valid_flits;

   always @(*) begin : valid_flits_comb
      integer i;
      // Set first element
      valid_flits[fifo_depth-1] = fifo_write_ptr[fifo_depth];
      for (i=fifo_depth-2;i>=0;i=i-1) begin
         valid_flits[i] = fifo_write_ptr[i+1] | valid_flits[i+1];
      end
   end

   assign full_packet = |(last_flits[fifo_depth-1:0] & valid_flits);

   assign pop = out_valid & out_ready;
   assign push = in_valid & in_ready;

   assign out_flit = fifo_data[0];
   assign out_valid = full_packet;

   assign in_ready = !fifo_write_ptr[fifo_depth];

   always @(*) begin : findfirstlast
      reg [size_width-1:0] i;
      reg [size_width-1:0] s;
      reg    found;

      s = 0;
      found = 0;

      for (i=0;i<fifo_depth;i=i+1) begin
         if (last_flits[i] && !found) begin
            s = i+1;
            found = 1;
         end
      end
      out_size = s;
   end

   always @(posedge clk) begin
      if (rst) begin
         fifo_write_ptr <= {{fifo_depth{1'b0}},1'b1};
      end else if (push & !pop) begin
         fifo_write_ptr <= fifo_write_ptr << 1;
      end else if (!push & pop) begin
         fifo_write_ptr <= fifo_write_ptr >> 1;
      end
   end

   always @(posedge clk) begin : shift_register
      if (rst) begin
         last_flits <= {fifo_depth+1{1'b0}};
      end else begin : shift
         integer i;
         for (i=0;i<fifo_depth-1;i=i+1) begin
            if (pop) begin
               if (push & fifo_write_ptr[i+1]) begin
                  fifo_data[i] <= in_flit;
                  last_flits[i] <= in_is_last;
               end else begin
                  fifo_data[i] <= fifo_data[i+1];
                  last_flits[i] <= last_flits[i+1];
               end
            end else if (push & fifo_write_ptr[i]) begin
               fifo_data[i] <= in_flit;
               last_flits[i] <= in_is_last;
            end
         end // for (i=0;i<fifo_depth-1;i=i+1)

         // Handle last element
         if (pop &  push & fifo_write_ptr[i+1]) begin
            fifo_data[i] <= in_flit;
            last_flits[i] <= in_is_last;
         end else if (push & fifo_write_ptr[i]) begin
            fifo_data[i] <= in_flit;
            last_flits[i] <= in_is_last;
         end
      end
   end // block: shift_register

endmodule
