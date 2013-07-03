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
 * Clock manager for the ZTEX 1.15b/d boards
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

// TODO:
//
// - add io_locked to reset (not directly, as this leads to errors for
//  NUM_IO_CLOCKS==0)
module clockmanager_ztex115(
`ifdef OPTIMSOC_CDC_DYNAMIC
   cdc_conf, cdc_enable,
`endif
   /*AUTOARG*/
   // Outputs
   clk_ct, clk_io, clk_dbg, clk_noc, rst_sys, rst_cpu,
   // Inputs
   clk, rst, cpu_reset, cpu_start, sys_halt
   );

   parameter NUM_CT_CLOCKS = 1;
   parameter CT_CLOCKS_MUL = 2;
   parameter CT_CLOCKS_DIV = 4;

   parameter DBG_CLOCK_MUL = 2;
   parameter DBG_CLOCK_DIV = 2;

   parameter NOC_CLOCK_MUL = 3;
   parameter NOC_CLOCK_DIV = 4;

   parameter NUM_IO_CLOCKS = 0;
   parameter IO_CLOCK0_MUL = 2;
   parameter IO_CLOCK0_DIV = 2;
   parameter IO_CLOCK1_MUL = 2;
   parameter IO_CLOCK1_DIV = 2;
   
   parameter RST_WIDTH = 128;

   input clk;
   input rst;

   output [NUM_CT_CLOCKS-1:0] clk_ct;
   output [NUM_IO_CLOCKS-1:0] clk_io;
   output                     clk_dbg;
   output                     clk_noc;

   output                     rst_sys;
   output                     rst_cpu;

   input                      cpu_reset;
   input                      cpu_start;

   input                      sys_halt;
   
`ifdef OPTIMSOC_CDC_DYNAMIC
   input [NUM_CT_CLOCKS*3-1:0] cdc_conf;
   input [NUM_CT_CLOCKS-1:0]   cdc_enable;
`endif


   wire                       clk_buffered;

   wire [NUM_CT_CLOCKS-1:0]   clk_ct_unbuffered;
   wire [NUM_IO_CLOCKS-1:0]   clk_io_unbuffered;
   wire                       clk_dbg_unbuffered;
   wire                       clk_noc_unbuffered;

   wire [NUM_CT_CLOCKS-1:0]   clk_ct_locked;
   wire [NUM_IO_CLOCKS-1:0]   clk_io_locked;
   wire                       clk_dbg_locked;
   wire                       clk_noc_locked;

   wire [NUM_CT_CLOCKS-1:0]   clk_en_ct;
   wire [NUM_IO_CLOCKS-1:0]   clk_en_io;
   wire                       clk_en_dbg;
   wire                       clk_en_noc;

   reg                        hold_cpu;

   genvar i;
   generate
      for (i=0;i<NUM_CT_CLOCKS;i=i+1) begin
         assign clk_en_ct[i] = clk_ct_locked[i] & ~sys_halt;
      end
      for (i=0;i<NUM_IO_CLOCKS;i=i+1) begin
         assign clk_en_io[i] = clk_io_locked[i];
      end
   endgenerate

   assign clk_en_noc = clk_noc_locked & ~sys_halt;
   assign clk_en_dbg = clk_dbg_locked;

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

      if (NUM_IO_CLOCKS > 0) begin
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
      if (NUM_IO_CLOCKS > 1) begin
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

endmodule // clockmanager_ztex115
