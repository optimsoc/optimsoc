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
 * This module converts the data received by the usb-module into
 * valid 16-bit packets.
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 */

`include "lisnoc16_def.vh"

module lisnoc16_usb_to_noc (/*AUTOARG*/
   // Outputs
   in_usb_ready, out_noc_data, out_noc_valid,
   // Inputs
   clk, rst, in_usb_data, in_usb_valid, out_noc_ready
   );

   input clk;
   input rst;

   //USB-Interface
   input [15:0] in_usb_data;
   input        in_usb_valid;
   output in_usb_ready;

   //NOC-Interface
   output reg [17:0] out_noc_data;
   output reg        out_noc_valid;
   input             out_noc_ready;

   reg [`LD_MAX_NOC16_PACKET_LENGTH-1 : 0] flit_counter; //starts at one, since zero is reserved for dummy data
   reg [`LD_MAX_NOC16_PACKET_LENGTH-1 : 0] nxt_flit_counter;
   reg [`LD_MAX_NOC16_PACKET_LENGTH-1 : 0] packet16_length;
   reg [`LD_MAX_NOC16_PACKET_LENGTH-1 : 0] nxt_packet16_length;


   reg                  state;
   reg                  nxt_state;

   localparam STATE_USB_HEADER = 1'b0;
   localparam STATE_GEN_PACKET = 1'b1;

   assign in_usb_ready = out_noc_ready;


   //comb. part of FSM
   always @ (*) begin: fsm_comb
      //default:
      out_noc_valid = 1'b0;
      nxt_state = state;
      nxt_packet16_length = packet16_length;
      nxt_flit_counter = flit_counter;
      out_noc_data = 0; //TODO set to undef.


      case (state)

        //TODO: "nxt_flit_counter = 1;"-statements could be reduced.

        STATE_USB_HEADER: begin
           nxt_packet16_length = in_usb_data[`LD_MAX_NOC16_PACKET_LENGTH-1 : 0]; //save length of current NOC16-packet
           out_noc_valid = 1'b0;
           nxt_flit_counter = 1; //reset flit counter, but flit_counter should already be one!

           if (in_usb_valid & out_noc_ready) begin
              if(in_usb_data[`LD_MAX_NOC16_PACKET_LENGTH-1 : 0] == 0) begin //usb-dummy data
                 nxt_state = STATE_USB_HEADER;
              end else begin
                 nxt_state = STATE_GEN_PACKET;
              end
           end
        end
        STATE_GEN_PACKET: begin

           //forward flow control
           out_noc_valid = in_usb_valid;

           if (in_usb_valid && out_noc_ready) begin

              nxt_flit_counter = flit_counter+1;
           //   out_noc_valid = 1'b1; can creat combinatorical loop with out_noc_ready

              //flit_type detection
              if (flit_counter == 1) begin
                 if (flit_counter == packet16_length) begin
                    out_noc_data[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_SINGLE;
                    nxt_state = STATE_USB_HEADER;
                    nxt_flit_counter = 1; //reset flit-counter
                 end else begin
                    out_noc_data[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_HEADER;
                    nxt_state = STATE_GEN_PACKET;
                 end
              end else if (flit_counter == packet16_length) begin
                 out_noc_data[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_LAST;
                 nxt_state = STATE_USB_HEADER;
                 nxt_flit_counter = 1; //reset flit-counter
              end else begin
                 out_noc_data[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_PAYLOAD;
                 nxt_state = STATE_GEN_PACKET;
              end

              // forward data
              out_noc_data[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB] = in_usb_data;
           end // if (in_usb_valid && out_noc_ready)
        end

        default: begin
           //nothing
        end
      endcase // case (state)
   end // block: fsm_comb



   //seq. part of FSM
   always @ (posedge clk) begin: fsm_seq
      if (rst) begin
         state <= STATE_USB_HEADER;
         flit_counter <= 1;
         packet16_length <= 0; //is'n needed
      end else begin
         state <= nxt_state;
         flit_counter <= nxt_flit_counter;
         packet16_length <= nxt_packet16_length;
      end
   end // block: fsm_seq


endmodule // lisnoc16_usb_to_noc
