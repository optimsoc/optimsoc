/* Copyright (c) 2014-2015 by the author(s)
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
 * GLIP JTAG: Input FSM
 *
 * Author(s):
 *   Alexandra Weber <sandra.eli.weber@tum.de>
 *   Jan Alexander Wessel <jan.wessel@tum.de>
 */
module glip_jtag_input_fsm(/*AUTOARG*/
   // Outputs
   fifo_data, fifo_valid, error, jtag_reset, acc_cnt,
   // Inputs
   rst, clk, tdi, shift, update, capture, fifo_ready
   );

   parameter WORD_WIDTH = 16;
   parameter NUM_WORDS = 3;

   localparam PACKET_COUNTER_LENGTH = WORD_WIDTH;

   // sync in buffer with limited storage
   parameter SYNC_FIFO_DEPTH = NUM_WORDS;

   input rst;

   // JTAG interface
   input clk;
   input tdi;

   // tap signal
   input shift;
   input update;
   input capture;

   // fifo interface
   input fifo_ready;
   output [WORD_WIDTH-1:0] fifo_data;
   output fifo_valid;

   output reg error;
   reg nxt_error;

   output reg jtag_reset;
   reg nxt_jtag_reset;

   reg [2:0] input_state;
   reg [2:0] nxt_input_state;

   // this counter counts the bits until the word is complete
   reg [WORD_WIDTH-1:0] bit_cnt;
   reg [WORD_WIDTH-1:0] nxt_bit_cnt;

   // this register stores the number of valid words
   reg [PACKET_COUNTER_LENGTH-1:0] in_reg_cnt;
   reg [PACKET_COUNTER_LENGTH-1:0] nxt_in_reg_cnt;

   // puts out the number of accepted/correctly written words
   output reg [PACKET_COUNTER_LENGTH-1:0] acc_cnt;
   reg [PACKET_COUNTER_LENGTH-1:0] nxt_acc_cnt;

   reg in_valid;
   wire in_ready;
   reg [WORD_WIDTH-1:0] in_reg_data;
   reg [WORD_WIDTH-1:0] nxt_in_reg_data;
   reg [WORD_WIDTH-1:0] in_flit;

   localparam IDLE = 3'b000;
   localparam READ_PACKET_LENGTH = 3'b001;
   localparam IGNORE_READ_LENGTH = 3'b010;
   localparam READ_PACKET = 3'b011;
   localparam DROP_PACKET = 3'b100;

   glip_jtag_fifo
      #(.LENGTH(SYNC_FIFO_DEPTH),
        .FLIT_DATA_WIDTH(WORD_WIDTH),
        .FLIT_TYPE_WIDTH(0),
        .FIFO_FREE_SPACE_WIDTH(NUM_WORDS))
      sync_fifo (
         // Outputs
         .in_ready         (in_ready),
         .out_flit         (fifo_data),
         .out_valid        (fifo_valid),
         .fifo_free_space  (),
         // Inputs
         .clk              (clk),
         .rst              (rst),
         .in_flit          (in_flit),
         .in_valid         (in_valid),
         .out_ready        (fifo_ready));


   always @ (*) begin: input_fsm
      nxt_input_state = input_state;
      nxt_bit_cnt = bit_cnt;
      in_valid = 0;
      in_flit = 0;
      nxt_in_reg_data = in_reg_data;
      nxt_jtag_reset = jtag_reset;
      nxt_in_reg_cnt = in_reg_cnt;
      nxt_error = error;
      nxt_acc_cnt = acc_cnt;

      case (input_state)
         IDLE: begin : input_fsm_is_idle
            nxt_error = 1'b0;
            nxt_bit_cnt = 'h0;
            nxt_in_reg_data = 'h0;
            nxt_in_reg_cnt = 0;
            nxt_jtag_reset = 1'b0;
            nxt_acc_cnt = 0;
            if(shift == 1'b1) begin
               nxt_input_state = READ_PACKET_LENGTH;
               nxt_bit_cnt = bit_cnt + 1;
               nxt_in_reg_cnt[bit_cnt] = tdi;
            end
         end

         READ_PACKET_LENGTH: begin: read_length_of_the_incoming_packet
            // the first bits include the number of valid words
            // which are following
            nxt_in_reg_cnt[bit_cnt] = tdi;
            if (bit_cnt < WORD_WIDTH - 1) begin
               nxt_bit_cnt = bit_cnt + 1;
            end else if (bit_cnt == WORD_WIDTH - 1) begin
               // TODO: generate error if too much packets are sent
               nxt_bit_cnt = 0;
               if (&{tdi, nxt_in_reg_cnt[WORD_WIDTH-2:0]}) begin
                  // if there are only ones in the first bits, it is a jtag_reset
                  // signal
                  nxt_input_state = DROP_PACKET;
                  nxt_jtag_reset = 1'b1;
                  nxt_error = 1'b0;
               end else if (|{tdi, nxt_in_reg_cnt[WORD_WIDTH-2:0]}) begin
                  //a write is happening, with at least 1 packet coming through
                  nxt_input_state = IGNORE_READ_LENGTH;
                  nxt_jtag_reset = 1'b0;
                  nxt_error = 1'b0;
               end else begin
                  //no packet size sent, equivalent to a read -> disregard data
                  nxt_input_state = DROP_PACKET;
                  nxt_jtag_reset = 1'b0;
               end
            end else begin
               nxt_input_state = READ_PACKET_LENGTH;
               nxt_error = 1'b0;
            end
         end

         IGNORE_READ_LENGTH: begin : ignore_while_the_number_of_read_packets_is_sent
            nxt_in_reg_cnt = in_reg_cnt;
            if (bit_cnt == WORD_WIDTH-1) begin
               nxt_bit_cnt = 0;
               nxt_input_state = READ_PACKET;
            end else begin
               nxt_bit_cnt = bit_cnt + 1;
               nxt_input_state = IGNORE_READ_LENGTH;
            end
         end

         READ_PACKET: begin: read_the_incoming_data
            nxt_error = 1'b0;
            nxt_in_reg_cnt = in_reg_cnt;
            // the incoming data bits are buffered in a register until
            // a word is complete. Afterwards word is sent to the synchronous
            // FIFO
            if (bit_cnt < WORD_WIDTH-1) begin
               nxt_bit_cnt = bit_cnt + 1;
               nxt_in_reg_data[bit_cnt] = tdi;
            end else if (bit_cnt == WORD_WIDTH-1) begin
               nxt_in_reg_data[bit_cnt] = tdi;
               in_flit = { tdi, nxt_in_reg_data[WORD_WIDTH-2:0] };
               nxt_bit_cnt = 0;
               if (in_ready == 1) begin
                  in_valid = 1;
                  nxt_acc_cnt = acc_cnt + 1;
                  nxt_in_reg_cnt = in_reg_cnt - 1;
                  if (nxt_in_reg_cnt == 0) begin
                     nxt_input_state = DROP_PACKET;
                  end else begin
                     //more packets coming through
                     nxt_input_state = READ_PACKET;
                  end
               end else begin
                  //fifo is full, drop all further incoming data to preserve data order
                  nxt_input_state = DROP_PACKET;
               end
            end
         end

         DROP_PACKET: begin: drop_the_packet
            nxt_bit_cnt = 0;
            nxt_in_reg_cnt = in_reg_cnt;
            if (update || capture) begin
               // the incoming bits are dropped
               // until the shifting is complete
               nxt_input_state = IDLE;
               nxt_error = 1'b0;
               nxt_jtag_reset = 1'b0;
            end else begin
               nxt_input_state = DROP_PACKET;
               nxt_jtag_reset = 1'b0;
            end
         end
      endcase
   end

   always @ (posedge clk) begin
      if (rst || update == 1'b1) begin
         jtag_reset <= 1'b0;
         input_state <= DROP_PACKET;
         bit_cnt <= 'h0;
         in_reg_data <= 'h0;
         in_reg_cnt <= 'h0;
         acc_cnt <= 0;
      end else begin
         jtag_reset <= nxt_jtag_reset;
         input_state <= nxt_input_state;
         bit_cnt <= nxt_bit_cnt;
         in_reg_data <= nxt_in_reg_data;
         in_reg_cnt <= nxt_in_reg_cnt;
         acc_cnt <= nxt_acc_cnt;
      end
      error <= nxt_error;
   end

endmodule
