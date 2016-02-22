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
 * Memory Access Module (MAM)
 *
 * This module gives write access to a memory through the Debug NoC.
 *
 * All communication uses the configuration virtual channel
 * (DBG_NOC_CONF_VCHANNEL).
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module mam(
`ifdef OPTIMSOC_CLOCKDOMAINS
           clk_cdc,
`endif
           /*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready,
   sys_clk_disable, wb_adr_o, wb_bte_o, wb_cti_o, wb_cyc_o, wb_dat_o,
   wb_sel_o, wb_stb_o, wb_cab_o, wb_we_o,
   // Inputs
   clk, rst, dbgnoc_out_ready, dbgnoc_in_flit, dbgnoc_in_valid,
   sys_clk_is_halted, wb_ack_i, wb_err_i, wb_rty_i, wb_dat_i
   );

   // Debug NoC
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;

   parameter DBG_NOC_VCHANNELS = 1;
   parameter DBG_NOC_TRACE_VCHANNEL = 0;
   parameter DBG_NOC_CONF_VCHANNEL = 0;

   // size of the configuration memory (16 bit words)
   localparam CONF_MEM_SIZE = 2;

   parameter MEMORY_ID = 'hx;

   localparam MODULE_TYPE_MAM = 8'h07;
   localparam MODULE_VERSION_MAM = 8'h00;

   // NoC FSM states
   localparam STATE_NOC_WIDTH = 3; // FSM with 2^STATE_NOC_WIDTH = 8 states
   localparam STATE_NOC_READ_HEADER = 0;
   localparam STATE_NOC_CONF_SEND = 1;
   localparam STATE_NOC_READ_ADDR_MSB = 2;
   localparam STATE_NOC_READ_ADDR_LSB = 3;
   localparam STATE_NOC_READ_DATA_MSB = 4;
   localparam STATE_NOC_READ_DATA_MSB_CONSECUTIVE = 5;
   localparam STATE_NOC_READ_DATA_LSB = 6;
   // note: increase STATE_NOC_WIDTH if you have more than 8 FSM states!

   // Wishbone FSM states
   localparam STATE_WB_WIDTH = 2; // FSM with 2^STATE_WB_WIDTH = 4 states
   localparam STATE_WB_IDLE = 0;
   localparam STATE_WB_SETUP = 1;
   localparam STATE_WB_WAIT_FOR_ACK = 2;
   // note: increase STATE_WB_WIDTH if you have more than 4 FSM states!

   input clk;
   input rst;
`ifdef OPTIMSOC_CLOCKDOMAINS
   input clk_cdc;
`endif

   // Debug NoC interface
   output [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // system control interface
   output reg sys_clk_disable;
   input  sys_clk_is_halted;

   // Wishbone MASTER Memory Interface
   output reg [31:0] wb_adr_o;
   output [1:0]      wb_bte_o;
   output [2:0]      wb_cti_o;
   output reg        wb_cyc_o;
   output reg [31:0] wb_dat_o;
   output reg [3:0]  wb_sel_o;
   output reg        wb_stb_o;
   output            wb_cab_o;
   output reg        wb_we_o;

   input             wb_ack_i;
   input             wb_err_i;
   input             wb_rty_i;
   input [31:0]      wb_dat_i;

   // Debug NoC sending interface multiplexing
   // The configuration interface requests to send via the dbgnoc_conf_out_rts
   // signal, this module grants the request by setting dbgnoc_conf_out_cts.
   // The configuration interface gets the dbgnoc_conf_out_ready signal as
   // dbgnoc_out_ready.
   wire dbgnoc_conf_out_rts;
   reg dbgnoc_conf_out_cts;
   wire dbgnoc_conf_out_ready;

   wire dbgnoc_conf_out_valid;
   wire dbgnoc_data_out_valid;

   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_conf_out_flit;
   wire [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_data_out_flit;

   wire dbgnoc_conf_in_valid;
   reg dbgnoc_dat_in_ready;
   wire dbgnoc_conf_in_ready;

   // configuration memory
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_in;
   reg [CONF_MEM_SIZE-1:0] conf_mem_flat_in_valid;
   wire conf_mem_flat_in_ack;
   wire [CONF_MEM_SIZE*16-1:0] conf_mem_flat_out;

   // un-flatten conf_mem_in to conf_mem_flat_in
   reg [15:0] conf_mem_in [CONF_MEM_SIZE-1:0];
   genvar i;
   generate
      for (i = 0; i < CONF_MEM_SIZE; i = i + 1) begin : gen_conf_mem_in
         assign conf_mem_flat_in[((i+1)*16)-1:i*16] = conf_mem_in[i];
      end
   endgenerate

   // NoC FSM states
   reg [STATE_NOC_WIDTH-1:0] fsm_noc_state;
   reg [STATE_NOC_WIDTH-1:0] fsm_noc_state_next;

   // Wishbone FSM states
   reg [STATE_WB_WIDTH-1:0] fsm_wb_state;
   reg [STATE_WB_WIDTH-1:0] fsm_wb_state_next;

   // Internal control signals used in NoC FSM
   reg reg_addr_msb;
   reg reg_addr_lsb;
   reg reg_data_msb;
   reg reg_data_lsb;
   reg inc_write_addr;

   // Signals for the FIFO between NoC and Wishbone FSM
   //
   // The data is a a single word to be written to memory together with
   // its address, structured like this:
   // ------------------
   // |  63:32  | 31:0 |
   // ------------------
   //   address   data
   reg [63:0] mem_write_fifo_in_data;
   reg mem_write_fifo_in_valid;
   wire mem_write_fifo_in_ready;
   reg mem_write_fifo_out_ready;
   wire [63:0] mem_write_fifo_out_data;
   wire mem_write_fifo_out_valid;


   // configuration interface
   /* dbgnoc_conf_if AUTO_TEMPLATE(
      .\(.*\)(\1), // suppress explict port widths
    ); */
   dbgnoc_conf_if
      #(.MEM_SIZE(CONF_MEM_SIZE),
        .MEM_INIT_ZERO(0))
      u_dbgnoc_conf_if(.dbgnoc_out_ready(dbgnoc_conf_out_ready),
                       .dbgnoc_out_rts  (dbgnoc_conf_out_rts),
                       .dbgnoc_out_valid(dbgnoc_conf_out_valid),
                       .dbgnoc_out_flit (dbgnoc_conf_out_flit[DBG_NOC_FLIT_WIDTH-1:0]),
                       .dbgnoc_in_valid (dbgnoc_conf_in_valid),
                       .dbgnoc_in_ready (dbgnoc_conf_in_ready),

                       /*AUTOINST*/
                       // Outputs
                       .conf_mem_flat_out(conf_mem_flat_out),    // Templated
                       .conf_mem_flat_in_ack(conf_mem_flat_in_ack), // Templated
                       // Inputs
                       .clk             (clk),                   // Templated
                       .rst             (rst),                   // Templated
                       .dbgnoc_in_flit  (dbgnoc_in_flit),        // Templated
                       .conf_mem_flat_in(conf_mem_flat_in),      // Templated
                       .conf_mem_flat_in_valid(conf_mem_flat_in_valid)); // Templated

   // FIFO bridge between NoC and Wishbone part of this module
   lisnoc_fifo
      #(.LENGTH(4),
        .flit_data_width(64),
        .flit_type_width(0))
      u_mem_write_fifo(.clk(clk),
                       .rst(rst),

                       .in_flit(mem_write_fifo_in_data),
                       .in_valid(mem_write_fifo_in_valid),
                       .in_ready(mem_write_fifo_in_ready),

                       .out_ready(mem_write_fifo_out_ready),
                       .out_flit(mem_write_fifo_out_data),
                       .out_valid(mem_write_fifo_out_valid));

   // Debug NoC multiplexing between configuration and main functionality
   assign dbgnoc_conf_out_ready = dbgnoc_conf_out_cts & dbgnoc_out_ready[DBG_NOC_CONF_VCHANNEL];
   assign dbgnoc_out_valid = {DBG_NOC_VCHANNELS{1'b0}} |
                             (dbgnoc_conf_out_valid << DBG_NOC_CONF_VCHANNEL) |
                             (dbgnoc_data_out_valid << DBG_NOC_CONF_VCHANNEL);

   assign dbgnoc_out_flit = (dbgnoc_conf_out_valid ? dbgnoc_conf_out_flit : dbgnoc_data_out_flit);
   assign dbgnoc_in_ready = {DBG_NOC_VCHANNELS{1'b0}} |
                            ((dbgnoc_dat_in_ready &
                              dbgnoc_conf_in_ready &
                              mem_write_fifo_in_ready)
                             << DBG_NOC_CONF_VCHANNEL);
   assign dbgnoc_conf_in_valid = dbgnoc_in_valid[DBG_NOC_CONF_VCHANNEL];

   // we currently don't send any data from the main module
   assign dbgnoc_data_out_valid = 1'b0;
   assign dbgnoc_data_out_flit = {DBG_NOC_FLIT_WIDTH{1'b0}};

   // Tie unused Wishbone output ports to GND
   assign wb_bte_o = 2'b0;
   assign wb_cti_o = 3'b0;
   assign wb_cab_o = 1'b0;

   // Synchronous part of the NoC FSM
   always @ (posedge clk) begin
      if (rst) begin
         // Initialize configuration memory
         // module description
         conf_mem_in[0] <= {MODULE_TYPE_MAM, MODULE_VERSION_MAM};
         conf_mem_in[1] <= MEMORY_ID;
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b1}};

         sys_clk_disable <= 0;
         fsm_noc_state <= STATE_NOC_READ_HEADER;
         mem_write_fifo_in_valid <= 0;
         mem_write_fifo_in_data <= 64'h0000_0000_0000_0000;
      end else begin
         conf_mem_flat_in_valid <= {CONF_MEM_SIZE{1'b0}};

         // XXX: Implement as shift register?
         if (reg_addr_msb) begin
            mem_write_fifo_in_data[63:48] <= dbgnoc_in_flit[15:0];
         end
         if (reg_addr_lsb) begin
            mem_write_fifo_in_data[47:32] <= dbgnoc_in_flit[15:0];
         end
         if (reg_data_msb) begin
            mem_write_fifo_in_data[31:16] <= dbgnoc_in_flit[15:0];
         end
         if (reg_data_lsb) begin
            mem_write_fifo_in_data[15:0] <= dbgnoc_in_flit[15:0];
            mem_write_fifo_in_valid <= 1;
         end else begin
            mem_write_fifo_in_valid <= 0;
         end

         if (inc_write_addr) begin
            mem_write_fifo_in_data[63:32] <= mem_write_fifo_in_data[63:32] + 4;
         end

         fsm_noc_state <= fsm_noc_state_next;
      end
   end

   // next-state and output logic for NoC FSM
   always @(*) begin
      reg_addr_msb = 1'b0;
      reg_addr_lsb = 1'b0;
      reg_data_msb = 1'b0;
      reg_data_lsb = 1'b0;
      dbgnoc_conf_out_cts = 1'b0;
      dbgnoc_dat_in_ready = 1'b1;
      inc_write_addr = 1'b0;
      fsm_noc_state_next = STATE_NOC_READ_HEADER;

      case (fsm_noc_state)
         STATE_NOC_READ_HEADER: begin

            if (~dbgnoc_in_valid && dbgnoc_conf_out_rts) begin
               // the config module wants to send; this cannot happen if
               // a valid flit is already on the input, as it would be discarded
               // if we let the module sent now
               fsm_noc_state_next = STATE_NOC_CONF_SEND;

            end else if (dbgnoc_in_valid &&
                         dbgnoc_in_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_HEADER &&
                         dbgnoc_in_flit[`PACKET16_CLASS_MSB:`PACKET16_CLASS_LSB] == 3'b111 &&
                         dbgnoc_in_flit[7:0] == 8'h0) begin
               // we found a header of a memory write packet
               fsm_noc_state_next = STATE_NOC_READ_ADDR_MSB;

            end else begin
               fsm_noc_state_next = STATE_NOC_READ_HEADER;
            end
         end

         STATE_NOC_CONF_SEND: begin
            // we cannot accept incoming data while the config module is sending
            dbgnoc_dat_in_ready = 1'b0;

            if (dbgnoc_conf_out_rts | dbgnoc_conf_out_valid) begin
               fsm_noc_state_next = STATE_NOC_CONF_SEND;
               dbgnoc_conf_out_cts = 1;
            end else begin
               fsm_noc_state_next = STATE_NOC_READ_HEADER;
               dbgnoc_conf_out_cts = 0;
            end
         end

         STATE_NOC_READ_ADDR_MSB: begin
            if (dbgnoc_in_valid) begin
               reg_addr_msb = 1'b1;
               fsm_noc_state_next = STATE_NOC_READ_ADDR_LSB;
            end else begin
               fsm_noc_state_next = STATE_NOC_READ_ADDR_MSB;
            end
         end

         STATE_NOC_READ_ADDR_LSB: begin
            if (dbgnoc_in_valid) begin
               reg_addr_lsb = 1'b1;
               fsm_noc_state_next = STATE_NOC_READ_DATA_MSB;
            end else begin
               fsm_noc_state_next = STATE_NOC_READ_ADDR_LSB;
            end
         end

         STATE_NOC_READ_DATA_MSB: begin
            if (dbgnoc_in_valid) begin
               reg_data_msb = 1'b1;
               fsm_noc_state_next = STATE_NOC_READ_DATA_LSB;
            end else begin
               fsm_noc_state_next = STATE_NOC_READ_DATA_MSB;
            end
         end

         STATE_NOC_READ_DATA_MSB_CONSECUTIVE: begin
            if (dbgnoc_in_valid) begin
               reg_data_msb = 1'b1;
               inc_write_addr = 1'b1;
               fsm_noc_state_next = STATE_NOC_READ_DATA_LSB;
            end else begin
               fsm_noc_state_next = STATE_NOC_READ_DATA_MSB_CONSECUTIVE;
            end
         end

         STATE_NOC_READ_DATA_LSB: begin
            if (dbgnoc_in_valid) begin
               reg_data_lsb = 1'b1;

               if (dbgnoc_in_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_LAST) begin
                  fsm_noc_state_next = STATE_NOC_READ_HEADER;
               end else begin
                  fsm_noc_state_next = STATE_NOC_READ_DATA_MSB_CONSECUTIVE;
               end
            end else begin
               fsm_noc_state_next = STATE_NOC_READ_DATA_LSB;
            end
         end
      endcase
   end

   // synchronous part of the Wishbone FSM
   always @(posedge clk) begin
      if (rst) begin
         fsm_wb_state <= STATE_WB_IDLE;
      end else begin
         fsm_wb_state <= fsm_wb_state_next;

         if (mem_write_fifo_out_ready) begin
            wb_adr_o <= mem_write_fifo_out_data[63:32];
            wb_dat_o <= mem_write_fifo_out_data[31:0];
         end
      end
   end

   // next-state and output logic for Wishbone FSM
   always @(*) begin
      mem_write_fifo_out_ready = 1'b0;
      fsm_wb_state_next = STATE_WB_IDLE;

      wb_sel_o = 4'b0000;
      wb_we_o = 1'b0;
      wb_stb_o = 1'b0;
      wb_cyc_o = 1'b0;

      case (fsm_wb_state)
         STATE_WB_IDLE: begin
            wb_sel_o = 4'b0000;
            wb_we_o = 1'b0;
            wb_stb_o = 1'b0;
            wb_cyc_o = 1'b0;

            if (mem_write_fifo_out_valid) begin
               mem_write_fifo_out_ready = 1'b1;
               fsm_wb_state_next = STATE_WB_SETUP;
            end else begin
               mem_write_fifo_out_ready = 1'b0;
               fsm_wb_state_next = STATE_WB_IDLE;
            end
         end

         STATE_WB_SETUP: begin
            wb_sel_o = 4'b1111;
            wb_we_o = 1'b1;
            wb_stb_o = 1'b1;
            wb_cyc_o = 1'b1;

            fsm_wb_state_next = STATE_WB_WAIT_FOR_ACK;
         end

         STATE_WB_WAIT_FOR_ACK: begin
            wb_sel_o = 4'b1111;
            wb_we_o = 1'b1;
            wb_stb_o = 1'b1;
            wb_cyc_o = 1'b1;

            if (wb_ack_i == 1'b1) begin
               fsm_wb_state_next = STATE_WB_IDLE;
            end else begin
               fsm_wb_state_next = STATE_WB_WAIT_FOR_ACK;
            end
         end
      endcase
   end

endmodule
