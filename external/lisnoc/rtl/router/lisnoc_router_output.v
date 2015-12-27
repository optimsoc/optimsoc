/* Copyright (c) 2015 by the author(s)
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
 * This module is an output port.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *
 * TODO:
 *  - when FIFO is empty, the flit can go to decode directly
 */

`include "lisnoc_def.vh"

module lisnoc_router_output (/*AUTOARG*/
   // Outputs
   link_flit, link_valid, switch_read,
   // Inputs
   clk, rst, link_ready, switch_request, switch_flit
   );

   parameter  flit_data_width = 32;
   parameter  flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;

   parameter use_prio = 0;
   parameter ph_prio_width = 4;
   parameter ph_prio_offset = 5;

   parameter vchannels = 1;
   parameter ports = 5;

   parameter fifo_length = 4;

   input clk, rst;
   output [flit_width-1:0] link_flit;  // current flit
   output [vchannels-1:0]   link_valid; // current valid for which channel
   input [vchannels-1:0]    link_ready; // notification when registered

   // The switch interface
   input [ports*vchannels-1:0]             switch_request;     // direction requests
   input [flit_width*vchannels*ports-1:0] switch_flit;        // corresponding flit
   output [ports*vchannels-1:0]            switch_read;        // destination acknowledge

   wire [vchannels-1:0] valid;
   wire [flit_width*vchannels-1:0] flit;
   wire [vchannels-1:0] ready;

   genvar               v,p;

   for (v=0;v<vchannels;v=v+1) begin: vchannel
      wire [flit_width*ports-1:0] input_flits;
      wire [flit_width-1:0] arbiter_flit;
      wire arbiter_valid;
      wire fifo_ready;

      for (p=0;p<ports;p=p+1) begin
         assign input_flits[(p+1)*flit_width-1:p*flit_width] = switch_flit[(p*vchannels+v+1)*flit_width-1:(p*vchannels+v)*flit_width];
      end

      if (use_prio == 0) begin
        lisnoc_router_arbiter
        #(.vchannels(vchannels), .ports(ports),.flit_data_width(flit_data_width),.flit_type_width(flit_type_width))
        arbiter(// Outputs
                            .read_o                     (switch_read[(v+1)*ports-1:v*ports]),
                            .flit_o                     (arbiter_flit),
                            .valid_o                    (arbiter_valid),
                            // Inputs
                            .clk                        (clk),
                            .rst                        (rst),
                            .flit_i                     (input_flits),
                            .request_i          (switch_request[(v+1)*ports-1:v*ports]),
                            .ready_i                    (fifo_ready));

      end else if (use_prio == 1)begin
        lisnoc_router_arbiter_prio
        #(.vchannels(vchannels), .ports(ports),.flit_data_width(flit_data_width),.flit_type_width(flit_type_width),.ph_prio_width(ph_prio_width),
        .ph_prio_offset(ph_prio_offset))
        arbiter(// Outputs
                            .read_o                      (switch_read[(v+1)*ports-1:v*ports]),
                            .flit_o                      (arbiter_flit),
                            .valid_o                    (arbiter_valid),
                // Inputs
                            .clk                        (clk),
                            .rst                        (rst),
                            .flit_i                      (input_flits),
                            .request_i  (switch_request[(v+1)*ports-1:v*ports]),
                            .ready_i                    (fifo_ready));
      end

      /* lisnoc_fifo AUTO_TEMPLATE (
       .in_ready  (fifo_ready),
       .out_flit  (flit[(v+1)*flit_width-1:v*flit_width]),
       .out_valid (valid[v]),
       .in_flit   (arbiter_flit),
       .in_valid  (arbiter_valid),
       .out_ready (ready[v]),
       );*/

      lisnoc_fifo #(.LENGTH(fifo_length),.flit_data_width(flit_data_width),
            .flit_type_width(flit_type_width))
         fifo (/*AUTOINST*/
               // Outputs
               .in_ready                (fifo_ready),            // Templated
               .out_flit                (flit[(v+1)*flit_width-1:v*flit_width]), // Templated
               .out_valid               (valid[v]),              // Templated
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .in_flit                 (arbiter_flit),          // Templated
               .in_valid                (arbiter_valid),         // Templated
               .out_ready               (ready[v]));             // Templated
   end // block: vchannel

   /* lisnoc_router_output_arbiter AUTO_TEMPLATE (
    .fifo_ready_o (ready),
    .link_valid_o (link_valid),
    .link_flit_o  (link_flit),
    .fifo_valid_i (valid),
    .fifo_flit_i  (flit),
    .link_ready_i (link_ready),
    ); */
   lisnoc_router_output_arbiter
      #(.vchannels(vchannels),.flit_data_width(flit_data_width), .flit_type_width(flit_type_width))
      output_arbiter(/*AUTOINST*/
                     // Outputs
                     .fifo_ready_o      (ready),                 // Templated
                     .link_valid_o      (link_valid),            // Templated
                     .link_flit_o       (link_flit),             // Templated
                     // Inputs
                     .clk               (clk),
                     .rst               (rst),
                     .fifo_valid_i      (valid),                 // Templated
                     .fifo_flit_i       (flit),                  // Templated
                     .link_ready_i      (link_ready));           // Templated

endmodule // noc_router_output

`include "lisnoc_undef.vh"
