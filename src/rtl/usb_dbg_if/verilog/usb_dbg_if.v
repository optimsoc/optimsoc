/* Copyright (c) 2013 by the author(s)
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
 * This is the Debug NoC <-> USB interface.
 *
 * The USB interface protocol used by this module has no notion of virtual
 * channels. This means:
 * - On the output side (FPGA -> host), all virtual channels from the Debug
 *   NoC are serialized into one channel
 * - On the input side (host -> FPGA), all data is injected into the Debug
 *   NoC on one predefined virtual channel. It can be set using the parameter
 *   DBG_NOC_USB_VCHANNEL.
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 *   Philipp Wagner <mail@philipp-wagner.com>
 */

`include "dbg_config.vh"
`include "lisnoc16_def.vh"

module usb_dbg_if(/*AUTOARG*/
   // Outputs
   fx2_slrd, fx2_slwr, fx2_sloe, fx2_pktend, fx2_fifoadr, fx2_fd_out,
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready,
   // Inputs
   clk_sys, rst, fx2_clk, fx2_fd_in, fx2_epout_fifo_empty,
   fx2_epin_fifo_full, fx2_epin_fifo_almost_full, dbgnoc_out_ready,
   dbgnoc_in_flit, dbgnoc_in_valid
   );

   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_USB_VCHANNEL = 0;

   parameter MAX_DBGNOC_TO_USB_PACKET_LENGTH = `MAX_DBGNOC_TO_USB_PACKET_LENGTH;
   localparam MAX_DBGNOC_TO_USB_PACKET_LENGTH_WIDTH = clog2(MAX_DBGNOC_TO_USB_PACKET_LENGTH);

   input clk_sys;
   input rst;

   // EZ-USB FX2 USB interface
   input fx2_clk;
   output fx2_slrd;
   output fx2_slwr;
   output fx2_sloe;
   output fx2_pktend;
   output [1:0] fx2_fifoadr;
   output [15:0] fx2_fd_out;
   input [15:0] fx2_fd_in;
   input fx2_epout_fifo_empty;
   input fx2_epin_fifo_full;
   input fx2_epin_fifo_almost_full;

   // Debug NoC interface
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // connecting wires
   wire [DBG_NOC_DATA_WIDTH-1:0] in_data;
   wire in_ready;
   wire in_valid;
   wire [DBG_NOC_DATA_WIDTH-1:0] out_data;
   wire out_ready;
   wire out_valid;

   // lisnoc_vc_serializer
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit_ser;
   wire dbgnoc_in_ready_ser;
   wire dbgnoc_in_valid_ser;

   // lisnoc deserializer
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit_ser;
   wire dbgnoc_out_ready_ser;
   wire dbgnoc_out_valid_ser;


   // EZ-USB FX2 communication interface
   fx2_usb_comm
      u_comm(.clk_sys(clk_sys),
             .rst(rst),

             // incoming/outgoing data in clk_sys domain
             .in_data(in_data),
             .in_ready(in_ready),
             .in_valid(in_valid),
             .out_data(out_data),
             .out_ready(out_ready),
             .out_valid(out_valid),

             // FX2 signals in fx2_clk domain
             .fx2_clk(fx2_clk),
             .fx2_slrd(fx2_slrd),
             .fx2_slwr(fx2_slwr),
             .fx2_sloe(fx2_sloe),
             .fx2_pktend(fx2_pktend),
             .fx2_fifoadr(fx2_fifoadr),
             .fx2_fd_out(fx2_fd_out),
             .fx2_fd_in(fx2_fd_in),
             .fx2_epout_fifo_empty(fx2_epout_fifo_empty),
             .fx2_epin_fifo_full(fx2_epin_fifo_full),
             .fx2_epin_fifo_almost_full(fx2_epin_fifo_almost_full));

   // USB -> Debug NoC bridge
   lisnoc16_usb_to_noc
      u_usb_to_noc(.clk(clk_sys),
                   .rst(rst),

                   // USB interface (in)
                   .in_usb_data(in_data),
                   .in_usb_valid(in_valid),
                   .in_usb_ready(in_ready),

                   // Debug NoC Interface (out)
                   .out_noc_data(dbgnoc_out_flit_ser),
                   .out_noc_valid(dbgnoc_out_valid_ser),
                   .out_noc_ready(dbgnoc_out_ready_ser));

   // Debug NoC -> USB bridge
   lisnoc16_usb_from_noc
      #(.fifo_depth(MAX_DBGNOC_TO_USB_PACKET_LENGTH))
      u_usb_from_noc(.clk(clk_sys),
                     .rst(rst),

                     // control
                     .flush_manual(1'b0),

                     // USB interface (out)
                     .out_usb_data(out_data),
                     .out_usb_valid(out_valid),
                     .out_usb_ready(out_ready),

                     // Debug NoC interface (in)
                     .in_noc_data(dbgnoc_in_flit_ser),
                     .in_noc_valid(dbgnoc_in_valid_ser),
                     .in_noc_ready(dbgnoc_in_ready_ser));


   // Debug NoC -> USB
   // All virtual channels from the Debug NoC are serialized into one channel
   // for USB transmission
   lisnoc_vc_serializer
      #(.vchannels(DBG_NOC_VCHANNELS),
        .flit_width(DBG_NOC_FLIT_WIDTH))
      u_vc_serializer(.clk(clk_sys),
                      .rst(rst),

                      // input: multiple virtual channels
                      .data_mvc(dbgnoc_in_flit),
                      .valid_mvc(dbgnoc_in_valid),
                      .ready_mvc(dbgnoc_in_ready),

                      // output: one virtual channel
                      .data_ser(dbgnoc_in_flit_ser),
                      .valid_ser(dbgnoc_in_valid_ser),
                      .ready_ser(dbgnoc_in_ready_ser));

   // USB -> Debug NoC
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
