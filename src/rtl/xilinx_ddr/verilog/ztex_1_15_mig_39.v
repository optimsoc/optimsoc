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
 * Synthesis wrapper file for the DDR2 MIG black box module 
 * (mig_39_wrapper.edf).
 *
 * (c) 2012 by the author(s)
 *
 * Author(s):
 *    Hans-Christian Wild, hans-christian.wild@mytum.de  
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */
 
module ztex_1_15_mig_39 (/*AUTOARG*/
   // Outputs
   mcb3_dram_a, mcb3_dram_ba, mcb3_dram_ras_n, mcb3_dram_cas_n,
   mcb3_dram_we_n, mcb3_dram_cke, mcb3_dram_dm, mcb3_dram_udm,
   c3_calib_done, c3_clk0, c3_rst0, mcb3_dram_ck, mcb3_dram_ck_n,
   c3_p0_cmd_empty, c3_p0_cmd_full, c3_p0_wr_full, c3_p0_wr_empty,
   c3_p0_wr_count, c3_p0_wr_underrun, c3_p0_wr_error, c3_p0_rd_data,
   c3_p0_rd_full, c3_p0_rd_empty, c3_p0_rd_count, c3_p0_rd_overflow,
   c3_p0_rd_error, c3_p1_cmd_empty, c3_p1_cmd_full, c3_p1_wr_full,
   c3_p1_wr_empty, c3_p1_wr_count, c3_p1_wr_underrun, c3_p1_wr_error,
   c3_p1_rd_data, c3_p1_rd_full, c3_p1_rd_empty, c3_p1_rd_count,
   c3_p1_rd_overflow, c3_p1_rd_error, c3_p2_cmd_empty, c3_p2_cmd_full,
   c3_p2_wr_full, c3_p2_wr_empty, c3_p2_wr_count, c3_p2_wr_underrun,
   c3_p2_wr_error, c3_p2_rd_data, c3_p2_rd_full, c3_p2_rd_empty,
   c3_p2_rd_count, c3_p2_rd_overflow, c3_p2_rd_error, c3_p3_cmd_empty,
   c3_p3_cmd_full, c3_p3_wr_full, c3_p3_wr_empty, c3_p3_wr_count,
   c3_p3_wr_underrun, c3_p3_wr_error, c3_p3_rd_data, c3_p3_rd_full,
   c3_p3_rd_empty, c3_p3_rd_count, c3_p3_rd_overflow, c3_p3_rd_error,
   // Inouts
   mcb3_dram_dq, mcb3_dram_udqs, mcb3_dram_udqs_n, mcb3_rzq, mcb3_zio,
   mcb3_dram_dqs, mcb3_dram_dqs_n,
   // Inputs
   c3_sys_clk, c3_sys_rst_i, c3_p0_cmd_clk, c3_p0_cmd_en,
   c3_p0_cmd_instr, c3_p0_cmd_bl, c3_p0_cmd_byte_addr, c3_p0_wr_clk,
   c3_p0_wr_en, c3_p0_wr_mask, c3_p0_wr_data, c3_p0_rd_clk,
   c3_p0_rd_en, c3_p1_cmd_clk, c3_p1_cmd_en, c3_p1_cmd_instr,
   c3_p1_cmd_bl, c3_p1_cmd_byte_addr, c3_p1_wr_clk, c3_p1_wr_en,
   c3_p1_wr_mask, c3_p1_wr_data, c3_p1_rd_clk, c3_p1_rd_en,
   c3_p2_cmd_clk, c3_p2_cmd_en, c3_p2_cmd_instr, c3_p2_cmd_bl,
   c3_p2_cmd_byte_addr, c3_p2_wr_clk, c3_p2_wr_en, c3_p2_wr_mask,
   c3_p2_wr_data, c3_p2_rd_clk, c3_p2_rd_en, c3_p3_cmd_clk,
   c3_p3_cmd_en, c3_p3_cmd_instr, c3_p3_cmd_bl, c3_p3_cmd_byte_addr,
   c3_p3_wr_clk, c3_p3_wr_en, c3_p3_wr_mask, c3_p3_wr_data,
   c3_p3_rd_clk, c3_p3_rd_en
   ) /* synthesis syn_black_box */;
   
   inout [15:0]         mcb3_dram_dq;
   output [12:0]        mcb3_dram_a;
   output [2:0]         mcb3_dram_ba;
   output               mcb3_dram_ras_n;
   output               mcb3_dram_cas_n;
   output               mcb3_dram_we_n;
   output               mcb3_dram_cke;
   output               mcb3_dram_dm;
   inout                mcb3_dram_udqs;
   inout                mcb3_dram_udqs_n;
   inout                mcb3_rzq;
   inout                mcb3_zio;
   output               mcb3_dram_udm;
   input                c3_sys_clk;
   input                c3_sys_rst_i;
   output               c3_calib_done;
   output               c3_clk0;
   output               c3_rst0;
   inout                mcb3_dram_dqs;
   inout                mcb3_dram_dqs_n;
   output               mcb3_dram_ck;
   output               mcb3_dram_ck_n;
   input                c3_p0_cmd_clk;
   input                c3_p0_cmd_en;
   input [2:0]          c3_p0_cmd_instr;
   input [5:0]          c3_p0_cmd_bl;
   input [29:0]         c3_p0_cmd_byte_addr;
   output               c3_p0_cmd_empty;
   output               c3_p0_cmd_full;
   input                c3_p0_wr_clk;
   input                c3_p0_wr_en;
   input [3:0]          c3_p0_wr_mask;
   input [31:0]         c3_p0_wr_data;
   output               c3_p0_wr_full;
   output               c3_p0_wr_empty;
   output [6:0]         c3_p0_wr_count;
   output               c3_p0_wr_underrun;
   output               c3_p0_wr_error;
   input                c3_p0_rd_clk;
   input                c3_p0_rd_en;
   output [31:0]        c3_p0_rd_data;
   output               c3_p0_rd_full;
   output               c3_p0_rd_empty;
   output [6:0]         c3_p0_rd_count;
   output               c3_p0_rd_overflow;
   output               c3_p0_rd_error;
   input                c3_p1_cmd_clk;
   input                c3_p1_cmd_en;
   input [2:0]          c3_p1_cmd_instr;
   input [5:0]          c3_p1_cmd_bl;
   input [29:0]         c3_p1_cmd_byte_addr;
   output               c3_p1_cmd_empty;
   output               c3_p1_cmd_full;
   input                c3_p1_wr_clk;
   input                c3_p1_wr_en;
   input [3:0]          c3_p1_wr_mask;
   input [31:0]         c3_p1_wr_data;
   output               c3_p1_wr_full;
   output               c3_p1_wr_empty;
   output [6:0]         c3_p1_wr_count;
   output               c3_p1_wr_underrun;
   output               c3_p1_wr_error;
   input                c3_p1_rd_clk;
   input                c3_p1_rd_en;
   output [31:0]        c3_p1_rd_data;
   output               c3_p1_rd_full;
   output               c3_p1_rd_empty;
   output [6:0]         c3_p1_rd_count;
   output               c3_p1_rd_overflow;
   output               c3_p1_rd_error;
   input                c3_p2_cmd_clk;
   input                c3_p2_cmd_en;
   input [2:0]          c3_p2_cmd_instr;
   input [5:0]          c3_p2_cmd_bl;
   input [29:0]         c3_p2_cmd_byte_addr;
   output               c3_p2_cmd_empty;
   output               c3_p2_cmd_full;
   input                c3_p2_wr_clk;
   input                c3_p2_wr_en;
   input [3:0]          c3_p2_wr_mask;
   input [31:0]         c3_p2_wr_data;
   output               c3_p2_wr_full;
   output               c3_p2_wr_empty;
   output [6:0]         c3_p2_wr_count;
   output               c3_p2_wr_underrun;
   output               c3_p2_wr_error;
   input                c3_p2_rd_clk;
   input                c3_p2_rd_en;
   output [31:0]        c3_p2_rd_data;
   output               c3_p2_rd_full;
   output               c3_p2_rd_empty;
   output [6:0]         c3_p2_rd_count;
   output               c3_p2_rd_overflow;
   output               c3_p2_rd_error;
   input                c3_p3_cmd_clk;
   input                c3_p3_cmd_en;
   input [2:0]          c3_p3_cmd_instr;
   input [5:0]          c3_p3_cmd_bl;
   input [29:0]         c3_p3_cmd_byte_addr;
   output               c3_p3_cmd_empty;
   output               c3_p3_cmd_full;
   input                c3_p3_wr_clk;
   input                c3_p3_wr_en;
   input [3:0]          c3_p3_wr_mask;
   input [31:0]         c3_p3_wr_data;
   output               c3_p3_wr_full;
   output               c3_p3_wr_empty;
   output [6:0]         c3_p3_wr_count;
   output               c3_p3_wr_underrun;
   output               c3_p3_wr_error;
   input                c3_p3_rd_clk;
   input                c3_p3_rd_en;
   output [31:0]        c3_p3_rd_data;
   output               c3_p3_rd_full;
   output               c3_p3_rd_empty;
   output [6:0]         c3_p3_rd_count;
   output               c3_p3_rd_overflow;
   output               c3_p3_rd_error;

endmodule 

// Local Variables:
// verilog-library-directories:("../../*/verilog/")
// verilog-auto-inst-param-value: t
// End: