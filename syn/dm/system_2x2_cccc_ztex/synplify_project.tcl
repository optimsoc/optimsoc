set OPTIMSOC_TCL $env(OPTIMSOC_TCL)
set SYNDIR [file dirname [info script]]

source $OPTIMSOC_TCL/optimsoc_synplify.tcl

project -new

optimsoc_inc_dir $SYNDIR
optimsoc_add_file $SYNDIR/system_2x2_cccc_ztex.ucf
optimsoc_add_file $SYNDIR/system_2x2_cccc_ztex.fdc
optimsoc_add_file $SYNDIR/system_2x2_cccc_ztex.v
optimsoc_add_file $SYNDIR/fifo_usb_dual.ngc
optimsoc_add_file $SYNDIR/fifo_usb_dual.v
optimsoc_add_file $SYNDIR/fifo_usb_to_noc.ngc
optimsoc_add_file $SYNDIR/fifo_usb_to_noc.v

optimsoc_add_file $OPTIMSOC/syn/ipcores/xilinx/ztex_ddr/ztex_1_15_mig_39.edf
optimsoc_add_file $SYNDIR/system_2x2_cccc_ztex_ddr.ucf

optimsoc_add_module system_2x2_cccc_dm
optimsoc_add_module debug_system
optimsoc_add_module clockmanager.ztex115
optimsoc_add_module usb_dbg_if
optimsoc_add_module xilinx_ddr.ztex
optimsoc_add_module sram.spartan6

################################################################################
# implementation: "rev_1_15d_bram"
################################################################################

impl -add rev_1_15d_bram -type fpga

# implementation attributes
set_option -vlog_std sysv
set_option -project_relative_includes 1
set_option -include_path [optimsoc_get_incdirs]

# pr_1 attributes
set_option -job pr_1 -add par
set_option -job pr_1 -option enable_run 1
set_option -job pr_1 -option run_backannotation 0

# device options
set_option -technology Spartan6
set_option -part XC6SLX150
set_option -package CSG484
set_option -speed_grade -3
set_option -part_companion ""

# compilation/mapping options
set_option -use_fsm_explorer 0
set_option -top_module "system_2x2_cccc_ztex"

# mapper_options
set_option -frequency auto
set_option -write_verilog 0
set_option -write_vhdl 0

# xilinx_options
set_option -enhanced_optimization 0

# Xilinx Spartan3
set_option -run_prop_extract 1
set_option -maxfan 10000

set_option -disable_io_insertion 0

set_option -pipe 1
set_option -retiming 0
set_option -update_models_cp 0
set_option -fixgatedclocks 3
set_option -fixgeneratedclocks 3
set_option -no_sequential_opt 1
set_option -resolve_multiple_driver 1

# Xilinx Spartan6
set_option -enable_prepacking 1

# NFilter
set_option -popfeed 0
set_option -constprop 0
set_option -createhierarchy 0

# Xilinx
set_option -fc_phys_opt 0

# sequential_optimization_options
set_option -symbolic_fsm_compiler 1

# Compiler Options
set_option -compiler_compatible 0
set_option -resource_sharing 1
set_option -multi_file_compilation_unit 1
set_option -ucf "$SYNDIR/system_2x2_cccc_ztex.ucf"

#automatic place and route (vendor) options
set_option -write_apr_constraint 1

#set result format/file last
project -result_file "$OPTIMSOC_SYN_OUTDIR/rev_1_15d_bram/system_2x2_cccc_ztex115d_bram.edf"

################################################################################
# implementation: "rev_1_15b_ddr"
################################################################################

impl -add rev_1_15b_ddr -type fpga

# implementation attributes
set_option -vlog_std sysv
set_option -project_relative_includes 1
set_option -include_path [optimsoc_get_incdirs]

set_option -hdl_define -set "OPTIMSOC_CTRAM_WIRES"

# pr_1 attributes
set_option -job pr_1 -add par
set_option -job pr_1 -option enable_run 1
set_option -job pr_1 -option run_backannotation 0

# device options
set_option -technology Spartan6
set_option -part XC6SLX75
set_option -package CSG484
set_option -speed_grade -3
set_option -part_companion ""

# compilation/mapping options
set_option -use_fsm_explorer 0
set_option -top_module "system_2x2_cccc_ztex"

# mapper_options
set_option -frequency auto
set_option -write_verilog 0
set_option -write_vhdl 0

# xilinx_options
set_option -enhanced_optimization 0

# Xilinx Spartan3
set_option -run_prop_extract 1
set_option -maxfan 10000

# I/O insertation must be disabled for DDR2 memory
# set OPTIMSOC_MANUAL_IOBUF above if you do this!
#set_option -disable_io_insertion 1
set_option -disable_io_insertion 0

set_option -pipe 1
set_option -retiming 0
set_option -update_models_cp 0
set_option -fixgatedclocks 3
set_option -fixgeneratedclocks 3
set_option -no_sequential_opt 1
set_option -resolve_multiple_driver 1

# Xilinx Spartan6
set_option -enable_prepacking 1

# NFilter
set_option -popfeed 0
set_option -constprop 0
set_option -createhierarchy 0

# Xilinx
set_option -fc_phys_opt 0

# sequential_optimization_options
set_option -symbolic_fsm_compiler 1

# Compiler Options
set_option -compiler_compatible 0
set_option -resource_sharing 1
set_option -multi_file_compilation_unit 1
set_option -ucf "$SYNDIR/system_2x2_cccc_ztex.ucf $SYNDIR/system_2x2_cccc_ztex_ddr.ucf"

#automatic place and route (vendor) options
set_option -write_apr_constraint 1

#set result format/file last
project -result_file "$OPTIMSOC_SYN_OUTDIR/rev_1_15b_ddr/system_2x2_cccc_ztex115b_ddr.edf"


################################################################################
# implementation: "rev_1_15d_ddr"
################################################################################

impl -add rev_1_15d_ddr -type fpga

# implementation attributes
set_option -vlog_std sysv
set_option -project_relative_includes 1
set_option -include_path [optimsoc_get_incdirs]

set_option -hdl_define -set "OPTIMSOC_CTRAM_WIRES"

# pr_1 attributes
set_option -job pr_1 -add par
set_option -job pr_1 -option enable_run 1
set_option -job pr_1 -option run_backannotation 0

# device options
set_option -technology Spartan6
set_option -part XC6SLX150
set_option -package CSG484
set_option -speed_grade -3
set_option -part_companion ""

# compilation/mapping options
set_option -use_fsm_explorer 0
set_option -top_module "system_2x2_cccc_ztex"

# mapper_options
set_option -frequency auto
set_option -write_verilog 0
set_option -write_vhdl 0

# xilinx_options
set_option -enhanced_optimization 0

# Xilinx Spartan3
set_option -run_prop_extract 1
set_option -maxfan 10000

set_option -disable_io_insertion 0

set_option -pipe 1
set_option -retiming 0
set_option -update_models_cp 0
set_option -fixgatedclocks 3
set_option -fixgeneratedclocks 3
set_option -no_sequential_opt 1
set_option -resolve_multiple_driver 1

# Xilinx Spartan6
set_option -enable_prepacking 1

# NFilter
set_option -popfeed 0
set_option -constprop 0
set_option -createhierarchy 0

# Xilinx
set_option -fc_phys_opt 0

# sequential_optimization_options
set_option -symbolic_fsm_compiler 1

# Compiler Options
set_option -compiler_compatible 0
set_option -resource_sharing 1
set_option -multi_file_compilation_unit 1
set_option -ucf "$SYNDIR/system_2x2_cccc_ztex.ucf $SYNDIR/system_2x2_cccc_ztex_ddr.ucf"

#automatic place and route (vendor) options
set_option -write_apr_constraint 1

#set result format/file last
project -result_file "$OPTIMSOC_SYN_OUTDIR/rev_1_15d_ddr/system_2x2_cccc_ztex115d_ddr.edf"

project -save synplify.prj

