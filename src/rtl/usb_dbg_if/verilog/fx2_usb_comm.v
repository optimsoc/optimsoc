/* Copyright (c) 2013 by the author(s)
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
 * This is the EZ-USB FX2 USB communication interface. It transfers data words
 * to and from the external USB interface, using the FX2's Slave FIFO mode.
 * The FX2 needs to have an appropriate firmware installed supporting this
 * transfer mode.
 *
 * "TRM" in this file stands for the Cypress EZ-USB FX2 Technical Reference
 * Manual, available at http://www.cypress.com/?rID=38232
 *
 * Author(s):
 *   Philipp Wagner <mail@philipp-wagner.com>
 */

module fx2_usb_comm (/*AUTOARG*/
   // Outputs
   fx2_slrd, fx2_slwr, fx2_sloe, fx2_pktend, fx2_fifoadr, fx2_fd_out,
   in_data, in_valid, out_ready,
   // Inputs
   clk_sys, rst, fx2_clk, fx2_fd_in, fx2_epout_fifo_empty,
   fx2_epin_fifo_full, fx2_epin_fifo_almost_full, in_ready, out_data,
   out_valid
   );

   // force sending a short USB burst after this number of fx2_clk cycles if no
   // data is available for sending
   parameter FORCE_SEND_TIMEOUT = 1024;

   localparam STATE_WIDTH = 2;
   localparam STATE_IDLE = 2'b00;
   localparam STATE_SEND = 2'b01;
   localparam STATE_SEND_2 = 2'b10;
   localparam STATE_RCV = 2'b11;

   reg [STATE_WIDTH-1:0] state;
   reg [STATE_WIDTH-1:0] nxt_state;

   input clk_sys;
   input rst;

   // NOTE: All fx2_* signals are ACTIVE LOW!
   input fx2_clk;
   output reg fx2_slrd;
   output reg fx2_slwr;
   output fx2_sloe;
   output fx2_pktend;
   output reg [1:0] fx2_fifoadr;
   input [15:0] fx2_fd_in;
   output [15:0] fx2_fd_out;
   // USB OUT endpoint FIFO is empty, i.e. no data is available to receive
   input fx2_epout_fifo_empty;
   // USB IN endpoint FIFO is full, i.e. no data can be sent
   input fx2_epin_fifo_full;
   // USB IN endpoint FIFO is almost full, only one more word can be sent
   input fx2_epin_fifo_almost_full;

   // Debug NoC interface (in/out are seen from the Debug NoC side)
   // The width is hardcoded (unlike in all other modules) to enforce the
   // 16-bit word-width requirement for this module. If you change the Debug
   // NoC width, you will need to change the whole communication protocol.
   output [15:0] in_data;
   input in_ready;
   output in_valid;
   input [15:0] out_data;
   output out_ready;
   input out_valid;

   wire rcv_fifo_full;
   reg rcv_fifo_wr_en;

   wire send_fifo_empty;
   reg send_fifo_rd_en;
   wire send_fifo_valid;
   wire send_fifo_full;
   assign out_ready = ~send_fifo_full;

   // utility signals to make the code easier to read
   // those signals will be removed during synthesis and cause no performance
   // impact they just help reading esp. due to the active low signals
   wire sending_data_available;
   assign sending_data_available = ~send_fifo_empty;

   wire can_send_data;
   assign can_send_data = fx2_epin_fifo_almost_full; // active low!

   wire receiving_data_available;
   assign receiving_data_available = fx2_epout_fifo_empty; // active low

   wire can_receive_data;
   assign can_receive_data = ~rcv_fifo_full;


   reg [$clog2(FORCE_SEND_TIMEOUT+1)-1:0] no_send_cnt;
   reg rst_no_send_cnt;

   wire force_usb_sending;
   assign force_usb_sending = (no_send_cnt == FORCE_SEND_TIMEOUT);
   assign fx2_pktend = ~force_usb_sending;

   // enable output driver of FX2 chip if we want to read
   // coupling these two signals is ok according to the TRM
   assign fx2_sloe = fx2_slrd;


   // Send data FIFO (Debug Noc -> USB)
   fifo_usb_dual
      u_send_fifo(.rst(rst),

                  .wr_clk(clk_sys),
                  .wr_en(out_valid),
                  .full(send_fifo_full),
                  .din(out_data),

                  .rd_clk(fx2_clk),
                  .rd_en(send_fifo_rd_en),
                  .dout(fx2_fd_out),
                  .valid(send_fifo_valid),
                  .empty(send_fifo_empty));


   // Receive data FIFO (USB -> Debug NoC)
   // This FIFO was created with
   //  - Independent clocks in BRAM
   //  - First-Word-Fall-Through
   //  - Write width 16, write depth 16, read width 16
   //  - valid flag, active high
   //  - synchronized reset pin
   
   fifo_usb_to_noc
      u_rcv_fifo(.rst(rst),

                 .wr_clk(fx2_clk),
                 .wr_en(rcv_fifo_wr_en),
                 .full(rcv_fifo_full),
                 .din(fx2_fd_in),

                 .rd_clk(clk_sys),
                 .rd_en(in_ready),
                 .dout(in_data),
                 .valid(in_valid),
                 .empty()); // valid does not go up if FIFO is empty


   always @ (posedge fx2_clk) begin
      if (rst || rst_no_send_cnt || force_usb_sending) begin
         no_send_cnt <= 0;
      end else begin
         no_send_cnt <= no_send_cnt + 1;
      end
   end

   always @ (posedge fx2_clk) begin
      if (rst) begin
         state <= STATE_IDLE;
      end else begin
         state <= nxt_state;
      end
   end

   always @ (*) begin
      // default values
      fx2_slwr = 1; // active low
      fx2_slrd = 1; // active low

      rcv_fifo_wr_en = 0;
      send_fifo_rd_en = 0;
      rst_no_send_cnt = 0;
      fx2_fifoadr = 2'b00;

      case (state)
         STATE_IDLE: begin
            if (sending_data_available && can_send_data) begin
               nxt_state = STATE_SEND;
            end else if (receiving_data_available && can_receive_data) begin
               nxt_state = STATE_RCV;
            end else begin
               nxt_state = STATE_IDLE;
            end
         end

         STATE_SEND: begin
            send_fifo_rd_en = 1; // takes one cycle
            nxt_state = STATE_SEND_2;
         end

         STATE_SEND_2: begin
            if (receiving_data_available && can_receive_data) begin
               nxt_state = STATE_RCV;
            end else if (sending_data_available && can_send_data) begin
               nxt_state = STATE_SEND_2;
               send_fifo_rd_en = 1;
            end else begin
               nxt_state = STATE_IDLE;
            end

            fx2_slwr = ~send_fifo_valid; // active low
            rst_no_send_cnt = send_fifo_valid;
            fx2_fifoadr = 2'b10; // EP6 (TRM p. 104)
         end

         STATE_RCV: begin
            if (sending_data_available && can_send_data) begin
               nxt_state = STATE_SEND;
            end else if (receiving_data_available && can_receive_data) begin
               nxt_state = STATE_RCV;
               fx2_slrd = 0; // active low
               fx2_fifoadr = 2'b00; // EP2 (TRM p. 104)
               rcv_fifo_wr_en = 1;
            end else begin
               nxt_state = STATE_IDLE;
            end
         end
      endcase
   end
endmodule
