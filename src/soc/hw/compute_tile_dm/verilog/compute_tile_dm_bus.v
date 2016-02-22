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
 * Wishbone bus toplevel with three masters and three slaves
 *
 * Default slave addresses (set through the Sx_ADDR parameters):
 *   slave 0: 0x00000000-0x7fffffff
 *   slave 1: 0xe0000000-0xefffffff
 *   slave 2: 0xf0000000-0xffffffff
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */
module compute_tile_dm_bus(/*AUTOARG*/
   // Outputs
   m_0_dat_o, m_0_ack_o, m_0_err_o, m_0_rty_o, m_1_dat_o, m_1_ack_o,
   m_1_err_o, m_1_rty_o, m_2_dat_o, m_2_ack_o, m_2_err_o, m_2_rty_o,
   s_0_dat_o, s_0_adr_o, s_0_sel_o, s_0_we_o, s_0_cyc_o, s_0_stb_o,
   s_0_cab_o, s_0_cti_o, s_0_bte_o, s_1_dat_o, s_1_adr_o, s_1_sel_o,
   s_1_we_o, s_1_cyc_o, s_1_stb_o, s_1_cab_o, s_1_cti_o, s_1_bte_o,
   s_2_dat_o, s_2_adr_o, s_2_sel_o, s_2_we_o, s_2_cyc_o, s_2_stb_o,
   s_2_cab_o, s_2_cti_o, s_2_bte_o,
   // Inputs
   clk_i, rst_i, m_0_dat_i, m_0_adr_i, m_0_sel_i, m_0_we_i, m_0_cyc_i,
   m_0_stb_i, m_0_cab_i, m_0_cti_i, m_0_bte_i, m_1_dat_i, m_1_adr_i,
   m_1_sel_i, m_1_we_i, m_1_cyc_i, m_1_stb_i, m_1_cab_i, m_1_cti_i,
   m_1_bte_i, m_2_dat_i, m_2_adr_i, m_2_sel_i, m_2_we_i, m_2_cyc_i,
   m_2_stb_i, m_2_cab_i, m_2_cti_i, m_2_bte_i, s_0_dat_i, s_0_ack_i,
   s_0_err_i, s_0_rty_i, s_1_dat_i, s_1_ack_i, s_1_err_i, s_1_rty_i,
   s_2_dat_i, s_2_ack_i, s_2_err_i, s_2_rty_i
   );

   parameter DW = 32; // data width
   parameter AW = 32; // address width
   parameter SW = 4;  // select width

   parameter S0_ADDR_W = 1;
   parameter S0_ADDR   = 1'h0;
   parameter S1_ADDR_W = 4;
   parameter S1_ADDR   = 4'hE;
   parameter S2_ADDR_W = 4;
   parameter S2_ADDR   = 4'hF;

   localparam MBUSW    = AW + SW + DW + 9;
   localparam SBUSW    = 3;
   localparam MSELECTW = 3;
   localparam SSELECTW = 3;
   localparam GW       = 2;


   input                clk_i, rst_i;

   // master 0
   input [DW-1:0]       m_0_dat_i;
   output [DW-1:0]      m_0_dat_o;
   input [AW-1:0]       m_0_adr_i;
   input [SW-1:0]       m_0_sel_i;
   input                m_0_we_i;
   input                m_0_cyc_i;
   input                m_0_stb_i;
   input                m_0_cab_i;
   input [2:0]          m_0_cti_i;
   input [1:0]          m_0_bte_i;
   output               m_0_ack_o;
   output               m_0_err_o;
   output               m_0_rty_o;

   // master 1
   input [DW-1:0]       m_1_dat_i;
   output [DW-1:0]      m_1_dat_o;
   input [AW-1:0]       m_1_adr_i;
   input [SW-1:0]       m_1_sel_i;
   input                m_1_we_i;
   input                m_1_cyc_i;
   input                m_1_stb_i;
   input                m_1_cab_i;
   input [2:0]          m_1_cti_i;
   input [1:0]          m_1_bte_i;
   output               m_1_ack_o;
   output               m_1_err_o;
   output               m_1_rty_o;

   // master 2
   input [DW-1:0]       m_2_dat_i;
   output [DW-1:0]      m_2_dat_o;
   input [AW-1:0]       m_2_adr_i;
   input [SW-1:0]       m_2_sel_i;
   input                m_2_we_i;
   input                m_2_cyc_i;
   input                m_2_stb_i;
   input                m_2_cab_i;
   input [2:0]          m_2_cti_i;
   input [1:0]          m_2_bte_i;
   output               m_2_ack_o;
   output               m_2_err_o;
   output               m_2_rty_o;

   // slave 0
   input [DW-1:0]       s_0_dat_i;
   output [DW-1:0]      s_0_dat_o;
   output [AW-1:0]      s_0_adr_o;
   output [SW-1:0]      s_0_sel_o;
   output               s_0_we_o;
   output               s_0_cyc_o;
   output               s_0_stb_o;
   output               s_0_cab_o;
   output [2:0]         s_0_cti_o;
   output [1:0]         s_0_bte_o;
   input                s_0_ack_i;
   input                s_0_err_i;
   input                s_0_rty_i;

   // slave 1
   input [DW-1:0]       s_1_dat_i;
   output [DW-1:0]      s_1_dat_o;
   output [AW-1:0]      s_1_adr_o;
   output [SW-1:0]      s_1_sel_o;
   output               s_1_we_o;
   output               s_1_cyc_o;
   output               s_1_stb_o;
   output               s_1_cab_o;
   output [2:0]         s_1_cti_o;
   output [1:0]         s_1_bte_o;
   input                s_1_ack_i;
   input                s_1_err_i;
   input                s_1_rty_i;

   // slave 2
   input [DW-1:0]       s_2_dat_i;
   output [DW-1:0]      s_2_dat_o;
   output [AW-1:0]      s_2_adr_o;
   output [SW-1:0]      s_2_sel_o;
   output               s_2_we_o;
   output               s_2_cyc_o;
   output               s_2_stb_o;
   output               s_2_cab_o;
   output [2:0]         s_2_cti_o;
   output [1:0]         s_2_bte_o;
   input                s_2_ack_i;
   input                s_2_err_i;
   input                s_2_rty_i;


   wire [MSELECTW -1:0] i_gnt_arb;
   wire [GW-1:0]        gnt;
   reg  [SSELECTW -1:0] i_ssel_dec;
   reg  [MBUSW -1:0]    i_bus_m;
   wire [DW -1:0]       i_dat_s;
   wire [SBUSW -1:0]    i_bus_s;

   assign m_0_dat_o = i_dat_s;
   assign {m_0_ack_o, m_0_err_o, m_0_rty_o} = i_bus_s & {3{i_gnt_arb[0]}};

   assign m_1_dat_o = i_dat_s;
   assign {m_1_ack_o, m_1_err_o, m_1_rty_o} = i_bus_s & {3{i_gnt_arb[1]}};

   assign m_2_dat_o = i_dat_s;
   assign {m_2_ack_o, m_2_err_o, m_2_rty_o} = i_bus_s & {3{i_gnt_arb[2]}};


   assign i_bus_s = { s_0_ack_i | s_1_ack_i | s_2_ack_i,
                      s_0_err_i | s_1_err_i | s_2_err_i,
                      s_0_rty_i | s_1_rty_i | s_2_rty_i };


   assign {s_0_adr_o, s_0_sel_o, s_0_dat_o, s_0_we_o, s_0_cab_o, s_0_cti_o, s_0_bte_o, s_0_cyc_o} = i_bus_m[MBUSW -1:1];
   assign s_0_stb_o = i_bus_m[1] & i_bus_m[0] & i_ssel_dec[0];

   assign {s_1_adr_o, s_1_sel_o, s_1_dat_o, s_1_we_o, s_1_cab_o, s_1_cti_o, s_1_bte_o, s_1_cyc_o} = i_bus_m[MBUSW -1:1];
   assign s_1_stb_o = i_bus_m[1] & i_bus_m[0] & i_ssel_dec[1];

   assign {s_2_adr_o, s_2_sel_o, s_2_dat_o, s_2_we_o, s_2_cab_o, s_2_cti_o, s_2_bte_o, s_2_cyc_o} = i_bus_m[MBUSW -1:1];
   assign s_2_stb_o = i_bus_m[1] & i_bus_m[0] & i_ssel_dec[2];


   always @(*) begin
      case (gnt)
         2'd0:
            i_bus_m = {m_0_adr_i, m_0_sel_i, m_0_dat_i, m_0_we_i, m_0_cab_i, m_0_cti_i, m_0_bte_i, m_0_cyc_i, m_0_stb_i};
         2'd1:
            i_bus_m = {m_1_adr_i, m_1_sel_i, m_1_dat_i, m_1_we_i, m_1_cab_i, m_1_cti_i, m_1_bte_i, m_1_cyc_i, m_1_stb_i};
         2'd2:
            i_bus_m = {m_2_adr_i, m_2_sel_i, m_2_dat_i, m_2_we_i, m_2_cab_i, m_2_cti_i, m_2_bte_i, m_2_cyc_i, m_2_stb_i};
         default:
            i_bus_m = {MBUSW{1'b0}};
      endcase
   end

   assign i_dat_s = i_ssel_dec[0] ? s_0_dat_i: i_ssel_dec[1] ? s_1_dat_i: i_ssel_dec[2] ? s_2_dat_i : {DW{1'b0}};

   assign i_gnt_arb[0] = (gnt == 2'd0);
   assign i_gnt_arb[1] = (gnt == 2'd1);
   assign i_gnt_arb[2] = (gnt == 2'd2);


   compute_tile_dm_bus_arbiter
      u_compute_tile_bus_arbiter(.clk(clk_i),
                                 .rst(rst_i),
                                 .req({m_2_cyc_i, m_1_cyc_i, m_0_cyc_i}),
                                 .gnt(gnt));

   wire [2:0]      m_0_ssel_dec;
   wire [2:0]      m_1_ssel_dec;
   wire [2:0]      m_2_ssel_dec;


   always @(*) begin
      case (gnt)
         2'd0:
            i_ssel_dec = m_0_ssel_dec;
         2'd1:
            i_ssel_dec = m_1_ssel_dec;
         2'd2:
            i_ssel_dec = m_2_ssel_dec;
         default:
            i_ssel_dec = {SSELECTW{1'b0}};
      endcase
   end

   assign m_0_ssel_dec[0] = (m_0_adr_i[AW -1 : AW - S0_ADDR_W ] == S0_ADDR);
   assign m_0_ssel_dec[1] = (m_0_adr_i[AW -1 : AW - S1_ADDR_W ] == S1_ADDR);
   assign m_0_ssel_dec[2] = (m_0_adr_i[AW -1 : AW - S2_ADDR_W ] == S2_ADDR);

   assign m_1_ssel_dec[0] = (m_1_adr_i[AW -1 : AW - S0_ADDR_W ] == S0_ADDR);
   assign m_1_ssel_dec[1] = (m_1_adr_i[AW -1 : AW - S1_ADDR_W ] == S1_ADDR);
   assign m_1_ssel_dec[2] = (m_1_adr_i[AW -1 : AW - S2_ADDR_W ] == S2_ADDR);

   assign m_2_ssel_dec[0] = (m_2_adr_i[AW -1 : AW - S0_ADDR_W ] == S0_ADDR);
   assign m_2_ssel_dec[1] = (m_2_adr_i[AW -1 : AW - S1_ADDR_W ] == S1_ADDR);
   assign m_2_ssel_dec[2] = (m_2_adr_i[AW -1 : AW - S2_ADDR_W ] == S2_ADDR);
endmodule
