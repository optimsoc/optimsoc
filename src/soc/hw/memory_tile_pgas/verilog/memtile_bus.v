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
 * Wishbone bus for the memory tile with two masters and one slave
 *
 * Slave address mapping:
 *   slave 0: 0x00000000-0x7fffffff
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module memtile_bus(/*AUTOARG*/
   // Outputs
   m_0_dat_o, m_0_ack_o, m_0_err_o, m_0_rty_o, m_1_dat_o, m_1_ack_o,
   m_1_err_o, m_1_rty_o, s_0_dat_o, s_0_adr_o, s_0_sel_o, s_0_we_o,
   s_0_cyc_o, s_0_stb_o, s_0_cab_o,
   // Inputs
   clk_i, rst_i, m_0_dat_i, m_0_adr_i, m_0_sel_i, m_0_we_i, m_0_cyc_i,
   m_0_stb_i, m_0_cab_i, m_1_dat_i, m_1_adr_i, m_1_sel_i, m_1_we_i,
   m_1_cyc_i, m_1_stb_i, m_1_cab_i, s_0_dat_i, s_0_ack_i, s_0_err_i,
   s_0_rty_i
   );

   parameter DW = 32;
   parameter AW = 32;
   parameter SW = 4;

   parameter S0_ADDR_W = 1;
   parameter S0_ADDR   = 1'd0;

   parameter MBUSW    = AW + SW + DW + 4;
   parameter SBUSW    = 3;
   parameter MSELECTW = 2;
   parameter SSELECTW = 1;
   parameter GW       = 1;

   input      clk_i, rst_i;

   // master 0
   input [DW-1:0]    m_0_dat_i;
   output [DW-1:0]   m_0_dat_o;
   input [AW-1:0]    m_0_adr_i;
   input [SW-1:0]    m_0_sel_i;
   input             m_0_we_i;
   input             m_0_cyc_i;
   input             m_0_stb_i;
   input             m_0_cab_i;
   output            m_0_ack_o;
   output            m_0_err_o;
   output            m_0_rty_o;

   // master 1
   input [DW-1:0]    m_1_dat_i;
   output [DW-1:0]   m_1_dat_o;
   input [AW-1:0]    m_1_adr_i;
   input [SW-1:0]    m_1_sel_i;
   input             m_1_we_i;
   input             m_1_cyc_i;
   input             m_1_stb_i;
   input             m_1_cab_i;
   output            m_1_ack_o;
   output            m_1_err_o;
   output            m_1_rty_o;

   // slave 0
   input [DW-1:0]    s_0_dat_i;
   output [DW-1:0]   s_0_dat_o;
   output [AW-1:0]   s_0_adr_o;
   output [SW-1:0]   s_0_sel_o;
   output            s_0_we_o;
   output            s_0_cyc_o;
   output            s_0_stb_o;
   output            s_0_cab_o;
   input             s_0_ack_i;
   input             s_0_err_i;
   input             s_0_rty_i;


   wire  [MSELECTW -1:0] i_gnt_arb;
   wire  [GW-1:0]        gnt;
   reg   [SSELECTW -1:0] i_ssel_dec;
   reg   [MBUSW -1:0]    i_bus_m;
   wire  [DW -1:0]       i_dat_s;
   wire  [SBUSW -1:0]    i_bus_s;


   assign m_0_dat_o = i_dat_s;
   assign  {m_0_ack_o, m_0_err_o, m_0_rty_o} = i_bus_s & {3{i_gnt_arb[0]}};

   assign m_1_dat_o = i_dat_s;
   assign  {m_1_ack_o, m_1_err_o, m_1_rty_o} = i_bus_s & {3{i_gnt_arb[1]}};


   assign i_bus_s = { s_0_ack_i,
                      s_0_err_i,
                      s_0_rty_i };


   assign {s_0_adr_o, s_0_sel_o, s_0_dat_o, s_0_we_o, s_0_cab_o, s_0_cyc_o} = i_bus_m[MBUSW -1:1];
   assign s_0_stb_o = i_bus_m[1] & i_bus_m[0] & i_ssel_dec[0];


   always @(*) begin
      case (gnt)
         1'd0:
            i_bus_m = {m_0_adr_i, m_0_sel_i, m_0_dat_i, m_0_we_i, m_0_cab_i, m_0_cyc_i, m_0_stb_i};
         1'd1:
            i_bus_m = {m_1_adr_i, m_1_sel_i, m_1_dat_i, m_1_we_i, m_1_cab_i, m_1_cyc_i, m_1_stb_i};
         default:
            i_bus_m =  {MBUSW{1'b0}};
      endcase
   end

   assign i_dat_s = i_ssel_dec[0] ? s_0_dat_i : {DW{1'b0}};

   assign i_gnt_arb[0] = (gnt == 1'd0);
   assign i_gnt_arb[1] = (gnt == 1'd1);


   memtile_bus_arbiter
      u_memtile_bus_arbiter(.clk(clk_i),
                            .rst(rst_i),
                            .req({m_1_cyc_i, m_0_cyc_i}),
                            .gnt(gnt));

   wire [SSELECTW-1:0]   m_0_ssel_dec;
   wire [SSELECTW-1:0]   m_1_ssel_dec;


   always @(*) begin
      case (gnt)
         1'd0:
            i_ssel_dec = m_0_ssel_dec;
         1'd1:
            i_ssel_dec = m_1_ssel_dec;
         default:
            i_ssel_dec = {SSELECTW{1'b0}};
     endcase
   end

   assign m_0_ssel_dec[0] = (m_0_adr_i[AW -1 : AW - S0_ADDR_W ] == S0_ADDR);
   assign m_1_ssel_dec[0] = (m_1_adr_i[AW -1 : AW - S0_ADDR_W ] == S0_ADDR);
endmodule
