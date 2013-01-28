/**
 * This file is part of OpTiMSoC.
 * 
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation, either version 3 of 
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 * 
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 * 
 * =================================================================
 * 
 * This is the compare-and-swap (CAS) unit as instantiated between the
 * processor and the bus. It is accessed via memory mapped registers
 * and performs the CAS operation.
 * 
 * (c) 2009-2013 by the author(s)
 * 
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

module wb_cas_unit(
    input clk_i,
    input rst_i,
    input [31:0] wb_core_dat_i,
    input [31:0] wb_core_adr_i,
    output [31:0] wb_core_dat_o,
    input [3:0] wb_core_sel_i,
    input [1:0] wb_core_bte_i,
    input [2:0] wb_core_cti_i,
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
    output [1:0] wb_bus_bte_o,
    output [2:0] wb_bus_cti_o,
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
   assign wb_bus_bte_o = bypass ? wb_core_bte_i : 2'b00;
   assign wb_bus_cti_o = bypass ? wb_core_cti_i : 3'b000;
                                                                        
endmodule
