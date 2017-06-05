
module vcu108_loopback_clock
  #(parameter FREQ = 32'hx)
   (
   // Clock in ports
   input       clk_in_p,
   input       clk_in_n,
   // Clock out ports
   output      clk_out,
   // Status and control signals
   input       rst,
   output      locked
   );

   localparam CLK_DIVISOR = 32'd1200000000 / FREQ;

   // Input buffering
   //------------------------------------
   wire clk_in_clk_wiz_0;

   IBUFDS clkin1_ibufds
      (.O  (clk_in_clk_wiz_0),
      .I  (clk_in_p),
      .IB (clk_in_n));


   // Clocking PRIMITIVE
   //------------------------------------

   wire        clk_out1_clk_wiz_0;
   wire        clkfbout_clk_wiz_0;

 
    PLLE3_ADV
  #(
    .COMPENSATION         ("AUTO"),
    .STARTUP_WAIT         ("FALSE"),
    .DIVCLK_DIVIDE        (1),
    .CLKFBOUT_MULT        (4),
    .CLKFBOUT_PHASE       (0.000),
    .CLKOUT0_DIVIDE       (CLK_DIVISOR),
    .CLKOUT0_PHASE        (0.000),
    .CLKOUT0_DUTY_CYCLE   (0.500),
    .CLKIN_PERIOD         (3.332))
  plle3_adv_inst
    // Output clocks
   (
    .CLKFBOUT            (clkfbout_clk_wiz_0),
    .CLKOUT0             (clk_out1_clk_wiz_0),
    .CLKOUT0B            (),
    .CLKOUT1             (),
    .CLKOUT1B            (),
     // Input clock control
    .CLKFBIN             (clkfbout_clk_wiz_0),
    .CLKIN               (clk_in_clk_wiz_0),
    // Ports for dynamic reconfiguration
    .DADDR               (7'h0),
    .DCLK                (1'b0),
    .DEN                 (1'b0),
    .DI                  (16'h0),
    .DO                  (),
    .DRDY                (),
    .DWE                 (1'b0),
    .CLKOUTPHYEN         (1'b0),
    .CLKOUTPHY           (),
    // Other control and status signals
    .LOCKED              (locked),
    .PWRDWN              (1'b0),
    .RST                 (rst));

   // Output buffering
   //-----------------------------------

   BUFG clkout1_buf
      (.O   (clk_out),
      .I   (clk_out1_clk_wiz_0));

endmodule