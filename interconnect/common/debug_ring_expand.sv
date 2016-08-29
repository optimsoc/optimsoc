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

import dii_package::dii_flit;

module debug_ring_expand
  #(parameter PORTS = 1,
    parameter BUFFER_SIZE = 4)
   (input clk, rst,
    input  [PORTS-1:0][9:0] id_map,
    input  dii_flit [PORTS-1:0] dii_in, output [PORTS-1:0] dii_in_ready,
    output dii_flit [PORTS-1:0] dii_out, input [PORTS-1:0] dii_out_ready,
    input  dii_flit [1:0] ext_in, output [1:0] ext_in_ready, // extension input ports
    output dii_flit [1:0] ext_out, input [1:0] ext_out_ready // extension output ports
   );

   genvar i;

   dii_flit [1:0][PORTS:0] chain;
   logic [1:0][PORTS:0] chain_ready;

   generate
      for(i=0; i<PORTS; i++) begin
         ring_router
           #(.BUFFER_SIZE(BUFFER_SIZE))
         u_router(
                  .*,
                  .id              ( id_map[i]                   ),
                  .ring_in0        ( chain[0][i]                 ),
                  .ring_in0_ready  ( chain_ready[0][i]           ),
                  .ring_in1        ( chain[1][i]                 ),
                  .ring_in1_ready  ( chain_ready[1][i]           ),
                  .ring_out0       ( chain[0][i+1]               ),
                  .ring_out0_ready ( chain_ready[0][i+1]         ),
                  .ring_out1       ( chain[1][i+1]               ),
                  .ring_out1_ready ( chain_ready[1][i+1]         ),
                  .local_in        ( dii_in[i]                   ),
                  .local_in_ready  ( dii_in_ready[i]             ),
                  .local_out       ( dii_out[i]                  ),
                  .local_out_ready ( dii_out_ready[i]            )
                  );
      end // for (i=0; i<PORTS, i++)
   endgenerate

   // the expanded ports
   generate
      for(i=0; i<2; i++) begin
         assign chain[i][0] = ext_in[i];
         assign ext_in_ready[i] = chain_ready[i][0];
         assign ext_out[i] = chain[i][PORTS];
         assign chain_ready[i][PORTS] = ext_out_ready[i];
      end
   endgenerate

endmodule // debug_ring_expand
