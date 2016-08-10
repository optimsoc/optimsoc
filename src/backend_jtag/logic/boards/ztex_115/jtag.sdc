#
# GLIP JTAG Clock Constraints for use by Synopsys Synplify and Xilinx Vivado
#

# we assume tck == 15 MHz; adjust as needed
create_clock [get_ports {tck}] -name {tck} -period {66.667}
