`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 24.06.2016 09:39:28
// Design Name: 
// Module Name: camera_module
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


module camera_module(
    // Inputs
    clk, rst, wb_cyc_i, wb_dat_i, wb_stb_i, wb_adr_i, wb_we_i, wb_sel_i,
    // External Inputs
    PCLK, VSYNC, HREF, D,
    // Outputs
    wb_dat_o, wb_ack_o, wb_err_o, wb_rty_o, irq_new_frame, config_finished, out_first_word,
    // External Outputs
    SIOC, SIOD, RESET, PWDN, XVCLK

    );
    
    // WISHBONE bus width
    parameter wb_width = 32;
    // camera data bus width
    parameter d_width = 8;
    
    input clk, rst;
    input wb_cyc_i, wb_stb_i, wb_we_i;
    input [wb_width-1:0] wb_adr_i;
    input [wb_width-1:0] wb_dat_i;
    input [3:0] wb_sel_i;
    
    input PCLK, VSYNC, HREF;
    input [d_width-1:0] D; 
    
    output [wb_width-1:0] wb_dat_o;
    output wb_ack_o, wb_err_o, wb_rty_o;
    output irq_new_frame;
    output [1:0] out_first_word;
    output config_finished;
    output SIOC, RESET, PWDN, XVCLK;
    inout SIOD;
    
    
    // deactivate camera reset
    assign RESET = 1;
    // deactivate camera power down
    assign PWDN = 0;
    
    // wb_err_o not used
    assign wb_err_o = 1'b0;
    
    wire data_stb_i;
    wire ctrl_stb_i;
    wire data_ack_o;
    wire ctrl_ack_o;
    wire data_rty_o;
    wire ctrl_rty_o;
    wire [wb_width-1:0] data_dat_o;
    wire [wb_width-1:0] ctrl_dat_o;
       
    // wb_adr_i[7:0] decides if control_module or data_module is addressed
    // wb_adr_i[7:0] = 0x00 to 0x7F => control_module
    // wb_adr_i[7:0] = 0x80 to 0xFF => data_module
    
    assign data_stb_i = (wb_adr_i[7] == 1) ? wb_stb_i : 1'b0;
    assign ctrl_stb_i = (wb_adr_i[7] == 1) ? 1'b0 : wb_stb_i;
    assign wb_ack_o = (wb_adr_i[7] == 1) ? data_ack_o : ctrl_ack_o;
    assign wb_dat_o = (wb_adr_i[7] == 1) ? data_dat_o : ctrl_dat_o;
    assign wb_rty_o = (wb_adr_i[7] == 1) ? data_rty_o : ctrl_rty_o;
    
   
    control_module inst_control_module (
        .clk(clk),
        .rst(rst),
        .wb_we_i(wb_we_i),
        .wb_stb_i(ctrl_stb_i),
        .wb_dat_i(wb_dat_i),
        .wb_ack_o(ctrl_ack_o),
        .wb_rty_o(ctrl_rty_o),
        .wb_dat_o(ctrl_dat_o),
        .sioc(SIOC),
        .siod(SIOD),
        .config_finished(config_finished)
        );

    data_module inst_data_module (
        .clk(clk),
        .rst(rst),
        .wb_stb_i(data_stb_i),
        .PCLK(PCLK),
        .VSYNC(VSYNC),
        .HREF(HREF),
        .D(D),
        .wb_dat_o(data_dat_o),
        .wb_ack_o(data_ack_o),
        .wb_rty_o(data_rty_o),
        .irq_new_frame(irq_new_frame),
        .buffer_fifo_full(),
        .out_first_word
        );

    /*    
    clk_wiz_0 xvclk_gen (
        // Clock in ports
        .clk_in1(clk), // IMPORTANT: input clk_in1 set to 100 MHz at the moment
        // Clock out ports
        .clk_out1(XVCLK), // output clk_out1 set to 24 MHz at the moment
        // Status and control signals
        .reset(rst) // input reset     
        );
	*/          
        
    
    
  
endmodule
