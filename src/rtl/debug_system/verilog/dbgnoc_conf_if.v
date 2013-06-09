/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Debug NoC Configuration interface
 *
 * This module can be used as submodule inside a debug module for
 * writing/reading configuration registers over the Debug NoC.
 *
 * Burst read and write operations are supported, the maximum burst size is 4
 * 16-bit words.
 *
 * TODO: Validate read and write register addresses.
 * TODO: Add possibility to mark certain registers read-only
 * TODO: Add atomic writes for bursts.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, mail@philipp-wagner.com
 */

`include "dbg_config.vh"

`include "lisnoc_def.vh"
`include "lisnoc16_def.vh"

module dbgnoc_conf_if(/*AUTOARG*/
   // Outputs
   dbgnoc_out_flit, dbgnoc_out_valid, dbgnoc_in_ready, dbgnoc_out_rts,
   conf_mem_flat_out, conf_mem_flat_in_ack,
   // Inputs
   clk, rst, dbgnoc_out_ready, dbgnoc_in_flit, dbgnoc_in_valid,
   conf_mem_flat_in, conf_mem_flat_in_valid
   );

   // parameters for the Debug NoC interface
   parameter DBG_NOC_DATA_WIDTH = `FLIT16_CONTENT_WIDTH;
   parameter DBG_NOC_FLIT_TYPE_WIDTH = `FLIT16_TYPE_WIDTH;
   localparam DBG_NOC_FLIT_WIDTH = DBG_NOC_DATA_WIDTH + DBG_NOC_FLIT_TYPE_WIDTH;
   parameter DBG_NOC_PH_DEST_WIDTH = `FLIT16_DEST_WIDTH;
   parameter DBG_NOC_PH_CLASS_WIDTH = `PACKET16_CLASS_WIDTH;
   localparam DBG_NOC_PH_ID_WIDTH = DBG_NOC_DATA_WIDTH - DBG_NOC_PH_DEST_WIDTH - DBG_NOC_PH_CLASS_WIDTH;
   parameter DBG_NOC_VCHANNELS = `DBG_NOC_VCHANNELS;

   // memory configuration
   // number of 16 bit-sized configuration registers
   parameter MEM_SIZE = 6;
   // initialize configuration registers to 0 on reset
   // set to 0 if you want to initialize the memory yourself on reset
   parameter MEM_INIT_ZERO = 1;

   // FSM states
   localparam STATE_WIDTH = 3;
   localparam STATE_WAIT_FOR_REQUEST = 0;
   localparam STATE_READ_REQUEST = 1;
   localparam STATE_REPLY_REG_READ_HEADER = 2;
   localparam STATE_REPLY_REG_READ_PAYLOAD = 3;
   localparam STATE_DO_REG_WRITE = 4;

   input clk;
   input rst;

   // Debug NoC interface (IN = NoC -> TCM; OUT = TCM -> NoC)
   output reg [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_out_flit;
   output reg [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_valid;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_out_ready;
   input [DBG_NOC_FLIT_WIDTH-1:0] dbgnoc_in_flit;
   input [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_valid;
   output reg [DBG_NOC_VCHANNELS-1:0] dbgnoc_in_ready;

   // we are ready to send on the output port; waiting for dbgnoc_out_ready
   output reg dbgnoc_out_rts;

   // FSM
   reg [STATE_WIDTH-1:0] fsm_state;
   reg [STATE_WIDTH-1:0] fsm_state_next;

   reg [DBG_NOC_FLIT_WIDTH-1:0] sampled_flit;
   reg sampled_flit_valid;

   reg [5:0] reg_rdwr_addr;
   reg [1:0] reg_read_burst_length;

   reg burst_goto_next;
   reg do_sample_flit;
   reg init_data_regs;
   reg write_flit_into_conf_mem;

   // configuration registers (16 bit wide)
   reg [15:0] conf_mem [MEM_SIZE-1:0];

   output [MEM_SIZE*16-1:0] conf_mem_flat_out;
   generate
      genvar i;
      for (i = 0; i < MEM_SIZE; i = i + 1) begin : gen_conf_mem_flat_out
         assign conf_mem_flat_out[((i+1)*16)-1 : i*16] = conf_mem[i][15:0];
      end
   endgenerate

   input [MEM_SIZE*16-1:0] conf_mem_flat_in;
   input [MEM_SIZE-1:0] conf_mem_flat_in_valid;
   output reg conf_mem_flat_in_ack;

   integer k;

   // init configuration registers to 0 on reset
   generate
      genvar j;
      for (j=0; j<MEM_SIZE; j=j+1) begin : reset_conf_mem
         always @ (posedge clk) begin
            if (MEM_INIT_ZERO && rst) begin
               conf_mem[j] <= 16'h0000;
            end
         end
      end
   endgenerate

   always @ (posedge clk) begin
      if (rst) begin
         fsm_state <= STATE_WAIT_FOR_REQUEST;

         reg_rdwr_addr <= 0;
         reg_read_burst_length <= 0;

         // Debug NoC link
         sampled_flit <= 18'bx;
         sampled_flit_valid <= 0;
      end else if (!dbgnoc_out_rts || dbgnoc_out_ready) begin
         // stall FSM while waiting for CTS
         fsm_state <= fsm_state_next;

         if (burst_goto_next) begin
            reg_rdwr_addr <= reg_rdwr_addr + 1;
            reg_read_burst_length <= reg_read_burst_length - 1;
         end

         if (dbgnoc_in_valid && do_sample_flit) begin
            sampled_flit_valid <= 1;
            sampled_flit <= dbgnoc_in_flit;
         end else begin
            sampled_flit_valid <= 0;
            sampled_flit <= 18'bx;
         end

         if (dbgnoc_in_valid && init_data_regs) begin
            reg_rdwr_addr <= dbgnoc_in_flit[7:2];
            reg_read_burst_length <= dbgnoc_in_flit[1:0];
         end

         if (write_flit_into_conf_mem) begin
            conf_mem[reg_rdwr_addr] <= sampled_flit[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB];
            conf_mem_flat_in_ack <= 0;
         end else if (conf_mem_flat_in_valid) begin
            for (k = 0; k < MEM_SIZE; k = k + 1) begin
               if (conf_mem_flat_in_valid[k] == 1'b1) begin
                  // NOTE: requires Verilog-2001! For older versions, rewrite as loop.
                  conf_mem[k] <= conf_mem_flat_in[k*16 +: 16];
               end
            end
            conf_mem_flat_in_ack <= 1;
         end else begin
            conf_mem_flat_in_ack <= 0;
         end
      end
   end

   // FSM next state logic: package compressed trace data into individual flits
   always @ (*) begin : fsm
      // default values
      dbgnoc_out_flit = 18'b0;
      dbgnoc_in_ready = 1'b0;
      dbgnoc_out_valid = 1'b0;
      dbgnoc_out_rts = 1'b0;
      burst_goto_next = 1'b0;
      do_sample_flit = 1'b0;
      init_data_regs = 0;
      fsm_state_next = fsm_state;
      write_flit_into_conf_mem = 0;

      case (fsm_state)
         STATE_WAIT_FOR_REQUEST: begin
            dbgnoc_in_ready = 1'b1;
            dbgnoc_out_valid = 1'b0;

            if (dbgnoc_in_valid) begin
               fsm_state_next = STATE_READ_REQUEST;
               do_sample_flit = 1;
               init_data_regs = 1;
            end else begin
               fsm_state_next = STATE_WAIT_FOR_REQUEST;
               do_sample_flit = 0;
               init_data_regs = 0;
            end
         end

         STATE_READ_REQUEST: begin
            dbgnoc_in_ready = 1'b0;
            dbgnoc_out_valid = 1'b0;

            if (sampled_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_SINGLE &&
                sampled_flit[`PACKET16_CLASS_MSB:`PACKET16_CLASS_LSB] == `DBG_NOC_CLASS_REG_READ_REQ) begin
               // we got a read request

               // TODO: validate the register address and burst length

               // send register read header
               dbgnoc_out_rts = 1;
               fsm_state_next = STATE_REPLY_REG_READ_HEADER;

            end else if (sampled_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_HEADER &&
                         sampled_flit[`PACKET16_CLASS_MSB:`PACKET16_CLASS_LSB] == `DBG_NOC_CLASS_REG_WRITE_REQ) begin
               // we got a write request

               // TODO: validate the register address
               fsm_state_next = STATE_DO_REG_WRITE;
               // continue to sample following flits (at least one coming up)
               do_sample_flit = 1;
               dbgnoc_in_ready = 1'b1;
            end else begin
               fsm_state_next = STATE_WAIT_FOR_REQUEST;
            end
         end

         STATE_REPLY_REG_READ_HEADER: begin
            dbgnoc_out_flit = {
               `FLIT16_TYPE_HEADER,           // type: header
               `DBG_NOC_ADDR_EXTERNALIF,      // destination: external interface
               `DBG_NOC_CLASS_REG_READ_RESP,  // class: 0x01
               reg_rdwr_addr, 2'b00};         // payload
            dbgnoc_out_valid = 1;

            dbgnoc_out_rts = 1;
            fsm_state_next = STATE_REPLY_REG_READ_PAYLOAD;
         end

         STATE_REPLY_REG_READ_PAYLOAD: begin
            if (reg_read_burst_length == 0) begin
               dbgnoc_out_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_LAST;

               dbgnoc_out_rts = 0;
               fsm_state_next = STATE_WAIT_FOR_REQUEST;
            end else begin
               dbgnoc_out_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] = `FLIT16_TYPE_PAYLOAD;

               dbgnoc_out_rts = 1;
               fsm_state_next = STATE_REPLY_REG_READ_PAYLOAD;
            end

            if (reg_rdwr_addr >= MEM_SIZE) begin
               // simply send zero data if the register address is not valid
               // TODO: Inform the sender about this problem.
               dbgnoc_out_flit[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB] = 0;
               $warning("Attempting to read from out-of-bounds register address 0x%x (valid range: [0x00; 0x%02x]).",
                        reg_rdwr_addr, MEM_SIZE-1);
            end else begin
               dbgnoc_out_flit[`FLIT16_CONTENT_MSB:`FLIT16_CONTENT_LSB] = conf_mem[reg_rdwr_addr];
            end
            dbgnoc_out_valid = 1;

            burst_goto_next = 1;
         end

         STATE_DO_REG_WRITE: begin
            dbgnoc_out_valid = 0;
            dbgnoc_in_ready = 1;

            if (sampled_flit_valid) begin
               write_flit_into_conf_mem = 1;
            end

            if (!sampled_flit_valid) begin
               fsm_state_next = STATE_DO_REG_WRITE;
               burst_goto_next = 0;
               do_sample_flit = 1;
            end else if (sampled_flit[`FLIT16_TYPE_MSB:`FLIT16_TYPE_LSB] == `FLIT16_TYPE_LAST) begin
               fsm_state_next = STATE_WAIT_FOR_REQUEST;
               burst_goto_next = 0;
            end else begin
               fsm_state_next = STATE_DO_REG_WRITE;
               burst_goto_next = 1;
               do_sample_flit = 1;
            end
         end
      endcase
   end

endmodule
