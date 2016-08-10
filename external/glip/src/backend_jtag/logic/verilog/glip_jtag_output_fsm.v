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
 * GLIP JTAG: Output FSM
 *
 * Author(s):
 *   Alexandra Weber <sandra.eli.weber@tum.de>
 *   Jan Alexander Wessel <jan.wessel@tum.de>
 */
module glip_jtag_output_fsm(
   // input ports
   rst,
   clk,
   tdi,
   shift, update,
   in_error, in_written,
   fifo_data, fifo_valid,
   //output ports
   tdo,
   fifo_ready
   );

   parameter WORD_WIDTH = 16;
   parameter NUM_WORDS_SHIFT = 1;

   input rst;
   // jtag interface
   input clk;
   output reg tdo;
   input tdi;

   // tap signal
   input shift;
   input update;
   input in_error;
   input [WORD_WIDTH-1:0] in_written;

   // fifo interface
   output reg fifo_ready;
   input [WORD_WIDTH-1:0] fifo_data;
   input fifo_valid;

   // this register stores the FIFO data for output
   reg [WORD_WIDTH-1:0] out_reg;
   reg [WORD_WIDTH-1:0] nxt_out_reg;

   reg [3:0] output_state;
   reg [3:0] nxt_output_state;


   // this counts the number of words which were already sent out
   reg [WORD_WIDTH-1:0] word_cnt;
   reg [WORD_WIDTH-1:0] nxt_word_cnt;

   // this counts the number of valid words which were sent out
   reg [WORD_WIDTH-1:0] packet_count;
   reg [WORD_WIDTH-1:0] nxt_packet_count;

   // this is to buffer the number of correctly written words
   reg [WORD_WIDTH-1:0] free;
   reg [WORD_WIDTH-1:0] nxt_free;

   // this counter counts the bits until the word is complete
   reg [WORD_WIDTH-1:0] bit_cnt;
   reg [WORD_WIDTH-1:0] nxt_bit_cnt;

   // this register stores the higher number of requested words
   reg [WORD_WIDTH-1:0] in_reg_cnt;
   reg [WORD_WIDTH-1:0] nxt_in_reg_cnt;

   reg [WORD_WIDTH-1:0] write_length_reg;
   reg [WORD_WIDTH-1:0] nxt_write_length_reg;
   reg [WORD_WIDTH-1:0] read_length_reg;
   reg [WORD_WIDTH-1:0] nxt_read_length_reg;

   localparam IDLE = 4'b0000;
   localparam WRITE_LENGTH = 4'b0001;
   localparam READ_LENGTH = 4'b0010;
   localparam NO_DATA = 4'b0011;
   localparam SEND_DATA = 4'b0100;
   localparam SEND_COUNT_WRITE = 4'b0101;
   localparam SEND_COUNT_READ = 4'b0110;
   localparam CONFIG_DISC = 4'b1111;

   always @ (*) begin: output_fsm

      nxt_output_state = output_state;
      nxt_word_cnt = word_cnt;
      nxt_bit_cnt = bit_cnt;
      nxt_packet_count = packet_count;
      nxt_out_reg = out_reg;
      nxt_free = free;
      nxt_in_reg_cnt = in_reg_cnt;
      nxt_read_length_reg = read_length_reg;
      nxt_write_length_reg = write_length_reg;

      fifo_ready = 0;

      case (output_state)
         IDLE: begin : output_fsm_is_idle
            fifo_ready = 0;
            nxt_word_cnt = 0;
            nxt_packet_count = 0;
            nxt_in_reg_cnt = 0;
            nxt_free = 0;
            tdo = fifo_data[0];
            nxt_write_length_reg = 0;
            nxt_read_length_reg = 0;
            if (shift == 1'b1) begin
               nxt_output_state = WRITE_LENGTH;
               nxt_bit_cnt = bit_cnt + 1;
               nxt_in_reg_cnt[bit_cnt] = tdi;
            end
         end

         WRITE_LENGTH : begin : read_number_of_following_input_words
            // the first bits include the number of valid words
            // which are following
            tdo = fifo_data[0];
            nxt_in_reg_cnt[bit_cnt] = tdi;
            if (bit_cnt < WORD_WIDTH-1) begin
               nxt_bit_cnt = bit_cnt + 1;
               nxt_output_state = WRITE_LENGTH;
            end else begin
               nxt_bit_cnt = 0;
               if (&{tdi, nxt_in_reg_cnt[WORD_WIDTH-2:0]}) begin
                  nxt_output_state = CONFIG_DISC;
               end else begin
                  nxt_output_state = READ_LENGTH;
                  nxt_write_length_reg = {tdi, nxt_in_reg_cnt[WORD_WIDTH-2:0]};
               end
            end
         end

         READ_LENGTH : begin : how_many_packets_are_wanted
            tdo = fifo_data[0];
            nxt_in_reg_cnt[bit_cnt] = tdi;
            if (bit_cnt < WORD_WIDTH-1) begin
               nxt_bit_cnt = bit_cnt + 1;
               nxt_output_state = READ_LENGTH;
            end else begin
               nxt_out_reg[WORD_WIDTH-1:0] = fifo_data;
               nxt_bit_cnt = 0;
               nxt_read_length_reg = {tdi, nxt_in_reg_cnt[WORD_WIDTH-2:0]};
               if (|{tdi, nxt_in_reg_cnt[WORD_WIDTH-2:0]}) begin
                  if (fifo_valid) begin
                     // the FIFO has valid data
                     nxt_output_state = SEND_DATA;
                     fifo_ready = 1;
                  end else begin
                     // the FIFO has no valid data, so nothing is sent out
                     nxt_output_state = NO_DATA;
                  end
                  if (write_length_reg > {tdi, nxt_in_reg_cnt[WORD_WIDTH-2:0]}) begin
                     nxt_in_reg_cnt = write_length_reg;
                  end
               end else begin
                  if (write_length_reg > 0) begin
                     nxt_output_state = NO_DATA;
                     nxt_in_reg_cnt = write_length_reg;
                  end else begin
                     nxt_output_state = SEND_COUNT_WRITE;
                  end
               end
            end
          end

         SEND_DATA: begin: send_data
            // the current bit in out_reg is put out
            tdo = out_reg[bit_cnt];
            nxt_in_reg_cnt = in_reg_cnt;
            if (shift == 1'b1) begin
               if (bit_cnt == WORD_WIDTH-1) begin
                  nxt_bit_cnt = 0;
                  if (word_cnt == in_reg_cnt - 1) begin
                     // the last two words are used to communicate the number
                     // of valid words which were sent and for the number
                     // of correctly written words
                     nxt_output_state = SEND_COUNT_WRITE;
                     nxt_packet_count = word_cnt + 1;
                  end else begin
                     if (word_cnt == read_length_reg - 1) begin
                        nxt_output_state = NO_DATA;
                        nxt_packet_count = word_cnt + 1;
                     end else begin
                        nxt_word_cnt = word_cnt + 1;
                        if (fifo_valid) begin
                           nxt_out_reg[WORD_WIDTH-1:0] = fifo_data;
                           nxt_output_state = SEND_DATA;
                           fifo_ready = 1;
                        end else begin
                           nxt_packet_count = word_cnt + 1;
                           nxt_output_state = NO_DATA;
                           fifo_ready = 0;
                        end
                     end
                  end
               end else begin
                  nxt_bit_cnt = bit_cnt + 1;
                  nxt_output_state = SEND_DATA;
               end
            end else begin
               nxt_output_state = SEND_DATA;
            end
         end

        NO_DATA: begin : no_data
           // this is to send only zeros if there is no valid
           // data in the output FIFO or no more data is wanted
           tdo = 1'b0;
           nxt_in_reg_cnt = in_reg_cnt;
           if (shift == 1'b1) begin
               if (bit_cnt == WORD_WIDTH-1) begin
                  nxt_bit_cnt = 0;
                  if (word_cnt == in_reg_cnt - 1) begin
                     nxt_output_state = SEND_COUNT_WRITE;
                  end else begin
                     nxt_word_cnt = word_cnt + 1;
                     nxt_output_state = NO_DATA;
                  end
               end else begin
                  nxt_bit_cnt = bit_cnt + 1;
                  nxt_output_state = NO_DATA;
               end
           end else begin
               nxt_output_state = NO_DATA;
           end
        end

        SEND_COUNT_WRITE: begin: send_count_write
           // this sends the number of correctly written words
           // if an error occured in the input module then it sends
           // only ones
           nxt_bit_cnt = bit_cnt + 1;
           nxt_in_reg_cnt = in_reg_cnt;
           if (in_error) begin
               tdo = 1'b1;
           end else begin
              if (bit_cnt == 0) begin
                 nxt_free = in_written;
                 tdo = in_written[0];
              end else begin
                 tdo = free[bit_cnt];
              end
           end
           if (bit_cnt == WORD_WIDTH-1) begin
              nxt_output_state = SEND_COUNT_READ;
              nxt_bit_cnt = 0;
           end
        end

        SEND_COUNT_READ: begin: send_count_read
           // this sends the number of validly read words
           tdo = packet_count[bit_cnt];
           nxt_in_reg_cnt = in_reg_cnt;
           if (bit_cnt < WORD_WIDTH-1) begin
              nxt_bit_cnt = bit_cnt + 1;
           end else begin
              nxt_bit_cnt = 0;
              nxt_output_state = IDLE;
           end
         end

        CONFIG_DISC: begin: comm_is_reset
           tdo = 1'b0;
           nxt_bit_cnt = 0;
           nxt_in_reg_cnt = in_reg_cnt;
           if (update) begin
              nxt_output_state = IDLE;
           end
        end

        default: begin
           tdo = 1'bx;
        end
      endcase
   end

   always @ (posedge clk)begin
      if (update == 1'b1) begin
         output_state <= 0;
         word_cnt <= 'h0;
         out_reg <= 'h0;
         bit_cnt <= 'h0;
         in_reg_cnt <= 'h0;
         packet_count <= 0;
         free <= 0;
         write_length_reg <= 0;
         read_length_reg <= 0;
      end else begin
         output_state <= nxt_output_state;
         word_cnt <= nxt_word_cnt;
         bit_cnt <= nxt_bit_cnt;
         in_reg_cnt <= nxt_in_reg_cnt;
         out_reg <= nxt_out_reg;
         packet_count <= nxt_packet_count;
         free <= nxt_free;
         write_length_reg <= nxt_write_length_reg;
         read_length_reg <= nxt_read_length_reg;
      end
   end
endmodule
