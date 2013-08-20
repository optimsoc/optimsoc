/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
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
 * Wishbone to MIG native interface wrapper. This wrapper acts as a
 * wishbone slave device and and be attached to one bidirectional
 * 32 bit port of the mig_39 module.
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Hans-Christian Wild, hans-christian.wild@mytum.de
 */

module wb_mig_if (/*AUTOARG*/
   // Outputs
   wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o, cmd_clk_o, cmd_en_o,
   cmd_instr_o, cmd_bl_o, cmd_byte_addr_o, wr_clk_o, wr_en_o,
   wr_mask_o, wr_data_o, rd_clk_o, rd_en_o,
   // Inputs
   wb_adr_i, wb_bte_i, wb_cti_i, wb_cyc_i, wb_dat_i, wb_sel_i,
   wb_stb_i, wb_we_i, wb_clk_i, wb_rst_i, cmd_empty_i, cmd_full_i,
   wr_full_i, wr_empty_i, wr_count_i, wr_underrun_i, wr_error_i,
   rd_data_i, rd_full_i, rd_empty_i, rd_count_i, rd_overflow_i,
   rd_error_i
   );

   parameter EN_BURSTS = 0; // do not activate yet
   parameter WR_BURST_W = 4; // dont care atm
   parameter RD_BURST_W = 4; // dont care atm

   // Wishbone connection
   output [31:0]   wb_dat_o;
   output          wb_ack_o;
   output          wb_err_o;
   output          wb_rty_o;
   input [31:0]    wb_adr_i;
   input [1:0]     wb_bte_i;
   input [2:0]     wb_cti_i;
   input           wb_cyc_i;
   input [31:0]    wb_dat_i;
   input [3:0]     wb_sel_i;
   input           wb_stb_i;
   input           wb_we_i;
   input           wb_clk_i;
   input           wb_rst_i;

   //Memory connection
   output          cmd_clk_o;
   output          cmd_en_o;
   output [2:0]    cmd_instr_o;
   output [5:0]    cmd_bl_o;
   output [29:0]   cmd_byte_addr_o;
   input           cmd_empty_i;
   input           cmd_full_i;
   output          wr_clk_o;
   output          wr_en_o;
   output [3:0]    wr_mask_o;
   output [31:0]   wr_data_o;
   input           wr_full_i;
   input           wr_empty_i;
   input [6:0]     wr_count_i;
   input           wr_underrun_i;
   input           wr_error_i;
   output          rd_clk_o;
   output          rd_en_o;
   input [31:0]    rd_data_i;
   input           rd_full_i;
   input           rd_empty_i;
   input [6:0]     rd_count_i;
   input           rd_overflow_i;
   input           rd_error_i;


   //Wires
   wire            clk;
   wire            rst;

   /*AUTOREG*/
   // Beginning of automatic regs (for this module's undeclared outputs)
   reg                  cmd_en_o;
   reg                  rd_en_o;
   reg                  wb_ack_o;
   reg                  wr_en_o;
   // End of automatics
   reg                  instr;

   // Clocks and Reset
   assign clk = wb_clk_i;
   assign cmd_clk_o = clk;
   assign rd_clk_o = clk;
   assign wr_clk_o = clk;

   assign rst = wb_rst_i;

   // WB to Memory & Memory to WB
   assign wr_data_o = wb_dat_i;
   assign wb_dat_o = rd_data_i;

   wire [27:0]          addr_i = wb_adr_i[29:2];

   reg [27:0] cmd_word_addr_s;
   always @(posedge wb_clk_i) begin
      if (wb_rst_i) begin
         cmd_word_addr_s <= 28'b0;
      end else begin
         cmd_word_addr_s <= addr_i;
      end
   end

   // MIG allows only word wise addressing
   assign cmd_byte_addr_o = {cmd_word_addr_s, 2'b0};

   assign wr_mask_o = ~wb_sel_i;
   assign cmd_bl_o = 6'b0;
   assign cmd_instr_o = {2'b0, instr};
   assign wb_err_o = rd_error_i | wr_error_i | wr_underrun_i | rd_overflow_i;
   assign wb_rty_o = 1'b0;

   // WB to internal

   assign wb_active = wb_cyc_i & wb_stb_i;

   //-------------------------------------------------------------------------------------
   // Burst data generation
   //
   // To allow burst, some additional data informations need to be generated and stored.
   // As we do not wait until all words have actually been cleared out of the FIFOs of
   // the MIG, we cannot use its "wr_count_i" output and must count the amount of words
   // that belong to the current burst our selves.
   //-------------------------------------------------------------------------------------

   reg [WR_BURST_W-1:0] burst_len;

   always @(posedge wb_clk_i) begin
      if (wb_rst_i) begin
         burst_len <= {WR_BURST_W{1'b1}};
      end else if(wb_cti_i == 3'b010 || wb_cti_i == 3'b111) begin
         burst_len <= burst_len + 1;
      end else if(cmd_en_o) begin
         burst_len <= {WR_BURST_W{1'b1}};
      end
   end

   //-------------------------------------------------------------------------------------
   // Main FSM
   //
   // This FSM generates all control signals to control the wishbone bus, the memory
   // interface as well as internal data.
   //-------------------------------------------------------------------------------------

   localparam IDLE = 6'h0;
   localparam READ = 6'h1;
   localparam READ_ACK = 6'h2;
   localparam READ_WAIT = 6'h3;
   localparam WRITE = 6'h4;
   localparam WRITE_CMD = 6'h5;
   localparam WRITE_BURST_START = 6'h6;
   localparam WRITE_CMD_EARLY = 6'h7;
   localparam WRITE_WAIT = 6'h8;

   reg [5:0] wb_cstate;
   reg [5:0] wb_nstate;

   always @(posedge wb_clk_i) begin
      if (rst) begin
         wb_cstate <= IDLE;
      end else begin
         wb_cstate <= wb_nstate;
      end
   end

   always @(/*AS*/burst_len or cmd_full_i or rd_empty_i or rst
            or wb_active or wb_cstate or wb_cti_i or wb_err_o
            or wb_we_i or wr_full_i) begin
      if (rst) begin
         wb_nstate <= IDLE;
      end else begin
         case (wb_cstate)
           IDLE, WRITE_CMD: begin
              if (wb_err_o) begin
                 wb_nstate <= IDLE;
              end else if (wb_active & ~cmd_full_i) begin
                 if (wb_we_i) begin
                    if (~wr_full_i) begin
                     //  if (!EN_BURSTS || wb_cti_i == 3'b000) begin
                          wb_nstate <= WRITE;
                     // // end else begin
                     //     wb_nstate <= WRITE_BURST_START;
                     //  end
                    end else begin
                       wb_nstate <= IDLE;
                    end
                 end else begin
                    wb_nstate <= READ;
                 end
              end else begin
                 wb_nstate <= IDLE;
              end
           end
           READ: begin
              wb_nstate <= READ_WAIT;
           end
           READ_WAIT: begin
              if (~rd_empty_i) begin
                 wb_nstate <= READ_ACK;
              end else begin
                 wb_nstate <= READ_WAIT;
              end
           end
           READ_ACK: begin
              wb_nstate <= IDLE;
           end
           WRITE, WRITE_CMD_EARLY: begin
              if (!EN_BURSTS || wb_cti_i == 3'b000 || wb_cti_i == 3'b111) begin
                 wb_nstate <= WRITE_CMD;
              end else if (wb_cti_i == 3'b010 && &burst_len) begin
                 wb_nstate <= WRITE_CMD_EARLY;
              end else begin
                 wb_nstate <= WRITE;
              end
           end
           WRITE_WAIT: begin
              if (!wr_full_i) begin
                 wb_nstate <= WRITE;
              end else begin
                 wb_nstate <= WRITE_WAIT;
              end
           end
           WRITE_BURST_START: begin
              wb_nstate <= WRITE;
           end
           default: begin
              wb_nstate <= IDLE;
           end
         endcase
      end
   end // always @ (posedge clk)

   always @(wb_cstate) begin
      case(wb_cstate)
        IDLE: begin
           rd_en_o <= 0;
           wr_en_o <= 0;
           cmd_en_o <= 0;
           wb_ack_o <= 0;
           instr <= 0;
        end
        READ: begin
           rd_en_o <= 0;
           wr_en_o <= 0;
           cmd_en_o <= 1;
           wb_ack_o <= 0;
           instr <= 1;
        end
        READ_WAIT: begin
           rd_en_o <= 0;
           wr_en_o <= 0;
           cmd_en_o <= 0;
           wb_ack_o <= 0;
           instr <= 1;
        end
        READ_ACK: begin
           rd_en_o <= 1;
           wr_en_o <= 0;
           cmd_en_o <= 0;
           wb_ack_o <= 1;
           instr <= 1;
        end
        WRITE: begin
           rd_en_o <= 0;
           wr_en_o <= 1;
           cmd_en_o <= 0;
           wb_ack_o <= 1;
           instr <= 0;
        end
        WRITE_WAIT: begin
           rd_en_o <= 0;
           wr_en_o <= 0;
           cmd_en_o <= 0;
           wb_ack_o <= 0;
           instr <= 0;
        end
        WRITE_CMD: begin
           rd_en_o <= 0;
           wr_en_o <= 0;
           cmd_en_o <= 1;
           wb_ack_o <= 0;
           instr <= 0;
        end
        WRITE_CMD_EARLY: begin
           rd_en_o <= 0;
           wr_en_o <= 1;
           cmd_en_o <= 1;
           wb_ack_o <= 0;
           instr <= 0;
        end
        default: begin
           rd_en_o <= 0;
           wr_en_o <= 0;
           cmd_en_o <= 0;
           wb_ack_o <= 0;
           instr <= 0;
        end
      endcase
   end

endmodule // wb_mem_if

// Local Variables:
// verilog-library-directories:("../../*/verilog/" "*")
// verilog-auto-inst-param-value: t
// End:
