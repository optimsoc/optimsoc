// Copyright 2016-2018 by the authors
//
// Copyright and related rights are licensed under the Solderpad
// Hardware License, Version 0.51 (the "License"); you may not use
// this file except in compliance with the License. You may obtain a
// copy of the License at http://solderpad.org/licenses/SHL-0.51.
// Unless required by applicable law or agreed to in writing,
// software, hardware and materials distributed under this License is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the
// License.
//
// Authors:
//    Philipp Wagner <philipp.wagner@tum.de>
//    Stefan Wallentowitz <stefan@wallentowitz.de>

import dii_package::dii_flit;

/**
 * Module: osd_event_packetization
 *
 * Package data into one or multiple DI event packet (TYPE == EVENT). If the
 * maximum number of flits (MAX_PKT_LEN) is exceeded two or more packets are
 * generated. The first packet has TYPE_SUB set to 0, the continued packets
 * have TYPE_SUB set to 1.
 *
 * If overflow is set an overflow packet is generated. An overflow packet
 * has TYPE_SUB set to 5 and contains the number of overflowed packets in the
 * first payload flit. The number of overflowed packets is passed in
 * event_data[15:0].
 */
module osd_event_packetization #(
   /**
    * The maximum length of a DI packet in flits, including the header flits
    */
   parameter MAX_PKT_LEN = 12,

   /**
    * The maximum number of payload words the packet could consist of.
    * The actual number of payload words is given by data_num_words.
    */
   parameter MAX_DATA_NUM_WORDS = 'hx
) (
   input                                     clk,
   input                                     rst,

   output dii_flit                           debug_out,
   input                                     debug_out_ready,

   // DI address of this module (SRC)
   input [15:0]                              id,

   // DI address of the event destination (DEST)
   input [15:0]                              dest,
   // Generate an overflow packet
   input                                     overflow,

   // a new event is available
   input                                     event_available,
   // the packet has been sent
   output logic                              event_consumed,

   // number of data words this event consists of
   input [$clog2(MAX_DATA_NUM_WORDS+1)-1:0]  data_num_words,

   // data request: index of the data word
   output [$clog2(MAX_DATA_NUM_WORDS)-1:0]   data_req_idx,
   // data request: request is valid
   output                                    data_req_valid,

   // a data word
   input [15:0]                              data
);

   localparam NUM_HEADER_FLITS = 3; // header flits: SRC, DEST, FLAGS
   localparam MAX_PAYLOAD_LEN = MAX_PKT_LEN - NUM_HEADER_FLITS;

   // packet counter within a single event transfer
   localparam PKG_CNT_WIDTH = $clog2((MAX_DATA_NUM_WORDS + (MAX_PAYLOAD_LEN - 1)) / MAX_PAYLOAD_LEN);
   localparam PKG_CNT_WIDTH_NONZERO = PKG_CNT_WIDTH == 0 ? 1 : PKG_CNT_WIDTH;
   logic [PKG_CNT_WIDTH_NONZERO-1:0] pkg_cnt, nxt_pkg_cnt;

   // number of packets required to transfer the event data
   // cnt from 0..(num_pkgs-1) => num_pkgs requires one more bit
   localparam NUM_PKGS_WIDTH = PKG_CNT_WIDTH + 1;
   logic [NUM_PKGS_WIDTH-1:0] num_pkgs;
   assign num_pkgs = NUM_PKGS_WIDTH'((data_num_words + (MAX_PAYLOAD_LEN - 1)) / MAX_PAYLOAD_LEN);

   // data word of event data
   logic [$clog2(MAX_DATA_NUM_WORDS)-1:0] word_cnt, nxt_word_cnt;

   // payload flit within the currently sent packet
   logic [$clog2(MAX_PAYLOAD_LEN)-1:0] payload_flit_cnt, nxt_payload_flit_cnt;

   // FSM states
   enum {
      IDLE_DEST,
      DESTINATION,
      SOURCE, FLAGS,
      OVERFLOW,
      PAYLOAD
   } state, nxt_state;

   assign data_req_idx = word_cnt;
   assign data_req_valid = (state == PAYLOAD || state == OVERFLOW);

   localparam TYPE_SUB_LAST = 4'h0;
   localparam TYPE_SUB_CONTINUE = 4'h1;
   localparam TYPE_SUB_OVERFLOW = 4'h5;

   always_ff @(posedge clk) begin
      if (rst) begin
         word_cnt <= 0;
         payload_flit_cnt <= 0;
         pkg_cnt <= 0;
         state <= IDLE_DEST;
      end else begin
         word_cnt <= nxt_word_cnt;
         payload_flit_cnt <= nxt_payload_flit_cnt;
         pkg_cnt <= nxt_pkg_cnt;
         state <= nxt_state;
      end
   end

   always_comb begin
      event_consumed = 0;
      debug_out.valid = 0;
      debug_out.data = 'x;
      debug_out.last = 0;
      nxt_state = state;
      nxt_word_cnt = word_cnt;
      nxt_payload_flit_cnt = payload_flit_cnt;
      nxt_pkg_cnt = pkg_cnt;

      case (state)
        IDLE_DEST: begin
           debug_out.data = 16'h0;
           if (event_available) begin
              debug_out.valid = 1;
              debug_out.data = dest;
              nxt_payload_flit_cnt = 0;
              if (debug_out_ready) begin
                 nxt_state = SOURCE;
              end
           end
        end
        SOURCE: begin
           debug_out.valid = 1;
           debug_out.data = id;

           if (debug_out_ready) begin
              nxt_state = FLAGS;
           end
        end
        FLAGS: begin
           debug_out.data[15:14] = 2'b10; // TYPE == EVENT

           // TYPE_SUB
           if (overflow) begin
              debug_out.data[13:10] = TYPE_SUB_OVERFLOW;
           end else begin
              if (pkg_cnt == num_pkgs - 1) begin
                 debug_out.data[13:10] = TYPE_SUB_LAST;
              end else begin
                 debug_out.data[13:10] = TYPE_SUB_CONTINUE;
              end
           end

           debug_out.data[9:0] = 10'h0; // reserved
           debug_out.valid = 1;

           if (debug_out_ready)
              nxt_state = overflow ? OVERFLOW : PAYLOAD;
        end
        OVERFLOW: begin
           debug_out.valid = 1;
           debug_out.data = data;
           debug_out.last = 1;
           if (debug_out_ready) begin
              nxt_state = IDLE_DEST;
              event_consumed = 1'b1;
           end
        end
        PAYLOAD: begin
           debug_out.valid = 1;

           if (word_cnt < data_num_words - 1) begin
              debug_out.data = data;
              debug_out.last = (payload_flit_cnt == MAX_PAYLOAD_LEN - 1);

              if (debug_out_ready) begin
                 nxt_word_cnt = word_cnt + 1;

                 if (payload_flit_cnt == MAX_PAYLOAD_LEN - 1) begin
                    // we need to continue the transfer in the next packet
                    nxt_state = IDLE_DEST;
                    nxt_pkg_cnt = pkg_cnt + 1;
                    nxt_payload_flit_cnt = 0;
                 end else begin
                    nxt_state = PAYLOAD;
                    nxt_payload_flit_cnt = payload_flit_cnt + 1;
                 end

              end
           end else begin
              // last payload word of the transfer
              debug_out.last = 1;
              debug_out.data = data;

              if (debug_out_ready) begin
                 event_consumed = 1'b1;
                 nxt_state = IDLE_DEST;
                 nxt_pkg_cnt = 0;
                 nxt_word_cnt = 0;
              end
           end
        end
      endcase
   end

endmodule
