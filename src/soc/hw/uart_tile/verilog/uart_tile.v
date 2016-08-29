`include "uart_defines.v"

// The ready signal is driven by the uart, although lcd is always ready

module uart_tile(
`ifdef OPTIMSOC_UART_LCD_ENABLE
                 lcd_data, lcd_en, lcd_rw, lcd_rs,
`endif
   /*AUTOARG*/
   // Outputs
   stx_pad_o, noc_in_ready, noc_out_flit, noc_out_valid,
   // Inputs
   clk, clk_uart, rst, srx_pad_i, noc_in_flit, noc_in_valid,
   noc_out_ready
   );

   parameter ID = 0;
   parameter VCHANNELS = 3;
   parameter USE_VCHANNEL = 0;

   parameter ph_dest_width = 5;
   parameter ph_cls_width = 3;
   parameter ph_src_width = 5;

   parameter destination = 0;
   parameter pkt_class = 0;

   parameter noc_data_width = 32;
   parameter noc_type_width = 2;
   localparam NOC_FLIT_WIDTH = noc_data_width+noc_type_width;

   parameter UART_BAUD_RATE = 115200;

   localparam uart_data_width = `UART_DATA_WIDTH;
   localparam uart_addr_width = `UART_ADDR_WIDTH;

   input clk;
   input clk_uart;
   input rst;

   output stx_pad_o;
   input  srx_pad_i;

   input [NOC_FLIT_WIDTH-1:0] noc_in_flit;
   input [VCHANNELS-1:0]      noc_in_valid;
   output [VCHANNELS-1:0]     noc_in_ready;
   output [NOC_FLIT_WIDTH-1:0] noc_out_flit;
   output [VCHANNELS-1:0]      noc_out_valid;
   input [VCHANNELS-1:0]       noc_out_ready;

   wire [NOC_FLIT_WIDTH-1:0] noc_in_flit_cdc;
   wire                      noc_in_valid_cdc;
   wire                      noc_in_ready_cdc;
   wire [NOC_FLIT_WIDTH-1:0] noc_out_flit_cdc;
   wire                      noc_out_valid_cdc;
   wire                      noc_out_ready_cdc;

   wire                      fifo_in_full;
   assign noc_in_ready = {VCHANNELS{~fifo_in_full}};
   wire                      fifo_in_empty;
   assign noc_in_valid_cdc = ~fifo_in_empty;

   cdc_fifo
    #(.DW(NOC_FLIT_WIDTH))
     u_incdc(// Outputs
           .wr_full                     (fifo_in_full),
           .rd_empty                    (fifo_in_empty),
           .rd_data                     (noc_in_flit_cdc),
           // Inputs
           .wr_clk                      (clk),
           .rd_clk                      (clk_uart),
           .wr_rst                      (~rst),
           .rd_rst                      (~rst),
           .rd_en                       (noc_in_ready_cdc),
           .wr_en                       (|noc_in_valid),
           .wr_data                     (noc_in_flit));

   wire                      fifo_out_full;
   assign noc_out_ready_cdc = ~fifo_out_full;
   wire                      fifo_out_empty;
   assign noc_out_valid[USE_VCHANNEL] = ~fifo_out_empty;

   genvar               v;
   generate
      for (v=0;v<VCHANNELS;v=v+1) begin
         if (v!=USE_VCHANNEL) begin
            assign noc_out_valid[v] = 1'b0;
         end
      end
   endgenerate

   /* cdc_fifo AUTO_TEMPLATE(
    .wr_clk (clk_uart),
    .wr_en  (noc_out_valid_cdc),
    .wr_rst (~rst),
    .wr_data (noc_out_flit_cdc),
    .wr_full (fifo_out_full),
    .rd_clk (clk),
    .rd_en  (|noc_out_ready),
    .rd_rst (~rst),
    .rd_data (noc_out_flit),
    .rd_empty (fifo_out_empty),
    ) */

   cdc_fifo
    #(.DW(NOC_FLIT_WIDTH))
   u_outcdc(/*AUTOINST*/
            // Outputs
            .wr_full                    (fifo_out_full),         // Templated
            .rd_empty                   (fifo_out_empty),        // Templated
            .rd_data                    (noc_out_flit),          // Templated
            // Inputs
            .wr_clk                     (clk_uart),              // Templated
            .rd_clk                     (clk),                   // Templated
            .wr_rst                     (~rst),                  // Templated
            .rd_rst                     (~rst),                  // Templated
            .rd_en                      (|noc_out_ready),        // Templated
            .wr_en                      (noc_out_valid_cdc),     // Templated
            .wr_data                    (noc_out_flit_cdc));     // Templated



`ifdef OPTIMSOC_UART_LCD_ENABLE
   output [3:0]                lcd_data;
   output                      lcd_en;
   output                      lcd_rw;
   output                      lcd_rs;
`endif

   /*AUTOWIRE*/
   // Beginning of automatic wires (for undeclared instantiated-module outputs)
   wire [7:0]           in_char_data;           // From u_noc2char of noc2char.v
   wire                 in_char_ready;          // From u_char2uart of char2uart.v
   wire                 in_char_valid;          // From u_noc2char of noc2char.v
   wire [7:0]           out_char_data;          // From u_char2uart of char2uart.v
   wire                 out_char_ready;         // From u_char2noc of char2noc.v
   wire                 out_char_valid;         // From u_char2uart of char2uart.v
   wire                 wb_ack_o;               // From u_uart of uart_top.v
   wire [uart_addr_width-1:0] wb_adr_i;         // From u_char2uart of char2uart.v
   wire                 wb_cyc_i;               // From u_char2uart of char2uart.v
   wire [uart_data_width-1:0] wb_dat_i;         // From u_char2uart of char2uart.v
   wire [(uart_data_width)-1:0] wb_dat_o;       // From u_uart of uart_top.v
   wire                 wb_int_o;               // From u_uart of uart_top.v
   wire [3:0]           wb_sel_i;               // From u_char2uart of char2uart.v
   wire                 wb_stb_i;               // From u_char2uart of char2uart.v
   wire                 wb_we_i;                // From u_char2uart of char2uart.v
   // End of automatics

   wire                 uart_in_valid;

`ifdef OPTIMSOC_UART_LCD_ENABLE
   wire                 lcd_in_valid;

   /*
    * ---+------+-----+-----+------+
    * .. | type | row | col | char |
    * ---+------+-----+-----+------+
    *      13      12   11-8   7-0
    *
    * UART packets: type=0, row, col unused
    * LCD packets: type=1, row, col of lcd
    */

//   assign uart_in_valid = |noc_in_valid[use_vchannel] &
   assign uart_in_valid = noc_in_valid_cdc &
                          !noc_in_flit_cdc[13];
//   assign lcd_in_valid = noc_in_valid[use_vchannel] &
   assign lcd_in_valid = noc_in_valid_cdc &
                         noc_in_flit_cdc[13];
`else // !`ifdef OPTIMSOC_UART_LCD_ENABLE
//   assign uart_in_valid = noc_in_valid[use_vchannel];
   assign uart_in_valid = noc_in_valid_cdc;
`endif

   /* noc2char AUTO_TEMPLATE(
    .noc_flit  (noc_in_flit_cdc[]),
    .noc_valid (uart_in_valid),
    .noc_ready (noc_in_ready_cdc),
    .char_\(.*\) (in_char_\1[]),
    ); */

   noc2char
     u_noc2char(/*AUTOINST*/
                // Outputs
                .noc_ready              (noc_in_ready_cdc),      // Templated
                .char_valid             (in_char_valid),         // Templated
                .char_data              (in_char_data[7:0]),     // Templated
                // Inputs
                .clk                    (clk),
                .rst                    (rst),
                .noc_flit               (noc_in_flit_cdc[33:0]), // Templated
                .noc_valid              (uart_in_valid),         // Templated
                .char_ready             (in_char_ready));        // Templated

   char2uart
     #(.UART_BAUD_RATE(UART_BAUD_RATE))
   u_char2uart(/*AUTOINST*/
               // Outputs
               .in_char_ready           (in_char_ready),
               .out_char_data           (out_char_data[7:0]),
               .out_char_valid          (out_char_valid),
               .wb_we_i                 (wb_we_i),
               .wb_stb_i                (wb_stb_i),
               .wb_cyc_i                (wb_cyc_i),
               .wb_adr_i                (wb_adr_i[uart_addr_width-1:0]),
               .wb_sel_i                (wb_sel_i[3:0]),
               .wb_dat_i                (wb_dat_i[uart_data_width-1:0]),
               // Inputs
               .clk                     (clk),
               .rst                     (rst),
               .in_char_data            (in_char_data[7:0]),
               .in_char_valid           (in_char_valid),
               .out_char_ready          (out_char_ready),
               .wb_dat_o                (wb_dat_o[uart_data_width-1:0]),
               .wb_ack_o                (wb_ack_o),
               .wb_int_o                (wb_int_o));

   /* char2noc AUTO_TEMPLATE(
    .noc_flit  (noc_out_flit_cdc[]),
    .noc_valid (noc_out_valid_cdc),
    .noc_ready (noc_out_ready_cdc),
    .char_\(.*\) (out_char_\1[]),
    ); */

   char2noc
   #(.ph_dest_width(ph_dest_width),.ph_cls_width(ph_cls_width),.ph_src_width(ph_src_width),.destination(destination),.pkt_class(pkt_class),.id(ID))
     u_char2noc(/*AUTOINST*/
                // Outputs
                .noc_flit               (noc_out_flit_cdc[33:0]), // Templated
                .noc_valid              (noc_out_valid_cdc),     // Templated
                .char_ready             (out_char_ready),        // Templated
                // Inputs
                .clk                    (clk),
                .rst                    (rst),
                .noc_ready              (noc_out_ready_cdc),     // Templated
                .char_valid             (out_char_valid),        // Templated
                .char_data              (out_char_data[7:0]));   // Templated


   /* uart_top AUTO_TEMPLATE(
    .srx_pad_i (srx_pad_i),
    .cts_pad_i (1'b1),
    .dsr_pad_i (1'b1),
    .ri_pad_i  (1'b1),
    .dcd_pad_i (1'b1),
    .rts_pad_o (),
    .dtr_pad_o (),
    .int_o     (wb_int_o),
    .baud_o    (),
    .wb_clk_i  (clk),
    .wb_rst_i  (rst),
    ); */

   uart_top
     #(/*AUTOINSTPARAM*/
       // Parameters
       .uart_data_width                 (uart_data_width),
       .uart_addr_width                 (uart_addr_width))
   u_uart (
`ifdef UART_HAS_BAUDRATE_OUTPUT
           .baud_o                    (),
`endif
           /*AUTOINST*/
           // Outputs
           .wb_dat_o                    (wb_dat_o[(uart_data_width)-1:0]),
           .wb_ack_o                    (wb_ack_o),
           .int_o                       (wb_int_o),              // Templated
           .stx_pad_o                   (stx_pad_o),
           .rts_pad_o                   (),                      // Templated
           .dtr_pad_o                   (),                      // Templated
           // Inputs
           .wb_clk_i                    (clk),                   // Templated
           .wb_rst_i                    (rst),                   // Templated
           .wb_adr_i                    (wb_adr_i[(uart_addr_width)-1:0]),
           .wb_dat_i                    (wb_dat_i[(uart_data_width)-1:0]),
           .wb_we_i                     (wb_we_i),
           .wb_stb_i                    (wb_stb_i),
           .wb_cyc_i                    (wb_cyc_i),
           .wb_sel_i                    (wb_sel_i[3:0]),
           .srx_pad_i                   (srx_pad_i),             // Templated
           .cts_pad_i                   (1'b1),                  // Templated
           .dsr_pad_i                   (1'b1),                  // Templated
           .ri_pad_i                    (1'b1),                  // Templated
           .dcd_pad_i                   (1'b1));                         // Templated

`ifdef OPTIMSOC_UART_LCD_ENABLE
   /* lcd_display AUTO_TEMPLATE(
    .data (lcd_data[]),
    .en   (lcd_en),
    .rw   (lcd_rw),
    .rs   (lcd_rs),
    .in_data (noc_in_flit[7:0]),
    .in_enable (lcd_in_valid),
    .in_pos (noc_in_flit[11:8]),
    .in_row (noc_in_flit[12]),
    ); */
   lcd_display
     u_lcd(/*AUTOINST*/
           // Outputs
           .data                        (lcd_data[3:0]),         // Templated
           .en                          (lcd_en),                // Templated
           .rw                          (lcd_rw),                // Templated
           .rs                          (lcd_rs),                // Templated
           // Inputs
           .clk                         (clk),
           .rst                         (rst),
           .in_data                     (noc_in_flit[7:0]),      // Templated
           .in_enable                   (lcd_in_valid),          // Templated
           .in_pos                      (noc_in_flit[11:8]),     // Templated
           .in_row                      (noc_in_flit[12]));      // Templated
`endif
endmodule // uart_tile

// Local Variables:
// verilog-auto-inst-param-value:t
// verilog-library-directories:("." "../../*/verilog")
// End:
