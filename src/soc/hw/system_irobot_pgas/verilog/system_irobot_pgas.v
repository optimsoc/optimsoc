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
 * iRobot toplevel file
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

`include "lisnoc_def.vh"

module system_irobot_pgas(
`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
   cdc_conf, cdc_enable,
 `endif
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   trace_itm,
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   trace_stm,
`endif
   /*AUTOARG*/
   // Outputs
   ut3_tx, ut4_tx, ut5_tx, ht1_in_ready, ht1_out_flit, ht1_out_valid,
   wb_mt2_adr_o, wb_mt2_cyc_o, wb_mt2_dat_o, wb_mt2_sel_o,
   wb_mt2_stb_o, wb_mt2_we_o, wb_mt2_cti_o, wb_mt2_bte_o,
   // Inputs
   clk_ct, clk_noc, clk_uart, rst_cpu, rst_sys, ut3_rx, ut4_rx,
   ut5_rx, ht1_in_flit, ht1_in_valid, ht1_out_ready, wb_mt2_ack_i,
   wb_mt2_rty_i, wb_mt2_err_i, wb_mt2_dat_i
   );

   parameter NOC_DATA_WIDTH = 32;
   parameter NOC_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_DATA_WIDTH + NOC_TYPE_WIDTH;

   localparam FLIT_WIDTH = NOC_FLIT_WIDTH;

   localparam VCHANNELS = `VCHANNELS;

   parameter PGAS_ADDRW = 'bx;

   input clk_ct;
   input clk_noc;
   input clk_uart;
   
   input rst_cpu;
   input rst_sys;

   output ut3_tx;
   input  ut3_rx;
   output ut4_tx;
   input  ut4_rx;
   output ut5_tx;
   input  ut5_rx;

`ifdef OPTIMSOC_CLOCKDOMAINS
 `ifdef OPTIMSOC_CDC_DYNAMIC
   output [2:0] cdc_conf;
   output cdc_enable;
 `endif
`endif

   input [NOC_FLIT_WIDTH-1:0] ht1_in_flit;
   input [VCHANNELS-1:0]      ht1_in_valid;
   output [VCHANNELS-1:0]     ht1_in_ready;

   // Flits from tiles->NoC
   output [NOC_FLIT_WIDTH-1:0] ht1_out_flit;
   output [VCHANNELS-1:0]      ht1_out_valid;
   input [VCHANNELS-1:0]       ht1_out_ready;

   output [31:0] wb_mt2_adr_o;
   output        wb_mt2_cyc_o;
   output [31:0] wb_mt2_dat_o;
   output [3:0]  wb_mt2_sel_o;
   output        wb_mt2_stb_o;
   output        wb_mt2_we_o;
   output [2:0]  wb_mt2_cti_o;
   output [1:0]  wb_mt2_bte_o;
   input         wb_mt2_ack_i;
   input         wb_mt2_rty_i;
   input         wb_mt2_err_i;
   input [31:0]  wb_mt2_dat_i;

`ifdef OPTIMSOC_DEBUG_ENABLE_ITM
   output [`DEBUG_ITM_PORTWIDTH-1:0] trace_itm;
`endif
`ifdef OPTIMSOC_DEBUG_ENABLE_STM
   output [`DEBUG_STM_PORTWIDTH-1:0] trace_stm;
`endif

   // Flits from NoC to the tiles
   wire [NOC_FLIT_WIDTH-1:0] link_in_flit[0:5];
   wire [VCHANNELS-1:0]      link_in_valid[0:5];
   wire [VCHANNELS-1:0]      link_in_ready[0:5];

   // Flits from tiles to the NoC
   wire [NOC_FLIT_WIDTH-1:0] link_out_flit[0:5];
   wire [VCHANNELS-1:0]      link_out_valid[0:5];
   wire [VCHANNELS-1:0]      link_out_ready[0:5];

   assign ht1_in_ready = link_out_ready[1];
   assign link_out_valid[1] = ht1_in_valid;
   assign link_out_flit[1]  = ht1_in_flit;
   assign ht1_out_valid = link_in_valid[1];
   assign ht1_out_flit = link_in_flit[1];
   assign link_in_ready[1]  = ht1_out_ready;

   wire [NOC_FLIT_WIDTH-1:0] link_0to1_flit;
   wire [VCHANNELS-1:0]      link_0to1_valid;
   wire [VCHANNELS-1:0]      link_0to1_ready;
   wire [NOC_FLIT_WIDTH-1:0] link_1to0_flit;
   wire [VCHANNELS-1:0]      link_1to0_valid;
   wire [VCHANNELS-1:0]      link_1to0_ready;

   /* lisnoc_router AUTO_TEMPLATE(
    .out_flit  ({link_0to1_flit,link_in_flit[2],link_in_flit[1],link_in_flit[0]}),
    .out_valid ({link_0to1_valid,link_in_valid[2],link_in_valid[1],link_in_valid[0]}),
    .out_ready ({link_0to1_ready,link_in_ready[2],link_in_ready[1],link_in_ready[0]}),
    .in_flit  ({link_1to0_flit,link_out_flit[2],link_out_flit[1],link_out_flit[0]}),
    .in_valid ({link_1to0_valid,link_out_valid[2],link_out_valid[1],link_out_valid[0]}),
    .in_ready ({link_1to0_ready,link_out_ready[2],link_out_ready[1],link_out_ready[0]}),
    .clk (clk_noc),
    .rst (rst_sys),
    ); */
   lisnoc_router
     #(.num_dests                       (6),
       .vchannels                       (3),
       .input_ports                     (4),
       .output_ports                    (4),
       .in_fifo_length                  (2),
       .out_fifo_length                 (2),
       .lookup                          ({4'b0001,4'b0010,4'b0100,4'b1000,4'b1000,4'b1000}))
     u_router0(// Outputs
              .out_flit                 ({link_0to1_flit,link_in_flit[2],link_in_flit[1],link_in_flit[0]}),
              .out_valid                ({link_0to1_valid,link_in_valid[2],link_in_valid[1],link_in_valid[0]}),
              .in_ready                 ({link_1to0_ready,link_out_ready[2],link_out_ready[1],link_out_ready[0]}),
              // Inputs
              .clk                      (clk_noc),
              .rst                      (rst_sys),
              .out_ready                ({link_0to1_ready,link_in_ready[2],link_in_ready[1],link_in_ready[0]}),
              .in_flit                  ({link_1to0_flit,link_out_flit[2],link_out_flit[1],link_out_flit[0]}),
              .in_valid                 ({link_1to0_valid,link_out_valid[2],link_out_valid[1],link_out_valid[0]}));

      lisnoc_router
     #(.num_dests                       (6),
       .vchannels                       (3),
       .input_ports                     (4),
       .output_ports                    (4),
       .in_fifo_length                  (2),
       .out_fifo_length                 (2),
       .lookup                          ({4'b1000,4'b1000,4'b1000,4'b0001,4'b0010,4'b0100}))
     u_router1(// Outputs
              .out_flit                 ({link_1to0_flit,link_in_flit[5],link_in_flit[4],link_in_flit[3]}),
              .out_valid                ({link_1to0_valid,link_in_valid[5],link_in_valid[4],link_in_valid[3]}),
              .in_ready                 ({link_0to1_ready,link_out_ready[5],link_out_ready[4],link_out_ready[3]}),
              // Inputs
              .clk                      (clk_noc),
              .rst                      (rst_sys),
              .out_ready                ({link_1to0_ready,link_in_ready[5],link_in_ready[4],link_in_ready[3]}),
              .in_flit                  ({link_0to1_flit,link_out_flit[5],link_out_flit[4],link_out_flit[3]}),
              .in_valid                 ({link_0to1_valid,link_out_valid[5],link_out_valid[4],link_out_valid[3]}));

   /* compute_tile_pgas AUTO_TEMPLATE(
    .clk (clk_ct),
    .noc_\(.*\) (link_\1[@][]),
    .cpu_stall  (1'b0),
    );*/
   compute_tile_pgas
      #(.VCHANNELS(VCHANNELS),
        .ID(0),
        .PGAS_BASE(0),
        .PGAS_ADDRW(PGAS_ADDRW),
        .MEMORYID(2))
   u_ct0(
`ifdef OPTIMSOC_CLOCKDOMAINS
        .clk_noc(clk_noc),
 `ifdef OPTIMSOC_CDC_DYNAMIC
        .cdc_conf(cdc_conf[2:0]),
        .cdc_enable(cdc_enable),
 `endif
`endif
         /*AUTOINST*/
         // Outputs
         .noc_in_ready                  (link_in_ready[0][VCHANNELS-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[0][NOC_FLIT_WIDTH-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[0][VCHANNELS-1:0]), // Templated
         .trace_itm                     (trace_itm[`DEBUG_ITM_PORTWIDTH-1:0]),
         .trace_stm                     (trace_stm[`DEBUG_STM_PORTWIDTH-1:0]),
         // Inputs
         .clk                           (clk_ct),                // Templated
         .rst_cpu                       (rst_cpu),
         .rst_sys                       (rst_sys),
         .noc_in_flit                   (link_in_flit[0][NOC_FLIT_WIDTH-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[0][VCHANNELS-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[0][VCHANNELS-1:0]), // Templated
         .cpu_stall                     (1'b0));                         // Templated

   /* memory_tile_pgas AUTO_TEMPLATE(
    .noc_\(.*\) (link_\1[@][]),
    .clk        (clk_noc),
    .rst        (rst_sys),
    .wb_\(.*\)  (wb_mt@_\1[]),
    );*/
   memory_tile_pgas
      #(.ID(2),
        .VCHANNELS(VCHANNELS))
      u_mt2(/*AUTOINST*/
            // Outputs
            .noc_in_ready               (link_in_ready[2][VCHANNELS-1:0]), // Templated
            .noc_out_flit               (link_out_flit[2][NOC_FLIT_WIDTH-1:0]), // Templated
            .noc_out_valid              (link_out_valid[2][VCHANNELS-1:0]), // Templated
            .wb_adr_o                   (wb_mt2_adr_o[31:0]),    // Templated
            .wb_cyc_o                   (wb_mt2_cyc_o),          // Templated
            .wb_dat_o                   (wb_mt2_dat_o[31:0]),    // Templated
            .wb_sel_o                   (wb_mt2_sel_o[3:0]),     // Templated
            .wb_stb_o                   (wb_mt2_stb_o),          // Templated
            .wb_we_o                    (wb_mt2_we_o),           // Templated
            .wb_cti_o                   (wb_mt2_cti_o[2:0]),     // Templated
            .wb_bte_o                   (wb_mt2_bte_o[1:0]),     // Templated
            // Inputs
            .clk                        (clk_noc),               // Templated
            .rst                        (rst_sys),               // Templated
            .noc_in_flit                (link_in_flit[2][NOC_FLIT_WIDTH-1:0]), // Templated
            .noc_in_valid               (link_in_valid[2][VCHANNELS-1:0]), // Templated
            .noc_out_ready              (link_out_ready[2][VCHANNELS-1:0]), // Templated
            .wb_ack_i                   (wb_mt2_ack_i),          // Templated
            .wb_rty_i                   (wb_mt2_rty_i),          // Templated
            .wb_err_i                   (wb_mt2_err_i),          // Templated
            .wb_dat_i                   (wb_mt2_dat_i[31:0]));   // Templated


   /* uart_tile AUTO_TEMPLATE(
    .noc_\(.*\) (link_\1[@][]),
    .clk (clk_noc),
    .rst(rst_sys),
    .stx_pad_o (ut@_tx),
    .srx_pad_i (ut@_rx),
    );*/

   
   uart_tile
     #(.VCHANNELS(VCHANNELS),.ID(3),.USE_VCHANNEL(2),.UART_BAUD_RATE(57600))
   u_ut3(
`ifdef OPTIMSOC_UART_LCD_ENABLE
         .lcd_data                      (),
         .lcd_en                        (),
         .lcd_rw                        (),
         .lcd_rs                        (),
`endif   
         /*AUTOINST*/
         // Outputs
         .stx_pad_o                     (ut3_tx),                // Templated
         .noc_in_ready                  (link_in_ready[3][VCHANNELS-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[3][NOC_FLIT_WIDTH-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[3][VCHANNELS-1:0]), // Templated
         // Inputs
         .clk                           (clk_noc),               // Templated
         .clk_uart                      (clk_uart),
         .rst                           (rst_sys),               // Templated
         .srx_pad_i                     (ut3_rx),                // Templated
         .noc_in_flit                   (link_in_flit[3][NOC_FLIT_WIDTH-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[3][VCHANNELS-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[3][VCHANNELS-1:0])); // Templated

   uart_tile
     #(.VCHANNELS(VCHANNELS),.ID(4),.USE_VCHANNEL(2),.UART_BAUD_RATE(57600))
   u_ut4(
`ifdef OPTIMSOC_UART_LCD_ENABLE
         .lcd_data                      (),
         .lcd_en                        (),
         .lcd_rw                        (),
         .lcd_rs                        (),
`endif   
         /*AUTOINST*/
         // Outputs
         .stx_pad_o                     (ut4_tx),                // Templated
         .noc_in_ready                  (link_in_ready[4][VCHANNELS-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[4][NOC_FLIT_WIDTH-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[4][VCHANNELS-1:0]), // Templated
         // Inputs
         .clk                           (clk_noc),               // Templated
         .clk_uart                      (clk_uart),
         .rst                           (rst_sys),               // Templated
         .srx_pad_i                     (ut4_rx),                // Templated
         .noc_in_flit                   (link_in_flit[4][NOC_FLIT_WIDTH-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[4][VCHANNELS-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[4][VCHANNELS-1:0])); // Templated

   
   /* soccerboard_tile AUTO_TEMPLATE(
    .noc_\(.*\) (link_\1[@][]),
    .clk (clk_noc),
    .rst(rst_sys),
    .soccertx_pad_o (ut@_tx),
    .soccerrx_pad_i (ut@_rx),
    );*/
   soccerboard_tile
     #(.VCHANNELS(VCHANNELS),.ID(5),.USE_VCHANNEL(2))
   u_ut5(
         /*AUTOINST*/
         // Outputs
         .soccertx_pad_o                (ut5_tx),                // Templated
         .noc_in_ready                  (link_in_ready[5][VCHANNELS-1:0]), // Templated
         .noc_out_flit                  (link_out_flit[5][NOC_FLIT_WIDTH-1:0]), // Templated
         .noc_out_valid                 (link_out_valid[5][VCHANNELS-1:0]), // Templated
         // Inputs
         .clk                           (clk_noc),               // Templated
         .clk_uart                      (clk_uart),
         .rst                           (rst_sys),               // Templated
         .soccerrx_pad_i                (ut5_rx),                // Templated
         .noc_in_flit                   (link_in_flit[5][NOC_FLIT_WIDTH-1:0]), // Templated
         .noc_in_valid                  (link_in_valid[5][VCHANNELS-1:0]), // Templated
         .noc_out_ready                 (link_out_ready[5][VCHANNELS-1:0])); // Templated
   
endmodule

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/router/" "../../*/verilog")
// verilog-auto-inst-param-value: t
// End:
