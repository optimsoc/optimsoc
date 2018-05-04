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
 * GLIP Toplevel Interface for the JTAG backend
 *
 * Author(s):
 *   Alexandra Weber <sandra.eli.weber@tum.de>
 *   Jan Alexander Wessel <jan.wessel@tum.de>
 */
module glip_jtag_toplevel(/*AUTOARG*/
   // Outputs
   com_rst, tdo, fifo_out_ready, fifo_in_data, fifo_in_valid,
   ctrl_logic_rst,
   // Inputs
   clk, rst, tck, tms, tdi, fifo_out_data, fifo_out_valid,
   fifo_in_ready
   );

   // Width of one word
   parameter WORD_WIDTH = 16;
   // size of sync buffer
   parameter NUM_WORDS = 3;

   // These are extra words for the valid signal and the number of free places
   // in the input FIFO
   localparam SHIFT_EXTRA_WORDS = 2;


   localparam NUM_WORDS_SHIFT = NUM_WORDS + SHIFT_EXTRA_WORDS;

   // This parameter determines the depth of the CDC FIFOs
   localparam CDC_ADDRSIZE = 4;

   // Clock/Reset
   input         clk;
   input         rst;
   output        com_rst;

   // JTAG Interface
   input tck;
   input tms;
   input tdi;
   output reg tdo;

   // GLIP FIFO Interface
   input [WORD_WIDTH-1:0]  fifo_out_data;
   input                   fifo_out_valid;
   output                  fifo_out_ready;
   output [WORD_WIDTH-1:0] fifo_in_data;
   output                  fifo_in_valid;
   input                   fifo_in_ready;

   // GLIP Control Interface
   output        ctrl_logic_rst;


   wire jtag_reset; // reset signal from the JTAG state machine
   wire int_rst;
   assign int_rst = jtag_reset | rst;
   assign com_rst = int_rst;


   wire tdi_o;
   wire debug_select_o;
   wire shift_dr_o;
   wire pause_dr_o;
   wire update_dr_o;
   wire capture_dr_o;

   wire error;
   wire [WORD_WIDTH-1:0] acc_cnt;

   wire [WORD_WIDTH-1:0] out_data;
   wire out_fifo_empty;
   wire out_valid;
   assign out_valid = ~out_fifo_empty;
   wire out_ready;
   wire out_fifo_full;
   assign fifo_out_ready = ~out_fifo_full;

   wire [WORD_WIDTH-1:0] in_data;
   wire in_fifo_empty;
   assign fifo_in_valid = ~in_fifo_empty;

   // config discovery
   wire data_transfer;
   wire config_discovery;
   wire config_discovery_shift;

   assign config_discovery_shift = (shift_dr_o & debug_select_o);

   // TAP signals for data transfer
   wire data_transfer_shift;
   wire data_transfer_update;
   wire data_transfer_capture;
   assign data_transfer_shift = (shift_dr_o & debug_select_o);
   assign data_transfer_update = (update_dr_o & debug_select_o);
   assign data_transfer_capture = (capture_dr_o & debug_select_o);

   wire data_transfer_tdo;
   wire tap_tdo;
   wire config_discovery_tdo;

   always @ (*) begin
      if (config_discovery) begin
           tdo = config_discovery_tdo;
      end else if (data_transfer_shift) begin
           tdo = data_transfer_tdo;
      end else begin
           tdo = tap_tdo;
      end
   end

   // JTAG TAP
   glip_jtag_tap
      u_tap(
         // JTAG pins
         .tms_pad_i               (tms),
         .tck_pad_i               (tck),
         .trst_pad_i              (0),
         .tdi_pad_i               (tdi),
         .tdo_pad_o               (tap_tdo),
         .tdo_padoe_o             (),
         // TAP states
         .shift_dr_o              (shift_dr_o),
         .pause_dr_o              (pause_dr_o),
         .update_dr_o             (update_dr_o),
         .capture_dr_o            (capture_dr_o),
         // Select signals for boundary scan or mbist
         .extest_select_o         (),
         .sample_preload_select_o (),
         .mbist_select_o          (),
         .debug_select_o          (debug_select_o),
         // TDO signal that is connected to TDI of sub-modules.
         .tdo_o                   (tdi_o),
         // TDI signals from sub-modules
         .debug_tdi_i             (tdi_o),
         .bs_chain_tdi_i          (tdi_o),
         .mbist_tdi_i             (tdi_o));

   wire in_fifo_full;
   wire in_valid;

   fifo_dualclock_fwft #(
      .WIDTH(WORD_WIDTH),
      .DEPTH(2 ** CDC_ADDRSIZE))
   in_fifo (
      // write side
      .wr_clk     (tck),
      .wr_rst     (int_rst),
      .din        (in_data),
      .wr_en      (in_valid),
      .full       (in_fifo_full),
      .prog_full  (),

      // read side
      .rd_clk     (clk),
      .rd_rst     (int_rst),
      .dout       (fifo_in_data),
      .rd_en      (fifo_in_ready),
      .empty      (in_fifo_empty),
      .prog_empty ());

   glip_jtag_input_fsm
     #(.WORD_WIDTH(WORD_WIDTH),
       .NUM_WORDS(NUM_WORDS))
      u_input_fsm(
         // Output ports
         .fifo_data          (in_data),
         .fifo_valid         (in_valid),
         .error              (error),
         .jtag_reset         (jtag_reset),
         .acc_cnt            (acc_cnt),
         // Input ports
         .clk                (tck),
         .rst                (int_rst),
         .tdi                (tdi),
         .shift              (data_transfer_shift),
         .update             (data_transfer_update),
         .capture            (data_transfer_capture),
         .fifo_ready         (~in_fifo_full));

   glip_jtag_config_discovery_fsm
      #(.WORD_WIDTH(WORD_WIDTH),
        .NUM_WORDS(NUM_WORDS))
      u_conf_discovery_fsm(
         // Output ports
         .data_transfer            (data_transfer),
         .config_discovery         (config_discovery),
         .config_discovery_tdo     (config_discovery_tdo),
         .ctrl_logic_rst           (ctrl_logic_rst),
         // Input ports
         .clk                      (tck),
         .rst                      (int_rst),
         .shift                    (config_discovery_shift),
         .update                   (update_dr_o),
         .config_discovery_tdi     (tdi));

   glip_jtag_output_fsm
     #(.WORD_WIDTH(WORD_WIDTH),
       .NUM_WORDS_SHIFT(NUM_WORDS_SHIFT))
      u_output_fsm(
         // Output ports
         .tdo            (data_transfer_tdo),
         .fifo_ready     (out_ready),
         // Input ports
         .clk            (tck),
         .rst            (int_rst),
         .tdi            (tdi),
         .shift          (data_transfer_shift),
         .update         (data_transfer_update),
         .in_error       (error),
         .in_written     (acc_cnt),
         .fifo_data      (out_data),
         .fifo_valid     (out_valid));

   fifo_dualclock_fwft #(
      .WIDTH(WORD_WIDTH),
      .DEPTH(2 ** CDC_ADDRSIZE))
   out_fifo (
      // write side
      .wr_clk     (clk),
      .wr_rst     (int_rst),
      .din        (fifo_out_data),
      .wr_en      (fifo_out_valid),
      .full       (out_fifo_full),
      .prog_full  (),

      // read side
      .rd_clk     (tck),
      .rd_rst     (int_rst),
      .dout       (out_data),
      .rd_en      (out_ready),
      .empty      (out_fifo_empty),
      .prog_empty ());

endmodule
