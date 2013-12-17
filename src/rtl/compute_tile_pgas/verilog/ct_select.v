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
 * Wishbone bus with one master and two slaves
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module ct_select(/*AUTOPARAM*/);
   parameter S0_ADDR_W = 1;
   parameter S0_ADDR   = 1'd0;
   parameter S1_ADDR_W = 4;
   parameter S1_ADDR   = 4'd14;

   parameter SSELECTW = 2;

   // master
   input [31:0]  m_dat_i;
   input [31:0]  m_adr_i;
   input [3:0]   m_sel_i;
   input         m_we_i;
   input         m_cyc_i;
   input         m_stb_i;
   input [2:0]   m_cti_i;
   input [1:0]   m_bte_i;
   output        m_ack_o;
   output        m_err_o;
   output        m_rty_o;
   output [31:0] m_dat_o;

   // slave 0
   output [31:0] s_0_dat_o;
   output [31:0] s_0_adr_o;
   output [3:0]  s_0_sel_o;
   output        s_0_we_o;
   output        s_0_cyc_o;
   output        s_0_stb_o;
   output [2:0]  s_0_cti_o;
   output [1:0]  s_0_bte_o;
   input         s_0_ack_i;
   input         s_0_err_i;
   input         s_0_rty_i;
   input [31:0]  s_0_dat_i;

   // slave 1
   output [31:0] s_1_dat_o;
   output [31:0] s_1_adr_o;
   output [3:0]  s_1_sel_o;
   output        s_1_we_o;
   output        s_1_cyc_o;
   output        s_1_stb_o;
   output [2:0]  s_1_cti_o;
   output [1:0]  s_1_bte_o;
   input         s_1_ack_i;
   input         s_1_err_i;
   input         s_1_rty_i;
   input [31:0]  s_1_dat_i;

   wire [SSELECTW-1:0] sselect;

   assign sselect[0] = (m_adr_i[31:32-S0_ADDR_W] == S0_ADDR);
   assign sselect[1] = (m_adr_i[31:32-S1_ADDR_W] == S1_ADDR);


   assign s_0_stb_o = m_stb_i & sselect[0];
   assign s_0_dat_o = m_dat_i;
   assign s_0_adr_o = m_adr_i;
   assign s_0_sel_o = m_sel_i;
   assign s_0_we_o = m_we_i;
   assign s_0_cyc_o = m_cyc_i;
   assign s_0_cti_o = m_cti_i;
   assign s_0_bte_o = m_bte_i;

   assign s_1_stb_o = m_stb_i & sselect[1];
   assign s_1_dat_o = m_dat_i;
   assign s_1_adr_o = m_adr_i;
   assign s_1_sel_o = m_sel_i;
   assign s_1_we_o = m_we_i;
   assign s_1_cyc_o = m_cyc_i;
   assign s_1_cti_o = m_cti_i;
   assign s_1_bte_o = m_bte_i;


   reg [34:0] sbus;
   always @(*) begin
      case (sselect)
         2'b01:
            sbus = {s_0_dat_i,s_0_ack_i,s_0_err_i,s_0_rty_i};
         2'b10:
            sbus = {s_1_dat_i,s_1_ack_i,s_1_err_i,s_1_rty_i};
         default:
            sbus = {32'h0,1'b0,m_cyc_i&m_stb_i,1'b0};
      endcase
   end

   assign {m_dat_o,m_ack_o,m_err_o,m_rty_o} = sbus;
endmodule
