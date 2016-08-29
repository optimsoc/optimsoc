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

module ring_router_demux
  (
   input       clk, rst,
   input [9:0] id,
   input dii_flit in_ring, output in_ring_ready,
   output dii_flit out_local, input out_local_ready,
   output dii_flit out_ring, input out_ring_ready
   );

   assign out_local.data = in_ring.data;
   assign out_local.last = in_ring.last;
   assign out_ring.data = in_ring.data;
   assign out_ring.last = in_ring.last;

   reg         worm;
   reg         worm_local;

   logic       is_local;
   assign is_local = (in_ring.data[9:0] === id);

   always_ff @(posedge clk) begin
      if (rst) begin
         worm <= 0;
         worm_local <= 1'bx;
      end else begin
         if (!worm) begin
            worm_local <= is_local;
            if (in_ring_ready & in_ring.valid & !in_ring.last) begin
               worm <= 1;
            end
         end else begin
            if (in_ring_ready & in_ring.valid & in_ring.last) begin
               worm <= 0;
            end
         end
      end
   end

   logic switch_local;
   assign switch_local = worm ? worm_local : is_local;

   assign out_ring.valid = !switch_local & in_ring.valid;
   assign out_local.valid = switch_local & in_ring.valid;

   assign in_ring_ready = switch_local ? out_local_ready : out_ring_ready;

endmodule // ring_router_demux
