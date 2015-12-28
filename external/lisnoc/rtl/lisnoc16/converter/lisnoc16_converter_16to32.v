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
 * This module converts a 16-bit packet into a 32-bit packet by
 * packing two 16-bit flits into one 32-bit flit and deleting the header
 * of the 16-bit packet. Therefore the header of the 32-bit packet has
 * to be packed into the first two payloadflits of the 16-bit packet.
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 */


`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"
`include "lisnoc16_converter_def.vh"


//FIXME: ADD VC-Multiplexer

module lisnoc16_converter_16to32(/*AUTOARG*/
   // Outputs
   flit16_ready, flit32_out, flit32_valid,
   // Inputs
   clk, rst, flit16_in, flit16_valid, flit32_ready
   );

   parameter fifo_depth = 16; //INFO: Needs to be greater or equal to max_packet_length in relation to 32-Bit-Flits
   parameter vchannels_32 = 3;
   parameter vchannels_16 = 1;
   parameter use_vchannel_32 = 0;
   parameter packet16_class = 3'hx;

   input clk, rst;

   input [`FLIT16_WIDTH-1:0]  flit16_in; //+2bit for type
   input [vchannels_16-1 :0]  flit16_valid;
   output [vchannels_16-1:0]  flit16_ready;


   output [`FLIT32_WIDTH-1:0]       flit32_out;
   output [vchannels_32-1:0]        flit32_valid;
   input [vchannels_32-1:0]         flit32_ready;



   //signals for the multiplexer on the 16 bit side
   wire [`FLIT16_WIDTH-1:0] flit16_in_intern;
   wire flit16_valid_intern;
   reg flit16_ready_intern;


   //we use only one vchannel on the 32 bit side
   wire out_valid;
   wire in_ready;
   wire [`FLIT32_WIDTH-1:0] out_flit;

   assign flit32_valid = (out_valid << use_vchannel_32)| {vchannels_32{1'b0}};
   assign in_ready = flit32_ready[use_vchannel_32];
   assign flit32_out = out_flit;



   //this is the converted 32-Bit Structure:
   wire [`FLIT32_WIDTH-1:0]   flit32_intern; //Type+Content
   wire                       flit32_intern_ready;
   reg                        flit32_intern_valid;

   //internal signals
   reg [`FLIT16_DATA_WIDTH-1:0]  flit16_buffer;
   reg [`FLIT16_TYPE_WIDTH-1:0]  flit_type;
   reg                           buffer_data;

   //States of FSM
   `define STATE_WIDTH 3
   `define STATE_IDLE           3'b000
   `define STATE_BUFFER_SINGLE  3'b001
   `define STATE_BUFFER_PAYLOAD 3'b010
   `define STATE_CHECK_SINGLE   3'b011
   `define STATE_CHECK_LAST     3'b100

   reg [`STATE_WIDTH-1:0]  state;
   reg [`STATE_WIDTH-1:0]  nxt_state;








   //******** Conversion ********//
   always @ (posedge clk) begin: convertion_buffer
      if (rst) begin
         //no reset needed
      end else if (buffer_data) begin
         flit16_buffer <= flit16_in_intern[15:0]; //we don't need the flittype
      end
   end

   //create 34-Bit-Flit: 2Bit Flittype, 32Bit Data
   assign flit32_intern = {flit_type, flit16_buffer, flit16_in_intern[15:0]};



   //******** Control Logic ********//
   always @ (*) begin: FSM_logic

      //default values
      nxt_state = state;
      buffer_data = 1'b0;
      flit16_ready_intern = 1'b0;
      flit32_intern_valid = 1'b0;
      flit_type = `FLIT_TYPE_PAYLOAD;


      case(state)
         `STATE_IDLE: begin
            //Header of 16Bit-Packet

            flit16_ready_intern = 1'b1; //get next Flit
            flit32_intern_valid=1'b0;

            if (flit16_valid_intern &&
                flit16_in_intern[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_HEADER &&
                flit16_in_intern[`PACKET16_CLASS_MSB:`PACKET16_CLASS_LSB] == packet16_class) begin

               nxt_state = `STATE_BUFFER_SINGLE;
            end else begin
               nxt_state = `STATE_IDLE;
            end
         end
         `STATE_BUFFER_SINGLE: begin
            //First 16 Bit of the first 32-Bit-Flit.
            flit16_ready_intern = 1'b1;//we are always ready, since it's the first flit to be stored.
            flit32_intern_valid = 1'b0;
            if(flit16_valid_intern) begin
               buffer_data = 1'b1; //buffer the first 16 Bit
               nxt_state = `STATE_CHECK_SINGLE;
            end else begin
               nxt_state = `STATE_BUFFER_SINGLE;
            end
         end

         `STATE_CHECK_SINGLE: begin
            //Second 16 Bit of the first 32-Bit-Flit
            if(flit32_intern_ready) begin
                flit16_ready_intern = 1'b1;
            end

            if(flit16_valid_intern) begin
               //If that's the last-Flit of the 16-Bit-Packet we have a 32-Bit Single Packet
               //Else we have a 32-Bit Header-Flit
               if(flit16_in_intern[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_LAST) begin
                  flit_type = `FLIT16_TYPE_SINGLE;
                  if(flit32_intern_ready) begin //Noc is ready...
                     flit32_intern_valid = 1'b1; //...so we write to it
                     nxt_state = `STATE_IDLE;
                  end else begin
                     nxt_state = `STATE_CHECK_SINGLE;
                  end
               end else begin
                  flit_type = `FLIT16_TYPE_HEADER;
                  if(flit32_intern_ready) begin //Noc is ready...
                     flit32_intern_valid = 1'b1; //...so we write to it
                     nxt_state = `STATE_BUFFER_PAYLOAD;
                  end else begin
                     nxt_state =`STATE_CHECK_SINGLE;
                  end
               end
            end else begin // if (flit16_valid_intern)
               nxt_state = `STATE_CHECK_SINGLE;
            end // else: !if(flit16_valid_intern)
         end // case: `STATE_CHECK_SINGLE

         `STATE_BUFFER_PAYLOAD: begin
            //This are the first 16 Bit of a 32-Bit Payload- or Last-Flit
            flit16_ready_intern = 1'b1; //buffer is empty, since we read in the previous state
            flit32_intern_valid = 1'b0;
            if(flit16_valid_intern) begin
               nxt_state = `STATE_CHECK_LAST;
               buffer_data = 1'b1; //buffer the first 16 Bit
            end else begin
               nxt_state = `STATE_BUFFER_PAYLOAD;
            end
         end

         `STATE_CHECK_LAST: begin
            //this are the second 16 Bit of a 32-Bit Payload- or Last-Flit
            //If that's the LAST-Flit of the 16-Bit-Packet it's also the LAST-Flit
            //of the 32-Bit Packet
            if(flit32_intern_ready) begin
                flit16_ready_intern = 1'b1;
            end

            if(flit16_valid_intern) begin
               //If that's the last-Flit of the 16-Bit-Packet we have a 32-Bit LAST-Flit
               //Else we have a 32-Bit PAYLOAD-Flit
               if(flit16_in_intern[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_LAST) begin
                  flit_type = `FLIT16_TYPE_LAST;
                  if(flit32_intern_ready) begin //Noc is ready...
                     flit32_intern_valid = 1'b1; //...so we write to it //FIXME: possible combinatorial loop
                     //flit16_ready_intern = 1'b1; //...and we request more data
                     nxt_state = `STATE_IDLE;
                  end else begin
                     nxt_state = `STATE_CHECK_LAST;
                  end
               end else begin
                  flit_type = `FLIT16_TYPE_PAYLOAD;
                  if(flit32_intern_ready) begin //Noc is ready...
                     flit32_intern_valid = 1'b1; //...so we write to it //FIXME: possible combinatorial loop
            //         flit16_ready_intern = 1'b1; //...and we request more data
                     nxt_state = `STATE_BUFFER_PAYLOAD;
                  end else begin
                     nxt_state =`STATE_CHECK_SINGLE;
                  end
               end // else: !if(flit16_in_intern[`FLIT_TYPE_MSB:`FLIT_TYPE_LSB] == `FLIT_TYPE_LAST)
            end else begin // if (flit16_valid_intern)
               nxt_state = `STATE_CHECK_LAST;
            end // else: !if(flit16_valid_intern)
         end // case: `STATE_CHECK_LAST


         default: begin
            //nothing
         end
      endcase // case (state)
   end // block: FSM_logic



   always @ (posedge clk) begin: FSM_regs
      if (rst) begin
         state <= `STATE_IDLE;
      end else begin
         state <= nxt_state;
      end
   end


   //******** 32-Bit-Buffer ********//


   lisnoc_packet_buffer //Buffer stores a whole packet.
   //TODO: replace with a buffer that stores only half of a packet.
      #(.fifo_depth(fifo_depth))
   buffer (
      .clk(clk),
      .rst(rst),
      //to_noc32
      .out_ready(in_ready),
      .out_flit(out_flit),
      .out_valid(out_valid),
      .out_size(),
      //from_convertion
      .in_flit(flit32_intern),
      .in_valid(flit32_intern_valid),
      .in_ready(flit32_intern_ready)
      );


 /***** VC-Multiplexer ******/

 lisnoc_vc_multiplexer
        #(.vchannels(vchannels_16),.flit_width(`FLIT16_WIDTH))
        vc_mux_32 (.clk(clk),
                .rst(rst),

                //extern
                .data_i(flit16_in),
                .valid_i(flit16_valid),
                .ready_o(flit16_ready),

                //intern
                .data_o(flit16_in_intern),
                .valid_o(flit16_valid_intern),
                .ready_i(flit16_ready_intern));

endmodule
