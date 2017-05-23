/* Copyright (c) 2016-2017 by the author(s)
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
 * GLIP Toplevel Interface for the Cypress FX3 backend
 *
 * Author(s):
 *   Max Koenen <max.koenen@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

module glip_cypressfx3_toplevel
#(
   parameter WIDTH = 16,
   parameter BUFFER_DEPTH = 512,
   parameter FREQ_CLK_IO = 32'd100_000_000
)(
   // Cypress FX3 ports
   output                fx3_pclk,
   inout [WIDTH-1:0]     fx3_dq,
   output                fx3_slcs_n,
   output                fx3_sloe_n,
   output                fx3_slrd_n,
   output                fx3_slwr_n,
   output                fx3_pktend_n,
   output [1:0]          fx3_a,
   input                 fx3_flaga_n,
   input                 fx3_flagb_n,
   input                 fx3_flagc_n,
   input                 fx3_flagd_n,
   input                 fx3_com_rst,
   input                 fx3_logic_rst,
   output [2:0]          fx3_pmode,

   // Clock/Reset
   input                 clk, // logic clock (used for all fifo_* signals)
   input                 rst,
   input                 clk_io, // I/O clock (passed to FX3 as fx3_pclk)

   // GLIP FIFO Interface
   input                 fifo_out_valid,
   output                fifo_out_ready,
   input [WIDTH-1:0]     fifo_out_data,
   output                fifo_in_valid,
   input                 fifo_in_ready,
   output [WIDTH-1:0]    fifo_in_data,

   // GLIP Control Interface
   output                ctrl_logic_rst,
   output                com_rst
);

   localparam FORCE_SEND_TIMEOUT = 10000;
   localparam FX3_EPOUT = 2'b11;
   localparam FX3_EPIN = 2'b00;

   assign ctrl_logic_rst = fx3_logic_rst;

   // pass through I/O clock to FX3
   assign fx3_pclk = clk_io;

   wire  int_rst;
   assign int_rst = fx3_com_rst | rst;
   assign com_rst = int_rst;

   // Interface to the FIFOs from USB side
   wire  int_fifo_in_almost_full;
   reg   int_fifo_in_valid;
   reg   int_fifo_out_ready;
   wire  int_fifo_out_empty;

   wire  fx3_out_almost_empty;
   wire  fx3_out_empty;
   wire  fx3_in_almost_full;
   wire  fx3_in_full;

   reg       wr;
   reg       oe;
   reg       rd;
   reg [1:0] fifoadr;
   reg       pktend;

   wire [WIDTH-1:0]   fx3_dq_in;
   wire [WIDTH-1:0]   fx3_dq_out;
   assign fx3_dq_in = fx3_dq;
   assign fx3_dq = (fifoadr == FX3_EPIN ? fx3_dq_out : {WIDTH{1'hz}});

   assign fx3_slcs_n = 1'b0;

   assign fx3_out_empty = !fx3_flagc_n;
   assign fx3_out_almost_empty = !fx3_flagd_n;
   assign fx3_in_full = !fx3_flaga_n;
   assign fx3_in_almost_full = !fx3_flagb_n;

   // Wires for ingress cdc fifo
   wire [WIDTH-1:0]     ingress_cdc_wr_data;
   wire [WIDTH-1:0]     ingress_cdc_rd_data;
   wire                 ingress_cdc_wr_en;
   wire                 ingress_cdc_rd_en;
   wire                 ingress_cdc_rd_empty;

   // Wires for ingress fx3 buffer
   wire [WIDTH-1:0]     ingress_fx3_buffer_din;
   wire [WIDTH-1:0]     ingress_fx3_buffer_dout;
   wire                 ingress_fx3_buffer_wr_en;
   wire                 ingress_fx3_buffer_full;
   wire                 ingress_fx3_buffer_empty;
   wire                 ingress_fx3_buffer_rd_en;

   // Wires for egress fx3 buffer
   wire [WIDTH-1:0]     egress_fx3_buffer_din;
   wire [WIDTH-1:0]     egress_fx3_buffer_dout;
   wire                 egress_fx3_buffer_wr_en;
   wire                 egress_fx3_buffer_full;
   wire                 egress_fx3_buffer_empty;
   wire                 egress_fx3_buffer_rd_en;

   // Wires for egress cdc fifo
   wire [WIDTH-1:0]     egress_cdc_wr_data;
   wire [WIDTH-1:0]     egress_cdc_rd_data;
   wire                 egress_cdc_wr_en;
   wire                 egress_cdc_wr_full;
   wire                 egress_cdc_rd_en;
   wire                 egress_cdc_rd_empty;

   assign fx3_pmode = 3'bz1z;

   assign fx3_sloe_n = ~oe;
   assign fx3_slwr_n = ~wr;
   assign fx3_slrd_n = ~rd;
   assign fx3_a = fifoadr;
   assign fx3_pktend_n = ~pktend;

   // The timing of the FX3 FSM changes at high clk_io/fx3_pclk frequencies.
   // 70MHz chosen as threshold since frequencies between 63MHz and 80MHz should
   // be avoided anyways.
   generate
      if (FREQ_CLK_IO <= 32'd70_000_000) begin
         // Delay for reading from slave fifo (data will be available after
         // two clk cycles)
         reg         rd_delay;
         always @(posedge clk_io, posedge int_rst) begin
            if (int_rst) begin
               rd_delay <= 1'b0;
               int_fifo_in_valid <= 1'b0;
            end else begin
               rd_delay <= rd;
               int_fifo_in_valid <= rd_delay;
            end
         end
      end else if (FREQ_CLK_IO > 32'd70_000_000) begin
         // Delay for reading from slave fifo (data will be available after
         // three clk cycles)
         reg [1:0]   rd_delay;
         always @(posedge clk_io, posedge int_rst) begin
            if (int_rst) begin
               rd_delay[0] <= 1'b0;
               rd_delay[1] <= 1'b0;
               int_fifo_in_valid <= 1'b0;
            end else begin
               rd_delay[0] <= rd;
               rd_delay[1] <= rd_delay[0];
               int_fifo_in_valid <= rd_delay[1];
            end
         end
      end
   endgenerate

   reg [$clog2(FORCE_SEND_TIMEOUT+1)-1:0]  idle_counter;
   reg [$clog2(FORCE_SEND_TIMEOUT+1)-1:0]  nxt_idle_counter;

   // Single word read/write counter. Used to initiate single word reads/writes
   // in case the fsm stalls in 'STATE_WAIT_FLG_D'/'STATE_WAIT_FLG_B'.
   localparam SWRW_WAIT = 15;
   reg [$clog2(SWRW_WAIT+1)-1:0]           swrw_count;
   reg [$clog2(SWRW_WAIT+1)-1:0]           nxt_swrw_count;

   localparam STATE_IDLE = 0;
   localparam STATE_FLG_C_RCVD = 1;
   localparam STATE_WAIT_FLG_D = 2;
   localparam STATE_READ = 3;
   localparam STATE_READ_DRAIN_1 = 4;
   localparam STATE_READ_DRAIN_2 = 5;
   localparam STATE_READ_DRAIN_3 = 6;
   localparam STATE_READ_DRAIN_4 = 7;
   localparam STATE_READ_DRAIN_5 = 8;
   localparam STATE_READ_DRAIN_6 = 9;
   localparam STATE_FLG_A_RCVD = 10;
   localparam STATE_WAIT_FLG_B = 11;
   localparam STATE_WRITE = 12;
   localparam STATE_WRITE_DRAIN_1 = 13;
   localparam STATE_WRITE_DRAIN_2 = 14;
   localparam STATE_WRITE_FLUSH = 15;
   localparam STATE_SW_READ = 16;
   localparam STATE_SW_WRITE = 17;
   localparam STATE_SW_WRITE_WAITFLG = 18;

   reg [4:0]                               state;
   reg [4:0]                               nxt_state;

   reg                                     flush;
   reg                                     nxt_flush;

   /*
   // Debugging aid: count the number of received words. To be used as trigger
   // signal when the host software tells us "failed verification at word XY".
   (* mark_debug = "yes" *) reg [31:0] dbg_word_cnt;
   always @(posedge clk_io) begin
      if (int_rst) begin
         dbg_word_cnt <= 0;
      end else begin
         if (ingress_cdc_wr_en) begin
            dbg_word_cnt <= dbg_word_cnt + 1;
         end
      end
   end
   */

   always @(posedge clk_io) begin
      if (int_rst) begin
         state <= STATE_IDLE;
         idle_counter <= 0;
         flush <= 1;
         swrw_count <= 0;
      end else begin
         state <= nxt_state;
         idle_counter <= nxt_idle_counter;
         flush <= nxt_flush;
         swrw_count <= nxt_swrw_count;
      end
   end

   always @(*) begin
      nxt_state = state;

      if (idle_counter > 0) begin
         nxt_idle_counter = idle_counter - 1;
      end else begin
         nxt_idle_counter = 0;
      end

      if (swrw_count > 0) begin
         nxt_swrw_count = swrw_count - 1;
      end else begin
         nxt_swrw_count = 0;
      end

      if (!flush && (idle_counter == 1)) begin
         nxt_flush = 1;
      end else begin
         nxt_flush = flush;
      end

      oe = 0;
      rd = 0;
      wr = 0;
      pktend = 0;
      fifoadr = 2'bxx;

      int_fifo_out_ready = 0;

      case(state)
         STATE_IDLE: begin
            fifoadr = FX3_EPIN;
            // If outgoing CDC FIFO contains data and FX3 FIFO on path to host
            // has enough space -> write data to FX3
            if (!fx3_in_full && !int_fifo_out_empty) begin
               nxt_state = STATE_FLG_A_RCVD;
               // Send zero-length packet after time-out to flush fx3_in_fifo
            end else if (flush) begin
               fifoadr = FX3_EPIN;
               nxt_state = STATE_WRITE_FLUSH;
               // If FX3 FIFO on path from host contains data and incoming CDC
               // FIFO has enough space -> read data from FX3
            end else if (!fx3_out_empty && !int_fifo_in_almost_full) begin
               // We can read from FX3 if data is available and we can
               // receive data. We have to use the almost full
               // signal as we need to be capable of reading 3 words (1
               // delay of latching, 2 delay of interface, see fig 12)
               nxt_state = STATE_FLG_C_RCVD;
            end
         end

         STATE_FLG_A_RCVD: begin
            fifoadr = FX3_EPIN;
            nxt_idle_counter = 0;
            nxt_flush = 0;
            nxt_swrw_count = SWRW_WAIT;
            nxt_state = STATE_WAIT_FLG_B;
         end

         STATE_WAIT_FLG_B: begin
            fifoadr = FX3_EPIN;
            if (fx3_in_full) begin
               nxt_idle_counter = FORCE_SEND_TIMEOUT;
               nxt_state = STATE_IDLE;
            end else if (!fx3_in_almost_full) begin
               nxt_state = STATE_WRITE;
            end else if (swrw_count == 1) begin
               nxt_state = STATE_SW_WRITE;
            end
         end

         STATE_WRITE: begin
            fifoadr = FX3_EPIN;
            wr = 1;
            int_fifo_out_ready = 1;
            if (int_fifo_out_empty) begin
               wr = 0;
               int_fifo_out_ready = 0;
               nxt_idle_counter = FORCE_SEND_TIMEOUT;
               nxt_state = STATE_IDLE;
            end else if (fx3_in_almost_full) begin
               nxt_state = STATE_WRITE_DRAIN_1;
            end
         end

         STATE_SW_WRITE: begin
            fifoadr = FX3_EPIN;
            wr = 1;
            int_fifo_out_ready = 1;
            nxt_swrw_count = 3;
            nxt_state = STATE_SW_WRITE_WAITFLG;
         end

         STATE_SW_WRITE_WAITFLG: begin
            fifoadr = FX3_EPIN;
            if (nxt_swrw_count == 1) begin
               nxt_idle_counter = FORCE_SEND_TIMEOUT;
               nxt_state = STATE_IDLE;
            end
         end

         STATE_WRITE_DRAIN_1: begin
            fifoadr = FX3_EPIN;
            wr = 1;
            int_fifo_out_ready = 1;
            nxt_state = STATE_WRITE_DRAIN_2;
         end

         STATE_WRITE_DRAIN_2: begin
            fifoadr = FX3_EPIN;
            wr = 1;
            int_fifo_out_ready = 1;
            nxt_idle_counter = FORCE_SEND_TIMEOUT;
            nxt_state = STATE_IDLE;
         end

         STATE_WRITE_FLUSH: begin
            fifoadr = FX3_EPIN;
            nxt_flush = 0;
            nxt_state = STATE_IDLE;
            if (!fx3_in_full) begin
               pktend = 1;
            end
         end

         STATE_FLG_C_RCVD: begin
            fifoadr = FX3_EPOUT;
            nxt_swrw_count = SWRW_WAIT;
            nxt_state = STATE_WAIT_FLG_D;
         end

         STATE_WAIT_FLG_D: begin
            fifoadr = FX3_EPOUT;
            if (!fx3_out_almost_empty) begin
               nxt_state = STATE_READ;
            end else if (swrw_count == 1) begin
               nxt_state = STATE_SW_READ;
            end
         end

         STATE_READ: begin
            fifoadr = FX3_EPOUT;
            rd = 1;
            oe = 1;
            if (int_fifo_in_almost_full) begin
               nxt_state = STATE_READ_DRAIN_4;
            end else if (fx3_out_almost_empty) begin
               nxt_state = STATE_READ_DRAIN_1;
            end
         end

         STATE_SW_READ: begin
            fifoadr = FX3_EPOUT;
            rd = 1;
            oe = 1;
            nxt_state = STATE_READ_DRAIN_4;
         end

         STATE_READ_DRAIN_1: begin
            oe = 1;
            rd = 1;
            fifoadr = FX3_EPOUT;
            nxt_state = STATE_READ_DRAIN_2;
         end

         STATE_READ_DRAIN_2: begin
            oe = 1;
            rd = 1;
            fifoadr = FX3_EPOUT;
            nxt_state = STATE_READ_DRAIN_3;
         end

         STATE_READ_DRAIN_3: begin
            oe = 1;
            rd = 1;
            fifoadr = FX3_EPOUT;
            nxt_state = STATE_READ_DRAIN_4;
         end

         STATE_READ_DRAIN_4: begin
            oe = 1;
            fifoadr = FX3_EPOUT;
            nxt_state = STATE_READ_DRAIN_5;
         end

         STATE_READ_DRAIN_5: begin
            oe = 1;
            fifoadr = FX3_EPOUT;
            nxt_state = STATE_READ_DRAIN_6;
         end

         // Only necessary for high clock speed.
         STATE_READ_DRAIN_6: begin
            oe = 1;
            fifoadr = FX3_EPOUT;
            nxt_state = STATE_IDLE;
         end
      endcase
   end

   //------------------- ingress data path (host -> FPGA) ---------------------//

   // Connect FX3 -> ingress_cdc
   assign ingress_cdc_wr_data = fx3_dq_in;
   assign ingress_cdc_wr_en = int_fifo_in_valid & ~fx3_out_empty;

   wire ingress_cdc_wr_full;

   cdc_fifo
      #(.DW(WIDTH))
   u_ingress_cdc(
      // write side (clk_io)
      .wr_clk           (clk_io),
      .wr_rst           (~int_rst),
      .wr_full          (ingress_cdc_wr_full),
      .wr_data          (ingress_cdc_wr_data),
      .wr_en            (ingress_cdc_wr_en),

      // read side (clk)
      .rd_clk           (clk),
      .rd_rst           (~int_rst),
      .rd_empty         (ingress_cdc_rd_empty),
      .rd_data          (ingress_cdc_rd_data),
      .rd_en            (ingress_cdc_rd_en));

   // Connect ingress_cdc -> ingress_fx3_buffer.
   assign ingress_fx3_buffer_din = ingress_cdc_rd_data;
   assign ingress_fx3_buffer_wr_en = ~ingress_cdc_rd_empty;
   assign ingress_cdc_rd_en = ~ingress_fx3_buffer_full;

   // Stop read when ingress buffer is full.
   // This is necessary since the cdc fifo has no 'almost_full' signal and the
   // FX3 sends two more words after reading 'stops'.
   assign int_fifo_in_almost_full = ingress_fx3_buffer_full;

   fifo_sync_fwft
      #(.WIDTH(WIDTH),
      .DEPTH(BUFFER_DEPTH))
   u_ingress_fx3_buffer(
      .clk        (clk),
      .rst        (int_rst),

      .din        (ingress_fx3_buffer_din),
      .wr_en      (ingress_fx3_buffer_wr_en),
      .full       (ingress_fx3_buffer_full),
      .prog_full  (),

      .dout       (ingress_fx3_buffer_dout),
      .empty      (ingress_fx3_buffer_empty),
      .rd_en      (ingress_fx3_buffer_rd_en));

   // Connect ingress_fx3_buffer -> output interface (to be used in attached
   // logic)
   assign fifo_in_data = ingress_fx3_buffer_dout;
   assign fifo_in_valid = ~ingress_fx3_buffer_empty;
   assign ingress_fx3_buffer_rd_en = fifo_in_ready;

   //-------------------------- egress data path --------------------------//

   // Connect input interface -> egress_fx3_buffer
   assign egress_fx3_buffer_din = fifo_out_data;
   assign egress_fx3_buffer_wr_en = fifo_out_valid;
   assign fifo_out_ready = ~egress_fx3_buffer_full;

   fifo_sync_fwft
      #(.WIDTH(WIDTH),
      .DEPTH(BUFFER_DEPTH))
   u_egress_fx3_buffer(
      .clk   (clk),
      .rst   (int_rst),

      .din   (egress_fx3_buffer_din),
      .wr_en (egress_fx3_buffer_wr_en),
      .full  (egress_fx3_buffer_full),
      .prog_full(),

      .dout  (egress_fx3_buffer_dout),
      .empty (egress_fx3_buffer_empty),
      .rd_en (egress_fx3_buffer_rd_en));

   // Connect egress_fx3_buffer -> egress_cdc.
   assign egress_cdc_wr_data = egress_fx3_buffer_dout;
   assign egress_cdc_wr_en = ~egress_fx3_buffer_empty;
   assign egress_fx3_buffer_rd_en = ~egress_cdc_wr_full;

   cdc_fifo
      #(.DW(WIDTH))
   u_egress_cdc(
      .wr_clk           (clk),
      .wr_rst           (~int_rst),
      .wr_full          (egress_cdc_wr_full),
      .wr_data          (egress_cdc_wr_data),
      .wr_en            (egress_cdc_wr_en),

      .rd_clk           (clk_io),
      .rd_rst           (~int_rst),
      .rd_empty         (egress_cdc_rd_empty),
      .rd_data          (egress_cdc_rd_data),
      .rd_en            (egress_cdc_rd_en));

   // Connect egress_cdc -> FX3
   assign fx3_dq_out = egress_cdc_rd_data;
   assign int_fifo_out_empty = egress_cdc_rd_empty;
   assign egress_cdc_rd_en = int_fifo_out_ready;

endmodule
