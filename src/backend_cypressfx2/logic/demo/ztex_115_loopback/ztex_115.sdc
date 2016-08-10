#1001 create_clock {p:fx2_ifclk} -name {fx2_ifclk} -period {33.333}
create_clock [get_ports {fx2_ifclk}] -name {fx2_ifclk} -period {33.333}

#1002 create_clock {p:clk} -name {clk} -period {20.8333}
create_clock [get_ports {clk}] -name {clk} -period {20.8333}
