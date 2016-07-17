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

module debug_interface
  #(
    parameter SYSTEMID = 'x,
    parameter NUM_MODULES = 0
    )
  (
   input  clk,
   input  rst,
          
   glip_channel.slave glip_in,
   glip_channel.master glip_out,

   output dii_flit debug_out,
   input  debug_out_ready,
   input  dii_flit debug_in,
   output debug_in_ready,

   output sys_rst, cpu_rst
   );

   dii_flit him_to_scm;
   logic  him_to_scm_ready;
   
   osd_him
     u_him
       (.*,
        .dii_out       (him_to_scm),
        .dii_out_ready (him_to_scm_ready),
        .dii_in        (debug_in),
        .dii_in_ready  (debug_in_ready)
        );
   
   osd_scm
     #(.SYSTEMID (SYSTEMID),
       .NUM_MOD  (NUM_MODULES+1))
     u_scm
       (.*,
        .id  (1),
        .debug_in   (him_to_scm),
        .debug_in_ready (him_to_scm_ready)
       );
   
endmodule
