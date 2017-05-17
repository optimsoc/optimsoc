# FX3 Evaluation Board connected to the FMC HPC1 (J2) connector on the VCU108
# board - 32 bit interface

# FX3: DQ16, FMC: H29, FPGA: AN36
set_property PACKAGE_PIN AN36 [get_ports {fx3_dq[16]}]
# FX3: DQ17, FMC: H31, FPGA: AL35
set_property PACKAGE_PIN AL35 [get_ports {fx3_dq[17]}]
# FX3: DQ18, FMC: H32, FPGA: AL36
set_property PACKAGE_PIN AL36 [get_ports {fx3_dq[18]}]
# FX3: DQ19, FMC: H34, FPGA: AJ30
set_property PACKAGE_PIN AJ30 [get_ports {fx3_dq[19]}]
# FX3: DQ20, FMC: H35, FPGA: AJ31
set_property PACKAGE_PIN AJ31 [get_ports {fx3_dq[20]}]
# FX3: DQ21, FMC: H37, FPGA: AG31
set_property PACKAGE_PIN AG31 [get_ports {fx3_dq[21]}]
# FX3: DQ22, FMC: H38, FPGA: AH31
set_property PACKAGE_PIN AH31 [get_ports {fx3_dq[22]}]
# FX3: DQ23, FMC: G2, FPGA: AK34
set_property PACKAGE_PIN AK34 [get_ports {fx3_dq[23]}]
# FX3: DQ24, FMC: G3, FPGA: AL34
set_property PACKAGE_PIN AL34 [get_ports {fx3_dq[24]}]
# FX3: DQ25, FMC: G9, FPGA: N34
set_property PACKAGE_PIN N34 [get_ports {fx3_dq[25]}]
# FX3: DQ26, FMC: G10, FPGA: N35
set_property PACKAGE_PIN N35 [get_ports {fx3_dq[26]}]
# FX3: DQ27, FMC: G12, FPGA: M35
set_property PACKAGE_PIN M35 [get_ports {fx3_dq[27]}]
# FX3: DQ28, FMC: G13, FPGA: L35
set_property PACKAGE_PIN L35 [get_ports {fx3_dq[28]}]
# FX3: DQ29, FMC: G15, FPGA: R31
set_property PACKAGE_PIN R31 [get_ports {fx3_dq[29]}]
# FX3: DQ30, FMC: G16, FPGA: P31
set_property PACKAGE_PIN P31 [get_ports {fx3_dq[30]}]
# FX3: DQ31, FMC: G18, FPGA: U31
set_property PACKAGE_PIN U31 [get_ports {fx3_dq[31]}]

set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[16]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[17]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[18]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[19]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[20]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[21]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[22]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[23]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[24]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[25]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[26]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[27]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[28]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[29]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[30]}]
set_property IOSTANDARD LVCMOS18 [get_ports {fx3_dq[31]}]
