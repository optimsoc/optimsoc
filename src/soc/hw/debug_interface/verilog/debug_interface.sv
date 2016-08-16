/* Copyright (c) 2016 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * =============================================================================
 *
 * Debug interface between the host and the target system
 *
 * This module encapsulates the basic Open SoC Debug components, the Host
 * Interface Module (HIM) and the System Control Module (SCM).
 * The two modules need to be connected to the debug NoC (DII) using the
 * scm_* and him_* ports.
 *
 * Parameters:
 *   NUM_MODULES:
 *     Number of modules in the debug system *in addition* to the HIM and SCM
 *     modules.
 *   SYSTEMID:
 *     Unique ID for the system (used for auto-discovery of system properties on
 *     the host).
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

import dii_package::dii_flit;

module debug_interface
  #(
    parameter SYSTEMID = 'x,
    parameter NUM_MODULES = 0
    )
  (
   input  clk,
   input  rst,

   // GLIP host connection
   glip_channel.slave glip_in,
   glip_channel.master glip_out,

   // HIM DII connection
   output dii_flit him_out,
   input  him_out_ready,
   input  dii_flit him_in,
   output him_in_ready,

   // SCM DII connection
   output dii_flit scm_out,
   input  scm_out_ready,
   input  dii_flit scm_in,
   output scm_in_ready,

   // system reset request
   output sys_rst,

   // CPU reset request
   output cpu_rst
   );

   dii_flit him_to_scm;
   logic  him_to_scm_ready;

   osd_him
      u_him(
         .clk           (clk),
         .rst           (rst),
         .glip_in       (glip_in),
         .glip_out      (glip_out),
         .dii_out       (him_out),
         .dii_out_ready (him_out_ready),
         .dii_in        (him_in),
         .dii_in_ready  (him_in_ready)
      );

   osd_scm
      #(
         .SYSTEMID (SYSTEMID),
         .NUM_MOD  (NUM_MODULES+1)
      )
      u_scm (
         .clk(clk),
         .rst(rst),

         .id  (1), // SCM must be ID 1 for discovery to work
         .debug_in   (scm_in),
         .debug_in_ready (scm_in_ready),
         .debug_out (scm_out),
         .debug_out_ready (scm_out_ready),

         .sys_rst(sys_rst),
         .cpu_rst(cpu_rst)
     );

endmodule
