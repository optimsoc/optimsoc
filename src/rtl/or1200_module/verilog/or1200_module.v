

module or1200_module (/*AUTOARG*/
   // Outputs
   dbg_lss_o, dbg_is_o, dbg_wp_o, dbg_bp_o, dbg_dat_o, dbg_ack_o,
   iwb_cyc_o, iwb_adr_o, iwb_stb_o, iwb_we_o, iwb_sel_o, iwb_dat_o,
   iwb_bte_o, iwb_cti_o, dwb_cyc_o, dwb_adr_o, dwb_stb_o, dwb_we_o,
   dwb_sel_o, dwb_dat_o, dwb_bte_o, dwb_cti_o,
   // Inputs
   clk_i, bus_clk_i, rst_i, bus_rst_i, dbg_stall_i, dbg_ewt_i,
   dbg_stb_i, dbg_we_i, dbg_adr_i, dbg_dat_i, pic_ints_i, iwb_ack_i,
   iwb_err_i, iwb_rty_i, iwb_dat_i, dwb_ack_i, dwb_err_i, dwb_rty_i,
   dwb_dat_i
   );

   parameter id = 0;

   input          clk_i;
   input          bus_clk_i;
   input          rst_i;
   input          bus_rst_i;

   input          dbg_stall_i;  // External Stall Input
   input          dbg_ewt_i;    // External Watchpoint Trigger Input
   output [3:0]   dbg_lss_o;    // External Load/Store Unit Status
   output [1:0]   dbg_is_o;     // External Insn Fetch Status
   output [10:0]  dbg_wp_o;     // Watchpoints Outputs
   output         dbg_bp_o;     // Breakpoint Output
   input          dbg_stb_i;     // External Address/Data Strobe
   input          dbg_we_i;      // External Write Enable
   input [31:0]   dbg_adr_i;    // External Address Input
   input [31:0]   dbg_dat_i;    // External Data Input
   output [31:0]  dbg_dat_o;    // External Data Output
   output         dbg_ack_o;    // External Data Acknowledge (not WB compatible)

   input [19:0]   pic_ints_i;

   //
   // Instruction WISHBONE interface
   //
   input           iwb_ack_i;   // normal termination
   input           iwb_err_i;   // termination w/ error
   input           iwb_rty_i;   // termination w/ retry
   input [31:0]    iwb_dat_i;   // input data bus
   output          iwb_cyc_o;   // cycle valid output
   output [31:0]   iwb_adr_o;   // address bus outputs
   output          iwb_stb_o;   // strobe output
   output          iwb_we_o;    // indicates write transfer
   output [3:0]    iwb_sel_o;   // byte select outputs
   output [31:0]   iwb_dat_o;   // output data bus
   output [1:0]    iwb_bte_o;
   output [2:0]    iwb_cti_o;

   //
   // Data WISHBONE interface
   //
   input           dwb_ack_i;   // normal termination
   input           dwb_err_i;   // termination w/ error
   input           dwb_rty_i;   // termination w/ retry
   input [31:0]    dwb_dat_i;   // input data bus
   output          dwb_cyc_o;   // cycle valid output
   output [31:0]   dwb_adr_o;   // address bus outputs
   output          dwb_stb_o;   // strobe output
   output          dwb_we_o;    // indicates write transfer
   output [3:0]    dwb_sel_o;   // byte select outputs
   output [31:0]   dwb_dat_o;   // output data bus
   output [1:0]    dwb_bte_o;
   output [2:0]    dwb_cti_o;

   wire            core_ack_i;  // normal termination
   wire            core_err_i;  // termination w/ error
   wire            core_rty_i;  // termination w/ retry
   wire [31:0]     core_dat_i;  // input data bus
   wire            core_cyc_o;  // cycle valid wire
   wire [31:0]     core_adr_o;  // address bus outputs
   wire            core_stb_o;  // strobe output
   wire            core_we_o;   // indicates write transfer
   wire [3:0]      core_sel_o;  // byte select outputs
   wire [31:0]     core_dat_o;  // output data bus
   wire [1:0]      core_bte_o;
   wire [2:0]      core_cti_o;

`ifdef TRACING
   or1200_trace #(.id("core")) u_trace( .clk(clk_i) );
   initial u_trace.file = $fopen("trace");
   assign u_trace.wb_pc = u_cpu.or1200_cpu.or1200_except.wb_pc;
   assign u_trace.wb_freeze = u_cpu.or1200_cpu.or1200_except.wb_freeze;
   assign u_trace.wb_insn = u_cpu.or1200_cpu.wb_insn;
   genvar          i;
   for (i=0;i<32;i=i+1) begin
     assign u_trace.rf[i] = u_cpu.or1200_cpu.or1200_rf.rf_a.mem[i];
   end
`endif

   /* or1200_top AUTO_TEMPLATE(
    .pm_clksd_o      (),
    .pm_dc_gate_o    (),
    .pm_ic_gate_o    (),
    .pm_dmmu_gate_o  (),
    .pm_immu_gate_o  (),
    .pm_tt_gate_o    (),
    .pm_cpu_gate_o   (),
    .pm_wakeup_o     (),
    .pm_lvolt_o      (),
    .pm_cpustall_i   (1'b0),
    .iwb_clk_i       (bus_clk_i),
    .iwb_rst_i       (bus_rst_i),
    .dwb_clk_i       (bus_clk_i),
    .dwb_rst_i       (bus_rst_i),
    .dwb_\(.*\)      (core_\1[]),
    .clmode_i        (2'b00),
    ); */

   or1200_top #(.dw(32),.aw(32),.ppic_ints(20),.coreid(id))
   u_cpu(
`ifdef OR1200_WB_CAB
         .iwb_cab_o                     (iwb_cab_o),
         .dwb_cab_o                     (dwb_cab_o),
`endif
`ifdef OR1200_BIST
          .mbist_so_o   (mbist_so_o),
          .mbist_si_i   (mbist_si_i)
          .mbist_ctrl_i (mbist_ctrl_i[`OR1200_MBIST_CTRL_WIDTH-1:0]),
`endif
          /*AUTOINST*/
         // Outputs
         .iwb_cyc_o                     (iwb_cyc_o),
         .iwb_adr_o                     (iwb_adr_o[31:0]),
         .iwb_stb_o                     (iwb_stb_o),
         .iwb_we_o                      (iwb_we_o),
         .iwb_sel_o                     (iwb_sel_o[3:0]),
         .iwb_dat_o                     (iwb_dat_o[31:0]),
         .iwb_cti_o                     (iwb_cti_o[2:0]),
         .iwb_bte_o                     (iwb_bte_o[1:0]),
         .dwb_cyc_o                     (core_cyc_o),            // Templated
         .dwb_adr_o                     (core_adr_o[31:0]),      // Templated
         .dwb_stb_o                     (core_stb_o),            // Templated
         .dwb_we_o                      (core_we_o),             // Templated
         .dwb_sel_o                     (core_sel_o[3:0]),       // Templated
         .dwb_dat_o                     (core_dat_o[31:0]),      // Templated
         .dwb_cti_o                     (core_cti_o[2:0]),       // Templated
         .dwb_bte_o                     (core_bte_o[1:0]),       // Templated
         .dbg_lss_o                     (dbg_lss_o[3:0]),
         .dbg_is_o                      (dbg_is_o[1:0]),
         .dbg_wp_o                      (dbg_wp_o[10:0]),
         .dbg_bp_o                      (dbg_bp_o),
         .dbg_dat_o                     (dbg_dat_o[31:0]),
         .dbg_ack_o                     (dbg_ack_o),
         .pm_clksd_o                    (),                      // Templated
         .pm_dc_gate_o                  (),                      // Templated
         .pm_ic_gate_o                  (),                      // Templated
         .pm_dmmu_gate_o                (),                      // Templated
         .pm_immu_gate_o                (),                      // Templated
         .pm_tt_gate_o                  (),                      // Templated
         .pm_cpu_gate_o                 (),                      // Templated
         .pm_wakeup_o                   (),                      // Templated
         .pm_lvolt_o                    (),                      // Templated
         .sig_tick                      (),
         // Inputs
         .clk_i                         (clk_i),
         .rst_i                         (rst_i),
         .clmode_i                      (2'b00),                 // Templated
         .pic_ints_i                    (pic_ints_i[19:0]),
         .iwb_clk_i                     (bus_clk_i),             // Templated
         .iwb_rst_i                     (bus_rst_i),             // Templated
         .iwb_ack_i                     (iwb_ack_i),
         .iwb_err_i                     (iwb_err_i),
         .iwb_rty_i                     (iwb_rty_i),
         .iwb_dat_i                     (iwb_dat_i[31:0]),
         .dwb_clk_i                     (bus_clk_i),             // Templated
         .dwb_rst_i                     (bus_rst_i),             // Templated
         .dwb_ack_i                     (core_ack_i),            // Templated
         .dwb_err_i                     (core_err_i),            // Templated
         .dwb_rty_i                     (core_rty_i),            // Templated
         .dwb_dat_i                     (core_dat_i[31:0]),      // Templated
         .dbg_stall_i                   (dbg_stall_i),
         .dbg_ewt_i                     (dbg_ewt_i),
         .dbg_stb_i                     (dbg_stb_i),
         .dbg_we_i                      (dbg_we_i),
         .dbg_adr_i                     (dbg_adr_i[31:0]),
         .dbg_dat_i                     (dbg_dat_i[31:0]),
         .pm_cpustall_i                 (1'b0));                         // Templated

   /* wb_cas_unit AUTO_TEMPLATE(
    .clk_i          (bus_clk_i),
    .rst_i          (bus_rst_i),
    .wb_core_\(.*\)_i (core_\1_o[]),
    .wb_core_\(.*\)_o (core_\1_i[]),
    .wb_bus_\(.*\)  (dwb_\1[]),
    ); */
   wb_cas_unit u_cas (/*AUTOINST*/
                      // Outputs
                      .wb_core_dat_o    (core_dat_i[31:0]),      // Templated
                      .wb_core_ack_o    (core_ack_i),            // Templated
                      .wb_core_rty_o    (core_rty_i),            // Templated
                      .wb_core_err_o    (core_err_i),            // Templated
                      .wb_bus_dat_o     (dwb_dat_o[31:0]),       // Templated
                      .wb_bus_adr_o     (dwb_adr_o[31:0]),       // Templated
                      .wb_bus_sel_o     (dwb_sel_o[3:0]),        // Templated
                      .wb_bus_bte_o     (dwb_bte_o[1:0]),        // Templated
                      .wb_bus_cti_o     (dwb_cti_o[2:0]),        // Templated
                      .wb_bus_we_o      (dwb_we_o),              // Templated
                      .wb_bus_cyc_o     (dwb_cyc_o),             // Templated
                      .wb_bus_stb_o     (dwb_stb_o),             // Templated
                      // Inputs
                      .clk_i            (bus_clk_i),             // Templated
                      .rst_i            (bus_rst_i),             // Templated
                      .wb_core_dat_i    (core_dat_o[31:0]),      // Templated
                      .wb_core_adr_i    (core_adr_o[31:0]),      // Templated
                      .wb_core_sel_i    (core_sel_o[3:0]),       // Templated
                      .wb_core_bte_i    (core_bte_o[1:0]),       // Templated
                      .wb_core_cti_i    (core_cti_o[2:0]),       // Templated
                      .wb_core_we_i     (core_we_o),             // Templated
                      .wb_core_cyc_i    (core_cyc_o),            // Templated
                      .wb_core_stb_i    (core_stb_o),            // Templated
                      .wb_bus_dat_i     (dwb_dat_i[31:0]),       // Templated
                      .wb_bus_ack_i     (dwb_ack_i),             // Templated
                      .wb_bus_rty_i     (dwb_rty_i),             // Templated
                      .wb_bus_err_i     (dwb_err_i));            // Templated

   wire snoop_ack;
   wire snoop_we;
   wire [31:0] snoop_adr;

/* -----\/----- EXCLUDED -----\/-----
   /-* wb_llsc_unit AUTO_TEMPLATE(
    .clk_i          (bus_clk_i),
    .rst_i          (bus_rst_i),
    .wb_core_\(.*\)_i (core_\1_o[]),
    .wb_core_\(.*\)_o (core_\1_i[]),
    .wb_bus_\(.*\)  (dwb_\1[]),
    ); *-/
   wb_llsc_unit u_llcs (/-*AUTOINST*-/
                        // Outputs
                        .wb_core_dat_o  (core_dat_i[31:0]),
                        .wb_core_ack_o  (core_ack_i),
                        .wb_core_rty_o  (core_rty_i),
                        .wb_core_err_o  (core_err_i),
                        .wb_bus_dat_o   (dwb_dat_o[31:0]),
                        .wb_bus_adr_o   (dwb_adr_o[31:0]),
                        .wb_bus_sel_o   (dwb_sel_o[3:0]),
                        .wb_bus_we_o    (dwb_we_o),
                        .wb_bus_cyc_o   (dwb_cyc_o),
                        .wb_bus_stb_o   (dwb_stb_o),
                        // Inputs
                        .clk_i          (bus_clk_i),
                        .rst_i          (bus_rst_i),
                        .wb_core_dat_i  (core_dat_o[31:0]),
                        .wb_core_adr_i  (core_adr_o[31:0]),
                        .wb_core_sel_i  (core_sel_o[3:0]),
                        .wb_core_we_i   (core_we_o),
                        .wb_core_cyc_i  (core_cyc_o),
                        .wb_core_stb_i  (core_stb_o),
                        .wb_bus_dat_i   (dwb_dat_i[31:0]),
                        .wb_bus_ack_i   (dwb_ack_i),
                        .wb_bus_rty_i   (dwb_rty_i),
                        .wb_bus_err_i   (dwb_err_i),
                        .snoop_ack      (snoop_ack),
                        .snoop_we       (snoop_we),
                        .snoop_adr      (snoop_adr[31:0]));
 -----/\----- EXCLUDED -----/\----- */

endmodule // or1200_module

// Local Variables:
// verilog-library-directories:("." "../../or1200/verilog/" "../../wb_llsc_unit/verilog" "../../wb_cas_unit/verilog")
// verilog-auto-inst-param-value: t
// End:
