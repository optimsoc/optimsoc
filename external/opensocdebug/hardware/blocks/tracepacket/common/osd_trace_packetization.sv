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

import dii_package::dii_flit;

module osd_trace_packetization
  #(parameter WIDTH='x)
   (
    input             clk,
    input             rst,

    input [9:0]       id,

    input [WIDTH-1:0] trace_data,
    input             trace_overflow,
    input             trace_valid,
    output reg        trace_ready,

    output            dii_flit debug_out,
    input             debug_out_ready
    );

   localparam NUM_FLITS = ((WIDTH + 15) >> 4);
   localparam COUNTER_WIDTH = $clog2(NUM_FLITS);

   localparam FILL_LAST = NUM_FLITS*16 - WIDTH;

   reg [COUNTER_WIDTH-1:0] counter;
   logic [COUNTER_WIDTH-1:0] nxt_counter;

   enum { IDLE, SOURCE, STATUS, EVENT } state, nxt_state;

   always_ff @(posedge clk) begin
      if (rst) begin
         counter <= 0;
         state <= IDLE;
      end else begin
         counter <= nxt_counter;
         state <= nxt_state;
      end
   end

   always_comb begin
      trace_ready = 0;
      debug_out.valid = 0;
      debug_out.data = 'x;
      debug_out.last = 0;
      nxt_state = state;
      nxt_counter = counter;

      case (state)
        IDLE: begin
           debug_out.data = 16'h0;
           if (trace_valid) begin
              debug_out.valid = 1;
              if (debug_out_ready) begin
                 nxt_state = SOURCE;
              end
           end
        end
        SOURCE: begin
           debug_out.data[15:14] = 2'h2;           // Trace event
           debug_out.data[13:12] = 2'h0;           // Reserved
           debug_out.data[11]    = trace_overflow; // Status info
           debug_out.data[10]    = 1'b0;           // Bulk packet
           debug_out.data[9:0]   = id;
           debug_out.valid = 1;
           nxt_counter = 0;
           if (debug_out_ready)
             nxt_state = trace_overflow ? STATUS : EVENT;
        end // case: SOURCE
        STATUS: begin
           debug_out.valid = 1;
           debug_out.data = {1'b1, 5'h0, trace_data[9:0]};
           debug_out.last = 1;
           if (debug_out_ready) begin
              nxt_state = IDLE;
              trace_ready = 1;
           end
        end
        EVENT: begin
           debug_out.valid = 1;
           if (counter < NUM_FLITS - 1) begin
              debug_out.data = trace_data[(counter+1)*16-1 -: 16];
              if (debug_out_ready) begin
                 nxt_counter = counter + 1;
              end
           end else begin
              debug_out.last = 1;
              if (FILL_LAST > 0) begin
                 debug_out.data = { {FILL_LAST{1'b0}}, trace_data[WIDTH-1:WIDTH-(16-FILL_LAST)] };
              end else begin
                 debug_out.data = trace_data[(counter+1)*16-1 -: 16];
              end
              if (debug_out_ready) begin
                 trace_ready = 1;
                 nxt_state = IDLE;
              end
           end // else: !if(counter < NUM_FLITS - 1)
        end
      endcase
   end

endmodule // osd_trace_packetization
