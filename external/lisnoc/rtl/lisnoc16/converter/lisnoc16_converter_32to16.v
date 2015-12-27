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
 * This module converts one 32-bit packet into one 16-bit packet by
 * splitting the 32-bit flits into two 16-bit flits and adding the
 * 16-bit header. The destination of the 16 bit can be set by the
 * parameter "usb_if_dest".
 *
 * Author(s):
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 */
`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"
`include "lisnoc16_converter_def.vh"

module lisnoc16_converter_32to16(/*AUTOARG*/
   // Outputs
   in_ready32, out_flit16, out_valid16,
   // Inputs
   clk, rst, in_flit32, in_valid32, out_ready16
   );

   //INFO: Since this is a converting FIFO the signal-widths are hardcoded!

   parameter fifo_depth = 16; //in relation to the input-width <=> Number of Input-Flits
   parameter usb_if_dest = 0;
   parameter vchannels_32 = 3;
   parameter vchannels_16 = 1;
   parameter use_vchannel_16 = 0;

   parameter packet16_class = 3'b111;

   input  clk;
   input  rst;

   //converter input side
   input  [`FLIT32_WIDTH-1:0] in_flit32;  // input
   input  [vchannels_32-1:0]   in_valid32; // write_enable
   output [vchannels_32-1:0]   in_ready32; // accepting new data


   //converter output side
   output reg [`FLIT16_WIDTH-1:0] out_flit16;  // data_out
   output[vchannels_16-1:0]        out_valid16; // data available
   input [vchannels_16-1:0]        out_ready16; // read request

   //signals for the multiplexer on the 32bit side
   wire [`FLIT32_WIDTH-1:0] in_flit32_intern;
   wire in_valid32_intern;
   wire in_ready32_intern;

   //we use only one vchannel on the 16 bit side
   reg out_valid16_intern;
   wire out_ready16_intern;
   assign out_valid16 = (out_valid16_intern << use_vchannel_16)| {vchannels_16{1'b0}};
   assign out_ready16_intern = out_ready16[use_vchannel_16];



   //signals for the fifos
   reg   in_ready_H;
   reg   in_ready_L;

   reg   pop_H;
   reg   pop_L;

   reg   valid_H;
   reg   valid_L;

   reg [`FLIT16_WIDTH-1:0]       flit16_L;//contains Header-Info
   reg [`FLIT16_DATA_WIDTH-1:0]  flit16_H;

   //signals for the FSM
   reg [1:0]  state;
   reg [1:0]  nxt_state;

   `define STATE_GEN_HEADER 2'b01
   `define STATE_POP_H 2'b10
   `define STATE_POP_L 2'b11



   //******** FSM ********//

   always @ (*) begin: FSM_Logic

      //default
      nxt_state = state;
      out_valid16_intern = 1'b0;
      pop_L = 1'b0;
      pop_H = 1'b0;
      out_flit16= 0; //TODO: after debugging replace with undefined

      case(state)
         `STATE_GEN_HEADER: begin
            out_flit16[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_HEADER;
            out_flit16[`FLIT16_DEST_MSB:`FLIT16_DEST_LSB] =  usb_if_dest;
            out_flit16[`PACKET16_CLASS_MSB:`PACKET16_CLASS_LSB] = packet16_class;

            if (valid_H) begin //or (valid_L)
               //indecates that there's a new Packet in the Fifo
               //so we start transfering
               out_valid16_intern = 1'b1;

               //TODO: check if we have a Header in die FIFO
               //Header-Info is stored in flit16_L[17:16]
               //if everything works fine, we must have a Header oder Single!

               if(out_ready16_intern) begin
                  nxt_state = `STATE_POP_H;
               end else begin
                  nxt_state = `STATE_GEN_HEADER;
               end
            end // if (valid_L)
         end // case: `STATE_GEN_HEADER

         `STATE_POP_H: begin
            if(valid_H) begin
               out_valid16_intern = 1'b1;
               if(out_ready16_intern) begin
                  pop_H = 1'b1;
                  nxt_state = `STATE_POP_L;
                  out_flit16[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB] = flit16_H[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB];
                  out_flit16[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_PAYLOAD;
               end else begin
                  pop_H = 1'b0;
                  nxt_state = `STATE_POP_H;
               end
            end
         end // case: `STATE_POP_H

         `STATE_POP_L: begin
            if(valid_L) begin
               out_valid16_intern = 1'b1;
               if(out_ready16_intern) begin
                  pop_L = 1'b1;
                  out_flit16[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB] = flit16_L[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB];

                  if ((flit16_L[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT_TYPE_LAST) ||
                     // NOTE: There's no typo here!
                     // We test the type of the former 32-bit flit (`FLIT_TYPE_[LAST|SINGLE])
                     // but that information is stored at the FLIT16_TYPE-position
                     (flit16_L[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT_TYPE_SINGLE)) begin

                     out_flit16[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_LAST;
                     nxt_state = `STATE_GEN_HEADER;
                  end else begin
                     out_flit16[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_PAYLOAD;
                     nxt_state = `STATE_POP_H;
                  end
               end else begin // if (out_ready16_intern)
                  pop_L = 1'b0;
                  nxt_state = `STATE_POP_L;
               end // else: !if(out_ready16_intern)
            end
         end // case: `STATE_POP_L

         default: begin
            //nothing
         end
      endcase // case (state)
   end // block: FSM_Logic



   always @ (posedge clk) begin: FSM_regs
      if (rst) begin
         state <= `STATE_GEN_HEADER;
      end else begin
         state <= nxt_state;
      end
   end



   //******** 32 to 16 FIFO ********//
   assign in_ready32_intern = in_ready_L & in_ready_H; //both fifos must be ready to accept new data

   //FIXME: Drecks typenkonzept
   wire                       out_valid16_bL;
   wire                       in_ready_bL;
   wire [`FLIT16_WIDTH-1:0]   out_flit16_bL;

   wire                          out_valid16_bH;
   wire                          in_ready_bH;
   wire [`FLIT16_DATA_WIDTH-1:0] out_flit16_bH;


   always @(*) begin
      in_ready_L = in_ready_bL;
      flit16_L = out_flit16_bL;
      valid_L = out_valid16_bL;


      in_ready_H = in_ready_bH;
      flit16_H = out_flit16_bH;
      valid_H = out_valid16_bH;

   end

   lisnoc16_fifo
      #(.LENGTH(fifo_depth), .WIDTH(`FLIT16_WIDTH)) //contains header-info
      buffer_L (.clk(clk),
         .rst(rst),
         //Output
         .in_ready(in_ready_bL),
         .out_flit(out_flit16_bL),
         .out_valid(out_valid16_bL),
         //Input
         .in_flit({in_flit32_intern[33:32], in_flit32_intern[15:0]}), //save header info in the Lower Word
         .in_valid(in_valid32_intern),
         .out_ready(pop_L)
         );


   lisnoc16_fifo
      #(.LENGTH(fifo_depth), .WIDTH(`FLIT16_DATA_WIDTH))
      buffer_H (.clk(clk),
                .rst(rst),
                //Output
                .in_ready(in_ready_bH),
                .out_flit(out_flit16_bH),
                .out_valid(out_valid16_bH),
                //Input
                .in_flit(in_flit32_intern[31:16]),
                .in_valid(in_valid32_intern),
                .out_ready(pop_H));


 /***** VC-Multiplexer ******/

   lisnoc_vc_multiplexer
      #(.vchannels(vchannels_32),.flit_width(`FLIT32_WIDTH))
      vc_mux_32 (.clk(clk),
                 .rst(rst),

                 //extern
                 .data_i(in_flit32),
                 .valid_i(in_valid32),
                 .ready_o(in_ready32),

                 //intern
                 .data_o(in_flit32_intern),
                 .valid_o(in_valid32_intern),
                 .ready_i(in_ready32_intern));

endmodule
