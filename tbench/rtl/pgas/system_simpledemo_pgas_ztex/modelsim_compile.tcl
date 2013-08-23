set OPTIMSOC_TCL $env(OPTIMSOC_TCL)

source $OPTIMSOC_TCL/optimsoc_modelsim.tcl

optimsoc_inc_dir $OPTIMSOC/syn/pgas/system_simpledemo_pgas_ztex/
optimsoc_inc_dir $OPTIMSOC_RTL/debug_system/verilog
optimsoc_inc_dir $LISNOC_RTL/lisnoc16
optimsoc_add_file tb_system_simpledemo_pgas_ztex.sv
optimsoc_add_file $OPTIMSOC/syn/pgas/system_simpledemo_pgas_ztex/system_simpledemo_pgas_ztex.v
optimsoc_add_module clockmanager.ztex115
optimsoc_add_module usb_dbg_if
optimsoc_add_module debug_system
optimsoc_add_module sram.spartan6

optimsoc_add_file ${OPTIMSOC}/syn/pgas/system_simpledemo_pgas_ztex/fifo_usb_dual.v
optimsoc_add_file ${OPTIMSOC}/syn/pgas/system_simpledemo_pgas_ztex/fifo_usb_to_noc.v

optimsoc_add_file $env(XILINX_ISE)/verilog/src/glbl.v

optimsoc_add_module system_simpledemo_pgas

optimsoc_build_define OPTIMSOC_MAINMEM_BRAM
optimsoc_build_define OR1200_BOOT_ADR=32'h100

optimsoc_build