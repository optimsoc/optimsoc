optimsoc_inc_dir .
optimsoc_inc_dir $OPTIMSOC_RTL

optimsoc_add_file tb_system_2x2_cccc.sv
optimsoc_add_module system_2x2_cccc_dm

if { [info exists OPTIMSOC_VERILATOR] } {
    optimsoc_add_module trace_monitor.r3_checker
} else {
    optimsoc_add_module trace_monitor
}

optimsoc_build_define OR1200_BOOT_ADR=32'h100

optimsoc_build

