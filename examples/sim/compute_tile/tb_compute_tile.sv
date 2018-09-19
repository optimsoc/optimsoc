/* Copyright (c) 2012-2015 by the author(s)
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
 * A testbench for a simple systems with only one compute tile
 *
 * Parameters:
 *   USE_DEBUG:
 *     Enable the OSD-based debug system.
 *
 *   NUM_CORES:
 *     Number of CPU cores inside the compute tile (default: 1)
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

module tb_compute_tile
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

   // Simulation parameters
   parameter USE_DEBUG = 0;
   parameter integer NUM_CORES = 1;
   parameter integer LMEM_SIZE = 128*1024*1024;

   localparam base_config_t
     BASE_CONFIG = '{ NUMTILES: 1,
                      NUMCTS: 1,
                      CTLIST: {{63{16'hx}}, 16'h0},
                      CORES_PER_TILE: NUM_CORES,
                      GMEM_SIZE: 0,
                      GMEM_TILE: 0,
                      NOC_ENABLE_VCHANNELS: 0,
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
                      CORE_ENABLE_FPU: 0,
                      CORE_ENABLE_PERFCOUNTERS: 0,
                      NA_ENABLE_MPSIMPLE: 1,
                      NA_ENABLE_DMA: 1,
                      NA_DMA_GENIRQ: 1,
                      NA_DMA_ENTRIES: 4,
                      USE_DEBUG: 1'(USE_DEBUG),
                      DEBUG_STM: 1,
                      DEBUG_CTM: 1,
                      DEBUG_DEM_UART: 1,
                      DEBUG_SUBNET_BITS: 6,
                      DEBUG_LOCAL_SUBNET: 0,
                      DEBUG_ROUTER_BUFFER_SIZE: 4,
                      DEBUG_MAX_PKT_LEN: 12
                      };

   localparam config_t CONFIG = derive_config(BASE_CONFIG);

   logic             rst_sys, rst_cpu;

   logic             cpu_stall;
   assign cpu_stall = 0;

   // In Verilator, we feed clk and rst from the C++ toplevel, in ModelSim & Co.
   // these signals are generated inside this testbench.
`ifndef verilator
   reg               clk;
   reg               rst;
`endif

   wire [CONFIG.NOC_CHANNELS-1:0][CONFIG.NOC_FLIT_WIDTH-1:0] noc_in_flit;
   wire [CONFIG.NOC_CHANNELS-1:0]                            noc_in_last;
   wire [CONFIG.NOC_CHANNELS-1:0]                            noc_in_valid;
   wire [CONFIG.NOC_CHANNELS-1:0]                            noc_in_ready;
   wire [CONFIG.NOC_CHANNELS-1:0][CONFIG.NOC_FLIT_WIDTH-1:0] noc_out_flit;
   wire [CONFIG.NOC_CHANNELS-1:0]                            noc_out_last;
   wire [CONFIG.NOC_CHANNELS-1:0]                            noc_out_valid;
   wire [CONFIG.NOC_CHANNELS-1:0]                            noc_out_ready;

   assign noc_in_flit   = {CONFIG.NOC_FLIT_WIDTH*CONFIG.NOC_CHANNELS{1'bx}};
   assign noc_in_last   = {CONFIG.NOC_CHANNELS{1'bx}};
   assign noc_in_valid  = {CONFIG.NOC_CHANNELS{1'b0}};
   assign noc_out_ready = {CONFIG.NOC_CHANNELS{1'b0}};

   // Monitor system behavior in simulation
   mor1kx_trace_exec [NUM_CORES-1:0] trace;
   assign trace = u_compute_tile.trace;

   logic [31:0]                                              trace_r3 [0:NUM_CORES-1];

   wire [NUM_CORES-1:0]                                      termination;

   genvar                                                    i;
   generate
      for (i = 0; i < NUM_CORES; i = i + 1) begin
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
             .STDOUT_FILENAME({"stdout.",index2string(i)}),
             .TRACEFILE_FILENAME({"trace.",index2string(i)}),
             .ENABLE_TRACE(0),
             .ID(i),
             .TERM_CROSS_NUM(NUM_CORES)
             )
         u_mon0(
                .termination            (termination[i]),
                .clk                    (clk),
                .enable                 (trace[i].valid),
                .wb_pc                  (trace[i].pc),
                .wb_insn                (trace[i].insn),
                .r3                     (trace_r3[i]),
                .termination_all        (termination)
                );
      end
   endgenerate

   // OSD-based debug system
   dii_flit [1:0] debug_ring_in;
   dii_flit [1:0] debug_ring_out;
   logic [1:0] debug_ring_in_ready;
   logic [1:0] debug_ring_out_ready;

   generate
      if (CONFIG.USE_DEBUG == 1) begin
         glip_channel c_glip_in(.*);
         glip_channel c_glip_out(.*);

         logic com_rst, logic_rst;

         // TCP communication interface (simulation only)
         glip_tcp_toplevel
           u_glip(
                  .*,
                  .clk_io    (clk),
                  .clk_logic (clk),
                  .fifo_in   (c_glip_in),
                  .fifo_out  (c_glip_out)
                  );

         // System Interface
         debug_interface
           #(
             .SYSTEM_VENDOR_ID   (2),
             .SYSTEM_DEVICE_ID   (1),
             .NUM_MODULES (CONFIG.DEBUG_NUM_MODS),
             .SUBNET_BITS (CONFIG.DEBUG_SUBNET_BITS),
             .LOCAL_SUBNET (CONFIG.DEBUG_LOCAL_SUBNET),
             .MAX_PKT_LEN (CONFIG.DEBUG_MAX_PKT_LEN),
             .DEBUG_ROUTER_BUFFER_SIZE (CONFIG.DEBUG_ROUTER_BUFFER_SIZE)
             )
   u_debuginterface(
                    .clk           (clk),
                    .rst           (rst),

                    .sys_rst       (rst_sys),
                    .cpu_rst       (rst_cpu),

                    .glip_in       (c_glip_in),
                    .glip_out      (c_glip_out),

                    .ring_out       (debug_ring_in),
                    .ring_out_ready (debug_ring_in_ready),
                    .ring_in        (debug_ring_out),
                    .ring_in_ready  (debug_ring_out_ready)
                    );
end
endgenerate

   // Reset signals
   // In simulations with debug system, these signals can be triggered through
   // the host software. In simulations without debug systems, we only rely on
   // the global reset signal.
   generate
      if (USE_DEBUG == 0) begin
         assign rst_sys = rst;
         assign rst_cpu = rst;
      end
   endgenerate


   // The actual system: a single compute tile
   compute_tile_dm
     #(.CONFIG(CONFIG),
       .ID(0),
       .MEM_FILE("ct.vmem"),
       .DEBUG_BASEID((CONFIG.DEBUG_LOCAL_SUBNET << (16 - CONFIG.DEBUG_SUBNET_BITS)) + 1))
   u_compute_tile(
                  // Debug ring ports
                  .debug_ring_in(debug_ring_in),
                  .debug_ring_in_ready(debug_ring_in_ready),
                  .debug_ring_out(debug_ring_out),
                  .debug_ring_out_ready(debug_ring_out_ready),
                  // Outputs
                  .noc_in_ready      (noc_in_ready),
                  .noc_out_flit      (noc_out_flit),
                  .noc_out_last      (noc_out_last),
                  .noc_out_valid     (noc_out_valid),
                  // Inputs
                  .clk               (clk),
                  .rst_cpu           (rst_cpu),
                  .rst_sys           (rst_sys),
                  .rst_dbg           (rst),
                  .noc_in_flit       (noc_in_flit),
                  .noc_in_last       (noc_in_last),
                  .noc_in_valid      (noc_in_valid),
                  .noc_out_ready     (noc_out_ready),

                  // Unused
                  .wb_ext_adr_i (),
                  .wb_ext_cyc_i (),
                  .wb_ext_dat_i (),
                  .wb_ext_sel_i (),
                  .wb_ext_stb_i (),
                  .wb_ext_we_i  (),
                  .wb_ext_cab_i (),
                  .wb_ext_cti_i (),
                  .wb_ext_bte_i (),
                  .wb_ext_ack_o ('0),
                  .wb_ext_rty_o ('0),
                  .wb_ext_err_o ('0),
                  .wb_ext_dat_o ('0)
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

endmodule

// Local Variables:
// verilog-library-directories:("." "../../../../src/rtl/*/verilog")
// verilog-auto-inst-param-value: t
// End:
