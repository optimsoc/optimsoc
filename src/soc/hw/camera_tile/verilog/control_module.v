`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04.06.2016 16:38:11
// Design Name: 
// Module Name: OV2640_control_register
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


module control_module(
    // Inputs
    clk, rst, wb_dat_i, wb_we_i, wb_stb_i, // wb_cyc_i, wb_addr_i
    // Outputs
    wb_dat_o, wb_ack_o, wb_rty_o, sioc, siod, config_finished
    );
    
    parameter N = 32;
       
    input clk, rst;
    input wb_we_i;
    input wb_stb_i;
    input [N-1:0] wb_dat_i;
    
    output [N-1:0] wb_dat_o;
    output wb_ack_o;
    output wb_rty_o;
    output sioc;
    inout siod;
    
    output config_finished;
    
    wire en_mux_out;
    wire [N-1:0] d_mux_out;
    wire s_mux_rst_update;

    
    reg [N-1:0] control_reg = 0;
    

    assign en_mux_out = (s_mux_rst_update == 1'b0) ? (wb_stb_i & wb_we_i & !control_reg[0]) : 1'b1;
    assign d_mux_out = (s_mux_rst_update == 1'b0) ? wb_dat_i : (control_reg & 32'hfffffffe);
    
    //assign wb_ack_o = wb_stb_i & (!control_reg[0]);
    assign wb_ack_o = (control_reg[0] == 1) ? (wb_stb_i & !wb_we_i) : wb_stb_i;
    assign wb_dat_o = control_reg;
    // slave is only ready if UPDATE bit has been reset
    assign wb_rty_o = control_reg[0];
        
    always @ (posedge clk)
    begin
        if (rst == 1) begin
            control_reg <= 0;
        end else if (en_mux_out == 1) begin
            control_reg <= d_mux_out;
        end
    end 
    
    SCCB_FSM inst_SCCB_FSM (
        .clk(clk),
        .rst(rst),
        .UPDATE(control_reg[0]),
        .IMAGE_MODE(control_reg[2:1]),
        .RESOLUTION(control_reg[5:3]),
        .TEST_PATTERN(control_reg[6]),
        .CLKRC(control_reg[13:7]),
        .sioc(sioc),
        .siod(siod),
        .s_mux_rst_update(s_mux_rst_update),
        .config_finished(config_finished)
        //.wb_rty_o(wb_rty_o)
        );
               
endmodule
