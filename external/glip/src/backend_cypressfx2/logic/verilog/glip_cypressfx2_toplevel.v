/* Copyright (c) 2014 by the author(s)
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
 * GLIP Toplevel Interface for the Cypress FX2 backend
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */
module glip_cypressfx2_toplevel(/*AUTOARG*/
   // Outputs
   fx2_sloe_n, fx2_slrd_n, fx2_slwr_n, fx2_pktend_n, fx2_fifoadr,
   com_rst, fifo_out_ready, fifo_in_valid, fifo_in_data,
   ctrl_logic_rst,
   // Inouts
   fx2_fd,
   // Inputs
   fx2_ifclk, fx2_com_rst, fx2_logic_rst, fx2_flaga_n, fx2_flagb_n,
   fx2_flagc_n, fx2_flagd_n, clk, rst, fifo_out_valid, fifo_out_data,
   fifo_in_ready
   );

   // Number of cycles of the fx2_ifclk (30 MHz by default) until the send
   // buffer of the FX2 chip is flushed, even if not a full USB transfer can
   // be sent out (resulting in a short packet)
   parameter FORCE_SEND_TIMEOUT = 3_000_000; // 100 ms

   // Cypress FX2 ports
   input        fx2_ifclk;
   input        fx2_com_rst;
   input        fx2_logic_rst;
   inout [15:0] fx2_fd;
   output       fx2_sloe_n;
   output       fx2_slrd_n;
   output       fx2_slwr_n;
   output       fx2_pktend_n;
   output [1:0] fx2_fifoadr;
   input        fx2_flaga_n;
   input        fx2_flagb_n;
   input        fx2_flagc_n;
   input        fx2_flagd_n;

   // Clock/Reset
   input         clk;
   input         rst;
   output        com_rst;

   // GLIP FIFO Interface
   input         fifo_out_valid;
   output        fifo_out_ready;
   input [15:0]  fifo_out_data;
   output        fifo_in_valid;
   input         fifo_in_ready;
   output [15:0] fifo_in_data;

   // GLIP Control Interface
   output        ctrl_logic_rst;

   assign ctrl_logic_rst = fx2_logic_rst;

   wire int_rst;
   assign int_rst = fx2_com_rst | rst;
   assign com_rst = int_rst;

   wire          int_fifo_out_valid;
   wire [15:0]   int_fifo_out_data;
   reg           int_fifo_out_ready;

   reg           int_fifo_in_valid;
   wire [15:0]   int_fifo_in_data;
   wire          int_fifo_in_ready;

   wire          fx2_epout_fifo_empty;
   wire          fx2_epin_fifo_almost_full;
   wire          fx2_epin_fifo_full;

   assign fx2_epout_fifo_empty = ~fx2_flaga_n;
   assign fx2_epin_fifo_almost_full = ~fx2_flagd_n;
   assign fx2_epin_fifo_full = ~fx2_flagc_n;

   wire [15:0]   fx2_fd_in;
   wire [15:0]   fx2_fd_out;
   assign fx2_fd_in = fx2_fd;
   assign fx2_fd = (~fx2_slwr_n ? fx2_fd_out : 16'hz);

   assign fx2_fd_out = int_fifo_out_data;
   assign int_fifo_in_data = fx2_fd_in;

   reg           wr;
   reg           rd;
   reg [1:0]     fifoadr;
   reg           pktend;
   assign fx2_sloe_n = ~rd;
   assign fx2_slwr_n = ~wr;
   assign fx2_slrd_n = ~rd;
   assign fx2_fifoadr = fifoadr;
   assign fx2_pktend_n = ~pktend;

   reg [$clog2(FORCE_SEND_TIMEOUT+1)-1:0]  counter;
   reg [$clog2(FORCE_SEND_TIMEOUT+1)-1:0]  nxt_counter;

   reg        precedence;
   reg        nxt_precedence;
   localparam WRITE = 0;
   localparam READ = 1;


   always @(posedge fx2_ifclk) begin
      if (int_rst) begin
         precedence <= WRITE;
         counter <= 0;
      end else begin
         precedence <= nxt_precedence;
         counter <= nxt_counter;
      end
   end

   wire flush;
   assign flush = (counter == 1) & !can_write;

   wire can_write;
   wire can_read;
   wire prefer_write;
   wire prefer_read;

   assign can_write = !fx2_epin_fifo_full & int_fifo_out_valid;
   assign can_read = !fx2_epout_fifo_empty & int_fifo_in_ready;
   assign prefer_write = (precedence == WRITE);
   assign prefer_read = (precedence == READ);

   always @(*) begin
      // default values
      rd = 0;
      wr = 0;
      pktend = 0;

      int_fifo_out_ready = 0;
      int_fifo_in_valid = 0;
      fifoadr = 2'b00;
      nxt_precedence = WRITE;

      // Count down
      if (counter > 0) begin
         nxt_counter = counter - 1;
      end else begin
         nxt_counter = 0;
      end

      if (!int_rst) begin
         if (flush) begin
            pktend = 1;
            fifoadr = 2'b10;
         end else if (can_write & (prefer_write || !can_read)) begin
            wr = 1;
            int_fifo_out_ready = 1;
            fifoadr = 2'b10;
            nxt_counter = FORCE_SEND_TIMEOUT;
            nxt_precedence = READ;
         end else if (can_read & (prefer_read || !can_write)) begin
            rd = 1;
            int_fifo_in_valid = 1;
            nxt_precedence = WRITE;
         end
      end
   end

   // Clock domain crossing logic -> FX2
   wire out_full;
   wire out_empty;
   assign fifo_out_ready = ~out_full;
   assign int_fifo_out_valid = ~out_empty;

   cdc_fifo
      #(.DW(16))
      out_fifo_cdc(// Logic side (write input)
                   .wr_full(out_full),
                   .wr_clk(clk),
                   .wr_en(fifo_out_valid),
                   .wr_data(fifo_out_data),
                   .wr_rst(~int_rst),

                   // FX2 side (read output)
                   .rd_empty(out_empty),
                   .rd_data(int_fifo_out_data),
                   .rd_clk(fx2_ifclk),
                   .rd_rst(~int_rst),
                   .rd_en(int_fifo_out_ready));

   // Clock domain crossing FX2 -> logic
   wire in_full;
   wire in_empty;
   assign int_fifo_in_ready = ~in_full;
   assign fifo_in_valid = ~in_empty;

   cdc_fifo
      #(.DW(16))
      in_fifo_cdc(// FX2 side (write input)
                  .wr_full(in_full),
                  .wr_clk(fx2_ifclk),
                  .wr_en(int_fifo_in_valid),
                  .wr_data(int_fifo_in_data),
                  .wr_rst(~int_rst),

                  // Logic side (read output)
                  .rd_empty(in_empty),
                  .rd_data(fifo_in_data),
                  .rd_clk(clk),
                  .rd_rst(~int_rst),
                  .rd_en(fifo_in_ready));

endmodule

// Local Variables:
// verilog-library-directories:("." "cdc")
// verilog-auto-inst-param-value: t
// End:
