
import dii_package::dii_flit;

module debug_ring
  #(parameter PORTS = 1,
    parameter BUFFER_SIZE = 4)
   (input clk, rst,
    input  [PORTS-1:0][9:0] id_map,
    input  dii_flit [PORTS-1:0] dii_in, output [PORTS-1:0] dii_in_ready,
    output dii_flit [PORTS-1:0] dii_out, input [PORTS-1:0] dii_out_ready
   );

   dii_flit [1:0][1:0] ext_port;
   logic [1:0][1:0]    ext_port_ready;

   debug_ring_expand
     #(.PORTS(PORTS), .BUFFER_SIZE(BUFFER_SIZE))
   ring (
         .*,
         .ext_in        ( ext_port[0]       ),
         .ext_in_ready  ( ext_port_ready[0] ),
         .ext_out       ( ext_port[1]       ),
         .ext_out_ready ( ext_port_ready[1] )
         );

   // empty input for chain 0
   assign ext_port[0][0].valid = 1'b0;

   // connect the ends of chain 0 & 1
   assign ext_port[0][1] = ext_port[1][0];
   assign ext_port_ready[1][0] = ext_port_ready[0][1];

   // dump chain 1
   assign ext_port_ready[1][1] = 1'b1;

endmodule // debug_ring

