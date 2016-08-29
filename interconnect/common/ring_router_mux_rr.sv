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

module ring_router_mux_rr
  (
   input clk, rst,
   input dii_flit in0, output logic in0_ready,
   input dii_flit in1, output logic in1_ready,
   output dii_flit out_mux, input out_mux_ready
   );

   enum         { NOWORM0, NOWORM1, WORM0, WORM1 } state, nxt_state;

   always_ff @(posedge clk) begin
      if (rst) begin
         state <= NOWORM0;
      end else begin
         state <= nxt_state;
      end
   end

   always_comb begin
      nxt_state = state;
      out_mux.valid = 0;
      out_mux.data = 'x;
      out_mux.last = 'x;
      in0_ready = 0;
      in1_ready = 0;

      case (state)
        NOWORM0: begin
           if (in0.valid) begin
              in0_ready = out_mux_ready;
              out_mux = in0;
              out_mux.valid = 1;

              if (!in0.last) begin
                 nxt_state = WORM0;
              end
           end else if (in1.valid) begin
              in1_ready = out_mux_ready;
              out_mux = in1;
              out_mux.valid = 1;

              if (!in1.last) begin
                 nxt_state = WORM1;
              end
           end
        end
        NOWORM1: begin
           if (in1.valid) begin
              in1_ready = out_mux_ready;
              out_mux = in1;
              out_mux.valid = 1;

              if (!in1.last) begin
                 nxt_state = WORM1;
              end
           end else if (in0.valid) begin
              in0_ready = out_mux_ready;
              out_mux = in0;
              out_mux.valid = 1;

              if (!in0.last) begin
                 nxt_state = WORM0;
              end
           end
        end
        WORM0: begin
           in0_ready = out_mux_ready;
           out_mux = in0;

           if (out_mux.last & out_mux.valid & out_mux_ready) begin
              nxt_state = NOWORM1;
           end
        end
        WORM1: begin
           in1_ready = out_mux_ready;
           out_mux = in1;

           if (out_mux.last & out_mux.valid & out_mux_ready) begin
              nxt_state = NOWORM0;
           end
        end
      endcase // case (state)
   end

endmodule // ring_router_mux
