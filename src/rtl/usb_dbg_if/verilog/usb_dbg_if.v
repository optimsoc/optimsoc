/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * This is the Debug NoC <-> USB interface.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Michael Tempelmeier, michael.tempelmeier@tum.de
 *    Philipp Wagner, mail@philipp-wagner.com
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
   parameter DBG_NOC_VCHANNELS = `DBG_NOC_VCHANNELS;

   parameter MAX_DBGNOC_TO_USB_PACKET_LENGTH = `MAX_DBGNOC_TO_USB_PACKET_LENGTH;
   localparam MAX_DBGNOC_TO_USB_PACKET_LENGTH_WIDTH = $clog2(MAX_DBGNOC_TO_USB_PACKET_LENGTH);


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
                   .out_noc_data(dbgnoc_out_flit),
                   .out_noc_valid(dbgnoc_out_valid),
                   .out_noc_ready(dbgnoc_out_ready));

   // Debug NoC -> USB bridge
   lisnoc16_usb_from_noc
      #(.fifo_depth(MAX_DBGNOC_TO_USB_PACKET_LENGTH))
      u_usb_from_noc(.clk(clk_sys),
                     .rst(rst),

                     // USB interface (out)
                     .out_usb_data(out_data),
                     .out_usb_valid(out_valid),
                     .out_usb_ready(out_ready),

                     // Debug NoC interface (in)
                     .in_noc_data(dbgnoc_in_flit),
                     .in_noc_valid(dbgnoc_in_valid),
                     .in_noc_ready(dbgnoc_in_ready));
endmodule


//Local Variables:
//verilog-library-directories:("./")
//End:
