// Copyright 2016 by the authors
//
// Copyright and related rights are licensed under the Solderpad
// Hardware License, Version 0.51 (the "License"); you may not use
// this file except in compliance with the License. You may obtain a
// copy of the License at http://solderpad.org/licenses/SHL-0.51.
// Unless required by applicable law or agreed to in writing,
// software, hardware and materials distributed under this License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the
// License.
//
// Authors:
//    Wei Song <ws327@cam.ac.uk>
//    Stefan Wallentowitz <stefan@wallentowitz.de>

module osd_fifo
  #(parameter WIDTH = 'x,
    parameter DEPTH = 'x)
   (
      input              clk,
      input              rst,

      input [WIDTH-1:0]  in_data,
      input              in_valid,
      output             in_ready,

      output [WIDTH-1:0] out_data,
      output             out_valid,
      input              out_ready
      );

   // Signals for fifo
   reg [WIDTH-1:0] fifo_data [0:DEPTH-1]; //actual fifo
   reg [WIDTH-1:0] nxt_fifo_data [0:DEPTH-1];

   reg [DEPTH:0]   fifo_write_ptr;

   wire            pop;
   wire            push;

   assign pop = out_valid & out_ready;
   assign push = in_valid & in_ready;

   assign out_data = fifo_data[0];
   assign out_valid = !fifo_write_ptr[0];

   assign in_ready = !fifo_write_ptr[DEPTH];

   always @(posedge clk) begin
      if (rst) begin
         fifo_write_ptr <= {{DEPTH{1'b0}},1'b1};
      end else if (push & !pop) begin
         fifo_write_ptr <= fifo_write_ptr << 1;
      end else if (!push & pop) begin
         fifo_write_ptr <= fifo_write_ptr >> 1;
      end
   end

   always @(*) begin : shift_register_comb
      integer i;
      for (i=0;i<DEPTH;i=i+1) begin
         if (pop) begin
            if (push & fifo_write_ptr[i+1]) begin
               nxt_fifo_data[i] = in_data;
            end else if (i<DEPTH-1) begin
               nxt_fifo_data[i] = fifo_data[i+1];
            end else begin
               nxt_fifo_data[i] = fifo_data[i];
            end
         end else if (push & fifo_write_ptr[i]) begin
            nxt_fifo_data[i] = in_data;
         end else begin
            nxt_fifo_data[i] = fifo_data[i];
         end
      end
   end

   always @(posedge clk) begin : shift_register_seq
      integer i;
      for (i=0;i<DEPTH;i=i+1) begin
        fifo_data[i] <= nxt_fifo_data[i];
      end
   end

endmodule // noc_fifo
