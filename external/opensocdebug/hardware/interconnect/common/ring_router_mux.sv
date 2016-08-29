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

module ring_router_mux
  (
   input clk, rst,
   input dii_flit in_ring, output logic in_ring_ready,
   input dii_flit in_local, output logic in_local_ready,
   output dii_flit out_mux, input out_mux_ready
   );

   enum         { NOWORM, WORM_LOCAL, WORM_RING } state, nxt_state;

   always_ff @(posedge clk) begin
      if (rst) begin
         state <= NOWORM;
      end else begin
         state <= nxt_state;
      end
   end

   always_comb begin
      nxt_state = state;
      out_mux.valid = 0;
      out_mux.data = 'x;
      out_mux.last = 'x;
      in_ring_ready = 0;
      in_local_ready = 0;

      case (state)
        NOWORM: begin
           if (in_ring.valid) begin
              in_ring_ready = out_mux_ready;
              out_mux = in_ring;
              out_mux.valid = 1'b1;

              if (!in_ring.last) begin
                 nxt_state = WORM_RING;
              end
           end else if (in_local.valid) begin
              in_local_ready = out_mux_ready;
              out_mux = in_local;
              out_mux.valid = 1'b1;

              if (!in_local.last) begin
                 nxt_state = WORM_LOCAL;
              end
           end // if (in_local.valid)
        end // case: NOWORM
        WORM_RING: begin
           in_ring_ready = out_mux_ready;
           out_mux.valid = in_ring.valid;
           out_mux.last = in_ring.last;
           out_mux.data = in_ring.data;

           if (out_mux.last & out_mux.valid & out_mux_ready) begin
              nxt_state = NOWORM;
           end
        end
        WORM_LOCAL: begin
           in_local_ready = out_mux_ready;
           out_mux.valid = in_local.valid;
           out_mux.last = in_local.last;
           out_mux.data = in_local.data;

           if (out_mux.last & out_mux.valid & out_mux_ready) begin
              nxt_state = NOWORM;
           end
        end
      endcase // case (state)
   end

endmodule // ring_router_mux
