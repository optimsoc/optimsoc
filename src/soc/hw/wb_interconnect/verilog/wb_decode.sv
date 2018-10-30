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
 * This is a generic slave selector for the Wishbone bus (B3). The
 * number of slaves is configurable and up to ten slaves can be
 * connected with a configurable memory map.
 *
 * Instantiation example:
 *  wb_sselect
 *   #(.DATA_WIDTH(32), .ADDR_WIDTH(32),
 *     .SLAVES(2),
 *     .S0_ENABLE (1), .S0_RANGE_WIDTH(1), .S0_RANGE_MATCH(1'b0),
 *     .S1_ENABLE (1), .S1_RANGE_WIDTH(4), .S1_RANGE_MATCH(4'he))
 *  sselect(.clk_i(clk), rst_i(rst),
 *      .s_adr_o({m_adr_o[3],..,m_adr_o[0]},
 *      ...
 *      );
 *
 * DATA_WIDTH and ADDR_WIDTH are defined in bits. DATA_WIDTH must be
 * full bytes (i.e., multiple of 8)!
 *
 * The ports are flattened. That means, that all slaves share the bus
 * signal ports. With four slaves and a data width of 32 bit the
 * s_cyc_o port is 4 bit wide and the s_dat_o is 128 (=4*32) bit wide.
 * The signals are arranged from right to left, meaning the s_dat_o is
 * defined as [DATA_WIDTH*SLAVES-1:0] and each port s is assigned to
 * [(s+1)*DATA_WIDTH-1:s*DATA_WIDTH].
 *
 * The memory map is defined with the S?_RANGE_WIDTH and
 * S?_RANGE_MATCH parameters. The WIDTH sets the number of most
 * significant bits (i.e., those from the left) that are relevant to
 * define the memory range. The MATCH accordingly sets the value of
 * those bits of the address that define the memory range.
 *
 * Example (32 bit addresses):
 *  Slave 0: 0x00000000-0x7fffffff
 *  Slave 1: 0x80000000-0xbfffffff
 *  Slave 2: 0xe0000000-0xe0ffffff
 *
 * Slave 0 is defined by the uppermost bit, which is 0 for this
 * address range. Slave 1 is defined by the uppermost two bit, that
 * are 10 for the memory range. Slave 2 is defined by 8 bit which are
 * e0 for the memory range.
 *
 * This results in:
 *  S0_RANGE_WIDTH(1), S0_RANGE_MATCH(1'b0)
 *  S1_RANGE_WIDTH(2), S1_RANGE_MATCH(2'b10)
 *  S2_RANGE_WIDTH(8), S2_RANGE_MATCH(8'he0)
 *
 *
 * Finally, the slaves can be individually masked to ease
 * instantiation using the Sx_ENABLE parameter. By defaults all slaves
 * are enabled (and selected as long as x < SLAVES).
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module wb_decode
  #(
    /* User parameters */
    // Set the number of slaves
    parameter SLAVES = 1,

    // Set bus address and data width in bits
    // DATA_WIDTH must be a multiple of 8 (full bytes)!
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 32,

    /* Derived local parameters */
    // Width of byte select registers
    localparam SEL_WIDTH = DATA_WIDTH >> 3,

    // Memory range definitions, see above
    // The number of parameters actually limits the number of slaves as
    // there is no generic way that is handled by all tools to define
    // variable width parameter arrays.
    parameter S0_ENABLE = 1,
    parameter S0_RANGE_WIDTH = 1,
    parameter S0_RANGE_MATCH = 1'b0,
    parameter S1_ENABLE = 1,
    parameter S1_RANGE_WIDTH = 1,
    parameter S1_RANGE_MATCH = 1'b0,
    parameter S2_ENABLE = 1,
    parameter S2_RANGE_WIDTH = 1,
    parameter S2_RANGE_MATCH = 1'b0,
    parameter S3_ENABLE = 1,
    parameter S3_RANGE_WIDTH = 1,
    parameter S3_RANGE_MATCH = 1'b0,
    parameter S4_ENABLE = 1,
    parameter S4_RANGE_WIDTH = 1,
    parameter S4_RANGE_MATCH = 1'b0,
    parameter S5_ENABLE = 1,
    parameter S5_RANGE_WIDTH = 1,
    parameter S5_RANGE_MATCH = 1'b0,
    parameter S6_ENABLE = 1,
    parameter S6_RANGE_WIDTH = 1,
    parameter S6_RANGE_MATCH = 1'b0,
    parameter S7_ENABLE = 1,
    parameter S7_RANGE_WIDTH = 1,
    parameter S7_RANGE_MATCH = 1'b0,
    parameter S8_ENABLE = 1,
    parameter S8_RANGE_WIDTH = 1,
    parameter S8_RANGE_MATCH = 1'b0,
    parameter S9_ENABLE = 1,
    parameter S9_RANGE_WIDTH = 1,
    parameter S9_RANGE_MATCH = 1'b0
   )
   (
    /* Ports */
    input                              clk_i,
    input                              rst_i,

    input [ADDR_WIDTH-1:0]             m_adr_i,
    input [DATA_WIDTH-1:0]             m_dat_i,
    input                              m_cyc_i,
    input                              m_stb_i,
    input [SEL_WIDTH-1:0]              m_sel_i,
    input                              m_we_i,
    input [2:0]                        m_cti_i,
    input [1:0]                        m_bte_i,

    output reg [DATA_WIDTH-1:0]        m_dat_o,
    output                             m_ack_o,
    output                             m_err_o,
    output                             m_rty_o,

    output reg [ADDR_WIDTH*SLAVES-1:0] s_adr_o,
    output reg [DATA_WIDTH*SLAVES-1:0] s_dat_o,
    output reg [SLAVES-1:0]            s_cyc_o,
    output reg [SLAVES-1:0]            s_stb_o,
    output reg [SEL_WIDTH*SLAVES-1:0]  s_sel_o,
    output reg [SLAVES-1:0]            s_we_o,
    output reg [SLAVES*3-1:0]          s_cti_o,
    output reg [SLAVES*2-1:0]          s_bte_o,

    input [DATA_WIDTH*SLAVES-1:0]      s_dat_i,
    input [SLAVES-1:0]                 s_ack_i,
    input [SLAVES-1:0]                 s_err_i,
    input [SLAVES-1:0]                 s_rty_i
    );


   wire [SLAVES-1:0]               s_select;

   // Generate the slave select signals based on the master bus
   // address and the memory range parameters
   generate
      if (SLAVES > 0)
        assign s_select[0] = S0_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S0_RANGE_WIDTH] == S0_RANGE_MATCH[S0_RANGE_WIDTH-1:0]);
      if (SLAVES > 1)
        assign s_select[1] = S1_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S1_RANGE_WIDTH] == S1_RANGE_MATCH[S1_RANGE_WIDTH-1:0]);
      if (SLAVES > 2)
        assign s_select[2] = S2_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S2_RANGE_WIDTH] == S2_RANGE_MATCH[S2_RANGE_WIDTH-1:0]);
      if (SLAVES > 3)
        assign s_select[3] = S3_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S3_RANGE_WIDTH] == S3_RANGE_MATCH[S3_RANGE_WIDTH-1:0]);
      if (SLAVES > 4)
        assign s_select[4] = S4_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S4_RANGE_WIDTH] == S4_RANGE_MATCH[S4_RANGE_WIDTH-1:0]);
      if (SLAVES > 5)
        assign s_select[5] = S5_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S5_RANGE_WIDTH] == S5_RANGE_MATCH[S5_RANGE_WIDTH-1:0]);
      if (SLAVES > 6)
        assign s_select[6] = S6_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S6_RANGE_WIDTH] == S6_RANGE_MATCH[S6_RANGE_WIDTH-1:0]);
      if (SLAVES > 7)
        assign s_select[7] = S7_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S7_RANGE_WIDTH] == S7_RANGE_MATCH[S7_RANGE_WIDTH-1:0]);
      if (SLAVES > 8)
        assign s_select[8] = S8_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S8_RANGE_WIDTH] == S8_RANGE_MATCH[S8_RANGE_WIDTH-1:0]);
      if (SLAVES > 9)
        assign s_select[9] = S9_ENABLE & (m_adr_i[ADDR_WIDTH-1 -: S9_RANGE_WIDTH] == S9_RANGE_MATCH[S9_RANGE_WIDTH-1:0]);
   endgenerate

   // If two s_select are high or none, we might have an bus error
   wire                            bus_error;
   assign bus_error = ~^s_select;

   reg                             m_ack, m_err, m_rty;

   // Mux the slave bus based on the slave select signal (one hot!)
   always @(*) begin : bus_s_mux
      integer i;
      m_dat_o = {DATA_WIDTH{1'b0}};
      m_ack = 1'b0;
      m_err = 1'b0;
      m_rty = 1'b0;
      for (i = 0; i < SLAVES; i = i + 1) begin
         s_adr_o[i*ADDR_WIDTH +: ADDR_WIDTH] = m_adr_i;
         s_dat_o[i*DATA_WIDTH +: DATA_WIDTH] = m_dat_i;
         s_sel_o[i*SEL_WIDTH +: SEL_WIDTH] = m_sel_i;
         s_we_o[i] = m_we_i;
         s_cti_o[i*3 +: 3] = m_cti_i;
         s_bte_o[i*2 +: 2] = m_bte_i;

         s_cyc_o[i] = m_cyc_i & s_select[i];
         s_stb_o[i] = m_stb_i & s_select[i];

         if (s_select[i]) begin
            m_dat_o = s_dat_i[i*DATA_WIDTH +: DATA_WIDTH];
            m_ack = s_ack_i[i];
            m_err = s_err_i[i];
            m_rty = s_rty_i[i];
         end
      end
   end

   assign m_ack_o = m_ack & !bus_error;
   assign m_err_o = m_err | bus_error;
   assign m_rty_o = m_rty & !bus_error;

endmodule // wb_sselect
