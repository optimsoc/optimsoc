## Pin constraints for the Nexys4 DDR Rev. C
## The naming is based on the constraint file provided by Digilent, with all
## signal names converted to lowercase.
##
## Documentation:
## https://reference.digilentinc.com/_media/nexys4-ddr:nexys4ddr_rm.pdf

## 100 MHz Clock signal
set_property -dict {PACKAGE_PIN E3 IOSTANDARD LVCMOS33} [get_ports -quiet clk]
create_clock -period 10.000 -name sys_clk_pin -waveform {0.000 5.000} -add [get_ports -quiet clk]


## Switches



## LEDs

set_property -dict {PACKAGE_PIN H17 IOSTANDARD LVCMOS33} [get_ports -quiet {led[0]}]
set_property -dict {PACKAGE_PIN K15 IOSTANDARD LVCMOS33} [get_ports -quiet {led[1]}]
set_property -dict {PACKAGE_PIN J13 IOSTANDARD LVCMOS33} [get_ports -quiet {led[2]}]
set_property -dict {PACKAGE_PIN N14 IOSTANDARD LVCMOS33} [get_ports -quiet {led[3]}]
set_property -dict {PACKAGE_PIN R18 IOSTANDARD LVCMOS33} [get_ports -quiet {led[4]}]
set_property -dict {PACKAGE_PIN V17 IOSTANDARD LVCMOS33} [get_ports -quiet {led[5]}]
set_property -dict {PACKAGE_PIN U17 IOSTANDARD LVCMOS33} [get_ports -quiet {led[6]}]
set_property -dict {PACKAGE_PIN U16 IOSTANDARD LVCMOS33} [get_ports -quiet {led[7]}]
set_property -dict {PACKAGE_PIN V16 IOSTANDARD LVCMOS33} [get_ports -quiet {led[8]}]
set_property -dict {PACKAGE_PIN T15 IOSTANDARD LVCMOS33} [get_ports -quiet {led[9]}]
set_property -dict {PACKAGE_PIN U14 IOSTANDARD LVCMOS33} [get_ports -quiet {led[10]}]
set_property -dict {PACKAGE_PIN T16 IOSTANDARD LVCMOS33} [get_ports -quiet {led[11]}]
set_property -dict {PACKAGE_PIN V15 IOSTANDARD LVCMOS33} [get_ports -quiet {led[12]}]
set_property -dict {PACKAGE_PIN V14 IOSTANDARD LVCMOS33} [get_ports -quiet {led[13]}]
set_property -dict {PACKAGE_PIN V12 IOSTANDARD LVCMOS33} [get_ports -quiet {led[14]}]
set_property -dict {PACKAGE_PIN V11 IOSTANDARD LVCMOS33} [get_ports -quiet {led[15]}]



## 7 segment display




## Buttons

set_property -dict {PACKAGE_PIN C12 IOSTANDARD LVCMOS33} [get_ports -quiet cpu_resetn]



## Pmod Headers


## Pmod Header JA



## Pmod Header JB



## Pmod Header JC



## Pmod Header JD



## Pmod Header JXADC



## VGA Connector

#set_property -quiet -dict { PACKAGE_PIN A3    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_r[0] }]; #IO_L8N_T1_AD14N_35 Sch=vga_r[0]
#set_property -quiet -dict { PACKAGE_PIN B4    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_r[1] }]; #IO_L7N_T1_AD6N_35 Sch=vga_r[1]
#set_property -quiet -dict { PACKAGE_PIN C5    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_r[2] }]; #IO_L1N_T0_AD4N_35 Sch=vga_r[2]
#set_property -quiet -dict { PACKAGE_PIN A4    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_r[3] }]; #IO_L8P_T1_AD14P_35 Sch=vga_r[3]

#set_property -quiet -dict { PACKAGE_PIN C6    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_g[0] }]; #IO_L1P_T0_AD4P_35 Sch=vga_g[0]
#set_property -quiet -dict { PACKAGE_PIN A5    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_g[1] }]; #IO_L3N_T0_DQS_AD5N_35 Sch=vga_g[1]
#set_property -quiet -dict { PACKAGE_PIN B6    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_g[2] }]; #IO_L2N_T0_AD12N_35 Sch=vga_g[2]
#set_property -quiet -dict { PACKAGE_PIN A6    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_g[3] }]; #IO_L3P_T0_DQS_AD5P_35 Sch=vga_g[3]

#set_property -quiet -dict { PACKAGE_PIN B7    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_b[0] }]; #IO_L2P_T0_AD12P_35 Sch=vga_b[0]
#set_property -quiet -dict { PACKAGE_PIN C7    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_b[1] }]; #IO_L4N_T0_35 Sch=vga_b[1]
#set_property -quiet -dict { PACKAGE_PIN D7    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_b[2] }]; #IO_L6N_T0_VREF_35 Sch=vga_b[2]
#set_property -quiet -dict { PACKAGE_PIN D8    IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_b[3] }]; #IO_L4P_T0_35 Sch=vga_b[3]

#set_property -quiet -dict { PACKAGE_PIN B11   IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_hs }]; #IO_L4P_T0_15 Sch=vga_hs
#set_property -quiet -dict { PACKAGE_PIN B12   IOSTANDARD LVCMOS33 } [get_ports -quiet { vga_vs }]; #IO_L3N_T0_DQS_AD1N_15 Sch=vga_vs


## Micro SD Connector

#set_property -quiet -dict { PACKAGE_PIN E2    IOSTANDARD LVCMOS33 } [get_ports -quiet { sd_reset }]; #IO_L14P_T2_SRCC_35 Sch=sd_reset
#set_property -quiet -dict { PACKAGE_PIN A1    IOSTANDARD LVCMOS33 } [get_ports -quiet { sd_cd }]; #IO_L9N_T1_DQS_AD7N_35 Sch=sd_cd
#set_property -quiet -dict { PACKAGE_PIN B1    IOSTANDARD LVCMOS33 } [get_ports -quiet { sd_sck }]; #IO_L9P_T1_DQS_AD7P_35 Sch=sd_sck
#set_property -quiet -dict { PACKAGE_PIN C1    IOSTANDARD LVCMOS33 } [get_ports -quiet { sd_cmd }]; #IO_L16N_T2_35 Sch=sd_cmd
#set_property -quiet -dict { PACKAGE_PIN C2    IOSTANDARD LVCMOS33 } [get_ports -quiet { sd_dat[0] }]; #IO_L16P_T2_35 Sch=sd_dat[0]
#set_property -quiet -dict { PACKAGE_PIN E1    IOSTANDARD LVCMOS33 } [get_ports -quiet { sd_dat[1] }]; #IO_L18N_T2_35 Sch=sd_dat[1]
#set_property -quiet -dict { PACKAGE_PIN F1    IOSTANDARD LVCMOS33 } [get_ports -quiet { sd_dat[2] }]; #IO_L18P_T2_35 Sch=sd_dat[2]
#set_property -quiet -dict { PACKAGE_PIN D2    IOSTANDARD LVCMOS33 } [get_ports -quiet { sd_dat[3] }]; #IO_L14N_T2_SRCC_35 Sch=sd_dat[3]


## Accelerometer

#set_property -quiet -dict { PACKAGE_PIN E15   IOSTANDARD LVCMOS33 } [get_ports -quiet { acl_miso }]; #IO_L11P_T1_SRCC_15 Sch=acl_miso
#set_property -quiet -dict { PACKAGE_PIN F14   IOSTANDARD LVCMOS33 } [get_ports -quiet { acl_mosi }]; #IO_L5N_T0_AD9N_15 Sch=acl_mosi
#set_property -quiet -dict { PACKAGE_PIN F15   IOSTANDARD LVCMOS33 } [get_ports -quiet { acl_sclk }]; #IO_L14P_T2_SRCC_15 Sch=acl_sclk
#set_property -quiet -dict { PACKAGE_PIN D15   IOSTANDARD LVCMOS33 } [get_ports -quiet { acl_csn }]; #IO_L12P_T1_MRCC_15 Sch=acl_csn
#set_property -quiet -dict { PACKAGE_PIN B13   IOSTANDARD LVCMOS33 } [get_ports -quiet { acl_int[1] }]; #IO_L2P_T0_AD8P_15 Sch=acl_int[1]
#set_property -quiet -dict { PACKAGE_PIN C16   IOSTANDARD LVCMOS33 } [get_ports -quiet { acl_int[2] }]; #IO_L20P_T3_A20_15 Sch=acl_int[2]


## Temperature Sensor

#set_property -quiet -dict { PACKAGE_PIN C14   IOSTANDARD LVCMOS33 } [get_ports -quiet { tmp_scl }]; #IO_L1N_T0_AD0N_15 Sch=tmp_scl
#set_property -quiet -dict { PACKAGE_PIN C15   IOSTANDARD LVCMOS33 } [get_ports -quiet { tmp_sda }]; #IO_L12N_T1_MRCC_15 Sch=tmp_sda
#set_property -quiet -dict { PACKAGE_PIN D13   IOSTANDARD LVCMOS33 } [get_ports -quiet { tmp_int }]; #IO_L6N_T0_VREF_15 Sch=tmp_int
#set_property -quiet -dict { PACKAGE_PIN B14   IOSTANDARD LVCMOS33 } [get_ports -quiet { tmp_ct }]; #IO_L2N_T0_AD8N_15 Sch=tmp_ct

## Omnidirectional Microphone

#set_property -quiet -dict { PACKAGE_PIN J5    IOSTANDARD LVCMOS33 } [get_ports -quiet { m_clk }]; #IO_25_35 Sch=m_clk
#set_property -quiet -dict { PACKAGE_PIN H5    IOSTANDARD LVCMOS33 } [get_ports -quiet { m_data }]; #IO_L24N_T3_35 Sch=m_data
#set_property -quiet -dict { PACKAGE_PIN F5    IOSTANDARD LVCMOS33 } [get_ports -quiet { m_lrsel }]; #IO_0_35 Sch=m_lrsel


## PWM Audio Amplifier

#set_property -quiet -dict { PACKAGE_PIN A11   IOSTANDARD LVCMOS33 } [get_ports -quiet { aud_pwm }]; #IO_L4N_T0_15 Sch=aud_pwm
#set_property -quiet -dict { PACKAGE_PIN D12   IOSTANDARD LVCMOS33 } [get_ports -quiet { aud_sd }]; #IO_L6P_T0_15 Sch=aud_sd


## USB-RS232 Interface
## From FT2232H: http://www.ftdichip.com/Support/Documents/DataSheets/ICs/DS_FT2232H.pdf
## We left the pin names as they are used by Digilent, even though they are confusing for CTS/RTS
set_property -dict {PACKAGE_PIN C4 IOSTANDARD LVCMOS33} [get_ports -quiet uart_txd_in]
set_property -dict {PACKAGE_PIN D4 IOSTANDARD LVCMOS33} [get_ports -quiet uart_rxd_out]
# CTS from the PC (output on the FPGA); active low (despite the name)
set_property -dict {PACKAGE_PIN D3 IOSTANDARD LVCMOS33} [get_ports -quiet uart_cts]
# RTS from the PC (input on the FPGA); active low (despite the name)
set_property -dict {PACKAGE_PIN E5 IOSTANDARD LVCMOS33} [get_ports -quiet uart_rts]

## USB HID (PS/2)

#set_property -quiet -dict { PACKAGE_PIN F4    IOSTANDARD LVCMOS33 } [get_ports -quiet { ps2_clk }]; #IO_L13P_T2_MRCC_35 Sch=ps2_clk
#set_property -quiet -dict { PACKAGE_PIN B2    IOSTANDARD LVCMOS33 } [get_ports -quiet { ps2_data }]; #IO_L10N_T1_AD15N_35 Sch=ps2_data


## SMSC Ethernet PHY

set_property -dict {PACKAGE_PIN C9 IOSTANDARD LVCMOS33} [get_ports -quiet eth_mdc]
set_property -dict {PACKAGE_PIN A9 IOSTANDARD LVCMOS33} [get_ports -quiet eth_mdio]
set_property -dict {PACKAGE_PIN B3 IOSTANDARD LVCMOS33} [get_ports -quiet eth_rstn]
set_property -dict {PACKAGE_PIN D9 IOSTANDARD LVCMOS33} [get_ports -quiet eth_crsdv]
set_property -dict {PACKAGE_PIN C10 IOSTANDARD LVCMOS33} [get_ports -quiet eth_rxerr]
set_property -dict {PACKAGE_PIN C11 IOSTANDARD LVCMOS33} [get_ports -quiet {eth_rxd[0]}]
set_property -dict {PACKAGE_PIN D10 IOSTANDARD LVCMOS33} [get_ports -quiet {eth_rxd[1]}]
set_property -dict {PACKAGE_PIN B9 IOSTANDARD LVCMOS33} [get_ports -quiet eth_txen]
set_property -dict {PACKAGE_PIN A10 IOSTANDARD LVCMOS33} [get_ports -quiet {eth_txd[0]}]
set_property -dict {PACKAGE_PIN A8 IOSTANDARD LVCMOS33} [get_ports -quiet {eth_txd[1]}]
set_property -dict {PACKAGE_PIN D5 IOSTANDARD LVCMOS33} [get_ports -quiet eth_refclk]


## Quad SPI Flash

#set_property -quiet -dict { PACKAGE_PIN K17   IOSTANDARD LVCMOS33 } [get_ports -quiet { qspi_dq[0] }]; #IO_L1P_T0_D00_MOSI_14 Sch=qspi_dq[0]
#set_property -quiet -dict { PACKAGE_PIN K18   IOSTANDARD LVCMOS33 } [get_ports -quiet { qspi_dq[1] }]; #IO_L1N_T0_D01_DIN_14 Sch=qspi_dq[1]
#set_property -quiet -dict { PACKAGE_PIN L14   IOSTANDARD LVCMOS33 } [get_ports -quiet { qspi_dq[2] }]; #IO_L2P_T0_D02_14 Sch=qspi_dq[2]
#set_property -quiet -dict { PACKAGE_PIN M14   IOSTANDARD LVCMOS33 } [get_ports -quiet { qspi_dq[3] }]; #IO_L2N_T0_D03_14 Sch=qspi_dq[3]
#set_property -quiet -dict { PACKAGE_PIN L13   IOSTANDARD LVCMOS33 } [get_ports -quiet { qspi_csn }]; #IO_L6P_T0_FCS_B_14 Sch=qspi_csn






connect_debug_port u_ila_0/probe0 [get_nets [list {s_axis_txd_tkeep[0]} {s_axis_txd_tkeep[1]} {s_axis_txd_tkeep[2]} {s_axis_txd_tkeep[3]}]]
connect_debug_port u_ila_0/probe2 [get_nets [list {s_axis_txc_tkeep[0]} {s_axis_txc_tkeep[1]} {s_axis_txc_tkeep[2]} {s_axis_txc_tkeep[3]}]]
connect_debug_port u_ila_0/probe4 [get_nets [list {s_axi_wstrb[0]} {s_axi_wstrb[1]} {s_axi_wstrb[2]} {s_axi_wstrb[3]}]]
connect_debug_port u_ila_0/probe5 [get_nets [list {s_axi_wdata[0]} {s_axi_wdata[1]} {s_axi_wdata[2]} {s_axi_wdata[3]} {s_axi_wdata[4]} {s_axi_wdata[5]} {s_axi_wdata[6]} {s_axi_wdata[7]} {s_axi_wdata[8]} {s_axi_wdata[9]} {s_axi_wdata[10]} {s_axi_wdata[11]} {s_axi_wdata[12]} {s_axi_wdata[13]} {s_axi_wdata[14]} {s_axi_wdata[15]} {s_axi_wdata[16]} {s_axi_wdata[17]} {s_axi_wdata[18]} {s_axi_wdata[19]} {s_axi_wdata[20]} {s_axi_wdata[21]} {s_axi_wdata[22]} {s_axi_wdata[23]} {s_axi_wdata[24]} {s_axi_wdata[25]} {s_axi_wdata[26]} {s_axi_wdata[27]} {s_axi_wdata[28]} {s_axi_wdata[29]} {s_axi_wdata[30]} {s_axi_wdata[31]}]]
connect_debug_port u_ila_0/probe9 [get_nets [list {s_axi_awaddr[0]} {s_axi_awaddr[1]} {s_axi_awaddr[2]} {s_axi_awaddr[3]} {s_axi_awaddr[4]} {s_axi_awaddr[5]} {s_axi_awaddr[6]} {s_axi_awaddr[7]} {s_axi_awaddr[8]} {s_axi_awaddr[9]} {s_axi_awaddr[10]} {s_axi_awaddr[11]} {s_axi_awaddr[12]} {s_axi_awaddr[13]} {s_axi_awaddr[14]} {s_axi_awaddr[15]} {s_axi_awaddr[16]} {s_axi_awaddr[17]} {s_axi_awaddr[18]} {s_axi_awaddr[19]} {s_axi_awaddr[20]} {s_axi_awaddr[21]} {s_axi_awaddr[22]} {s_axi_awaddr[23]} {s_axi_awaddr[24]} {s_axi_awaddr[25]} {s_axi_awaddr[26]} {s_axi_awaddr[27]} {s_axi_awaddr[28]} {s_axi_awaddr[29]} {s_axi_awaddr[30]} {s_axi_awaddr[31]}]]
connect_debug_port u_ila_0/probe10 [get_nets [list {s_axi_araddr[0]} {s_axi_araddr[1]} {s_axi_araddr[2]} {s_axi_araddr[3]} {s_axi_araddr[4]} {s_axi_araddr[5]} {s_axi_araddr[6]} {s_axi_araddr[7]} {s_axi_araddr[8]} {s_axi_araddr[9]} {s_axi_araddr[10]} {s_axi_araddr[11]} {s_axi_araddr[12]} {s_axi_araddr[13]} {s_axi_araddr[14]} {s_axi_araddr[15]} {s_axi_araddr[16]} {s_axi_araddr[17]} {s_axi_araddr[18]} {s_axi_araddr[19]} {s_axi_araddr[20]} {s_axi_araddr[21]} {s_axi_araddr[22]} {s_axi_araddr[23]} {s_axi_araddr[24]} {s_axi_araddr[25]} {s_axi_araddr[26]} {s_axi_araddr[27]} {s_axi_araddr[28]} {s_axi_araddr[29]} {s_axi_araddr[30]} {s_axi_araddr[31]}]]
connect_debug_port u_ila_0/probe11 [get_nets [list {m_axis_rxd_tkeep[0]} {m_axis_rxd_tkeep[1]} {m_axis_rxd_tkeep[2]} {m_axis_rxd_tkeep[3]}]]
connect_debug_port u_ila_0/probe28 [get_nets [list s_axi_bready]]
connect_debug_port u_ila_0/probe30 [get_nets [list s_axi_rready]]




create_debug_core u_ila_0 ila
set_property ALL_PROBE_SAME_MU true [get_debug_cores u_ila_0]
set_property ALL_PROBE_SAME_MU_CNT 1 [get_debug_cores u_ila_0]
set_property C_ADV_TRIGGER false [get_debug_cores u_ila_0]
set_property C_DATA_DEPTH 2048 [get_debug_cores u_ila_0]
set_property C_EN_STRG_QUAL false [get_debug_cores u_ila_0]
set_property C_INPUT_PIPE_STAGES 0 [get_debug_cores u_ila_0]
set_property C_TRIGIN_EN false [get_debug_cores u_ila_0]
set_property C_TRIGOUT_EN false [get_debug_cores u_ila_0]
set_property port_width 1 [get_debug_ports u_ila_0/clk]
connect_debug_port u_ila_0/clk [get_nets [list u_board/u_mig_7series/u_mig_7series_mig/u_ddr2_infrastructure/CLK]]
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe0]
set_property port_width 2 [get_debug_ports u_ila_0/probe0]
connect_debug_port u_ila_0/probe0 [get_nets [list {eth_rxd_IBUF[0]} {eth_rxd_IBUF[1]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe1]
set_property port_width 4 [get_debug_ports u_ila_0/probe1]
connect_debug_port u_ila_0/probe1 [get_nets [list {mii_rxd[0]} {mii_rxd[1]} {mii_rxd[2]} {mii_rxd[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe2]
set_property port_width 2 [get_debug_ports u_ila_0/probe2]
connect_debug_port u_ila_0/probe2 [get_nets [list {eth_txd_OBUF[0]} {eth_txd_OBUF[1]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe3]
set_property port_width 32 [get_debug_ports u_ila_0/probe3]
connect_debug_port u_ila_0/probe3 [get_nets [list {fifo_s_axi_araddr[0]} {fifo_s_axi_araddr[1]} {fifo_s_axi_araddr[2]} {fifo_s_axi_araddr[3]} {fifo_s_axi_araddr[4]} {fifo_s_axi_araddr[5]} {fifo_s_axi_araddr[6]} {fifo_s_axi_araddr[7]} {fifo_s_axi_araddr[8]} {fifo_s_axi_araddr[9]} {fifo_s_axi_araddr[10]} {fifo_s_axi_araddr[11]} {fifo_s_axi_araddr[12]} {fifo_s_axi_araddr[13]} {fifo_s_axi_araddr[14]} {fifo_s_axi_araddr[15]} {fifo_s_axi_araddr[16]} {fifo_s_axi_araddr[17]} {fifo_s_axi_araddr[18]} {fifo_s_axi_araddr[19]} {fifo_s_axi_araddr[20]} {fifo_s_axi_araddr[21]} {fifo_s_axi_araddr[22]} {fifo_s_axi_araddr[23]} {fifo_s_axi_araddr[24]} {fifo_s_axi_araddr[25]} {fifo_s_axi_araddr[26]} {fifo_s_axi_araddr[27]} {fifo_s_axi_araddr[28]} {fifo_s_axi_araddr[29]} {fifo_s_axi_araddr[30]} {fifo_s_axi_araddr[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe4]
set_property port_width 32 [get_debug_ports u_ila_0/probe4]
connect_debug_port u_ila_0/probe4 [get_nets [list {fifo_s_axi_awaddr[0]} {fifo_s_axi_awaddr[1]} {fifo_s_axi_awaddr[2]} {fifo_s_axi_awaddr[3]} {fifo_s_axi_awaddr[4]} {fifo_s_axi_awaddr[5]} {fifo_s_axi_awaddr[6]} {fifo_s_axi_awaddr[7]} {fifo_s_axi_awaddr[8]} {fifo_s_axi_awaddr[9]} {fifo_s_axi_awaddr[10]} {fifo_s_axi_awaddr[11]} {fifo_s_axi_awaddr[12]} {fifo_s_axi_awaddr[13]} {fifo_s_axi_awaddr[14]} {fifo_s_axi_awaddr[15]} {fifo_s_axi_awaddr[16]} {fifo_s_axi_awaddr[17]} {fifo_s_axi_awaddr[18]} {fifo_s_axi_awaddr[19]} {fifo_s_axi_awaddr[20]} {fifo_s_axi_awaddr[21]} {fifo_s_axi_awaddr[22]} {fifo_s_axi_awaddr[23]} {fifo_s_axi_awaddr[24]} {fifo_s_axi_awaddr[25]} {fifo_s_axi_awaddr[26]} {fifo_s_axi_awaddr[27]} {fifo_s_axi_awaddr[28]} {fifo_s_axi_awaddr[29]} {fifo_s_axi_awaddr[30]} {fifo_s_axi_awaddr[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe5]
set_property port_width 2 [get_debug_ports u_ila_0/probe5]
connect_debug_port u_ila_0/probe5 [get_nets [list {fifo_s_axi_bresp[0]} {fifo_s_axi_bresp[1]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe6]
set_property port_width 32 [get_debug_ports u_ila_0/probe6]
connect_debug_port u_ila_0/probe6 [get_nets [list {fifo_s_axi_rdata[0]} {fifo_s_axi_rdata[1]} {fifo_s_axi_rdata[2]} {fifo_s_axi_rdata[3]} {fifo_s_axi_rdata[4]} {fifo_s_axi_rdata[5]} {fifo_s_axi_rdata[6]} {fifo_s_axi_rdata[7]} {fifo_s_axi_rdata[8]} {fifo_s_axi_rdata[9]} {fifo_s_axi_rdata[10]} {fifo_s_axi_rdata[11]} {fifo_s_axi_rdata[12]} {fifo_s_axi_rdata[13]} {fifo_s_axi_rdata[14]} {fifo_s_axi_rdata[15]} {fifo_s_axi_rdata[16]} {fifo_s_axi_rdata[17]} {fifo_s_axi_rdata[18]} {fifo_s_axi_rdata[19]} {fifo_s_axi_rdata[20]} {fifo_s_axi_rdata[21]} {fifo_s_axi_rdata[22]} {fifo_s_axi_rdata[23]} {fifo_s_axi_rdata[24]} {fifo_s_axi_rdata[25]} {fifo_s_axi_rdata[26]} {fifo_s_axi_rdata[27]} {fifo_s_axi_rdata[28]} {fifo_s_axi_rdata[29]} {fifo_s_axi_rdata[30]} {fifo_s_axi_rdata[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe7]
set_property port_width 2 [get_debug_ports u_ila_0/probe7]
connect_debug_port u_ila_0/probe7 [get_nets [list {fifo_s_axi_rresp[0]} {fifo_s_axi_rresp[1]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe8]
set_property port_width 32 [get_debug_ports u_ila_0/probe8]
connect_debug_port u_ila_0/probe8 [get_nets [list {fifo_s_axi_wdata[0]} {fifo_s_axi_wdata[1]} {fifo_s_axi_wdata[2]} {fifo_s_axi_wdata[3]} {fifo_s_axi_wdata[4]} {fifo_s_axi_wdata[5]} {fifo_s_axi_wdata[6]} {fifo_s_axi_wdata[7]} {fifo_s_axi_wdata[8]} {fifo_s_axi_wdata[9]} {fifo_s_axi_wdata[10]} {fifo_s_axi_wdata[11]} {fifo_s_axi_wdata[12]} {fifo_s_axi_wdata[13]} {fifo_s_axi_wdata[14]} {fifo_s_axi_wdata[15]} {fifo_s_axi_wdata[16]} {fifo_s_axi_wdata[17]} {fifo_s_axi_wdata[18]} {fifo_s_axi_wdata[19]} {fifo_s_axi_wdata[20]} {fifo_s_axi_wdata[21]} {fifo_s_axi_wdata[22]} {fifo_s_axi_wdata[23]} {fifo_s_axi_wdata[24]} {fifo_s_axi_wdata[25]} {fifo_s_axi_wdata[26]} {fifo_s_axi_wdata[27]} {fifo_s_axi_wdata[28]} {fifo_s_axi_wdata[29]} {fifo_s_axi_wdata[30]} {fifo_s_axi_wdata[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe9]
set_property port_width 4 [get_debug_ports u_ila_0/probe9]
connect_debug_port u_ila_0/probe9 [get_nets [list {fifo_s_axi_wstrb[0]} {fifo_s_axi_wstrb[1]} {fifo_s_axi_wstrb[2]} {fifo_s_axi_wstrb[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe10]
set_property port_width 4 [get_debug_ports u_ila_0/probe10]
connect_debug_port u_ila_0/probe10 [get_nets [list {mii_txd[0]} {mii_txd[1]} {mii_txd[2]} {mii_txd[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe11]
set_property port_width 32 [get_debug_ports u_ila_0/probe11]
connect_debug_port u_ila_0/probe11 [get_nets [list {m_axis_rxd_tdata[0]} {m_axis_rxd_tdata[1]} {m_axis_rxd_tdata[2]} {m_axis_rxd_tdata[3]} {m_axis_rxd_tdata[4]} {m_axis_rxd_tdata[5]} {m_axis_rxd_tdata[6]} {m_axis_rxd_tdata[7]} {m_axis_rxd_tdata[8]} {m_axis_rxd_tdata[9]} {m_axis_rxd_tdata[10]} {m_axis_rxd_tdata[11]} {m_axis_rxd_tdata[12]} {m_axis_rxd_tdata[13]} {m_axis_rxd_tdata[14]} {m_axis_rxd_tdata[15]} {m_axis_rxd_tdata[16]} {m_axis_rxd_tdata[17]} {m_axis_rxd_tdata[18]} {m_axis_rxd_tdata[19]} {m_axis_rxd_tdata[20]} {m_axis_rxd_tdata[21]} {m_axis_rxd_tdata[22]} {m_axis_rxd_tdata[23]} {m_axis_rxd_tdata[24]} {m_axis_rxd_tdata[25]} {m_axis_rxd_tdata[26]} {m_axis_rxd_tdata[27]} {m_axis_rxd_tdata[28]} {m_axis_rxd_tdata[29]} {m_axis_rxd_tdata[30]} {m_axis_rxd_tdata[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe12]
set_property port_width 4 [get_debug_ports u_ila_0/probe12]
connect_debug_port u_ila_0/probe12 [get_nets [list {m_axis_rxd_tkeep[0]} {m_axis_rxd_tkeep[1]} {m_axis_rxd_tkeep[2]} {m_axis_rxd_tkeep[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe13]
set_property port_width 32 [get_debug_ports u_ila_0/probe13]
connect_debug_port u_ila_0/probe13 [get_nets [list {s_axi_araddr[0]} {s_axi_araddr[1]} {s_axi_araddr[2]} {s_axi_araddr[3]} {s_axi_araddr[4]} {s_axi_araddr[5]} {s_axi_araddr[6]} {s_axi_araddr[7]} {s_axi_araddr[8]} {s_axi_araddr[9]} {s_axi_araddr[10]} {s_axi_araddr[11]} {s_axi_araddr[12]} {s_axi_araddr[13]} {s_axi_araddr[14]} {s_axi_araddr[15]} {s_axi_araddr[16]} {s_axi_araddr[17]} {s_axi_araddr[18]} {s_axi_araddr[19]} {s_axi_araddr[20]} {s_axi_araddr[21]} {s_axi_araddr[22]} {s_axi_araddr[23]} {s_axi_araddr[24]} {s_axi_araddr[25]} {s_axi_araddr[26]} {s_axi_araddr[27]} {s_axi_araddr[28]} {s_axi_araddr[29]} {s_axi_araddr[30]} {s_axi_araddr[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe14]
set_property port_width 32 [get_debug_ports u_ila_0/probe14]
connect_debug_port u_ila_0/probe14 [get_nets [list {s_axi_awaddr[0]} {s_axi_awaddr[1]} {s_axi_awaddr[2]} {s_axi_awaddr[3]} {s_axi_awaddr[4]} {s_axi_awaddr[5]} {s_axi_awaddr[6]} {s_axi_awaddr[7]} {s_axi_awaddr[8]} {s_axi_awaddr[9]} {s_axi_awaddr[10]} {s_axi_awaddr[11]} {s_axi_awaddr[12]} {s_axi_awaddr[13]} {s_axi_awaddr[14]} {s_axi_awaddr[15]} {s_axi_awaddr[16]} {s_axi_awaddr[17]} {s_axi_awaddr[18]} {s_axi_awaddr[19]} {s_axi_awaddr[20]} {s_axi_awaddr[21]} {s_axi_awaddr[22]} {s_axi_awaddr[23]} {s_axi_awaddr[24]} {s_axi_awaddr[25]} {s_axi_awaddr[26]} {s_axi_awaddr[27]} {s_axi_awaddr[28]} {s_axi_awaddr[29]} {s_axi_awaddr[30]} {s_axi_awaddr[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe15]
set_property port_width 2 [get_debug_ports u_ila_0/probe15]
connect_debug_port u_ila_0/probe15 [get_nets [list {s_axi_bresp[0]} {s_axi_bresp[1]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe16]
set_property port_width 32 [get_debug_ports u_ila_0/probe16]
connect_debug_port u_ila_0/probe16 [get_nets [list {s_axi_rdata[0]} {s_axi_rdata[1]} {s_axi_rdata[2]} {s_axi_rdata[3]} {s_axi_rdata[4]} {s_axi_rdata[5]} {s_axi_rdata[6]} {s_axi_rdata[7]} {s_axi_rdata[8]} {s_axi_rdata[9]} {s_axi_rdata[10]} {s_axi_rdata[11]} {s_axi_rdata[12]} {s_axi_rdata[13]} {s_axi_rdata[14]} {s_axi_rdata[15]} {s_axi_rdata[16]} {s_axi_rdata[17]} {s_axi_rdata[18]} {s_axi_rdata[19]} {s_axi_rdata[20]} {s_axi_rdata[21]} {s_axi_rdata[22]} {s_axi_rdata[23]} {s_axi_rdata[24]} {s_axi_rdata[25]} {s_axi_rdata[26]} {s_axi_rdata[27]} {s_axi_rdata[28]} {s_axi_rdata[29]} {s_axi_rdata[30]} {s_axi_rdata[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe17]
set_property port_width 2 [get_debug_ports u_ila_0/probe17]
connect_debug_port u_ila_0/probe17 [get_nets [list {s_axi_rresp[0]} {s_axi_rresp[1]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe18]
set_property port_width 32 [get_debug_ports u_ila_0/probe18]
connect_debug_port u_ila_0/probe18 [get_nets [list {s_axi_wdata[0]} {s_axi_wdata[1]} {s_axi_wdata[2]} {s_axi_wdata[3]} {s_axi_wdata[4]} {s_axi_wdata[5]} {s_axi_wdata[6]} {s_axi_wdata[7]} {s_axi_wdata[8]} {s_axi_wdata[9]} {s_axi_wdata[10]} {s_axi_wdata[11]} {s_axi_wdata[12]} {s_axi_wdata[13]} {s_axi_wdata[14]} {s_axi_wdata[15]} {s_axi_wdata[16]} {s_axi_wdata[17]} {s_axi_wdata[18]} {s_axi_wdata[19]} {s_axi_wdata[20]} {s_axi_wdata[21]} {s_axi_wdata[22]} {s_axi_wdata[23]} {s_axi_wdata[24]} {s_axi_wdata[25]} {s_axi_wdata[26]} {s_axi_wdata[27]} {s_axi_wdata[28]} {s_axi_wdata[29]} {s_axi_wdata[30]} {s_axi_wdata[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe19]
set_property port_width 4 [get_debug_ports u_ila_0/probe19]
connect_debug_port u_ila_0/probe19 [get_nets [list {s_axi_wstrb[0]} {s_axi_wstrb[1]} {s_axi_wstrb[2]} {s_axi_wstrb[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe20]
set_property port_width 32 [get_debug_ports u_ila_0/probe20]
connect_debug_port u_ila_0/probe20 [get_nets [list {s_axis_txc_tdata[0]} {s_axis_txc_tdata[1]} {s_axis_txc_tdata[2]} {s_axis_txc_tdata[3]} {s_axis_txc_tdata[4]} {s_axis_txc_tdata[5]} {s_axis_txc_tdata[6]} {s_axis_txc_tdata[7]} {s_axis_txc_tdata[8]} {s_axis_txc_tdata[9]} {s_axis_txc_tdata[10]} {s_axis_txc_tdata[11]} {s_axis_txc_tdata[12]} {s_axis_txc_tdata[13]} {s_axis_txc_tdata[14]} {s_axis_txc_tdata[15]} {s_axis_txc_tdata[16]} {s_axis_txc_tdata[17]} {s_axis_txc_tdata[18]} {s_axis_txc_tdata[19]} {s_axis_txc_tdata[20]} {s_axis_txc_tdata[21]} {s_axis_txc_tdata[22]} {s_axis_txc_tdata[23]} {s_axis_txc_tdata[24]} {s_axis_txc_tdata[25]} {s_axis_txc_tdata[26]} {s_axis_txc_tdata[27]} {s_axis_txc_tdata[28]} {s_axis_txc_tdata[29]} {s_axis_txc_tdata[30]} {s_axis_txc_tdata[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe21]
set_property port_width 32 [get_debug_ports u_ila_0/probe21]
connect_debug_port u_ila_0/probe21 [get_nets [list {s_axis_txd_tdata[0]} {s_axis_txd_tdata[1]} {s_axis_txd_tdata[2]} {s_axis_txd_tdata[3]} {s_axis_txd_tdata[4]} {s_axis_txd_tdata[5]} {s_axis_txd_tdata[6]} {s_axis_txd_tdata[7]} {s_axis_txd_tdata[8]} {s_axis_txd_tdata[9]} {s_axis_txd_tdata[10]} {s_axis_txd_tdata[11]} {s_axis_txd_tdata[12]} {s_axis_txd_tdata[13]} {s_axis_txd_tdata[14]} {s_axis_txd_tdata[15]} {s_axis_txd_tdata[16]} {s_axis_txd_tdata[17]} {s_axis_txd_tdata[18]} {s_axis_txd_tdata[19]} {s_axis_txd_tdata[20]} {s_axis_txd_tdata[21]} {s_axis_txd_tdata[22]} {s_axis_txd_tdata[23]} {s_axis_txd_tdata[24]} {s_axis_txd_tdata[25]} {s_axis_txd_tdata[26]} {s_axis_txd_tdata[27]} {s_axis_txd_tdata[28]} {s_axis_txd_tdata[29]} {s_axis_txd_tdata[30]} {s_axis_txd_tdata[31]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe22]
set_property port_width 4 [get_debug_ports u_ila_0/probe22]
connect_debug_port u_ila_0/probe22 [get_nets [list {s_axis_txd_tkeep[0]} {s_axis_txd_tkeep[1]} {s_axis_txd_tkeep[2]} {s_axis_txd_tkeep[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe23]
set_property port_width 4 [get_debug_ports u_ila_0/probe23]
connect_debug_port u_ila_0/probe23 [get_nets [list {s_axis_txc_tkeep[0]} {s_axis_txc_tkeep[1]} {s_axis_txc_tkeep[2]} {s_axis_txc_tkeep[3]}]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe24]
set_property port_width 1 [get_debug_ports u_ila_0/probe24]
connect_debug_port u_ila_0/probe24 [get_nets [list eth_crsdv_IBUF]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe25]
set_property port_width 1 [get_debug_ports u_ila_0/probe25]
connect_debug_port u_ila_0/probe25 [get_nets [list eth_mdc_OBUF]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe26]
set_property port_width 1 [get_debug_ports u_ila_0/probe26]
connect_debug_port u_ila_0/probe26 [get_nets [list eth_refclk_OBUF]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe27]
set_property port_width 1 [get_debug_ports u_ila_0/probe27]
connect_debug_port u_ila_0/probe27 [get_nets [list eth_rstn_OBUF]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe28]
set_property port_width 1 [get_debug_ports u_ila_0/probe28]
connect_debug_port u_ila_0/probe28 [get_nets [list eth_rxerr_IBUF]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe29]
set_property port_width 1 [get_debug_ports u_ila_0/probe29]
connect_debug_port u_ila_0/probe29 [get_nets [list eth_txen_OBUF]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe30]
set_property port_width 1 [get_debug_ports u_ila_0/probe30]
connect_debug_port u_ila_0/probe30 [get_nets [list fifo_s_axi_arready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe31]
set_property port_width 1 [get_debug_ports u_ila_0/probe31]
connect_debug_port u_ila_0/probe31 [get_nets [list fifo_s_axi_arvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe32]
set_property port_width 1 [get_debug_ports u_ila_0/probe32]
connect_debug_port u_ila_0/probe32 [get_nets [list fifo_s_axi_awready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe33]
set_property port_width 1 [get_debug_ports u_ila_0/probe33]
connect_debug_port u_ila_0/probe33 [get_nets [list fifo_s_axi_awvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe34]
set_property port_width 1 [get_debug_ports u_ila_0/probe34]
connect_debug_port u_ila_0/probe34 [get_nets [list fifo_s_axi_bready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe35]
set_property port_width 1 [get_debug_ports u_ila_0/probe35]
connect_debug_port u_ila_0/probe35 [get_nets [list fifo_s_axi_bvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe36]
set_property port_width 1 [get_debug_ports u_ila_0/probe36]
connect_debug_port u_ila_0/probe36 [get_nets [list fifo_s_axi_rready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe37]
set_property port_width 1 [get_debug_ports u_ila_0/probe37]
connect_debug_port u_ila_0/probe37 [get_nets [list fifo_s_axi_rvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe38]
set_property port_width 1 [get_debug_ports u_ila_0/probe38]
connect_debug_port u_ila_0/probe38 [get_nets [list fifo_s_axi_wready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe39]
set_property port_width 1 [get_debug_ports u_ila_0/probe39]
connect_debug_port u_ila_0/probe39 [get_nets [list fifo_s_axi_wvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe40]
set_property port_width 1 [get_debug_ports u_ila_0/probe40]
connect_debug_port u_ila_0/probe40 [get_nets [list m_axis_rxd_tlast]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe41]
set_property port_width 1 [get_debug_ports u_ila_0/probe41]
connect_debug_port u_ila_0/probe41 [get_nets [list m_axis_rxd_tready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe42]
set_property port_width 1 [get_debug_ports u_ila_0/probe42]
connect_debug_port u_ila_0/probe42 [get_nets [list m_axis_rxd_tvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe43]
set_property port_width 1 [get_debug_ports u_ila_0/probe43]
connect_debug_port u_ila_0/probe43 [get_nets [list s_axi_arready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe44]
set_property port_width 1 [get_debug_ports u_ila_0/probe44]
connect_debug_port u_ila_0/probe44 [get_nets [list s_axi_arvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe45]
set_property port_width 1 [get_debug_ports u_ila_0/probe45]
connect_debug_port u_ila_0/probe45 [get_nets [list s_axi_awready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe46]
set_property port_width 1 [get_debug_ports u_ila_0/probe46]
connect_debug_port u_ila_0/probe46 [get_nets [list s_axi_awvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe47]
set_property port_width 1 [get_debug_ports u_ila_0/probe47]
connect_debug_port u_ila_0/probe47 [get_nets [list s_axi_bready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe48]
set_property port_width 1 [get_debug_ports u_ila_0/probe48]
connect_debug_port u_ila_0/probe48 [get_nets [list s_axi_bvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe49]
set_property port_width 1 [get_debug_ports u_ila_0/probe49]
connect_debug_port u_ila_0/probe49 [get_nets [list s_axi_rready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe50]
set_property port_width 1 [get_debug_ports u_ila_0/probe50]
connect_debug_port u_ila_0/probe50 [get_nets [list s_axi_rvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe51]
set_property port_width 1 [get_debug_ports u_ila_0/probe51]
connect_debug_port u_ila_0/probe51 [get_nets [list s_axi_wready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe52]
set_property port_width 1 [get_debug_ports u_ila_0/probe52]
connect_debug_port u_ila_0/probe52 [get_nets [list s_axi_wvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe53]
set_property port_width 1 [get_debug_ports u_ila_0/probe53]
connect_debug_port u_ila_0/probe53 [get_nets [list s_axis_txc_tlast]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe54]
set_property port_width 1 [get_debug_ports u_ila_0/probe54]
connect_debug_port u_ila_0/probe54 [get_nets [list s_axis_txc_tready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe55]
set_property port_width 1 [get_debug_ports u_ila_0/probe55]
connect_debug_port u_ila_0/probe55 [get_nets [list s_axis_txc_tvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe56]
set_property port_width 1 [get_debug_ports u_ila_0/probe56]
connect_debug_port u_ila_0/probe56 [get_nets [list s_axis_txd_tlast]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe57]
set_property port_width 1 [get_debug_ports u_ila_0/probe57]
connect_debug_port u_ila_0/probe57 [get_nets [list s_axis_txd_tready]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe58]
set_property port_width 1 [get_debug_ports u_ila_0/probe58]
connect_debug_port u_ila_0/probe58 [get_nets [list s_axis_txd_tvalid]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe59]
set_property port_width 1 [get_debug_ports u_ila_0/probe59]
connect_debug_port u_ila_0/probe59 [get_nets [list mii_rx_clk]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe60]
set_property port_width 1 [get_debug_ports u_ila_0/probe60]
connect_debug_port u_ila_0/probe60 [get_nets [list mii_rx_dv]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe61]
set_property port_width 1 [get_debug_ports u_ila_0/probe61]
connect_debug_port u_ila_0/probe61 [get_nets [list mii_rx_er]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe62]
set_property port_width 1 [get_debug_ports u_ila_0/probe62]
connect_debug_port u_ila_0/probe62 [get_nets [list mii_tx_clk]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe63]
set_property port_width 1 [get_debug_ports u_ila_0/probe63]
connect_debug_port u_ila_0/probe63 [get_nets [list mii_tx_en]]
create_debug_port u_ila_0 probe
set_property PROBE_TYPE DATA_AND_TRIGGER [get_debug_ports u_ila_0/probe64]
set_property port_width 1 [get_debug_ports u_ila_0/probe64]
connect_debug_port u_ila_0/probe64 [get_nets [list mii_tx_er]]
set_property C_CLK_INPUT_FREQ_HZ 300000000 [get_debug_cores dbg_hub]
set_property C_ENABLE_CLK_DIVIDER false [get_debug_cores dbg_hub]
set_property C_USER_SCAN_CHAIN 1 [get_debug_cores dbg_hub]
connect_debug_port dbg_hub/clk [get_nets sys_clk]
