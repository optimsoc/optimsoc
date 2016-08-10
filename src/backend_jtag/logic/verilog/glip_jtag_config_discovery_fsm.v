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
 * GLIP JTAG: Configuration Discovery FSM
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Alexandra Weber <sandra.eli.weber@tum.de>
 *   Jan Alexander Wessel <jan.wessel@tum.de>
 */
 module glip_jtag_config_discovery_fsm(/*AUTOARG*/
   // Outputs
   data_transfer, config_discovery, config_discovery_tdo,
   ctrl_logic_rst,
   // Inputs
   config_discovery_tdi, clk, rst, shift, update
   );

   parameter WORD_WIDTH = 16;
   parameter NUM_WORDS = 3;

   input config_discovery_tdi;
   input clk;
   input rst;
   input shift;
   input update;

   output reg data_transfer;
   output reg config_discovery;
   output reg config_discovery_tdo;

   output reg ctrl_logic_rst;

   reg nxt_ctrl_logic_rst;
   reg nxt_data_transfer;
   reg nxt_config_discovery;

   reg [1:0] config_state;
   reg [1:0] nxt_config_state;

   reg [WORD_WIDTH-1:0] bit_cnt;
   reg [WORD_WIDTH-1:0] nxt_bit_cnt;

   reg [WORD_WIDTH-1:0] word_width = WORD_WIDTH;
   reg [WORD_WIDTH-1:0] words_per_transfer = NUM_WORDS;

   reg [WORD_WIDTH-1:0] in_reg;
   reg [WORD_WIDTH-1:0] nxt_in_reg;

   localparam IDLE = 2'b00;
   localparam SEND_WORD_WIDTH = 2'b01;
   localparam SEND_BUFFER_SIZE = 2'b10;
   localparam WAIT_FOR_UPDATE = 2'b11;

   always @ (*) begin: conf_fsm

      nxt_bit_cnt = bit_cnt;
      nxt_config_state = config_state;
      nxt_in_reg = in_reg;
      nxt_data_transfer = data_transfer;
      nxt_config_discovery = config_discovery;
      nxt_ctrl_logic_rst = 1'b0;

      case (config_state)

         IDLE: begin : config_discovery_fsm_is_idle
            config_discovery_tdo = 1'b0;
            if (shift) begin
               nxt_bit_cnt = bit_cnt + 1;
               nxt_in_reg[bit_cnt] = config_discovery_tdi;
               nxt_config_discovery = 1'b0;
               nxt_data_transfer = 1'b0;
               if (bit_cnt == WORD_WIDTH - 1) begin
                  nxt_bit_cnt = 0;
                  if (&{config_discovery_tdi, in_reg[WORD_WIDTH-2:0]}) begin
                     nxt_config_state = SEND_WORD_WIDTH;
                     nxt_config_discovery = 1'b1;
                     nxt_data_transfer = 1'b0;
                  end else begin
                     nxt_config_state = WAIT_FOR_UPDATE;
                     nxt_data_transfer = 1'b1;
                     nxt_config_discovery = 1'b0;
                  end
               end
            end
         end

         SEND_WORD_WIDTH: begin: output_of_word_width
            nxt_bit_cnt = bit_cnt + 1;
            nxt_config_discovery = 1'b1;
            nxt_data_transfer = 1'b0;
            config_discovery_tdo = word_width[bit_cnt];

            // read side channel data
            if (bit_cnt == 0 && config_discovery_tdi == 1'b1) begin
               // bit 0: logic reset
               nxt_ctrl_logic_rst = 1'b1;
            end

            if (bit_cnt == WORD_WIDTH - 1) begin
               nxt_bit_cnt = 0;
               nxt_config_state = SEND_BUFFER_SIZE;
            end
         end

         SEND_BUFFER_SIZE: begin: output_of_buffer_size
            nxt_bit_cnt = bit_cnt + 1;
            nxt_config_discovery = 1'b1;
            nxt_data_transfer = 1'b0;
            config_discovery_tdo = words_per_transfer[bit_cnt];
            if (bit_cnt == WORD_WIDTH - 1) begin
               nxt_bit_cnt = 0;
               nxt_config_state = WAIT_FOR_UPDATE;
               nxt_in_reg = 0;
            end
         end

         WAIT_FOR_UPDATE: begin: wait_for_new_shifting_cycle
            config_discovery_tdo = 1'b0;
            if (update) begin
               nxt_config_state = IDLE;
            end else begin
               nxt_config_state = WAIT_FOR_UPDATE;
            end
         end

      endcase
   end

   always @ (posedge clk) begin
      if (update) begin
         bit_cnt <= 0;
         config_state <= IDLE;
         in_reg <= 0;
         config_discovery <= 1'b0;
         data_transfer <= 1'b0;
         ctrl_logic_rst <= 1'b0;
      end else begin
         bit_cnt <= nxt_bit_cnt;
         config_state <= nxt_config_state;
         in_reg <= nxt_in_reg;
         data_transfer <= nxt_data_transfer;
         config_discovery <= nxt_config_discovery;
         ctrl_logic_rst <= nxt_ctrl_logic_rst;
      end
   end
endmodule
