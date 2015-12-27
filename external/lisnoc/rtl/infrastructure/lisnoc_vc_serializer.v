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
 * This is a serializer that serializes several incoming vchannels to a
 * single one. It ensures wormhole forwarding in a first-come,
 * first-served way. There must not be two valid vchannels at the
 * same time, so it cannot be used as an arbiter!
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 */


`include "lisnoc_def.vh"

module lisnoc_vc_serializer(/*AUTOARG*/
   // Outputs
   ready_mvc, valid_ser, data_ser,
   // Inputs
   clk, rst, valid_mvc, data_mvc, ready_ser
   );

   parameter vchannels = 3;
   parameter flit_width = 32;

   input clk;
   input rst;

   //n-vchannel in (multiple vchannels)
   input [vchannels-1:0] valid_mvc;
   output [vchannels-1:0] ready_mvc;
   input  [flit_width-1:0] data_mvc;

   //one vchannel out (serialized)
   input  ready_ser;
   output valid_ser;
   output [flit_width-1:0] data_ser;

   reg    state;
   reg    nxt_state;
   localparam STATE_READY = 1'b0;
   localparam STATE_VALID = 1'b1;

   reg [vchannels-1 :0]     channel_mask;
   reg [vchannels-1 :0]     nxt_channel_mask;

   assign data_ser  = data_mvc;
   assign valid_ser = |(valid_mvc & channel_mask);
   assign ready_mvc = channel_mask & {vchannels{ready_ser}};


   always @ (*) begin : fsm_logic
      //default:
      nxt_state = state;
      nxt_channel_mask = channel_mask;

      case (state)
         STATE_READY: begin
            if (ready_ser) begin
               if((data_mvc[flit_width-1:flit_width-2]  == `FLIT_TYPE_HEADER) && (|valid_mvc)) begin
                  //save the current vchannel if a new packet arrives
                  //if the packet is a single-flit we don't need this information since there are no
                  //following flits that have to be served on the same vchannel
                  nxt_state = STATE_VALID;
                  nxt_channel_mask = valid_mvc; //save the current channel;
               end
            end
         end // case: STATE_READY

         STATE_VALID: begin
            if (ready_ser) begin
               if((data_mvc[flit_width-1:flit_width-2] == `FLIT_TYPE_LAST) && (valid_mvc & channel_mask)) begin
                  //end of packet - we are ready for a new one
                  nxt_state = STATE_READY;
                  nxt_channel_mask = {vchannels{1'b1}};
               end
            end
         end
        default: begin
           //nothing
        end
      endcase // case (state)
   end // block: fsm_logic


   always @ (posedge clk) begin : fsm_reg
      if (rst) begin
         state = STATE_READY;
         channel_mask = {vchannels{1'b1}};
      end else begin
         state = nxt_state;
         channel_mask = nxt_channel_mask;
      end
   end




endmodule // lisnoc_vc_multiplexer
