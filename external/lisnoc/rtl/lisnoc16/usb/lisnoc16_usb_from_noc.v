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
 * This module converts 16-bit packets from the debug-noc so that they
 * can be transmitted by the usb-module.
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc16_def.vh"

module lisnoc16_usb_from_noc (/*AUTOARG*/
   // Outputs
   out_usb_data, out_usb_valid, in_noc_ready,
   // Inputs
   clk, rst, flush_manual, out_usb_ready, in_noc_data, in_noc_valid
   );

   input clk;
   input rst;

   input flush_manual;

   //USB-Interface
   output reg [15:0] out_usb_data;
   output reg        out_usb_valid;
   input             out_usb_ready;

   //NoC-Interface
   input [17:0]  in_noc_data;
   input         in_noc_valid;
   output        in_noc_ready;

   //parameter
   parameter fifo_depth = `MAX_NOC16_PACKET_LENGTH;

   parameter bulk_length = 256;
   parameter bulk_length_width = 8;

   //timeout before filling the usb-fifo with dummydata to reach the bulk_length
   parameter max_timeout = 1024; //in clk cycle
   parameter max_timeout_width = 10;

   // The default is to turn the flush timer on
   parameter FLUSH_TIMER_ENABLE = 1;

   //buffer-interface
   wire [17:0]   buffer_flit;
   wire          buffer_valid;
   reg           buffer_ready;
   wire [`LD_MAX_NOC16_PACKET_LENGTH-1:0] next_last_flit_position; //TODO: geht das wirklich gut? //hier sollen werte zwichen 1 und 32 rein


   //timer
   reg  [max_timeout_width - 1:0]         nxt_timer_counter;
   reg  [max_timeout_width - 1:0]         timer_counter;

   //FSM
   reg [1:0]                              state;
   reg [1:0]                              nxt_state;

   localparam STATE_IDLE = 2'b00;
   localparam STATE_USB_HEADER = 2'b01;
   localparam STATE_PUSH_DATA = 2'b10;
   localparam STATE_FLUSH = 2'b11;

   //flit_counter
   reg [bulk_length_width -1 :0]          flit_counter;
   reg [bulk_length_width -1 :0]          nxt_flit_counter;

   reg                                    do_flush;
   reg                                    nxt_do_flush;

 always @ (*) begin: FSM_comb

    //defaults
    nxt_state = state;
    nxt_timer_counter = timer_counter;
    nxt_flit_counter = flit_counter;
    buffer_ready = 1'b0;
    out_usb_valid = 1'b0;
    out_usb_data = 16'h0000;

    if (flush_manual) begin
       nxt_do_flush = 1;
    end else begin
       nxt_do_flush = do_flush;
    end

    case (state)
      STATE_IDLE: begin
         if(buffer_valid) begin //it MUST be a header- or single-flit, or our FSM is broken
                                // ==> next_last_flit_position has its correct value
            nxt_state = STATE_USB_HEADER;
         end else if (FLUSH_TIMER_ENABLE && (timer_counter == max_timeout-1)) begin
            nxt_state = STATE_FLUSH;
         end else if (flush_manual) begin
            nxt_state = STATE_FLUSH;
         end else if (flit_counter != 0) begin //usb-input-fifo is not empty
            nxt_timer_counter = timer_counter +1;
         end
      end

      STATE_USB_HEADER: begin
//         nxt_timer_counter = 0; //XXX: soll er deaktiviert werden? //FIXME!!!

         if(out_usb_ready) begin
            out_usb_data = next_last_flit_position; //it has its correct value and...
            out_usb_valid = 1'b1; //..is valid, since it was valid the state before!
            nxt_state = STATE_PUSH_DATA;
         end
      end

      STATE_PUSH_DATA: begin
         if(out_usb_ready) begin
            if(buffer_valid) begin

               out_usb_valid = 1'b1;
               buffer_ready = 1'b1;

               out_usb_data = buffer_flit[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB];

               if((buffer_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_LAST)||
                  (buffer_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_SINGLE)) begin
                  nxt_state = STATE_IDLE;
               end else begin
                  nxt_state = STATE_PUSH_DATA;
               end
            end // if (buffer_valid)
         end // if (out_usb_ready)
      end // case: STATE_PUSH_DATA

      STATE_FLUSH: begin
         out_usb_data = 16'h0000;
         nxt_timer_counter = 0; //reset counter
         nxt_do_flush = 0;
         if((flit_counter == 0) || buffer_valid) begin
          nxt_state = STATE_IDLE;
            out_usb_valid = 1'b0;
         end else begin
            nxt_state = STATE_FLUSH;
            out_usb_valid = 1'b1;
         end
      end

      default: begin
         //nothing
      end
    endcase // case (state)

    //counts the flits written to the usb-interface
    if (out_usb_valid && out_usb_ready) begin
       if (flit_counter == bulk_length) begin
          nxt_flit_counter = 0;
       end else begin
          nxt_flit_counter = flit_counter +1;
       end
    end else begin
       nxt_flit_counter = flit_counter;
    end

 end // block: FSM_comb


  always @ (posedge clk) begin: fsm_seq
    if (rst) begin
       state <= STATE_IDLE;
       timer_counter <= 0;
       flit_counter <= 0;
       do_flush <= 0;
     end else begin
       state <= nxt_state;
       timer_counter <= nxt_timer_counter;
       flit_counter <= nxt_flit_counter;
        do_flush <= nxt_do_flush;
     end
  end





lisnoc16_usb_packet_buffer #(.fifo_depth(fifo_depth))
input_buffer(
             // Inputs
             .clk(clk),
             .rst(rst),

             //buffer-out
             .out_ready(buffer_ready),
             .out_flit(buffer_flit),
             .out_valid(buffer_valid),
             .next_last_flit_position(next_last_flit_position),

             //NoC-IN
             .in_flit(in_noc_data),
             .in_valid(in_noc_valid),
             .in_ready(in_noc_ready)
             );


endmodule // lisnoc16_usb_from_noc
