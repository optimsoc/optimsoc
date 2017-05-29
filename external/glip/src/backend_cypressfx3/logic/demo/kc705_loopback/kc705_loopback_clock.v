
module kc705_loopback_clock
  (
   input  clk_in_p,
   input  clk_in_n,
   output clk_out,
   input  rst,
   output locked
   );

   parameter FREQ = 32'hx;
   localparam real DIVISOR_GHZ = (32'd1000000000 * 1.0) / FREQ;
   
   wire   clk_in;
   
   IBUFDS 
     ibufgds
       (.O  (clk_in),
	.I  (clk_in_p),
	.IB (clk_in_n));
   
   wire [15:0] do_unused;
   wire        drdy_unused;
   wire        psdone_unused;
   wire        locked_int;
   wire        clkfbout_clk_buf;
   wire        clkfbout_buf_clk_buf;
   wire        clkfboutb_unused;
   wire        clkout0b_unused;
   wire        clkout1_unused;
   wire        clkout1b_unused;
   wire        clkout2_unused;
   wire        clkout2b_unused;
   wire        clkout3_unused;
   wire        clkout3b_unused;
   wire        clkout4_unused;
   wire        clkout5_unused;
   wire        clkout6_unused;
   wire        clkfbstopped_unused;
   wire        clkinstopped_unused;

   wire        clk_out_clk_buf;
   
   MMCME2_BASE
     #(.BANDWIDTH            ("OPTIMIZED"),
       .CLKOUT4_CASCADE      ("FALSE"),
       .STARTUP_WAIT         ("FALSE"),
       .DIVCLK_DIVIDE        (1),
       .CLKFBOUT_MULT_F      (5.000),
       .CLKFBOUT_PHASE       (0.000),
       .CLKOUT0_DIVIDE_F     (DIVISOR_GHZ),
       .CLKOUT0_PHASE        (0.000),
       .CLKOUT0_DUTY_CYCLE   (0.500),
       .CLKIN1_PERIOD        (5.0))
   mmcm_adv_inst
     // Output clocks
     (
      .CLKFBOUT            (clkfbout_clk_buf),
      .CLKFBOUTB           (clkfboutb_unused),
      .CLKOUT0             (clk_out_clk_buf),
      .CLKOUT0B            (clkout0b_unused),
      .CLKOUT1             (clkout1_unused),
      .CLKOUT1B            (clkout1b_unused),
      .CLKOUT2             (clkout2_unused),
      .CLKOUT2B            (clkout2b_unused),
      .CLKOUT3             (clkout3_unused),
      .CLKOUT3B            (clkout3b_unused),
      .CLKOUT4             (clkout4_unused),
      .CLKOUT5             (clkout5_unused),
      .CLKOUT6             (clkout6_unused),
      // Input clock control
      .CLKFBIN             (clkfbout_buf_clk_buf),
      .CLKIN1              (clk_in),
      // Other control and status signals
      .LOCKED              (locked),
      .PWRDWN              (1'b0),
      .RST                 (rst));
   


  // Output buffering
  //-----------------------------------

  BUFG clkf_buf
   (.O (clkfbout_buf_clk_buf),
    .I (clkfbout_clk_buf));

  BUFG clkout0_buf
   (.O   (clk_out),
    .I   (clk_out_clk_buf));

endmodule // kc705_loopback_clock

