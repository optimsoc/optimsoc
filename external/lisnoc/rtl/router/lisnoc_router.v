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
 * This is the generic router toplevel.
 *
 * (c) 2011-2015 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"

module lisnoc_router( /*AUTOARG*/
   // Outputs
   out_flit, out_valid, in_ready,
   // Inputs
   clk, rst, out_ready, in_flit, in_valid
   );

   // FIXME The lowercase parameters are legacy, replace any use by the
   // uppercase variant!
   parameter  flit_data_width = 32;
   parameter  flit_type_width = 2;

   parameter  num_dests = 32;
   parameter  ph_dest_width = 5;

   parameter use_prio = 0;
   parameter ph_prio_width = 4;

   parameter vchannels = 1;

   parameter input_ports = 5;
   parameter output_ports = 5;

   parameter in_fifo_length = 4;
   parameter out_fifo_length = 4;

   parameter [output_ports*num_dests-1:0] lookup = {num_dests*output_ports{1'b0}};

   // Width of the actual flit data
   parameter  FLIT_DATA_WIDTH = flit_data_width;
   // Type width, use 2 for the moment
   parameter  FLIT_TYPE_WIDTH = flit_type_width;
   localparam FLIT_WIDTH = FLIT_DATA_WIDTH + FLIT_TYPE_WIDTH;

   // Number of destinations in the entire NoC
   parameter  NUM_DESTS = num_dests;
   // Width of the destination field in the packet header
   parameter  PH_DEST_WIDTH = ph_dest_width;

   // Use priorities
   parameter USE_PRIO = use_prio;
   // Width of the priority field in the packet header
   parameter PH_PRIO_WIDTH = ph_prio_width;

   // Number of virtual channels
   parameter VCHANNELS = vchannels;

   // Number of input ports
   parameter INPUT_PORTS = input_ports;
   // Number of output ports
   parameter OUTPUT_PORTS = output_ports;

   // Size of the input FIFOs
   parameter IN_FIFO_LENGTH = in_fifo_length;
   // Size of the output FIFOs
   parameter OUT_FIFO_LENGTH = out_fifo_length;

   // Lookup "table" (will synthesize as logic usually). Be careful
   // that it is ordered from MSB to LSB, meaning it can be easier
   // used for readable concatenation like
   //
   // {PORT_FOR_DEST0, PORT_FOR_DEST1, ..}
   parameter [output_ports*num_dests-1:0] LOOKUP = lookup;

   // Clock and reset
   input clk, rst;

   // Output interfaces (flat)
   output [OUTPUT_PORTS*FLIT_WIDTH-1:0] out_flit;
   output [OUTPUT_PORTS*VCHANNELS-1:0]  out_valid;
   input [OUTPUT_PORTS*VCHANNELS-1:0]   out_ready;

   // Input interfaces (flat)
   input [INPUT_PORTS*FLIT_WIDTH-1:0]   in_flit;
   input [INPUT_PORTS*VCHANNELS-1:0]    in_valid;
   output [INPUT_PORTS*VCHANNELS-1:0]   in_ready;

   // Array conversion
   wire [FLIT_WIDTH-1:0]                out_flit_array [0:OUTPUT_PORTS-1];
   wire [VCHANNELS-1:0]                 out_valid_array [0:OUTPUT_PORTS-1];
   wire [VCHANNELS-1:0]                 out_ready_array [0:OUTPUT_PORTS-1];

   wire [FLIT_WIDTH-1:0]                in_flit_array [0:INPUT_PORTS-1];
   wire [VCHANNELS-1:0]                 in_valid_array [0:INPUT_PORTS-1];
   wire [VCHANNELS-1:0]                 in_ready_array [0:INPUT_PORTS-1];

   genvar                 p;
   genvar                 op,v,ip;
   generate
      for (p = 0; p < OUTPUT_PORTS; p = p + 1) begin : output_arrays
         assign out_flit[(p+1)*FLIT_WIDTH-1:p*FLIT_WIDTH] = out_flit_array[p];
         assign out_valid[(p+1)*VCHANNELS-1:p*VCHANNELS]  = out_valid_array[p];
         assign out_ready_array[p] = out_ready[(p+1)*VCHANNELS-1:p*VCHANNELS];
      end
   endgenerate

   generate
      for (p = 0; p < INPUT_PORTS; p = p + 1) begin : input_arrays
         assign in_flit_array[p]  = in_flit[(p+1)*FLIT_WIDTH-1:p*FLIT_WIDTH];
         assign in_valid_array[p] = in_valid[(p+1)*VCHANNELS-1:p*VCHANNELS];
         assign in_ready[(p+1)*VCHANNELS-1:p*VCHANNELS] = in_ready_array[p];
      end
   endgenerate

   // Those are the switching wires
   wire [FLIT_WIDTH*VCHANNELS-1:0]   switch_in_flit[0:INPUT_PORTS-1];
   wire [OUTPUT_PORTS*VCHANNELS-1:0] switch_in_request[0:INPUT_PORTS-1];
   wire [OUTPUT_PORTS*VCHANNELS-1:0] switch_in_read[0:INPUT_PORTS-1];

   wire [FLIT_WIDTH*VCHANNELS*INPUT_PORTS-1:0] switch_out_flit[0:OUTPUT_PORTS-1];
   wire [INPUT_PORTS*VCHANNELS-1:0]            switch_out_request[0:OUTPUT_PORTS-1];
   wire [INPUT_PORTS*VCHANNELS-1:0]            switch_out_read[0:OUTPUT_PORTS-1];

   // Switch
   wire [FLIT_WIDTH*INPUT_PORTS*VCHANNELS-1:0] all_flits;

   generate
      for (p = 0; p < INPUT_PORTS; p = p + 1) begin : compose_all_flits
         assign all_flits[(p+1)*VCHANNELS*FLIT_WIDTH-1:p*VCHANNELS*FLIT_WIDTH] = switch_in_flit[p];
      end
   endgenerate

   generate
      for (p = 0; p < OUTPUT_PORTS; p = p + 1) begin : assign_all_flits
         assign switch_out_flit[p] = all_flits;
      end
   endgenerate

   generate
      for (op = 0; op < OUTPUT_PORTS; op = op + 1) begin: connect_switch
         for (v = 0; v < VCHANNELS; v = v + 1) begin
            for (ip = 0; ip < INPUT_PORTS; ip = ip + 1) begin
               assign switch_out_request[op][v*INPUT_PORTS+ip] = switch_in_request[ip][v*OUTPUT_PORTS+op];
               assign switch_in_read[ip][v*OUTPUT_PORTS+op]    = switch_out_read[op][v*INPUT_PORTS+ip];
            end
         end
      end
   endgenerate

   generate
      for (p = 0; p < INPUT_PORTS; p = p + 1) begin : inputs
         /* lisnoc_router_input AUTO_TEMPLATE (
         .link_ready     (in_ready_array[p]),
         .link_flit      (in_flit_array[p]),
         .link_valid     (in_valid_array[p]),
         .switch_request (switch_in_request[p]),
         .switch_flit    (switch_in_flit[p]),
         .switch_read    (switch_in_read[p]),
         );*/

         lisnoc_router_input
            #(.vchannels(VCHANNELS),.ports(OUTPUT_PORTS),
               .num_dests(NUM_DESTS),.lookup(LOOKUP),.flit_data_width(FLIT_DATA_WIDTH),
               .flit_type_width(FLIT_TYPE_WIDTH),.ph_dest_width(PH_DEST_WIDTH),
               .fifo_length(IN_FIFO_LENGTH))
         inputs(/*AUTOINST*/
                // Outputs
                .link_ready             (in_ready_array[p]),     // Templated
                .switch_request         (switch_in_request[p]),  // Templated
                .switch_flit            (switch_in_flit[p]),     // Templated
                // Inputs
                .clk                    (clk),
                .rst                    (rst),
                .link_flit              (in_flit_array[p]),      // Templated
                .link_valid             (in_valid_array[p]),     // Templated
                .switch_read            (switch_in_read[p]));    // Templated
      end // block: inputs
   endgenerate

   generate
      for (p = 0; p < OUTPUT_PORTS; p = p + 1) begin : outputs
         /* lisnoc_router_output AUTO_TEMPLATE (
         .link_flit      (out_flit_array[p]),
         .link_valid     (out_valid_array[p]),
         .link_ready     (out_ready_array[p]),
         .switch_read    (switch_out_read[p]),
         .switch_request (switch_out_request[p]),
         .switch_flit    (switch_out_flit[p]),
         );*/

         lisnoc_router_output

         #(.vchannels(VCHANNELS),.ports(INPUT_PORTS),
              .flit_data_width(FLIT_DATA_WIDTH),.flit_type_width(FLIT_TYPE_WIDTH),
              .fifo_length(OUT_FIFO_LENGTH),
              .use_prio(USE_PRIO),.ph_prio_width(PH_PRIO_WIDTH),.ph_prio_offset(PH_DEST_WIDTH))

         outputs (/*AUTOINST*/
                  // Outputs
                  .link_flit            (out_flit_array[p]),     // Templated
                  .link_valid           (out_valid_array[p]),    // Templated
                  .switch_read          (switch_out_read[p]),    // Templated
                  // Inputs
                  .clk                  (clk),
                  .rst                  (rst),
                  .link_ready           (out_ready_array[p]),    // Templated
                  .switch_request       (switch_out_request[p]), // Templated
                  .switch_flit          (switch_out_flit[p]));   // Templated
      end // block: outputs
   endgenerate

endmodule // lisnoc_router

`include "lisnoc_undef.vh"
