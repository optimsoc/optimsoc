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
//    Stefan Wallentowitz <stefan@wallentowitz.de>
//    Anuj Rao <anujnr@gmail.com>

module osd_tracesample
  #(parameter WIDTH = 16)
   (
    input              clk,
    input              rst,
    input [WIDTH-1:0]  sample_data,
    input              sample_valid,

    output [WIDTH-1:0] fifo_data,
    output             fifo_overflow,
    output             fifo_valid,
    input              fifo_ready
    );
   
   reg [9:0]          ov_counter;

   logic               passthrough;
   assign passthrough = (ov_counter == 0);
   
   assign fifo_data[9:0] = passthrough ? sample_data[9:0] : ov_counter;
   generate
      if (WIDTH > 10)
        assign fifo_data[WIDTH-1:10] = sample_data[WIDTH-1:10];
   endgenerate
   
   assign fifo_overflow = ~passthrough;
   assign fifo_valid = passthrough ? sample_valid : 1;

   logic               ov_increment, ov_saturate, ov_complete, ov_again;

   assign ov_increment = (sample_valid & !fifo_ready);
   assign ov_saturate  = &ov_counter;
   assign ov_complete  = fifo_overflow & fifo_ready & !sample_valid;
   assign ov_again     = fifo_overflow & fifo_ready & sample_valid;
   
   always_ff @(posedge clk) begin
     if (rst | ov_complete)
       ov_counter <= 0;
     else if (ov_again)
       ov_counter <= 1;
     else if (ov_increment & !ov_saturate)
       ov_counter <= ov_counter + 1;
   end

endmodule // osd_tracesample
         
      
