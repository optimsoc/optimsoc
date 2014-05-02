set OPTIMSOC_TCL $env(OPTIMSOC_TCL)

source $OPTIMSOC_TCL/optimsoc_modelsim.tcl

optimsoc_inc_dir .
optimsoc_inc_dir $OPTIMSOC_RTL

optimsoc_inc_dir $OPTIMSOC/syn/dm/system_2x2_cccc_ztex/

optimsoc_add_file tb_system_2x2_cccc_ztex.sv
optimsoc_add_file $OPTIMSOC/syn/dm/system_2x2_cccc_ztex/system_2x2_cccc_ztex.v
optimsoc_add_file $OPTIMSOC/syn/dm/system_2x2_cccc_ztex/fifo_usb_dual.v
optimsoc_add_file $OPTIMSOC/syn/dm/system_2x2_cccc_ztex/fifo_usb_to_noc.v

optimsoc_add_file $env(XILINX_ISE)/verilog/src/glbl.v

optimsoc_add_module system_2x2_cccc_dm
optimsoc_add_module debug_system
optimsoc_add_module clockmanager.ztex115
optimsoc_add_module usb_dbg_if
optimsoc_add_module sram.spartan6

optimsoc_build_define OR1200_BOOT_ADR=32'h100

# override the RAM settings in optimsoc_def.vh for simulation
optimsoc_build_define OPTIMSOC_SRAM_IMPLEMENTATION="PLAIN"
optimsoc_build_define OPTIMSOC_SRAM_VALIDATE_ADDRESS

optimsoc_build

