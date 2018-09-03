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
 * This is a generic wishbone bus (B3). The number of masters and
 * slaves are configurable. Ten slaves can be connected with a
 * configurable memory map.
 *
 * Instantiation example:
 *  wb_bus_b3
 *   #(.DATA_WIDTH(32), .ADDR_WIDTH(32),
 *     .MASTERS(4), .SLAVES(2),
 *     .S0_RANGE_WIDTH(1), .S0_RANGE_MATCH(1'b0),
 *     .S1_RANGE_WIDTH(4), .S1_RANGE_MATCH(4'he))
 *  bus(.clk_i(clk), rst_i(rst),
 *      .m_adr_i({m_adr_i[3],..,m_adr_i[0]},
 *      ...
 *      );
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
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module wb_bus_b3
  #(
    /* User parameters */
    // Set the number of masters and slaves
    parameter MASTERS = 2,
    parameter SLAVES = 1,

    // Set bus address and data width in bits
    // DATA_WIDTH must be a multiple of 8 (full bytes)!
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 32,

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
    parameter S9_RANGE_MATCH = 1'b0,

   /* Derived local parameters */
   // Width of byte select registers
   localparam SEL_WIDTH = DATA_WIDTH >> 3
    )
   (
       /* Ports */
       input                           clk_i,
       input                           rst_i,

       input [ADDR_WIDTH*MASTERS-1:0]  m_adr_i,
       input [DATA_WIDTH*MASTERS-1:0]  m_dat_i,
       input [MASTERS-1:0]             m_cyc_i,
       input [MASTERS-1:0]             m_stb_i,
       input [SEL_WIDTH*MASTERS-1:0]   m_sel_i,
       input [MASTERS-1:0]             m_we_i,
       input [MASTERS*3-1:0]           m_cti_i,
       input [MASTERS*2-1:0]           m_bte_i,

       output [DATA_WIDTH*MASTERS-1:0] m_dat_o,
       output [MASTERS-1:0]            m_ack_o,
       output [MASTERS-1:0]            m_err_o,
       output [MASTERS-1:0]            m_rty_o,

       output [ADDR_WIDTH*SLAVES-1:0]  s_adr_o,
       output [DATA_WIDTH*SLAVES-1:0]  s_dat_o,
       output [SLAVES-1:0]             s_cyc_o,
       output [SLAVES-1:0]             s_stb_o,
       output [SEL_WIDTH*SLAVES-1:0]   s_sel_o,
       output [SLAVES-1:0]             s_we_o,
       output [SLAVES*3-1:0]           s_cti_o,
       output [SLAVES*2-1:0]           s_bte_o,

       input [DATA_WIDTH*SLAVES-1:0]   s_dat_i,
       input [SLAVES-1:0]              s_ack_i,
       input [SLAVES-1:0]              s_err_i,
       input [SLAVES-1:0]              s_rty_i,

   // The snoop port forwards all write accesses on their success for
   // one cycle.
       output [DATA_WIDTH-1:0]         snoop_adr_o,
       output                          snoop_en_o,

       input                           bus_hold,
       output                          bus_hold_ack
    );


   wire [ADDR_WIDTH-1:0] bus_adr;
   wire [DATA_WIDTH-1:0] bus_wdat;
   wire                  bus_cyc;
   wire                  bus_stb;
   wire [SEL_WIDTH-1:0]  bus_sel;
   wire                  bus_we;
   wire [2:0]            bus_cti;
   wire [1:0]            bus_bte;

   wire [DATA_WIDTH-1:0] bus_rdat;
   wire                  bus_ack;
   wire                  bus_err;
   wire                  bus_rty;

   /* wb_mux AUTO_TEMPLATE(
    .s_dat_o    (bus_wdat),
    .s_dat_i    (bus_rdat),
    .s_\(.*\)_o (bus_\1),
    .s_\(.*\)_i (bus_\1),
    ); */
   wb_mux
     #(.MASTERS(MASTERS), .ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH))
   u_mux(/*AUTOINST*/
         // Outputs
         .m_dat_o                       (m_dat_o[DATA_WIDTH*MASTERS-1:0]),
         .m_ack_o                       (m_ack_o[MASTERS-1:0]),
         .m_err_o                       (m_err_o[MASTERS-1:0]),
         .m_rty_o                       (m_rty_o[MASTERS-1:0]),
         .s_adr_o                       (bus_adr),               // Templated
         .s_dat_o                       (bus_wdat),              // Templated
         .s_cyc_o                       (bus_cyc),               // Templated
         .s_stb_o                       (bus_stb),               // Templated
         .s_sel_o                       (bus_sel),               // Templated
         .s_we_o                        (bus_we),                // Templated
         .s_cti_o                       (bus_cti),               // Templated
         .s_bte_o                       (bus_bte),               // Templated
         .bus_hold_ack                  (bus_hold_ack),
         // Inputs
         .clk_i                         (clk_i),
         .rst_i                         (rst_i),
         .m_adr_i                       (m_adr_i[ADDR_WIDTH*MASTERS-1:0]),
         .m_dat_i                       (m_dat_i[DATA_WIDTH*MASTERS-1:0]),
         .m_cyc_i                       (m_cyc_i[MASTERS-1:0]),
         .m_stb_i                       (m_stb_i[MASTERS-1:0]),
         .m_sel_i                       (m_sel_i[SEL_WIDTH*MASTERS-1:0]),
         .m_we_i                        (m_we_i[MASTERS-1:0]),
         .m_cti_i                       (m_cti_i[MASTERS*3-1:0]),
         .m_bte_i                       (m_bte_i[MASTERS*2-1:0]),
         .s_dat_i                       (bus_rdat),              // Templated
         .s_ack_i                       (bus_ack),               // Templated
         .s_err_i                       (bus_err),               // Templated
         .s_rty_i                       (bus_rty),               // Templated
         .bus_hold                      (bus_hold));

   /* wb_decode AUTO_TEMPLATE(
    .m_dat_o (bus_rdat),
    .m_dat_i (bus_wdat),
    .m_\(.*\)_i (bus_\1),
    .m_\(.*\)_o (bus_\1),
    ); */
   wb_decode
     #(.SLAVES(SLAVES), .ADDR_WIDTH(ADDR_WIDTH), .DATA_WIDTH(DATA_WIDTH),
       .S0_ENABLE(S0_ENABLE),
       .S0_RANGE_WIDTH(S0_RANGE_WIDTH), .S0_RANGE_MATCH(S0_RANGE_MATCH),
       .S1_ENABLE(S1_ENABLE),
       .S1_RANGE_WIDTH(S1_RANGE_WIDTH), .S1_RANGE_MATCH(S1_RANGE_MATCH),
       .S2_ENABLE(S2_ENABLE),
       .S2_RANGE_WIDTH(S2_RANGE_WIDTH), .S2_RANGE_MATCH(S2_RANGE_MATCH),
       .S3_ENABLE(S3_ENABLE),
       .S3_RANGE_WIDTH(S3_RANGE_WIDTH), .S3_RANGE_MATCH(S3_RANGE_MATCH),
       .S4_ENABLE(S4_ENABLE),
       .S4_RANGE_WIDTH(S4_RANGE_WIDTH), .S4_RANGE_MATCH(S4_RANGE_MATCH),
       .S5_ENABLE(S5_ENABLE),
       .S5_RANGE_WIDTH(S5_RANGE_WIDTH), .S5_RANGE_MATCH(S5_RANGE_MATCH),
       .S6_ENABLE(S6_ENABLE),
       .S6_RANGE_WIDTH(S6_RANGE_WIDTH), .S6_RANGE_MATCH(S6_RANGE_MATCH),
       .S7_ENABLE(S7_ENABLE),
       .S7_RANGE_WIDTH(S7_RANGE_WIDTH), .S7_RANGE_MATCH(S7_RANGE_MATCH),
       .S8_ENABLE(S8_ENABLE),
       .S8_RANGE_WIDTH(S8_RANGE_WIDTH), .S8_RANGE_MATCH(S8_RANGE_MATCH),
       .S9_ENABLE(S9_ENABLE),
       .S9_RANGE_WIDTH(S9_RANGE_WIDTH), .S9_RANGE_MATCH(S9_RANGE_MATCH))
   u_decode(/*AUTOINST*/
            // Outputs
            .m_dat_o                    (bus_rdat),              // Templated
            .m_ack_o                    (bus_ack),               // Templated
            .m_err_o                    (bus_err),               // Templated
            .m_rty_o                    (bus_rty),               // Templated
            .s_adr_o                    (s_adr_o[ADDR_WIDTH*SLAVES-1:0]),
            .s_dat_o                    (s_dat_o[DATA_WIDTH*SLAVES-1:0]),
            .s_cyc_o                    (s_cyc_o[SLAVES-1:0]),
            .s_stb_o                    (s_stb_o[SLAVES-1:0]),
            .s_sel_o                    (s_sel_o[SEL_WIDTH*SLAVES-1:0]),
            .s_we_o                     (s_we_o[SLAVES-1:0]),
            .s_cti_o                    (s_cti_o[SLAVES*3-1:0]),
            .s_bte_o                    (s_bte_o[SLAVES*2-1:0]),
            // Inputs
            .clk_i                      (clk_i),
            .rst_i                      (rst_i),
            .m_adr_i                    (bus_adr),               // Templated
            .m_dat_i                    (bus_wdat),              // Templated
            .m_cyc_i                    (bus_cyc),               // Templated
            .m_stb_i                    (bus_stb),               // Templated
            .m_sel_i                    (bus_sel),               // Templated
            .m_we_i                     (bus_we),                // Templated
            .m_cti_i                    (bus_cti),               // Templated
            .m_bte_i                    (bus_bte),               // Templated
            .s_dat_i                    (s_dat_i[DATA_WIDTH*SLAVES-1:0]),
            .s_ack_i                    (s_ack_i[SLAVES-1:0]),
            .s_err_i                    (s_err_i[SLAVES-1:0]),
            .s_rty_i                    (s_rty_i[SLAVES-1:0]));

   // Snoop address comes direct from master bus
   assign snoop_adr_o = bus_adr;
   // Snoop on acknowledge and write. Mask with strobe to be sure
   // there actually is a something happing and no dangling signals
   // and always ack'ing slaves.
   assign snoop_en_o = bus_ack & bus_stb & bus_we;

endmodule // wb_bus_b3
