# TCL script to create Vivado 2014.1 project

# Set the reference directory for source file relative paths (by default the value is script directory path)
set origin_dir "."

# Set the directory path for the original project from where this script was exported
set orig_proj_dir "[file normalize "$origin_dir/loopback"]"

# Create project
create_project loopback ./loopback

# Set the directory path for the new project
set proj_dir [get_property directory [current_project]]

# Set project properties
set obj [get_projects loopback]
set_property "default_lib" "xil_defaultlib" $obj
set_property "part" "xc7a100tcsg324-2" $obj
set_property "simulator_language" "Mixed" $obj

# Create 'sources_1' fileset (if not found)
if {[string equal [get_filesets -quiet sources_1] ""]} {
  create_fileset -srcset sources_1
}

# Set 'sources_1' fileset object
set obj [get_filesets sources_1]
set files [list \
 "[file normalize "$origin_dir/../../verilog/cdc/cdc_wrptr_full.v"]"\
 "[file normalize "$origin_dir/../../verilog/cdc/cdc_sync_wr2rd.v"]"\
 "[file normalize "$origin_dir/../../verilog/cdc/cdc_sync_rd2wr.v"]"\
 "[file normalize "$origin_dir/../../verilog/cdc/cdc_rdptr_empty.v"]"\
 "[file normalize "$origin_dir/../../verilog/cdc/cdc_fifo_mem.v"]"\
 "[file normalize "$origin_dir/../../verilog/cdc/cdc_fifo.v"]"\
 "[file normalize "$origin_dir/../../verilog/glip_cypressfx2_toplevel.v"]"\
 "[file normalize "$origin_dir/ztex_213_loopback.v"]"\
]
add_files -norecurse -fileset $obj $files

# Set 'sources_1' fileset file properties for remote files
# None

# Set 'sources_1' fileset file properties for local files
# None

# Set 'sources_1' fileset properties
set obj [get_filesets sources_1]
set_property "top" "ztex_213_loopback" $obj

# Create 'constrs_1' fileset (if not found)
if {[string equal [get_filesets -quiet constrs_1] ""]} {
  create_fileset -constrset constrs_1
}

# Set 'constrs_1' fileset object
set obj [get_filesets constrs_1]

# Add/Import constrs file and set constrs file properties
set file "[file normalize "$origin_dir/../../boards/ztex_213/fx2.xdc"]"
set file_added [add_files -norecurse -fileset $obj $file]
set file "$origin_dir/../../boards/ztex_213/fx2.xdc"
set file [file normalize $file]
set file_obj [get_files -of_objects [get_filesets constrs_1] [list "*$file"]]
set_property "file_type" "XDC" $file_obj

# Add/Import constrs file and set constrs file properties
set file "[file normalize "$origin_dir/ztex_213.xdc"]"
set file_added [add_files -norecurse -fileset $obj $file]
set file "$origin_dir/ztex_213.xdc"
set file [file normalize $file]
set file_obj [get_files -of_objects [get_filesets constrs_1] [list "*$file"]]
set_property "file_type" "XDC" $file_obj

# Add/Import constrs file and set constrs file properties
set file "[file normalize "$origin_dir/ztex_213.sdc"]"
set file_added [add_files -norecurse -fileset $obj $file]
set file "$origin_dir/ztex_213.sdc"
set file [file normalize $file]
set file_obj [get_files -of_objects [get_filesets constrs_1] [list "*$file"]]
set_property "file_type" "XDC" $file_obj

# Add/Import constrs file and set constrs file properties
set file "[file normalize "$origin_dir/../../boards/ztex_213/fx2.sdc"]"
set file_added [add_files -norecurse -fileset $obj $file]
set file "$origin_dir/../../boards/ztex_213/fx2.sdc"
set file [file normalize $file]
set file_obj [get_files -of_objects [get_filesets constrs_1] [list "*$file"]]
set_property "file_type" "XDC" $file_obj

# Set 'constrs_1' fileset properties
set obj [get_filesets constrs_1]

# Create 'sim_1' fileset (if not found)
if {[string equal [get_filesets -quiet sim_1] ""]} {
  create_fileset -simset sim_1
}

# Set 'sim_1' fileset object
set obj [get_filesets sim_1]
# Empty (no sources present)

# Set 'sim_1' fileset properties
set obj [get_filesets sim_1]
set_property "top" "ztex_213_loopback" $obj

# Create 'synth_1' run (if not found)
if {[string equal [get_runs -quiet synth_1] ""]} {
  create_run -name synth_1 -part xc7a100tcsg324-2 -flow {Vivado Synthesis 2014} -strategy "Vivado Synthesis Defaults" -constrset constrs_1
} else {
  set_property strategy "Vivado Synthesis Defaults" [get_runs synth_1]
  set_property flow "Vivado Synthesis 2014" [get_runs synth_1]
}
set obj [get_runs synth_1]
set_property "part" "xc7a100tcsg324-2" $obj

# set the current synth run
current_run -synthesis [get_runs synth_1]

# Create 'impl_1' run (if not found)
if {[string equal [get_runs -quiet impl_1] ""]} {
  create_run -name impl_1 -part xc7a100tcsg324-2 -flow {Vivado Implementation 2014} -strategy "Vivado Implementation Defaults" -constrset constrs_1 -parent_run synth_1
} else {
  set_property strategy "Vivado Implementation Defaults" [get_runs impl_1]
  set_property flow "Vivado Implementation 2014" [get_runs impl_1]
}
set obj [get_runs impl_1]
set_property "part" "xc7a100tcsg324-2" $obj

# set the current impl run
current_run -implementation [get_runs impl_1]

puts "INFO: Project created:loopback"
