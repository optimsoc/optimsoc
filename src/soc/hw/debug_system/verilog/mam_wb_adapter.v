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
 * Wishbone SLAVE Adapter for the Memory Access Module (MAM)
 *
 * This adapter is made to be inserted in front of a Wishbone SLAVE memory.
 *
 * ---------------  WB Slave  ------------------  WB Slave  ----------
 * | WB INTERCON | ========== | mam_wb_adapter | ========== | Memory |
 * ---------------    wb_in   ------------------   wb_out   ----------
 *                                   ||
 *                            wb_mam || WB Master
 *                                   ||
 *                                 -------
 *                                 | mam |
 *                                 -------
 *
 * If the global define OPTIMSOC_DEBUG_ENABLE_MAM is not set the adapter is
 * transparent (e.g. reduced to wires).
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module mam_wb_adapter(
   wb_mam_ack_i, wb_mam_rty_i, wb_mam_err_i, wb_mam_dat_i, wb_mam_bte_o,
   wb_mam_adr_o, wb_mam_cyc_o, wb_mam_dat_o, wb_mam_sel_o, wb_mam_stb_o,
   wb_mam_we_o, wb_mam_cab_o, wb_mam_cti_o,
   /*AUTOARG*/
   // Outputs
   wb_in_ack_o, wb_in_err_o, wb_in_rty_o, wb_in_dat_o, wb_out_adr_i,
   wb_out_bte_i, wb_out_cti_i, wb_out_cyc_i, wb_out_dat_i,
   wb_out_sel_i, wb_out_stb_i, wb_out_we_i, wb_out_clk_i,
   wb_out_rst_i,
   // Inputs
   wb_in_adr_i, wb_in_bte_i, wb_in_cti_i, wb_in_cyc_i, wb_in_dat_i,
   wb_in_sel_i, wb_in_stb_i, wb_in_we_i, wb_in_clk_i, wb_in_rst_i,
   wb_out_ack_o, wb_out_err_o, wb_out_rty_o, wb_out_dat_o
   );

   // address width
   parameter AW = 32;

   // data width
   parameter DW = 32;

   parameter USE_DEBUG = 1;

   // byte select width
   localparam SW = (DW == 32) ? 4 :
                   (DW == 16) ? 2 :
                   (DW ==  8) ? 1 : 'hx;

   /*
    * +--------------+--------------+
    * | word address | byte in word |
    * +--------------+--------------+
    *     WORD_AW         BYTE_AW
    *        +----- AW -----+
    */
   localparam BYTE_AW = SW >> 1;
   localparam WORD_AW = AW - BYTE_AW;

   // Wishbone SLAVE interface: input side (to the CPU etc.)
   input [AW-1:0]  wb_in_adr_i;
   input [1:0]     wb_in_bte_i;
   input [2:0]     wb_in_cti_i;
   input           wb_in_cyc_i;
   input [DW-1:0]  wb_in_dat_i;
   input [SW-1:0]  wb_in_sel_i;
   input           wb_in_stb_i;
   input           wb_in_we_i;

   output          wb_in_ack_o;
   output          wb_in_err_o;
   output          wb_in_rty_o;
   output [DW-1:0] wb_in_dat_o;

   input           wb_in_clk_i;
   input           wb_in_rst_i;

   // Wishbone SLAVE interface: output side (to the memory)
   output [AW-1:0] wb_out_adr_i;
   output [1:0]    wb_out_bte_i;
   output [2:0]    wb_out_cti_i;
   output          wb_out_cyc_i;
   output [DW-1:0] wb_out_dat_i;
   output [SW-1:0] wb_out_sel_i;
   output          wb_out_stb_i;
   output          wb_out_we_i;

   input           wb_out_ack_o;
   input           wb_out_err_o;
   input           wb_out_rty_o;
   input [DW-1:0]  wb_out_dat_o;

   output          wb_out_clk_i;
   output          wb_out_rst_i;
   // we use a common clock for all this module!
   assign wb_out_clk_i = wb_in_clk_i;
   assign wb_out_rst_i = wb_in_rst_i;

   // MAM Wishbone MASTER interface (incoming)
   input [AW-1:0]  wb_mam_adr_o;
   input           wb_mam_cyc_o;
   input [DW-1:0]  wb_mam_dat_o;
   input [SW-1:0]  wb_mam_sel_o;
   input           wb_mam_stb_o;
   input           wb_mam_we_o;
   input           wb_mam_cab_o;
   input [2:0]     wb_mam_cti_o;
   input [1:0]     wb_mam_bte_o;
   output          wb_mam_ack_i;
   output          wb_mam_rty_i;
   output          wb_mam_err_i;
   output [DW-1:0] wb_mam_dat_i;

   if (USE_DEBUG == 1) begin

      localparam STATE_ARB_WIDTH = 2;
      localparam STATE_ARB_IDLE = 0;
      localparam STATE_ARB_ACCESS_MAM = 1;
      localparam STATE_ARB_ACCESS_CPU = 2;

      reg [STATE_ARB_WIDTH-1:0] fsm_arb_state;
      reg [STATE_ARB_WIDTH-1:0] fsm_arb_state_next;

      reg grant_access_cpu;
      reg grant_access_mam;
      reg access_cpu;

      // arbiter FSM: MAM has higher priority than CPU
      always @(posedge wb_in_clk_i) begin
         if (wb_in_rst_i) begin
            fsm_arb_state <= STATE_ARB_IDLE;
         end else begin
            fsm_arb_state <= fsm_arb_state_next;

            if (grant_access_cpu) begin
               access_cpu <= 1'b1;
            end else if (grant_access_mam) begin
               access_cpu <= 1'b0;
            end
         end
      end

      always @(*) begin
         grant_access_cpu = 1'b0;
         grant_access_mam = 1'b0;
         fsm_arb_state_next = STATE_ARB_IDLE;

         case (fsm_arb_state)
            STATE_ARB_IDLE: begin
               if (wb_mam_cyc_o == 1'b1) begin
                  fsm_arb_state_next = STATE_ARB_ACCESS_MAM;
               end else if (wb_in_cyc_i == 1'b1) begin
                  fsm_arb_state_next = STATE_ARB_ACCESS_CPU;
               end else begin
                  fsm_arb_state_next = STATE_ARB_IDLE;
               end
            end

            STATE_ARB_ACCESS_MAM: begin
               grant_access_mam = 1'b1;

               if (wb_mam_cyc_o == 1'b1) begin
                  fsm_arb_state_next = STATE_ARB_ACCESS_MAM;
               end else begin
                  fsm_arb_state_next = STATE_ARB_IDLE;
               end
            end
            //CPU may finish cycle before switching to MAM. May need changes if instant MAM access required
            STATE_ARB_ACCESS_CPU: begin
               grant_access_cpu = 1'b1;
               if (wb_in_cyc_i == 1'b1) begin
                  fsm_arb_state_next = STATE_ARB_ACCESS_CPU;
               end else if (wb_mam_cyc_o == 1'b1) begin
                  fsm_arb_state_next = STATE_ARB_ACCESS_MAM;
               end else begin
                  fsm_arb_state_next = STATE_ARB_IDLE;
               end
            end
         endcase
      end

      // MUX of signals TO the memory
      assign wb_out_adr_i = access_cpu ? wb_in_adr_i : wb_mam_adr_o;
      assign wb_out_bte_i = access_cpu ? wb_in_bte_i : wb_mam_bte_o;
      assign wb_out_cti_i = access_cpu ? wb_in_cti_i : wb_mam_cti_o;
      assign wb_out_cyc_i = access_cpu ? wb_in_cyc_i : wb_mam_cyc_o;
      assign wb_out_dat_i = access_cpu ? wb_in_dat_i : wb_mam_dat_o;
      assign wb_out_sel_i = access_cpu ? wb_in_sel_i : wb_mam_sel_o;
      assign wb_out_stb_i = access_cpu ? wb_in_stb_i : wb_mam_stb_o;
      assign wb_out_we_i = access_cpu ? wb_in_we_i : wb_mam_we_o;


      // MUX of signals FROM the memory
      assign wb_in_ack_o = access_cpu ? wb_out_ack_o : 1'b0;
      assign wb_in_err_o = access_cpu ? wb_out_err_o : 1'b0;
      assign wb_in_rty_o = access_cpu ? wb_out_rty_o : 1'b0;
      assign wb_in_dat_o = access_cpu ? wb_out_dat_o : {DW{1'b0}};

      assign wb_mam_ack_i = ~access_cpu ? wb_out_ack_o : 1'b0;
      assign wb_mam_err_i = ~access_cpu ? wb_out_err_o : 1'b0;
      assign wb_mam_rty_i = ~access_cpu ? wb_out_rty_o : 1'b0;
      assign wb_mam_dat_i = ~access_cpu ? wb_out_dat_o : {DW{1'b0}};
   end else begin // USE_DEBUG == 0
      assign wb_out_adr_i = wb_in_adr_i;
      assign wb_out_bte_i = wb_in_bte_i;
      assign wb_out_cti_i = wb_in_cti_i;
      assign wb_out_cyc_i = wb_in_cyc_i;
      assign wb_out_dat_i = wb_in_dat_i;
      assign wb_out_sel_i = wb_in_sel_i;
      assign wb_out_stb_i = wb_in_stb_i;
      assign wb_out_we_i = wb_in_we_i;

      assign wb_in_ack_o = wb_out_ack_o;
      assign wb_in_err_o = wb_out_err_o;
      assign wb_in_rty_o = wb_out_rty_o;
      assign wb_in_dat_o = wb_out_dat_o;

   end // if(USE_DEBUG == 1)

   `include "optimsoc_functions.vh"

endmodule
