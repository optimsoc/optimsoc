create_clock -period 10.000 -name clk -waveform {0.000 5.000} [get_ports clk]

set_property PACKAGE_PIN E3 [get_ports clk]
set_property PACKAGE_PIN C4 [get_ports uart_rx]
set_property PACKAGE_PIN D4 [get_ports uart_tx]
set_property PACKAGE_PIN E5 [get_ports uart_cts]
set_property PACKAGE_PIN D3 [get_ports uart_rts]
set_property PACKAGE_PIN C12 [get_ports rstn]


set_property IOSTANDARD LVCMOS33 [get_ports clk]
set_property IOSTANDARD LVCMOS33 [get_ports uart_rts]
set_property IOSTANDARD LVCMOS33 [get_ports uart_rx]
set_property IOSTANDARD LVCMOS33 [get_ports uart_tx]
set_property IOSTANDARD LVCMOS33 [get_ports rstn]
set_property IOSTANDARD LVCMOS33 [get_ports uart_cts]

set_property -dict {PACKAGE_PIN T10 IOSTANDARD LVCMOS33} [get_ports CA]
set_property -dict {PACKAGE_PIN R10 IOSTANDARD LVCMOS33} [get_ports CB]
set_property -dict {PACKAGE_PIN K16 IOSTANDARD LVCMOS33} [get_ports CC]
set_property -dict {PACKAGE_PIN K13 IOSTANDARD LVCMOS33} [get_ports CD]
set_property -dict {PACKAGE_PIN P15 IOSTANDARD LVCMOS33} [get_ports CE]
set_property -dict {PACKAGE_PIN T11 IOSTANDARD LVCMOS33} [get_ports CF]
set_property -dict {PACKAGE_PIN L18 IOSTANDARD LVCMOS33} [get_ports CG]

set_property -dict {PACKAGE_PIN H15 IOSTANDARD LVCMOS33} [get_ports DP]

set_property PACKAGE_PIN J17 [get_ports {AN[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {AN[0]}]
set_property PACKAGE_PIN J18 [get_ports {AN[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {AN[1]}]
set_property PACKAGE_PIN T9 [get_ports {AN[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {AN[2]}]
set_property PACKAGE_PIN J14 [get_ports {AN[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {AN[3]}]
set_property PACKAGE_PIN P14 [get_ports {AN[4]}]
set_property IOSTANDARD LVCMOS33 [get_ports {AN[4]}]
set_property PACKAGE_PIN T14 [get_ports {AN[5]}]
set_property IOSTANDARD LVCMOS33 [get_ports {AN[5]}]
set_property PACKAGE_PIN K2 [get_ports {AN[6]}]
set_property IOSTANDARD LVCMOS33 [get_ports {AN[6]}]
set_property PACKAGE_PIN U13 [get_ports {AN[7]}]
set_property IOSTANDARD LVCMOS33 [get_ports {AN[7]}]

set_property PACKAGE_PIN J15 [get_ports {switch[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {switch[0]}]
set_property PACKAGE_PIN L16 [get_ports {switch[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {switch[1]}]
set_property PACKAGE_PIN M13 [get_ports {switch[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {switch[2]}]

set_property PACKAGE_PIN N16 [get_ports redled]
set_property IOSTANDARD LVCMOS33 [get_ports redled]



