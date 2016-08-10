/* Copyright (c) 2014-2015 by the author(s)
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
 * GLIP JTAG: FIFO
 *
 * This FIFO implementation was originally part of LISNoC.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module glip_jtag_fifo(/*AUTOARG*/
   // Outputs
   in_ready, out_flit, out_valid,
   fifo_free_space,
   // Inputs
   clk, rst, in_flit, in_valid, out_ready
   );

   parameter FLIT_DATA_WIDTH = 32;
   parameter FLIT_TYPE_WIDTH = 2;
   parameter PACKET_LENGTH   = 0;

   localparam FLIT_WIDTH = FLIT_DATA_WIDTH+FLIT_TYPE_WIDTH;

   parameter LENGTH = 16;
   parameter FIFO_FREE_SPACE_WIDTH = 3;

   input  clk;
   input  rst;

   // FIFO input side
   input  [FLIT_WIDTH-1:0] in_flit;  // input
   input  in_valid;                   // write_enable
   output in_ready;                   // accepting new data

   //FIFO output side
   output [FLIT_WIDTH-1:0] out_flit;  // data_out
   output out_valid;                   // data available
   input  out_ready;                   // read request

   output reg [FIFO_FREE_SPACE_WIDTH-1:0] fifo_free_space;

   // Signals for fifo
   reg [FLIT_WIDTH-1:0] fifo_data [0:LENGTH-1]; //actual fifo

   reg [LENGTH:0]         fifo_write_ptr;

   wire                   pop;
   wire                   push;

   assign pop = out_valid & out_ready;
   assign push = in_valid & in_ready;

   assign out_flit = fifo_data[0];
   assign out_valid = !fifo_write_ptr[0];

   assign in_ready = !fifo_write_ptr[LENGTH];

   always @(posedge clk) begin
      if (rst) begin
         fifo_write_ptr <= {{LENGTH{1'b0}},1'b1};
         fifo_free_space <= LENGTH;
      end else if (push & !pop) begin
         fifo_write_ptr <= fifo_write_ptr << 1;
         fifo_free_space <=  fifo_free_space - 1;
      end else if (!push & pop) begin
         fifo_write_ptr <= fifo_write_ptr >> 1;
         fifo_free_space <=  fifo_free_space + 1;
      end
   end

   integer i;

   always @(posedge clk) begin : shift_register_seq
      for (i=0;i<LENGTH;i=i+1) begin
         if (pop) begin
            if (push & fifo_write_ptr[i+1]) begin
               fifo_data[i] <= in_flit;
            end else if (i<LENGTH-1) begin
               fifo_data[i] <= fifo_data[i+1];
            end else begin
               fifo_data[i] <= fifo_data[i];
            end
         end else if (push & fifo_write_ptr[i]) begin
            fifo_data[i] <= in_flit;
         end else begin
            fifo_data[i] <= fifo_data[i];
         end
      end
   end

endmodule
