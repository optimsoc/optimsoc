/* Copyright (c) 2016-2017 by the author(s)
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
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

/**
 * Debug interface between the host and the target system
 *
 * This module encapsulates the basic Open SoC Debug components, the Host
 * Interface Module (HIM) and the Subnet Controller Module (SCM).
 */
module debug_interface
  import dii_package::dii_flit;
  #(
    parameter SYSTEM_VENDOR_ID = 'x,
    parameter SYSTEM_DEVICE_ID = 'x,
    parameter NUM_MODULES = 0,
    parameter MAX_PKT_LEN = 'x,
    parameter SUBNET_BITS = 6,
    parameter LOCAL_SUBNET = 0,
    parameter DEBUG_ROUTER_BUFFER_SIZE = 4
  ) (
   input  clk,
   input  rst,

   // GLIP host connection
   glip_channel.slave glip_in,
   glip_channel.master glip_out,

   // ring connection
   output dii_flit [1:0] ring_out,
   input [1:0] ring_out_ready,
   input dii_flit [1:0] ring_in,
   output [1:0] ring_in_ready,

   // system reset request
   output sys_rst,

   // CPU reset request
   output cpu_rst
   );

   dii_flit ring_tie;
   assign ring_tie.valid = 0;
   logic  ring_tie_ready;

   dii_flit [1:0] dii_in;
   logic [1:0] dii_in_ready;
   dii_flit [1:0] dii_out;
   logic [1:0] dii_out_ready;


   dii_flit him_debug_in;
   logic him_debug_in_ready;
   dii_flit him_debug_out;
   logic him_debug_out_ready;

   dii_flit scm_debug_in;
   logic scm_debug_in_ready;
   dii_flit scm_debug_out;
   logic scm_debug_out_ready;


   ring_router_gateway
      #(.BUFFER_SIZE(DEBUG_ROUTER_BUFFER_SIZE), .SUBNET_BITS(SUBNET_BITS),
        .LOCAL_SUBNET(LOCAL_SUBNET))
      u_ring_router_gateway(
         .clk(clk),
         .rst(rst),

         // the gateway is always at local address 0
         .id(16'h0),

         .ring_in0(ring_in[0]),
         .ring_in0_ready(ring_in_ready[0]),
         .ring_in1(ring_in[1]),
         .ring_in1_ready(ring_in_ready[1]),


         .ring_out0(ring_out[0]),
         .ring_out0_ready(ring_out_ready[0]),
         .ring_out1(ring_out[1]),
         .ring_out1_ready(ring_out_ready[1]),

         // local traffic for address 0: SCM
         .local_in(scm_debug_out),
         .local_in_ready(scm_debug_out_ready),
         .local_out(scm_debug_in),
         .local_out_ready(scm_debug_in_ready),

         // traffic not belonging to LOCAL_SUBNET (sent out to the host)
         .ext_in(him_debug_out),
         .ext_in_ready(him_debug_out_ready),
         .ext_out(him_debug_in),
         .ext_out_ready(him_debug_in_ready)
      );

   // Host Interface: all traffic to foreign subnets goes through this interface
   osd_him
     #(.MAX_PKT_LEN(MAX_PKT_LEN))
   u_him
     (
      .clk             (clk),
      .rst             (rst),
      .glip_in         (glip_in),
      .glip_out        (glip_out),
      .dii_out         (him_debug_out),
      .dii_out_ready   (him_debug_out_ready),
      .dii_in          (him_debug_in),
      .dii_in_ready    (him_debug_in_ready)
      );

   // Subnet Control Module
   // Manages this subnet, i.e. the on-chip OSD part
   osd_scm
      #(
         .SYSTEM_VENDOR_ID(SYSTEM_VENDOR_ID),
         .SYSTEM_DEVICE_ID(SYSTEM_DEVICE_ID),
         .NUM_MOD(NUM_MODULES)
      )
      u_scm (
         .clk             (clk),
         .rst             (rst),

         .id              (16'd0), // must be 0

         .debug_in        (scm_debug_in),
         .debug_in_ready  (scm_debug_in_ready),
         .debug_out       (scm_debug_out),
         .debug_out_ready (scm_debug_out_ready),

         .sys_rst         (sys_rst),
         .cpu_rst         (cpu_rst)
     );

endmodule
