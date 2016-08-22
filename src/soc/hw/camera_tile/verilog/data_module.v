`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 12.06.2016 17:11:55
// Design Name: 
// Module Name: cam_data_module
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module data_module(
    // Inputs
    clk, rst, wb_stb_i,
    // External Inputs
    PCLK, VSYNC, HREF, D,
    // Outputs
    wb_dat_o, wb_ack_o, wb_rty_o, irq_new_frame, buffer_fifo_full, out_first_word
    );
    
    // WISHBONE bus width
    parameter wb_width = 32;
    // camera data bus width
    parameter d_width = 8;
    
    // Inputs
    input clk, rst, wb_stb_i;
    
    // External Inputs
    input PCLK, VSYNC, HREF;
    input [d_width-1:0] D;
    
    // Outputs
    output wb_ack_o, wb_rty_o;
    output [wb_width-1:0] wb_dat_o;
    output irq_new_frame;
    output buffer_fifo_full;
    output [1:0] out_first_word;
    
    wire w_full;
    wire w_empty;
    wire [d_width+1:0] w_rd_data;
    wire w_wr_en_FIFO;
    wire w_rd_en_FIFO;
    wire w_word_valid;
    wire [wb_width-1:0] w_pixels;
    wire w_in_ready;
    wire w_out_valid;
	wire w_irq_new_frame;
    //wire [17:0] w_out_address;
    
    
    assign irq_new_frame = w_irq_new_frame;

    
    cdc_fifo 
      #(.DW(10))
      inst_cdc_fifo (
        .wr_full(w_full),
        .rd_empty(w_empty),
        .rd_data(w_rd_data),
        .wr_clk(PCLK),
        .rd_clk(clk),
        .wr_rst(!rst),
        .rd_rst(!rst),
        .rd_en(w_wr_en_FIFO),
        .wr_en(w_rd_en_FIFO),
        .wr_data({HREF, VSYNC, D})
         );
    
    capture_FSM inst_capture_FSM (
        .clk(clk),
        .rst(rst),
        .start(1'b1),   // auf ein Register Bit von Camera Control Module legen
        .empty(w_empty),
        .full(w_full),
        .href(w_rd_data[9]),
        .vsync(w_rd_data[8]),
        .d(w_rd_data[d_width-1:0]),
        .wr_en_FIFO(w_wr_en_FIFO),
        .rd_en_FIFO(w_rd_en_FIFO),
        .irq_new_frame(w_irq_new_frame),
        .pixel_buffer(w_pixels),
        .word_valid(w_word_valid)
        //.addr(w_out_address)
         );
         
    assign buffer_fifo_full = !w_in_ready;
    assign wb_rty_o = !w_out_valid;
    assign wb_ack_o = w_out_valid;     

    lisnoc_fifo_cam inst_fifo (
        .in_ready(w_in_ready), 
        .out_flit(wb_dat_o),
        .out_valid(w_out_valid),
        .clk(clk),
        .rst(rst),
        .in_flit(w_pixels),
        .in_valid(w_word_valid),
        .out_ready(wb_stb_i), // read request PRUEFEN OB DAS FUNKTIONIERT, d. h. ob nicht ein Takt Versatz Probleme macht
		.irq_new_frame(w_irq_new_frame),
        .out_first_word(out_first_word)
        );         
         
         
        
endmodule
