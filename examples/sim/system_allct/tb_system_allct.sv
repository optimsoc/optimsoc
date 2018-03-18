/* Copyright (c) 2012-2018 by the author(s)
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
 * A testbench for a mesh-based system with distributed memory
 *
 * Parameters:
 *
 *   XDIM:
 *     Number tiles in x dimension (default: 2)
 *
 *   YDIM:
 *     Number tiles in y dimension (default: 2)
 *
 *   USE_DEBUG:
 *     Enable the OSD-based debug system.
 *
 *   NUM_CORES:
 *     Number of CPU cores inside each compute tile (default: 1)
 *
 *   LMEM_SIZE:
 *     Size of the local distributed memory in bytes (default: 32 MB)
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

`include "dbg_config.vh"

module tb_system_allct
  #(parameter integer XDIM = 2,
    parameter integer YDIM = 2,
    localparam TILES = XDIM*YDIM,
    parameter USE_DEBUG = 0,
    parameter ENABLE_VCHANNELS = 1*1,
    parameter integer NUM_CORES = 1*1, // bug in verilator would give a warning
    parameter integer LMEM_SIZE = 32*1024*1024
    )
    (
`ifdef verilator
     input clk,
     input rst
`endif
   );

   import dii_package::dii_flit;
   import opensocdebug::mor1kx_trace_exec;
   import optimsoc_config::*;
   import optimsoc_functions::*;

   localparam base_config_t
     BASE_CONFIG = '{ NUMTILES: TILES,
                      NUMCTS: TILES,
                      CTLIST: {{1024-TILES{1'b0}}, {TILES{1'b1}}},
                      CORES_PER_TILE: NUM_CORES,
                      GMEM_SIZE: 0,
                      GMEM_TILE: 'x,
                      NOC_ENABLE_VCHANNELS: ENABLE_VCHANNELS,
                      LMEM_SIZE: LMEM_SIZE,
                      LMEM_STYLE: PLAIN,
                      ENABLE_BOOTROM: 0,
                      BOOTROM_SIZE: 0,
                      ENABLE_DM: 1,
                      DM_BASE: 32'h0,
                      DM_SIZE: LMEM_SIZE,
                      ENABLE_PGAS: 0,
                      PGAS_BASE: 0,
                      PGAS_SIZE: 0,
                      NA_ENABLE_MPSIMPLE: 1,
                      NA_ENABLE_DMA: 1,
                      NA_DMA_GENIRQ: 1,
                      NA_DMA_ENTRIES: 4,
                      USE_DEBUG: 1'(USE_DEBUG),
                      DEBUG_STM: 1,
                      DEBUG_CTM: 1,
                      DEBUG_SUBNET_BITS: 6,
                      DEBUG_LOCAL_SUBNET: 0,
                      DEBUG_ROUTER_BUFFER_SIZE: 4,
                      DEBUG_MAX_PKT_LEN: 8
                      };

   localparam config_t CONFIG = derive_config(BASE_CONFIG);

   logic rst_sys, rst_cpu;

   logic cpu_stall;
   assign cpu_stall = 0;

// In Verilator, we feed clk and rst from the C++ toplevel, in ModelSim & Co.
// these signals are generated inside this testbench.
`ifndef verilator
   reg clk;
   reg rst;
`endif

   // Reset signals
   // In simulations with debug system, these signals can be triggered through
   // the host software. In simulations without debug systems, we only rely on
   // the global reset signal.
   generate
      if (CONFIG.USE_DEBUG == 0) begin : gen_use_debug_rst
         assign rst_sys = rst;
         assign rst_cpu = rst;
      end
   endgenerate

   glip_channel c_glip_in(.*);
   glip_channel c_glip_out(.*);

   logic com_rst, logic_rst;

   if (CONFIG.USE_DEBUG == 1) begin : gen_use_debug_glip
      // TCP communication interface (simulation only)
      glip_tcp_toplevel
        u_glip
          (
           .*,
           .clk_io    (clk),
           .clk_logic (clk),
           .fifo_in   (c_glip_in),
           .fifo_out  (c_glip_out)
           );
   end // if (CONFIG.USE_DEBUG == 1)

   // Monitor system behavior in simulation
   genvar t;
   genvar i;

   wire [CONFIG.NUMCTS*CONFIG.CORES_PER_TILE-1:0] termination;

   generate
      for (t = 0; t < CONFIG.NUMCTS; t = t + 1) begin : gen_tracemon_ct

         logic [31:0] trace_r3 [0:CONFIG.CORES_PER_TILE-1];
         mor1kx_trace_exec [CONFIG.CORES_PER_TILE-1:0] trace;
         assign trace = u_system.gen_ct[t].u_ct.trace;

         for (i = 0; i < CONFIG.CORES_PER_TILE; i = i + 1) begin : gen_tracemon_core
            r3_checker
               u_r3_checker(
                  .clk(clk),
                  .valid(trace[i].valid),
                  .we (trace[i].wben),
                  .addr (trace[i].wbreg),
                  .data (trace[i].wbdata),
                  .r3 (trace_r3[i])
               );

            trace_monitor
               #(
                  .STDOUT_FILENAME({"stdout.",index2string((t*CONFIG.CORES_PER_TILE)+i)}),
                  .TRACEFILE_FILENAME({"trace.",index2string((t*CONFIG.CORES_PER_TILE)+i)}),
                  .ENABLE_TRACE(0),
                  .ID((t*CONFIG.CORES_PER_TILE)+i),
                  .TERM_CROSS_NUM(CONFIG.NUMCTS*CONFIG.CORES_PER_TILE)
               )
               u_mon0(
                  .termination            (termination[(t*CONFIG.CORES_PER_TILE)+i]),
                  .clk                    (clk),
                  .enable                 (trace[i].valid),
                  .wb_pc                  (trace[i].pc),
                  .wb_insn                (trace[i].insn),
                  .r3                     (trace_r3[i]),
                  .termination_all        (termination)
              );
         end

      end
   endgenerate

   system_allct
     #(.CONFIG(CONFIG), .XDIM(2), .YDIM(2))
   u_system
     (.clk (clk),
      .rst (rst | logic_rst),
      .c_glip_in (c_glip_in),
      .c_glip_out (c_glip_out),

      .wb_ext_ack_o ('x),
      .wb_ext_err_o ('x),
      .wb_ext_rty_o ('x),
      .wb_ext_dat_o ('x),
      .wb_ext_adr_i (),
      .wb_ext_cyc_i (),
      .wb_ext_dat_i (),
      .wb_ext_sel_i (),
      .wb_ext_stb_i (),
      .wb_ext_we_i (),
      .wb_ext_cab_i (),
      .wb_ext_cti_i (),
      .wb_ext_bte_i ()
      );

// Generate testbench signals.
// In Verilator, these signals are generated in the C++ toplevel testbench
`ifndef verilator
   initial begin
      clk = 1'b1;
      rst = 1'b1;
      #15;
      rst = 1'b0;
   end

   always clk = #1.25 ~clk;
`endif

   export "DPI-C" function getXDIM;

   function integer getXDIM();
     getXDIM = XDIM;
   endfunction

   export "DPI-C" function getYDIM;

   function integer getYDIM();
     getYDIM = YDIM;
   endfunction

endmodule

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
