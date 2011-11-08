`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    23:13:58 07/11/2010 
// Design Name: 
// Module Name:    wb_exclusive_adapter 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module wb_cas_unit(
    input clk_i,
    input rst_i,
    input [31:0] wb_core_dat_i,
    input [31:0] wb_core_adr_i,
    output [31:0] wb_core_dat_o,
    input [3:0] wb_core_sel_i,
    input wb_core_we_i,
    input wb_core_cyc_i,
    input wb_core_stb_i,
    output wb_core_ack_o,
    output wb_core_rty_o,
    output wb_core_err_o,
    output [31:0] wb_bus_dat_o,
    output [31:0] wb_bus_adr_o,
    input [31:0] wb_bus_dat_i,
    output [3:0] wb_bus_sel_o,
    output wb_bus_we_o,
    input wb_bus_ack_i,
    input wb_bus_rty_i,
    input wb_bus_err_i,
    output wb_bus_cyc_o,
    output wb_bus_stb_o
    );

wire bypass;

wire [31:0] fsm_core_dat_i;
wire [31:0] fsm_core_adr_i;
wire [3:0]  fsm_core_sel_i;
wire        fsm_core_cyc_i;
wire        fsm_core_stb_i;
wire        fsm_core_we_i;
wire  [31:0] fsm_core_dat_o;
wire         fsm_core_ack_o;
wire         fsm_core_rty_o;
wire         fsm_core_err_o;

wire  [31:0] fsm_bus_dat_o;
wire  [31:0] fsm_bus_adr_o;
wire  [3:0]  fsm_bus_sel_o;
wire         fsm_bus_cyc_o;
wire         fsm_bus_stb_o;
wire         fsm_bus_we_o;
wire [31:0] fsm_bus_dat_i;
wire        fsm_bus_ack_i;
wire        fsm_bus_rty_i;
wire        fsm_bus_err_i;

wb_cas_fsm cas_fsm(
    .clk_i(clk_i),
    .rst_i(rst_i),
    .core_adr_i(fsm_core_adr_i),
	 .core_dat_i(fsm_core_dat_i),
	 .core_sel_i(fsm_core_sel_i),
    .core_we_i(fsm_core_we_i),
    .core_cyc_i(fsm_core_cyc_i),
    .core_stb_i(fsm_core_stb_i),
    .core_dat_o(fsm_core_dat_o),
    .core_ack_o(fsm_core_ack_o),
    .core_err_o(fsm_core_err_o),
    .core_rty_o(fsm_core_rty_o),
    .bus_adr_o(fsm_bus_adr_o),
    .bus_dat_o(fsm_bus_dat_o),
    .bus_sel_o(fsm_bus_sel_o),
    .bus_we_o(fsm_bus_we_o),
    .bus_cyc_o(fsm_bus_cyc_o),
    .bus_stb_o(fsm_bus_stb_o),
    .bus_ack_i(fsm_bus_ack_i),
    .bus_dat_i(fsm_bus_dat_i),
    .bus_err_i(fsm_bus_err_i),
    .bus_rty_i(fsm_bus_rty_i)
    );

assign bypass = ~(wb_core_adr_i == 32'h7ffffffc);

assign fsm_core_adr_i = wb_core_adr_i;
assign fsm_core_dat_i = wb_core_dat_i;
assign fsm_core_sel_i = wb_core_sel_i;
assign fsm_core_we_i  = wb_core_we_i;
assign fsm_core_cyc_i = bypass ? 1'b0 : wb_core_cyc_i;
assign fsm_core_stb_i = bypass ? 1'b0 : wb_core_stb_i;

assign fsm_bus_dat_i = wb_bus_dat_i;
assign fsm_bus_ack_i = bypass ? 1'b0 : wb_bus_ack_i;
assign fsm_bus_err_i = bypass ? 1'b0 : wb_bus_err_i;
assign fsm_bus_rty_i = bypass ? 1'b0 : wb_bus_rty_i;

assign wb_core_dat_o = bypass ? wb_bus_dat_i : fsm_core_dat_o;
assign wb_core_ack_o = bypass ? wb_bus_ack_i : fsm_core_ack_o;
assign wb_core_err_o = bypass ? wb_bus_err_i : fsm_core_err_o;
assign wb_core_rty_o = bypass ? wb_bus_rty_i : fsm_core_rty_o;

assign wb_bus_cyc_o = bypass ? wb_core_cyc_i : fsm_bus_cyc_o;
assign wb_bus_stb_o = bypass ? wb_core_stb_i : fsm_bus_stb_o;
assign wb_bus_adr_o = bypass ? wb_core_adr_i : fsm_bus_adr_o;
assign wb_bus_sel_o = bypass ? wb_core_sel_i : fsm_bus_sel_o;
assign wb_bus_dat_o = bypass ? wb_core_dat_i : fsm_bus_dat_o;
assign wb_bus_we_o  = bypass ? wb_core_we_i  : fsm_bus_we_o;
									
endmodule
