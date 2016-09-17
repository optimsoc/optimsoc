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
 * A 4x4 distributed memory system with 16 compute tiles
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

`include "lisnoc_def.vh"

`include "dbg_config.vh"

module system_4x4_c_dm(
   /*AUTOARG*/
   // Outputs
   trace,
   // Inputs
   clk, rst_sys, rst_cpu
   );

   // NoC parameters
   parameter NOC_DATA_WIDTH = 32;
   parameter NOC_TYPE_WIDTH = 2;
   localparam NOC_FLIT_WIDTH = NOC_DATA_WIDTH + NOC_TYPE_WIDTH;
   parameter VCHANNELS = `VCHANNELS;

   // compute tile parameters
   parameter MEM_FILE = "ct.vmem";
   parameter MEM_SIZE = 1*1024*1024; // 1 MByte


   input clk, rst_sys, rst_cpu;

   output [`DEBUG_TRACE_EXEC_WIDTH*16-1:0] trace;

   // Flits from NoC->tiles
   wire [NOC_FLIT_WIDTH-1:0] link_in_flit[0:15];
   wire [VCHANNELS-1:0]      link_in_valid[0:15];
   wire [VCHANNELS-1:0]      link_in_ready[0:15];

   // Flits from tiles->NoC
   wire [NOC_FLIT_WIDTH-1:0] link_out_flit[0:15];
   wire [VCHANNELS-1:0]      link_out_valid[0:15];
   wire [VCHANNELS-1:0]      link_out_ready[0:15];

   /* lisnoc_mesh4x4 AUTO_TEMPLATE(
    .link\(.*\)_in_\(.*\)_.* (link_out_\2[\1]),
    .link\(.*\)_out_\(.*\)_.* (link_in_\2[\1]),
    .clk(clk),
    .rst(rst_sys),
    ); */
   lisnoc_mesh4x4
      #(.vchannels(VCHANNELS))
      u_mesh(/*AUTOINST*/
             // Outputs
             .link0_in_ready_o          (link_out_ready[0]),     // Templated
             .link0_out_flit_o          (link_in_flit[0]),       // Templated
             .link0_out_valid_o         (link_in_valid[0]),      // Templated
             .link1_in_ready_o          (link_out_ready[1]),     // Templated
             .link1_out_flit_o          (link_in_flit[1]),       // Templated
             .link1_out_valid_o         (link_in_valid[1]),      // Templated
             .link2_in_ready_o          (link_out_ready[2]),     // Templated
             .link2_out_flit_o          (link_in_flit[2]),       // Templated
             .link2_out_valid_o         (link_in_valid[2]),      // Templated
             .link3_in_ready_o          (link_out_ready[3]),     // Templated
             .link3_out_flit_o          (link_in_flit[3]),       // Templated
             .link3_out_valid_o         (link_in_valid[3]),      // Templated
             .link4_in_ready_o          (link_out_ready[4]),     // Templated
             .link4_out_flit_o          (link_in_flit[4]),       // Templated
             .link4_out_valid_o         (link_in_valid[4]),      // Templated
             .link5_in_ready_o          (link_out_ready[5]),     // Templated
             .link5_out_flit_o          (link_in_flit[5]),       // Templated
             .link5_out_valid_o         (link_in_valid[5]),      // Templated
             .link6_in_ready_o          (link_out_ready[6]),     // Templated
             .link6_out_flit_o          (link_in_flit[6]),       // Templated
             .link6_out_valid_o         (link_in_valid[6]),      // Templated
             .link7_in_ready_o          (link_out_ready[7]),     // Templated
             .link7_out_flit_o          (link_in_flit[7]),       // Templated
             .link7_out_valid_o         (link_in_valid[7]),      // Templated
             .link8_in_ready_o          (link_out_ready[8]),     // Templated
             .link8_out_flit_o          (link_in_flit[8]),       // Templated
             .link8_out_valid_o         (link_in_valid[8]),      // Templated
             .link9_in_ready_o          (link_out_ready[9]),     // Templated
             .link9_out_flit_o          (link_in_flit[9]),       // Templated
             .link9_out_valid_o         (link_in_valid[9]),      // Templated
             .link10_in_ready_o         (link_out_ready[10]),    // Templated
             .link10_out_flit_o         (link_in_flit[10]),      // Templated
             .link10_out_valid_o        (link_in_valid[10]),     // Templated
             .link11_in_ready_o         (link_out_ready[11]),    // Templated
             .link11_out_flit_o         (link_in_flit[11]),      // Templated
             .link11_out_valid_o        (link_in_valid[11]),     // Templated
             .link12_in_ready_o         (link_out_ready[12]),    // Templated
             .link12_out_flit_o         (link_in_flit[12]),      // Templated
             .link12_out_valid_o        (link_in_valid[12]),     // Templated
             .link13_in_ready_o         (link_out_ready[13]),    // Templated
             .link13_out_flit_o         (link_in_flit[13]),      // Templated
             .link13_out_valid_o        (link_in_valid[13]),     // Templated
             .link14_in_ready_o         (link_out_ready[14]),    // Templated
             .link14_out_flit_o         (link_in_flit[14]),      // Templated
             .link14_out_valid_o        (link_in_valid[14]),     // Templated
             .link15_in_ready_o         (link_out_ready[15]),    // Templated
             .link15_out_flit_o         (link_in_flit[15]),      // Templated
             .link15_out_valid_o        (link_in_valid[15]),     // Templated
             // Inputs
             .clk                       (clk),                   // Templated
             .rst                       (rst_sys),               // Templated
             .link0_in_flit_i           (link_out_flit[0]),      // Templated
             .link0_in_valid_i          (link_out_valid[0]),     // Templated
             .link0_out_ready_i         (link_in_ready[0]),      // Templated
             .link1_in_flit_i           (link_out_flit[1]),      // Templated
             .link1_in_valid_i          (link_out_valid[1]),     // Templated
             .link1_out_ready_i         (link_in_ready[1]),      // Templated
             .link2_in_flit_i           (link_out_flit[2]),      // Templated
             .link2_in_valid_i          (link_out_valid[2]),     // Templated
             .link2_out_ready_i         (link_in_ready[2]),      // Templated
             .link3_in_flit_i           (link_out_flit[3]),      // Templated
             .link3_in_valid_i          (link_out_valid[3]),     // Templated
             .link3_out_ready_i         (link_in_ready[3]),      // Templated
             .link4_in_flit_i           (link_out_flit[4]),      // Templated
             .link4_in_valid_i          (link_out_valid[4]),     // Templated
             .link4_out_ready_i         (link_in_ready[4]),      // Templated
             .link5_in_flit_i           (link_out_flit[5]),      // Templated
             .link5_in_valid_i          (link_out_valid[5]),     // Templated
             .link5_out_ready_i         (link_in_ready[5]),      // Templated
             .link6_in_flit_i           (link_out_flit[6]),      // Templated
             .link6_in_valid_i          (link_out_valid[6]),     // Templated
             .link6_out_ready_i         (link_in_ready[6]),      // Templated
             .link7_in_flit_i           (link_out_flit[7]),      // Templated
             .link7_in_valid_i          (link_out_valid[7]),     // Templated
             .link7_out_ready_i         (link_in_ready[7]),      // Templated
             .link8_in_flit_i           (link_out_flit[8]),      // Templated
             .link8_in_valid_i          (link_out_valid[8]),     // Templated
             .link8_out_ready_i         (link_in_ready[8]),      // Templated
             .link9_in_flit_i           (link_out_flit[9]),      // Templated
             .link9_in_valid_i          (link_out_valid[9]),     // Templated
             .link9_out_ready_i         (link_in_ready[9]),      // Templated
             .link10_in_flit_i          (link_out_flit[10]),     // Templated
             .link10_in_valid_i         (link_out_valid[10]),    // Templated
             .link10_out_ready_i        (link_in_ready[10]),     // Templated
             .link11_in_flit_i          (link_out_flit[11]),     // Templated
             .link11_in_valid_i         (link_out_valid[11]),    // Templated
             .link11_out_ready_i        (link_in_ready[11]),     // Templated
             .link12_in_flit_i          (link_out_flit[12]),     // Templated
             .link12_in_valid_i         (link_out_valid[12]),    // Templated
             .link12_out_ready_i        (link_in_ready[12]),     // Templated
             .link13_in_flit_i          (link_out_flit[13]),     // Templated
             .link13_in_valid_i         (link_out_valid[13]),    // Templated
             .link13_out_ready_i        (link_in_ready[13]),     // Templated
             .link14_in_flit_i          (link_out_flit[14]),     // Templated
             .link14_in_valid_i         (link_out_valid[14]),    // Templated
             .link14_out_ready_i        (link_in_ready[14]),     // Templated
             .link15_in_flit_i          (link_out_flit[15]),     // Templated
             .link15_in_valid_i         (link_out_valid[15]),    // Templated
             .link15_out_ready_i        (link_in_ready[15]));    // Templated

   genvar i;
   generate
      for (i=0; i<16; i=i+1) begin : gen_ct
         compute_tile_dm
            #(.ID(i),
              .MEM_SIZE(MEM_SIZE),
              .MEM_FILE(MEM_FILE))
            u_ct(// Outputs
                 .trace                      (trace[(`DEBUG_TRACE_EXEC_WIDTH*(i+1))-1:`DEBUG_TRACE_EXEC_WIDTH*i]),
                 .noc_in_ready               (link_in_ready[i][VCHANNELS-1:0]),
                 .noc_out_flit               (link_out_flit[i][NOC_FLIT_WIDTH-1:0]),
                 .noc_out_valid              (link_out_valid[i][VCHANNELS-1:0]),
                 // Inputs
                 .clk                        (clk),
                 .rst_cpu                    (rst_cpu),
                 .rst_sys                    (rst_sys),
                 .noc_in_flit                (link_in_flit[i][NOC_FLIT_WIDTH-1:0]),
                 .noc_in_valid               (link_in_valid[i][VCHANNELS-1:0]),
                 .noc_out_ready              (link_out_ready[i][VCHANNELS-1:0]),
                 .cpu_stall                  (1'b0));
      end
   endgenerate

endmodule

`include "lisnoc_undef.vh"

// Local Variables:
// verilog-library-directories:("../../../../../lisnoc/rtl/meshs/" "../../*/verilog")
// verilog-auto-inst-param-value: t
// End:
