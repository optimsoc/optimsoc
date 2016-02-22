/* Copyright (c) 2012-2014 by the author(s)
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
 * This is the Debug NoC <-> FIFO interface.
 *
 * The FIFO interface protocol used by this module has no notion of virtual
 * channels. This means:
 * - On the output side (FPGA -> FIFO), all virtual channels from the Debug
 *   NoC are serialized into one channel
 * - On the input side (FIFO -> FPGA), all data is injected into the Debug
 *   NoC on one predefined virtual channel. It can be set using the parameter
 *   DBG_NOC_USB_VCHANNEL.
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 *   Philipp Wagner <mail@philipp-wagner.com>
 */

`include "dbg_config.vh"
`include "lisnoc16_def.vh"

module fifo_dbg_if(/*AUTOARG*/
   // Outputs
   fifo_out_valid, fifo_out_data, fifo_in_ready, dbgnoc_out_flit,
   dbgnoc_out_valid, dbgnoc_in_ready,
   // Inputs
   clk, rst, fifo_out_ready, fifo_in_valid, fifo_in_data,
   dbgnoc_out_ready, dbgnoc_in_flit, dbgnoc_in_valid
   );

   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_USB_VCHANNEL = 0;

   parameter MAX_DBGNOC_TO_USB_PACKET_LENGTH = `MAX_DBGNOC_TO_USB_PACKET_LENGTH;
   localparam MAX_DBGNOC_TO_USB_PACKET_LENGTH_WIDTH = clog2(MAX_DBGNOC_TO_USB_PACKET_LENGTH);

   input clk;
   input rst;

   // FIFO interface
   output        fifo_out_valid;
   input         fifo_out_ready;
   output [15:0] fifo_out_data;
   input         fifo_in_valid;
   output        fifo_in_ready;
   input [15:0]  fifo_in_data;

   // Debug NoC interface
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // lisnoc_vc_serializer
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit_ser;
   wire dbgnoc_in_ready_ser;
   wire dbgnoc_in_valid_ser;

   // lisnoc deserializer
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit_ser;
   wire dbgnoc_out_ready_ser;
   wire dbgnoc_out_valid_ser;

   // FIFO -> Debug NoC bridge
   lisnoc16_usb_to_noc
      u_usb_to_noc(.clk(clk),
                   .rst(rst),

                   // FIFO interface (in)
                   .in_usb_data(fifo_in_data),
                   .in_usb_valid(fifo_in_valid),
                   .in_usb_ready(fifo_in_ready),

                   // Debug NoC Interface (out)
                   .out_noc_data(dbgnoc_out_flit_ser),
                   .out_noc_valid(dbgnoc_out_valid_ser),
                   .out_noc_ready(dbgnoc_out_ready_ser));

   // Debug NoC -> FIFO bridge
   lisnoc16_usb_from_noc
      #(.fifo_depth(MAX_DBGNOC_TO_USB_PACKET_LENGTH))
      u_usb_from_noc(.clk(clk),
                     .rst(rst),

                     // control
                     .flush_manual(1'b0),

                     // FIFO interface (out)
                     .out_usb_data(fifo_out_data),
                     .out_usb_valid(fifo_out_valid),
                     .out_usb_ready(fifo_out_ready),

                     // Debug NoC interface (in)
                     .in_noc_data(dbgnoc_in_flit_ser),
                     .in_noc_valid(dbgnoc_in_valid_ser),
                     .in_noc_ready(dbgnoc_in_ready_ser));


   // Debug NoC -> FIFO
   // All virtual channels from the Debug NoC are serialized into one channel
   // for FIFO transmission
   lisnoc_vc_serializer
      #(.vchannels(DBG_NOC_VCHANNELS),
        .flit_width(DBG_NOC_FLIT_WIDTH))
      u_vc_serializer(.clk(clk),
                      .rst(rst),

                      // input: multiple virtual channels
                      .data_mvc(dbgnoc_in_flit),
                      .valid_mvc(dbgnoc_in_valid),
                      .ready_mvc(dbgnoc_in_ready),

                      // output: one virtual channel
                      .data_ser(dbgnoc_in_flit_ser),
                      .valid_ser(dbgnoc_in_valid_ser),
                      .ready_ser(dbgnoc_in_ready_ser));

   // FIFO -> Debug NoC
   // All data is sent out on one virtual channel
   assign dbgnoc_out_flit = dbgnoc_out_flit_ser;
   assign dbgnoc_out_valid = {DBG_NOC_VCHANNELS{1'b0}} |
                             (dbgnoc_out_valid_ser << DBG_NOC_USB_VCHANNEL);
   assign dbgnoc_out_ready_ser = dbgnoc_out_ready[DBG_NOC_USB_VCHANNEL];

   `include "optimsoc_functions.vh"
endmodule

//Local Variables:
//verilog-library-directories:("./")
//End:
