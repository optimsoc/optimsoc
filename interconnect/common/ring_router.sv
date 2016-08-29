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

module ring_router
  #(parameter BUFFER_SIZE=4)
   (
    input         clk, rst,

    input [9:0]   id,

    input dii_flit   ring_in0, output ring_in0_ready,
    input dii_flit   ring_in1, output ring_in1_ready,

    output dii_flit   ring_out0, input ring_out0_ready,
    output dii_flit   ring_out1, input ring_out1_ready,

    input dii_flit   local_in, output local_in_ready,
    output dii_flit   local_out, input local_out_ready
    );

   dii_flit ring_fwd0; logic ring_fwd0_ready;
   dii_flit ring_fwd1; logic ring_fwd1_ready;
   dii_flit ring_local0; logic ring_local0_ready;
   dii_flit ring_local1; logic ring_local1_ready;
   dii_flit ring_muxed; logic ring_muxed_ready;

   ring_router_demux
     u_demux0(.*,
              .in_ring         ( ring_in0          ),
              .in_ring_ready   ( ring_in0_ready    ),
              .out_local       ( ring_local0       ),
              .out_local_ready ( ring_local0_ready ),
              .out_ring        ( ring_fwd0         ),
              .out_ring_ready  ( ring_fwd0_ready   )
              );

   ring_router_demux
     u_demux1(.*,
              .in_ring         ( ring_in1          ),
              .in_ring_ready   ( ring_in1_ready    ),
              .out_local       ( ring_local1       ),
              .out_local_ready ( ring_local1_ready ),
              .out_ring        ( ring_fwd1         ),
              .out_ring_ready  ( ring_fwd1_ready   )
              );

   ring_router_mux_rr
     u_mux_local(.*,
                 .in0           ( ring_local0       ),
                 .in0_ready     ( ring_local0_ready ),
                 .in1           ( ring_local1       ),
                 .in1_ready     ( ring_local1_ready ),
                 .out_mux       ( local_out         ),
                 .out_mux_ready ( local_out_ready   )
                 );

   ring_router_mux
     u_mux_ring0(.*,
                 .in_ring        ( ring_fwd0        ),
                 .in_ring_ready  ( ring_fwd0_ready  ),
                 .in_local       ( local_in         ),
                 .in_local_ready ( local_in_ready   ),
                 .out_mux        ( ring_muxed       ),
                 .out_mux_ready  ( ring_muxed_ready )
                 );

   dii_buffer
     #(.BUF_SIZE(BUFFER_SIZE))
   u_buffer0(.*,
             .packet_size       (                  ),
             .flit_in           ( ring_muxed       ),
             .flit_in_ready     ( ring_muxed_ready ),
             .flit_out          ( ring_out0        ),
             .flit_out_ready    ( ring_out0_ready  )
             );

   dii_buffer
     #(.BUF_SIZE(BUFFER_SIZE))
   u_buffer1(.*,
             .packet_size       (                  ),
             .flit_in           ( ring_fwd1        ),
             .flit_in_ready     ( ring_fwd1_ready  ),
             .flit_out          ( ring_out1        ),
             .flit_out_ready    ( ring_out1_ready  )
             );

endmodule
