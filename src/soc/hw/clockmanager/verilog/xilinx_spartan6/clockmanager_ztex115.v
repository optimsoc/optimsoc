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
 * Clock manager for the ZTEX 1.15b/d boards
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

// TODO:
//
// - add io_locked to reset (not directly, as this leads to errors for
//  NUM_IO_CLOCKS==0)
module clockmanager_ztex115(
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

   // general parameters
   parameter NUM_CT_CLOCKS = 1;
`ifdef OPTIMSOC_IO_CLOCKS
   parameter NUM_IO_CLOCKS = 1;
`endif

   parameter ENABLE_DDR_CLOCK = 0;

   parameter RST_WIDTH = 128;

   // parameters when using clock domains
   parameter CT_CLOCKS_MUL = 2;
   parameter CT_CLOCKS_DIV = 4;
   parameter DBG_CLOCK_MUL = 2;
   parameter DBG_CLOCK_DIV = 2;
   parameter NOC_CLOCK_MUL = 3;
   parameter NOC_CLOCK_DIV = 4;

`ifdef OPTIMSOC_IO_CLOCKS
   parameter IO_CLOCK0_MUL = 2;
   parameter IO_CLOCK0_DIV = 2;
   parameter IO_CLOCK1_MUL = 2;
   parameter IO_CLOCK1_DIV = 2;
`endif

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

   wire                       clk_buffered;

`ifdef OPTIMSOC_CLOCKDOMAINS
   wire [NUM_CT_CLOCKS-1:0]   clk_ct_unbuffered;
   wire                       clk_dbg_unbuffered;
   wire                       clk_noc_unbuffered;
`endif

   wire                       clk_ddr_unbuffered;

   wire [NUM_CT_CLOCKS-1:0]   clk_ct_locked;
   wire                       clk_dbg_locked;
   wire                       clk_noc_locked;
   wire                       clk_ddr_locked;
   
   wire [NUM_CT_CLOCKS-1:0]   clk_en_ct;
   wire                       clk_en_dbg;
   wire                       clk_en_noc;
   wire                       clk_en_ddr;

`ifdef OPTIMSOC_IO_CLOCKS
   wire [NUM_IO_CLOCKS-1:0]   clk_io_unbuffered;
   wire [NUM_IO_CLOCKS-1:0]   clk_io_locked;
   wire [NUM_IO_CLOCKS-1:0]   clk_en_io;
`endif

   reg                        hold_cpu;

   genvar i;
   generate
      for (i=0;i<NUM_CT_CLOCKS;i=i+1) begin : gen_en_ct
         assign clk_en_ct[i] = clk_ct_locked[i] & ~sys_halt;
      end
`ifdef OPTIMSOC_IO_CLOCKS
      for (i=0;i<NUM_IO_CLOCKS;i=i+1) begin : gen_en_io
         assign clk_en_io[i] = clk_io_locked[i];
      end
`endif
   endgenerate

   assign sys_is_halted = ~(&clk_en_ct);

   assign clk_en_noc = clk_noc_locked & ~sys_halt;
   assign clk_en_dbg = clk_dbg_locked;
   assign clk_en_ddr = clk_ddr_locked;

   reg [RST_WIDTH-1:0] rst_pulse_sys;
   reg [RST_WIDTH-1:0] rst_pulse_cpu;

   assign rst_sys = |rst_pulse_sys;
   assign rst_cpu = |rst_pulse_cpu;

   reg                 rst_locked;

   always @(posedge clk_buffered) begin
      if (rst) begin
         rst_locked <= 1'b1;
      end else begin
         if (rst_locked) begin
            if (&clk_ct_locked & clk_noc_locked & clk_dbg_locked) begin
               rst_locked <= 1'b0;
            end else begin
               rst_locked <= 1'b1;
            end
         end else begin
            rst_locked <= 1'b0;
         end
      end
   end

   always @(posedge clk_buffered) begin
      if (rst_locked) begin
         rst_pulse_sys <= {RST_WIDTH{1'b1}};
      end else begin
         rst_pulse_sys <= {rst_pulse_sys[RST_WIDTH-2:0],1'b0};
      end
   end

   always @(posedge clk_buffered) begin
      if (rst_locked|cpu_reset) begin
         rst_pulse_cpu <= {RST_WIDTH{1'b1}};
         hold_cpu <= 1'b1;
      end else begin
         if (hold_cpu) begin
            rst_pulse_cpu <= rst_pulse_cpu;
         end else begin
            rst_pulse_cpu <= {rst_pulse_cpu[RST_WIDTH-2:0],1'b0};
         end
         if (cpu_start) begin
            hold_cpu <= 1'b0;
         end else begin
            hold_cpu <= hold_cpu;
         end
      end
   end

   IBUFG
      u_clk_ibufg(.O(clk_buffered),
                  .I(clk));

`ifndef OPTIMSOC_CLOCKDOMAINS
   wire clk_common_unbuffered;
   wire clk_gated_common;
   wire clk_ungated_common;

   BUFGCE
     u_clk_bufg_gated(.O(clk_gated_common),
                      .CE(clk_en_ct[0]),
                      .I(clk_common_unbuffered));

   BUFG
     u_clk_bufg_ungated(.O(clk_ungated_common),
                        .I(clk_common_unbuffered));

   DCM_CLKGEN
     #(.CLKFX_MULTIPLY(CLOCK_MUL),
       .CLKFX_DIVIDE(CLOCK_DIV),
       .CLKIN_PERIOD(20.83333),
       .SPREAD_SPECTRUM("NONE"),
       .STARTUP_WAIT("FALSE"),
       .CLKFX_MD_MAX(0.000))
   u_dcm(.CLKIN(clk_buffered),
         .CLKFX(clk_common_unbuffered),
         .RST(1'b0),
         .CLKFXDV (),
         .CLKFX180 (),
         .LOCKED(clk_noc_locked),
         .FREEZEDCM(1'b0),
         .PROGCLK(1'b0),
         .PROGDATA(1'b0),
         .PROGEN(1'b0),
         .STATUS(),
         .PROGDONE());

   generate
      for (i=0;i<NUM_CT_CLOCKS;i=i+1) begin : genctclocks
         assign clk_ct_locked[i] = 1'b1;
         assign clk_ct[i] = clk_gated_common;
      end
   endgenerate

//   assign clk_noc_locked = 1'b1; // Set by the DCM
   assign clk_noc = clk_gated_common;
   assign clk_dbg_locked = 1'b1;
   assign clk_dbg = clk_ungated_common;
`else
   generate
      for (i=0;i<NUM_CT_CLOCKS;i=i+1) begin : genctclocks
         wire rst_dcm;
         wire progen;
         wire progdata;
         wire progclk;
         wire progdone;
         wire [1:0] status;
         wire rdy_clk;
         wire [7:0] multi;
         wire [7:0] div;
         wire       cdc_enable_sync;
         wire       dcm_set;

         BUFGCE
            u_clk_ct_bufg_gated(.O(clk_ct[i]),
                                .CE(clk_en_ct[i]),
                                .I(clk_ct_unbuffered[i]));

         DCM_CLKGEN
            #(.CLKFX_MULTIPLY(CT_CLOCKS_MUL),
              .CLKFX_DIVIDE(CT_CLOCKS_DIV),
              .CLKIN_PERIOD(20.83333),
              .SPREAD_SPECTRUM("NONE"),
              .STARTUP_WAIT("FALSE"),
              .CLKFX_MD_MAX(0.000))
            u_dcm_ct(.CLKIN(clk_buffered),
                     .CLKFX(clk_ct_unbuffered[i]),
                     .RST(rst_dcm),
                     .LOCKED(clk_ct_locked[i]),
                     .FREEZEDCM(1'b0),
                     .PROGCLK(progclk),
                     .PROGDATA(progdata),
                     .PROGEN(progen),
                     .STATUS(status),
                     .PROGDONE(progdone));

`ifdef OPTIMSOC_CDC_DYNAMIC
         dcm_clkgen_ctrl
            u_dcm_ctrl(// Outputs
                       .rst_dcm          (rst_dcm),
                       .progen           (progen),
                       .progdata         (progdata),
                       .rdy_clk          (rdy_clk),
                       // Inputs
                       .multi            (multi[7:0]),
                       .div              (div[7:0]),
                       .clk_in           (clk_buffered),
                       .rst_ctrl         (rst),
                       .set_clk          (dcm_set),
                       .locked           (clk_ct_locked[i]),
                       .progdone         (progdone),
                       .status           (status[1]));

         clk_opt_ctrl
            u_dcm_opt_ctrl(// Outputs
                           .dcm_set              (dcm_set),
                           .ddc_rst              (),
                           .freq_mode            (),
                           .multi                (multi[7:0]),
                           .div                  (div[7:0]),
                           // Inputs
                           .clk                  (clk_buffered),
                           .rst_sys              (rst),
                           .req_clk              (cdc_enable_sync),
                           .f_req                (cdc_conf[(i+1)*3-1:i*3]));

         cdc_rqacksync
            u_rqacksync(// Outputs
                        .rd_clkreq   (cdc_enable_sync),
                        .wr_reqbusy  (),
                        // Inputs
                        .wr_clkreq   (cdc_enable[i]),
                        .rd_clk      (clk_buffered),
                        .wr_clk      (clk_ct[i]),
                        .rst         (rst));


         assign progclk = clk_buffered;
`else
         assign rst_dcm = 1'b0;
         assign progdata = 1'b0;
         assign progen = 1'b0;
         assign progclk = 1'b0;
`endif // `ifdef OPTIMSOC_CDC_DYNAMIC
      end // for (i=0;i<NUM_CT_CLOCKS;i=i+1)

   endgenerate
   BUFGCE
      u_clk_noc_bufg_gated(.O(clk_noc),
                           .CE(clk_en_noc),
                           .I(clk_noc_unbuffered));

   DCM_CLKGEN
      #(.CLKFX_MULTIPLY(NOC_CLOCK_MUL),
        .CLKFX_DIVIDE(NOC_CLOCK_DIV),
        .CLKIN_PERIOD(20.83333),
        .SPREAD_SPECTRUM("NONE"),
        .STARTUP_WAIT("FALSE"),
        .CLKFX_MD_MAX(0.000))
      u_dcm_noc(.CLKIN(clk_buffered),
                .CLKFX(clk_noc_unbuffered),
                .RST(1'b0),
                .LOCKED(clk_noc_locked),
                .FREEZEDCM(1'b0),
                .PROGCLK(1'b0),
                .PROGDATA(1'b0),
                .PROGEN(1'b0));

   BUFGCE
      u_clk_dbg_bufg_gated(.O(clk_dbg),
                           .CE(clk_en_dbg),
                           .I(clk_dbg_unbuffered));

   DCM_CLKGEN
      #(.CLKFX_MULTIPLY(DBG_CLOCK_MUL),
        .CLKFX_DIVIDE(DBG_CLOCK_DIV),
        .CLKIN_PERIOD(20.83333),
        .SPREAD_SPECTRUM("NONE"),
        .STARTUP_WAIT("FALSE"),
        .CLKFX_MD_MAX(0.000))
      u_dcm_dbg(.CLKIN(clk_buffered),
                .CLKFX(clk_dbg_unbuffered),
                .RST(1'b0),
                .LOCKED(clk_dbg_locked),
                .FREEZEDCM(1'b0),
                .PROGCLK(1'b0),
                .PROGDATA(1'b0),
                .PROGEN(1'b0));
`endif // !`ifndef OPTIMSOC_CLOCKDOMAINS

// Those are always separate clock domains
`ifdef OPTIMSOC_IO_CLOCKS
      if (NUM_IO_CLOCKS > 0) begin : gen_ioclk0
         BUFGCE
            u_clk_ct_bufg_gated(.O(clk_io[0]),
                                .CE(clk_en_io[0]),
                                .I(clk_io_unbuffered[0]));

         DCM_CLKGEN
            #(.CLKFX_MULTIPLY(IO_CLOCK0_MUL),
              .CLKFX_DIVIDE(IO_CLOCK0_DIV),
              .CLKIN_PERIOD(20.83333),
              .SPREAD_SPECTRUM("NONE"),
              .STARTUP_WAIT("FALSE"),
              .CLKFX_MD_MAX(0.000))
            u_dcm_io0(.CLKIN(clk_buffered),
                      .CLKFX(clk_io_unbuffered[0]),
                      .RST(1'b0),
                      .LOCKED(clk_io_locked[0]),
                      .FREEZEDCM(1'b0),
                      .PROGCLK(1'b0),
                      .PROGDATA(1'b0),
                      .PROGEN(1'b0),
                      .STATUS(),
                      .PROGDONE());
      end
      if (NUM_IO_CLOCKS > 1) begin : gen_ioclk1
         BUFGCE
            u_clk_ct_bufg_gated(.O(clk_io[1]),
                                .CE(clk_en_io[1]),
                                .I(clk_io_unbuffered[1]));

         DCM_CLKGEN
            #(.CLKFX_MULTIPLY(IO_CLOCK1_MUL),
              .CLKFX_DIVIDE(IO_CLOCK1_DIV),
              .CLKIN_PERIOD(20.83333),
              .SPREAD_SPECTRUM("NONE"),
              .STARTUP_WAIT("FALSE"),
              .CLKFX_MD_MAX(0.000))
            u_dcm_io0(.CLKIN(clk_buffered),
                      .CLKFX(clk_io_unbuffered[1]),
                      .RST(1'b0),
                      .LOCKED(clk_io_locked[1]),
                      .FREEZEDCM(1'b0),
                      .PROGCLK(1'b0),
                      .PROGDATA(1'b0),
                      .PROGEN(1'b0),
                      .STATUS(),
                      .PROGDONE());
      end
`endif

   generate
      if (ENABLE_DDR_CLOCK) begin : gen_ddrclk
         BUFGCE
            u_clk_ddr_bufg_gated(.O(clk_ddr),
                                .CE(1'b1),
                                .I(clk_ddr_unbuffered));

         DCM_CLKGEN
            #(.CLKFX_MULTIPLY(25),
              .CLKFX_DIVIDE(6),
              .CLKIN_PERIOD(20.83333),
              .SPREAD_SPECTRUM("NONE"),
              .STARTUP_WAIT("FALSE"),
              .CLKFX_MD_MAX(0.000))
            u_dcm_ddr(.CLKIN(clk_buffered),
                      .CLKFX(clk_ddr_unbuffered),
                      .RST(1'b0),
                      .LOCKED(clk_ddr_locked),
                      .FREEZEDCM(1'b0),
                      .PROGCLK(1'b0),
                      .PROGDATA(1'b0),
                      .PROGEN(1'b0),
                      .STATUS(),
                      .PROGDONE());
      end else begin // if (ENABLE_DDR_CLOCK)
         assign clk_ddr = 1'b0;
         assign clk_ddr_unbuffered = 1'b0;
         assign clk_ddr_locked = 1'b1;
      end
   endgenerate

endmodule // clockmanager_ztex115
