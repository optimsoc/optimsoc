set_property -dict {PACKAGE_PIN G31 IOSTANDARD DIFF_SSTL12} [get_ports clk_p]
set_property -dict {PACKAGE_PIN F31 IOSTANDARD DIFF_SSTL12} [get_ports clk_n]

# 300 MHz system clock
create_clock -period 3.333 -name clk_p [get_ports clk_p]
