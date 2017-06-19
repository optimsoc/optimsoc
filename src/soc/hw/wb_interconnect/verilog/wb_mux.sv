/* Copyright (c) 2013-2017 by the author(s)
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
 * This is a generic wishbone bus multiplexer (B3). The number of
 * masters is configurable.
 *
 * DATA_WIDTH and ADDR_WIDTH are defined in bits. DATA_WIDTH must be
 * full bytes (i.e., multiple of 8)!
 *
 * The ports are flattened. That means, that all masters share the bus
 * signal ports. With four masters and a data width of 32 bit the
 * m_cyc_i port is 4 bit wide and the m_dat_i is 128 (=4*32) bit wide.
 * The signals are arranged from right to left, meaning the m_dat_i is
 * defined as [DATA_WIDTH*MASTERS-1:0] and each port m is assigned to
 * [(m+1)*DATA_WIDTH-1:m*DATA_WIDTH].
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

// TODO: * check bus hold signal correctness

module wb_mux
  #(
    /* User parameters */
    // Set the number of slaves
    parameter MASTERS = 1,

    // Set bus address and data width in bits
    // DATA_WIDTH must be a multiple of 8 (full bytes)!
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 32,

   /* Derived local parameters */
   // Width of byte select registers
   localparam SEL_WIDTH = DATA_WIDTH >> 3
    )
   (
    /* Ports */
    input                               clk_i,
    input                               rst_i,

    input [ADDR_WIDTH*MASTERS-1:0]      m_adr_i,
    input [DATA_WIDTH*MASTERS-1:0]      m_dat_i,
    input [MASTERS-1:0]                 m_cyc_i,
    input [MASTERS-1:0]                 m_stb_i,
    input [SEL_WIDTH*MASTERS-1:0]       m_sel_i,
    input [MASTERS-1:0]                 m_we_i,
    input [MASTERS*3-1:0]               m_cti_i,
    input [MASTERS*2-1:0]               m_bte_i,

    output reg [DATA_WIDTH*MASTERS-1:0] m_dat_o,
    output reg [MASTERS-1:0]            m_ack_o,
    output reg [MASTERS-1:0]            m_err_o,
    output reg [MASTERS-1:0]            m_rty_o,

    output reg [ADDR_WIDTH-1:0]         s_adr_o,
    output reg [DATA_WIDTH-1:0]         s_dat_o,
    output reg                          s_cyc_o,
    output reg                          s_stb_o,
    output reg [SEL_WIDTH-1:0]          s_sel_o,
    output reg                          s_we_o,
    output reg [2:0]                    s_cti_o,
    output reg [1:0]                    s_bte_o,

    input [DATA_WIDTH-1:0]              s_dat_i,
    input                               s_ack_i,
    input                               s_err_i,
    input                               s_rty_i,

    input                               bus_hold,
    output reg                          bus_hold_ack
    );

   // The granted master is one hot encoded
   wire [MASTERS-1:0]     grant;
   // The granted master from previous cycle (register)
   reg [MASTERS-1:0]      prev_grant;

      // This is a net that masks the actual requests. The arbiter
   // selects a different master each cycle. Therefore we need to
   // actively control the return of the bus arbitration. That means
   // as long as the granted master still holds is cycle signal, we
   // mask out all other requests (be setting the requests to grant).
   // When the cycle signal is released, we set the request to all
   // masters cycle signals.
   reg [MASTERS-1:0] m_req;

   // This is the arbitration net from round robin
   wire [MASTERS-1:0] arb_grant;
   reg [MASTERS-1:0] prev_arb_grant;

   // It is masked with the bus_hold_ack to hold back the arbitration
   // as long as the bus is held
   assign grant = arb_grant & {MASTERS{!bus_hold_ack}};

   always @(*) begin
      if (|(m_cyc_i & prev_grant)) begin
         // The bus is not released this cycle
         m_req = prev_grant;
         bus_hold_ack = 1'b0;
      end else begin
         m_req = m_cyc_i;
         bus_hold_ack = bus_hold;
      end
   end

   // We register the grant signal. This is needed nevertheless for
   // fair arbitration (round robin)
   always @(posedge clk_i) begin
      if (rst_i) begin
         prev_arb_grant <= {{MASTERS-1{1'b0}},1'b1};
         prev_grant <= {{MASTERS-1{1'b0}},1'b1};
      end else begin
         prev_arb_grant <= arb_grant;
         prev_grant <= grant;
      end
   end

   /* wb_interconnect_arb_rr AUTO_TEMPLATE(
    .gnt     (prev_arb_grant),
    .nxt_gnt (arb_grant),
    .req     (m_req),
    ); */
   wb_interconnect_arb_rr
     #(.N(MASTERS))
     u_arbiter(/*AUTOINST*/
               // Outputs
               .nxt_gnt                 (arb_grant),             // Templated
               // Inputs
               .req                     (m_req),                 // Templated
               .gnt                     (prev_arb_grant));       // Templated

   // Mux the bus based on the grant signal which must be one hot!
   always @(*) begin : bus_m_mux
      integer i;
      s_adr_o = {ADDR_WIDTH{1'bx}};
      s_dat_o = {DATA_WIDTH{1'bx}};
      s_sel_o = {SEL_WIDTH{1'bx}};
      s_we_o = 1'bx;
      s_cti_o = 3'bx;
      s_bte_o = 2'bx;
      s_cyc_o = 1'b0;
      s_stb_o = 1'b0;

      for (i = 0; i < MASTERS; i = i + 1) begin
         m_dat_o[i*DATA_WIDTH +: DATA_WIDTH] = s_dat_i;
         m_ack_o[i] = grant[i] & s_ack_i;
         m_err_o[i] = grant[i] & s_err_i;
         m_rty_o[i] = grant[i] & s_rty_i;

         if (grant[i]) begin
            s_adr_o = m_adr_i[(i+1)*ADDR_WIDTH-1 -: ADDR_WIDTH];
            s_dat_o = m_dat_i[(i+1)*DATA_WIDTH-1 -: DATA_WIDTH];
            s_sel_o = m_sel_i[(i+1)*SEL_WIDTH-1 -: SEL_WIDTH];
            s_we_o = m_we_i[i];
            s_cti_o = m_cti_i[(i+1)*3-1 -: 3];
            s_bte_o = m_bte_i[(i+1)*2-1 -: 2];
            s_cyc_o = m_cyc_i[i];
            s_stb_o = m_stb_i[i];
         end
      end

   end


endmodule // wb_sselect
