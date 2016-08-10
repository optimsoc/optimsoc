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
 * TODO:
 *  - add clock domain crossing
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 */

`include "lisnoc16_def.vh"

module lisnoc16_usb_packet_buffer(/*AUTOARG*/
   // Outputs
   in_ready, out_flit, out_valid, next_last_flit_position,
   // Inputs
   clk, rst, in_flit, in_valid, out_ready
   );

   parameter  fifo_depth = `MAX_NOC16_PACKET_LENGTH;
   localparam ptr_width  = $clog2(fifo_depth);

   parameter READY = 1'b0, BUSY = 1'b1;

   //inputs
   input                   clk, rst;

   //NoC-IN
   input [`FLIT16_WIDTH-1:0] in_flit;
   input 		   in_valid;
   output 		   in_ready;


   //NoC-OUT
   output [`FLIT16_WIDTH-1:0] out_flit;
   output 		    out_valid;
   input 		    out_ready;

   //
   output reg [`LD_MAX_NOC16_PACKET_LENGTH-1:0] next_last_flit_position;



   // Signals for fifo
   reg [`FLIT16_WIDTH-1:0] fifo_data [0:fifo_depth]; //actual fifo
   reg [fifo_depth:0]      fifo_write_ptr;

   reg [fifo_depth:0]      last_flits;

   wire                    full_packet;
   wire                    pop;
   wire                    push;

   wire [`FLIT16_TYPE_WIDTH-1:0] in_flit_type;
   assign in_flit_type = in_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB];

   wire                          in_is_last;
   assign in_is_last = (in_flit_type == `FLIT16_TYPE_LAST) || (in_flit_type == `FLIT16_TYPE_SINGLE);

   reg [fifo_depth-1:0]        valid_flits;

   always @(*) begin : valid_flits_comb
      integer i;
      // Set first element
      valid_flits[fifo_depth-1] = fifo_write_ptr[fifo_depth];
      for (i=fifo_depth-2;i>=0;i=i-1) begin
         valid_flits[i] = fifo_write_ptr[i+1] | valid_flits[i+1];
      end
   end

   assign full_packet = |(last_flits & valid_flits);

   assign pop = out_valid & out_ready;
   assign push = in_valid & in_ready;

   assign out_flit = fifo_data[0];
   assign out_valid = full_packet;

   assign in_ready = !fifo_write_ptr[fifo_depth];

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



   //find first in last!

   always @(*) begin: find_first_one
      integer i;
      integer  not_done;
      not_done = 1;
      next_last_flit_position = 1;

      for (i=1; i<=`MAX_NOC16_PACKET_LENGTH; i = i+1) begin
         //TODO: genau schaun ob das gut geht - sollte eigentlich schon.
         if (not_done) begin
            if ((last_flits[i-1] == 1'b1) && valid_flits[i-1]) begin
               not_done = 0;
               next_last_flit_position = i;
            end
         end
      end
   end // block: find_first_one

endmodule
