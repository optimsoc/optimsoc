# FX3 Evaluation Board connected to the FMC HPC1 (J2) connector on the VCU108
# board

# FX3: A0, FMC: G37, FPGA: AG33
set_property PACKAGE_PIN AG33 [get_ports {fx3_a[0]}]
# FX3: A1, FMC: G36, FPGA: AG32
set_property PACKAGE_PIN AG32 [get_ports {fx3_a[1]}]

# FX3: COM_RST, FMC: C11, FPGA: AN34
set_property PACKAGE_PIN AN34 [get_ports fx3_com_rst]
# FX3: LOGIC_RST, FMC: C10, FPGA: AN35
set_property PACKAGE_PIN AN35 [get_ports fx3_logic_rst]

# FX3: FLAGA, FMC: G25, FPGA: AL31
set_property PACKAGE_PIN AL31 [get_ports fx3_flaga_n]
# FX3: FLAGB, FMC: G27, FPGA: AP36
set_property PACKAGE_PIN AP36 [get_ports fx3_flagb_n]
# FX3: FLAGC, FMC: G28, FPGA: AP37
set_property PACKAGE_PIN AP37 [get_ports fx3_flagc_n]
# FX3: FLAGD, FMC: G31, FPGA: AR38
set_property PACKAGE_PIN AR38 [get_ports fx3_flagd_n]

# FX3: PKTEND, FMC: G30, FPGA: AP38
set_property PACKAGE_PIN AP38 [get_ports fx3_pktend_n]

# FX3: SLCS, FMC: G19, FPGA: U32
set_property PACKAGE_PIN U32 [get_ports fx3_slcs_n]
# FX3: SLOE, FMC: G22, FPGA: AT37
set_property PACKAGE_PIN AT37 [get_ports fx3_sloe_n]
# FX3: SLRD, FMC: G24, FPGA: AL30
set_property PACKAGE_PIN AL30 [get_ports fx3_slrd_n]
# FX3: SLWR, FMC: G21, FPGA: AR37
set_property PACKAGE_PIN AR37 [get_ports fx3_slwr_n]


# FX3: PCLK, FMC: G6, FPGA: T33
set_property PACKAGE_PIN T33 [get_ports fx3_pclk]


# FX3: DQ0, FMC: H4, FPGA: R32
set_property PACKAGE_PIN R32 [get_ports {fx3_dq[0]}]
# FX3: DQ1, FMC: H7, FPGA: N33
set_property PACKAGE_PIN N33 [get_ports {fx3_dq[1]}]
# FX3: DQ2, FMC: H8, FPGA: M33
set_property PACKAGE_PIN M33 [get_ports {fx3_dq[2]}]
# FX3: DQ3, FMC: H10, FPGA: M37
set_property PACKAGE_PIN M37 [get_ports {fx3_dq[3]}]
# FX3: DQ4, FMC: H11, FPGA: L38
set_property PACKAGE_PIN L38 [get_ports {fx3_dq[4]}]
# FX3: DQ5, FMC: H13, FPGA: L34
set_property PACKAGE_PIN L34 [get_ports {fx3_dq[5]}]
# FX3: DQ6, FMC: H14, FPGA: K34
set_property PACKAGE_PIN K34 [get_ports {fx3_dq[6]}]
# FX3: DQ7, FMC: H16, FPGA: Y31
set_property PACKAGE_PIN Y31 [get_ports {fx3_dq[7]}]
# FX3: DQ8, FMC: H17, FPGA: W31
set_property PACKAGE_PIN W31 [get_ports {fx3_dq[8]}]
# FX3: DQ9, FMC: H19, FPGA: T34
set_property PACKAGE_PIN T34 [get_ports {fx3_dq[9]}]
# FX3: DQ10, FMC: H20, FPGA: T35
set_property PACKAGE_PIN T35 [get_ports {fx3_dq[10]}]
# FX3: DQ11, FMC: H22, FPGA: AT39
set_property PACKAGE_PIN AT39 [get_ports {fx3_dq[11]}]
# FX3: DQ12, FMC: H23, FPGA: AT40
set_property PACKAGE_PIN AT40 [get_ports {fx3_dq[12]}]
# FX3: DQ13, FMC: H25, FPGA: AT35
set_property PACKAGE_PIN AT35 [get_ports {fx3_dq[13]}]
# FX3: DQ14, FMC: H26, FPGA: AT36
set_property PACKAGE_PIN AT36 [get_ports {fx3_dq[14]}]
# FX3: DQ15, FMC: H28, FPGA: AM36
set_property PACKAGE_PIN AM36 [get_ports {fx3_dq[15]}]

# FX3: PMODE0, FMC: D11, FPGA: N38
set_property PACKAGE_PIN N38 [get_ports {fx3_pmode[0]}]
# FX3: PMODE1, FMC: D12, FPGA: M38
set_property PACKAGE_PIN M38 [get_ports {fx3_pmode[1]}]
# FX3: PMODE2, FMC: D14, FPGA: M36
set_property PACKAGE_PIN M36 [get_ports {fx3_pmode[2]}]


set_property IOSTANDARD LVCMOS18 [get_ports {fx3_pmode[0]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_pmode[1]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_pmode[2]}]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_flaga_n]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_flagb_n]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_flagc_n]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_flagd_n]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_com_rst]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_logic_rst]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_pclk]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_pktend_n]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_slcs_n]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_sloe_n]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_slrd_n]
set_property IOSTANDARD LVCMOS18 [get_ports fx3_slwr_n]

set_property IOSTANDARD LVCMOS18 [get_ports {fx3_a[1]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_a[0]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[15]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[14]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[13]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[12]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[11]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[10]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[9]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[8]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[7]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[6]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[5]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[4]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[3]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[2]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[1]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[0]}]
