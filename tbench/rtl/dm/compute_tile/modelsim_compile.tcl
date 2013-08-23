set OPTIMSOC_TCL $env(OPTIMSOC_TCL)

source $OPTIMSOC_TCL/optimsoc_modelsim.tcl

optimsoc_inc_dir .
optimsoc_inc_dir $OPTIMSOC_RTL

optimsoc_add_file tb_compute_tile.sv
optimsoc_add_module compute_tile_dm
optimsoc_add_module trace_monitor

optimsoc_build_define OR1200_BOOT_ADR=32'h100

optimsoc_build