set_property -dict {PACKAGE_PIN G31 IOSTANDARD DIFF_SSTL12} [get_ports clk_p]
set_property -dict {PACKAGE_PIN F31 IOSTANDARD DIFF_SSTL12} [get_ports clk_n]

# 300 MHz system clock
create_clock -period 3.333 -name clk_p [get_ports clk_p]

## Error LED
set_property -dict {PACKAGE_PIN AT32 IOSTANDARD LVCMOS12} [get_ports error_led]

## Idle state LED
set_property -dict {PACKAGE_PIN BA37 IOSTANDARD LVCMOS12} [get_ports idle_led]

## User switch to stall stress test (south switch)
set_property -dict {PACKAGE_PIN D9 IOSTANDARD LVCMOS12} [get_ports stall_flag]

## User switch to reset stress test (north switch)
set_property -dict {PACKAGE_PIN E34 IOSTANDARD LVCMOS12} [get_ports rst_sw]

## User switch to induce error (center switch)
set_property -dict {PACKAGE_PIN AW27 IOSTANDARD LVCMOS12} [get_ports error_flag]
