/* Copyright (c) 2015 by the author(s)
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
 * Clock manager for simulation targets
 *
 * To support fractions and multiples of the base clock, new clocks are
 * generated from the CLK_PERIOD parameter. Make sure that this parameter
 * matches the period of the clk input signal!
 *
 * No phase relationship of the output clocks with the input clock is specified.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

module clockmanager_sim(
`ifdef OPTIMSOC_CDC_DYNAMIC
   cdc_conf, cdc_enable,
`endif
`ifdef OPTIMSOC_IO_CLOCKS
   clk_io,
`endif
   /*AUTOARG*/
   // Outputs
   clk_ct, clk_dbg, clk_noc, clk_ddr, rst_sys, rst_cpu, sys_is_halted,
   // Inputs
   clk, rst, cpu_reset, cpu_start, sys_halt
   );

   // number of compute tile clocks
   parameter NUM_CT_CLOCKS = 1;

   // I/O clocks
`ifdef OPTIMSOC_IO_CLOCKS
   parameter NUM_IO_CLOCKS = 1; // range: [0;2]
`else
   parameter NUM_IO_CLOCKS = 0;
`endif

   parameter ENABLE_DDR_CLOCK = 0;

   parameter RST_WIDTH = 128;

   // period of the system clock, input "clk" [ns]
   parameter CLK_PERIOD = 5; // 200 MHz

`ifdef OPTIMSOC_IO_CLOCKS
   parameter IO_CLOCK0_MUL = 2;
   parameter IO_CLOCK0_DIV = 2;
   parameter IO_CLOCK1_MUL = 2;
   parameter IO_CLOCK1_DIV = 2;
`endif

   // parameters when using clock domains
   parameter CT_CLOCKS_MUL = 2;
   parameter CT_CLOCKS_DIV = 4;
   parameter DBG_CLOCK_MUL = 2;
   parameter DBG_CLOCK_DIV = 2;
   parameter NOC_CLOCK_MUL = 2;
   parameter NOC_CLOCK_DIV = 4;

   // parameters when no clock domains
   parameter CLOCK_MUL = 2;
   parameter CLOCK_DIV = 2;

   input clk;
   input rst;

   output [NUM_CT_CLOCKS-1:0] clk_ct;
`ifdef OPTIMSOC_IO_CLOCKS
   output [NUM_IO_CLOCKS-1:0] clk_io;
`endif
   output                     clk_dbg;
   output                     clk_noc;
   output                     clk_ddr;

   output                     rst_sys;
   output                     rst_cpu;

   input                      cpu_reset;
   input                      cpu_start;

   input                      sys_halt;

   output                     sys_is_halted;

`ifdef OPTIMSOC_CDC_DYNAMIC
   input [NUM_CT_CLOCKS*3-1:0] cdc_conf;
   input [NUM_CT_CLOCKS-1:0]   cdc_enable;
`endif

   wire [NUM_CT_CLOCKS-1:0]   clk_en_ct;
   wire                       clk_en_dbg;
   wire                       clk_en_noc;
   wire                       clk_en_ddr;

   reg                        hold_cpu;


`ifdef OPTIMSOC_CDC_DYNAMIC
   $fatal("OPTIMSOC_CDC_DYNAMIC is not supported in simulation at the moment.");
`endif

   // -------------------------------------------------------------------------
   // ------------------------------ RESET LOGIC ------------------------------
   // -------------------------------------------------------------------------
   genvar k;
   generate
      for (k=0;k<NUM_CT_CLOCKS;k=k+1) begin : gen_en_ct
         assign clk_en_ct[k] = ~sys_halt;
      end
`ifdef OPTIMSOC_IO_CLOCKS
      for (k=0;k<NUM_IO_CLOCKS;k=k+1) begin : gen_en_io
         assign clk_en_io[k] = 1'b1; // I/O is currently ungated!
      end
`endif
   endgenerate

   assign sys_is_halted = ~(&clk_en_ct);

   assign clk_en_noc = ~sys_halt;
   assign clk_en_dbg = 1'b1;
   assign clk_en_ddr = 1'b1;

   reg [RST_WIDTH-1:0] rst_pulse_sys;
   reg [RST_WIDTH-1:0] rst_pulse_cpu;

   assign rst_sys = |rst_pulse_sys;
   assign rst_cpu = |rst_pulse_cpu;

   // synchronize cpu_start to clk
   reg cpu_start_r;
   reg cpu_start_sync;
   always @(posedge clk or posedge cpu_start) begin
      if (cpu_start)
         {cpu_start_r, cpu_start_sync} <= 2'b11;
      else
         {cpu_start_r, cpu_start_sync} <= {cpu_start_sync, 1'b0};
   end

   always @(posedge clk) begin
      if (rst) begin
         rst_pulse_sys <= {RST_WIDTH{1'b1}};
      end else begin
         rst_pulse_sys <= {rst_pulse_sys[RST_WIDTH-2:0],1'b0};
      end
   end

   always @(posedge clk) begin
      if (rst|cpu_reset) begin
         rst_pulse_cpu <= {RST_WIDTH{1'b1}};
         hold_cpu <= 1'b1;
      end else begin
         if (hold_cpu) begin
            rst_pulse_cpu <= rst_pulse_cpu;
         end else begin
            rst_pulse_cpu <= {rst_pulse_cpu[RST_WIDTH-2:0],1'b0};
         end
         if (cpu_start_r) begin
            hold_cpu <= 1'b0;
         end else begin
            hold_cpu <= hold_cpu;
         end
      end
   end


   // -------------------------------------------------------------------------
   // ------------------------ CLOCKS: NO CLOCKDOMAINS ------------------------
   // -------------------------------------------------------------------------
`ifndef OPTIMSOC_CLOCKDOMAINS
   wire clk_gated_common;
   wire clk_ungated_common;

   clkmgr_sim
      #(.DIV(CLOCK_DIV),
        .MUL(CLOCK_MUL),
        .CLK_PERIOD(CLK_PERIOD))
      u_clkmgr_sim_ct(
         .clk(clk),
         .clk_out(clk_ungated_common),
         .rst(rst));

   assign clk_gated_common = clk_ungated_common & clk_en_ct[0];

   // compute tile clocks (gated)
   genvar i;
   generate
      for (i=0;i<NUM_CT_CLOCKS;i=i+1) begin : gen_ctclk
         assign clk_ct[i] = clk_gated_common;
      end
   endgenerate

   // NoC clock (gated)
   assign clk_noc = clk_gated_common;

   // Debug clock (ungated)
   assign clk_dbg = clk_ungated_common;
`endif


   // -------------------------------------------------------------------------
   // ----------------------- CLOCKS: WITH CLOCKDOMAINS -----------------------
   // -------------------------------------------------------------------------
`ifdef OPTIMSOC_CLOCKDOMAINS
   // compute tile clocks (gated)
   genvar i;
   generate
      for (i=0; i<NUM_CT_CLOCKS; i=i+1) begin : gen_ctclk
         wire clk_ct_ungated;

         clkmgr_sim
            #(.DIV(CT_CLOCKS_DIV),
              .MUL(CT_CLOCKS_MUL),
              .CLK_PERIOD(CLK_PERIOD))
            u_clkmgr_sim_ct(
               .clk(clk),
               .clk_out(clk_ct_ungated),
               .rst(rst));

         clk_ct[i] = clk_ct_ungated & clk_en_ct[i];
      end
   endgenerate

   // NoC clock (gated)
   wire clk_noc_ungated;

   clkmgr_sim
      #(.DIV(NOC_CLOCK_DIV),
        .MUL(NOC_CLOCK_MUL),
        .CLK_PERIOD(CLK_PERIOD))
      u_clkmgr_sim_ct(
         .clk(clk),
         .clk_out(clk_noc_ungated),
         .rst(rst));

   clk_noc = clk_noc_ungated & clk_en_noc;

   // Debug clock (ungated)
   clkmgr_sim
      #(.DIV(DBG_CLOCK_DIV),
        .MUL(DBG_CLOCK_MUL),
        .CLK_PERIOD(CLK_PERIOD))
      u_clkmgr_sim_ct(
         .clk(clk),
         .clk_out(clk_dbg),
         .rst(rst));
`endif


   // -------------------------------------------------------------------------
   // ------------------------------ CLOCKS: I/O ------------------------------
   // -------------------------------------------------------------------------
`ifdef OPTIMSOC_IO_CLOCKS
   genvar j;

   if (NUM_IO_CLOCKS == 1) begin
      wire clk_io_ungated;

      clkmgr_sim
         #(.DIV(IO_CLOCK0_DIV),
           .MUL(IO_CLOCK0_MUL),
           .CLK_PERIOD(CLK_PERIOD))
         u_clkmgr_sim_ct(
            .clk(clk),
            .clk_out(clk_io_ungated),
            .rst(rst));

      clk_io[0] = clk_io_ungated & clk_en_io[0];
   end

   // we use the parameters IO_CLOCK1_{DIV,MUL} for all subsequent clocks
   // (I/O clock 1..NUM_IO_CLOCKS-1)
   generate
      for (j=1; j<NUM_IO_CLOCKS; j=j+1) begin : gen_ioclk
         wire clk_io_ungated;

         clkmgr_sim
            #(.DIV(IO_CLOCK1_DIV),
              .MUL(IO_CLOCK1_MUL),
              .CLK_PERIOD(CLK_PERIOD))
            u_clkmgr_sim_ct(
               .clk(clk),
               .clk_out(clk_io_ungated),
               .rst(rst));

         clk_io[j] = clk_io_ungated & clk_en_io[j];
      end
   endgenerate
`endif


   // -------------------------------------------------------------------------
   // --------------------------- CLOCKS: DDR MEMORY --------------------------
   // -------------------------------------------------------------------------
   generate
      if (ENABLE_DDR_CLOCK) begin : gen_ddrclk
         //$fatal("ENABLE_DDR_CLOCK == 1 is not supported in simulation.");
      end else begin
         assign clk_ddr = 1'b0;
      end
   endgenerate

endmodule // clockmanager_sim
