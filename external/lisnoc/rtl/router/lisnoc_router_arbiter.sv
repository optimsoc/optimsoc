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
 * This is the arbiter for each vchannel on the output port. It works
 * as round robin arbiter at the moment.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Andreas Lankes <andreas.lankes@tum.de>
 */

`include "lisnoc_def.vh"

module lisnoc_router_arbiter (/*AUTOARG*/
   // Outputs
   read_o, flit_o, valid_o,
   // Inputs
   clk, rst, flit_i, request_i, ready_i
   );

   // Parameter definitions
   parameter flit_data_width = 32;
   parameter flit_type_width = 2;
   localparam flit_width = flit_data_width+flit_type_width;

   parameter vchannels = 1;
   parameter ports = 5;
   localparam ports_width = $clog2(ports);

   input clk;
   input rst;

   // switch side, receiving side
   input [flit_width*ports-1:0]  flit_i; // flit data from all input ports
   input [ports-1:0]             request_i; // request signal from each vc of each input port
   output reg [ports-1:0]        read_o; // select signal for each vc of each input port

   // FIFO side, output side
   output [flit_width-1:0] flit_o;
   output reg              valid_o;
   input                   ready_i;

   reg nxt_activeroute;
   reg activeroute;

   reg [ports-1:0] activeport;
   wire [ports-1:0] port;

   reg [ports_width-1:0] portnum;
   reg [ports_width-1:0] activeportnum;

   wire [flit_width-1:0] flit_i_array [0:ports-1];

   genvar p;
   generate
      for (p=0;p<ports;p=p+1) begin
         assign flit_i_array[p] = flit_i[flit_width*(p+1)-1:flit_width*p];
      end
   endgenerate


   wire [1:0] flit_type;
   assign flit_type = flit_i_array[portnum][flit_width-1:flit_width-2];

   assign flit_o = flit_i_array[portnum];

   wire [ports-1:0] req_masked;
   assign req_masked = {ports{~activeroute & ready_i}} & request_i;

   /* lisnoc_arb_rr AUTO_TEMPLATE(
    .req  (req_masked),
    .gnt  (activeport),
    .nxt_gnt (port),
    ); */
   lisnoc_arb_rr
      #(.N(ports))
      u_arb(/*AUTOINST*/
            // Outputs
            .nxt_gnt                    (port),                  // Templated
            // Inputs
            .req                        (req_masked),            // Templated
            .gnt                        (activeport));            // Templated


   always @(*) begin : convertonehot
      integer i;
      portnum = 0;
      activeportnum = 0;
      for (i=0;i<ports;i=i+1) begin
         if (port[i])
            portnum = i;
         if(activeport[i])
            activeportnum = i;
      end
   end

   always @(*) begin
      nxt_activeroute = activeroute;
      read_o = {ports{1'b0}};

      if (activeroute) begin
         if (request_i[activeportnum] && ready_i) begin
            read_o[activeportnum] = 1'b1;
            valid_o = 1'b1;
            if (flit_type[1])
               nxt_activeroute = 1'b0;
         end else begin
            valid_o = 1'b0;
            read_o[activeportnum] = 1'b0;
         end
      end else begin
         valid_o = 1'b0;
         if (|request_i & ready_i) begin
            valid_o = 1'b1;
            nxt_activeroute = !flit_type[1];
            read_o[portnum] = 1'b1;
         end
      end
   end


   always @(posedge clk) begin
      if (rst) begin
         activeroute <= 0;
         activeport <= {{ports-1{1'b0}},1'b1};
      end else begin
         activeroute <= nxt_activeroute;
         activeport <= port;
      end
   end


endmodule // noc_router_arbiter

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../")
// verilog-auto-inst-param-value: t
// End:
