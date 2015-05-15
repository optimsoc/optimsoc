
puts {
  OpTiMSoC Verilator compile script
}

set file_list { }
set inc_dirs { }
set defines { }
set build_defines { }

set OPTIMSOC_RTL $env(OPTIMSOC_RTL)
set OPTIMSOC $env(OPTIMSOC)
set LISNOC_RTL $env(LISNOC_RTL)

set optimsoc_current_module ""
set optimsoc_current_submodule ""

proc optimsoc_add_file filename {
    global file_list
    global optimsoc_current_module
    global optimsoc_current_submodule
    global OPTIMSOC_RTL
    if { $optimsoc_current_module=="" } {
	if { [lsearch $file_list $filename] == -1 } {
	    lappend file_list $filename
	}
    } else {
	set fname $OPTIMSOC_RTL/$optimsoc_current_module/verilog/$filename
	if { [lsearch $file_list $fname] == -1 } {
	    lappend file_list $fname
	}
    }
    return {}
}

proc lisnoc_add_file filename {
    global file_list
    global optimsoc_current_module
    global optimsoc_current_submodule
    global LISNOC_RTL

    if { $optimsoc_current_module=="" } {
	if { [lsearch $file_list $filename] == -1 } {
	    lappend file_list $filename
	}
    } else {
	if { [lsearch $file_list $LISNOC_RTL/$filename] == -1 } {
	    lappend file_list $LISNOC_RTL/$filename
	}
    }
    return {}
}

proc optimsoc_inc_dir dir {
    global inc_dirs
    if { [lsearch $inc_dirs $dir] == -1 } {
	lappend inc_dirs "$dir"
    }
    return {}
}

proc optimsoc_add_module module {
    global OPTIMSOC_RTL
    global optimsoc_current_module
    global optimsoc_current_submodule
    set index [string first "." $module]
    if { $index==-1 } {
	set optimsoc_current_module $module
	set optimsoc_current_submodule $module
    } else {
	set optimsoc_current_module [string range $module 0 [expr { $index - 1 }]]
	set optimsoc_current_submodule [string range $module [expr { $index + 1 }] end]
    }
    uplevel #0 source $OPTIMSOC_RTL/$optimsoc_current_module/scripts/$optimsoc_current_submodule.tcl
    set optimsoc_current_module ""
    set optimsoc_current_submodule ""
    return {}
}

proc optimsoc_add_define define {
    global defines
    lappend defines $define
    return {}
}

proc optimsoc_build_define define {
    global build_defines
    lappend build_defines $define
    return {}
}

proc optimsoc_build {} {
    global file_list
    global inc_dirs
    global defines
    global build_defines

    set defs [concat $defines $build_defines]

    # Open .vc file
    set fp [open "verilator.vc" w]

    # write includes to file
    foreach d $inc_dirs {
	puts $fp "+incdir+$d"
    }

    puts $fp ""

    # write defines to file
    foreach d $defs {
	puts $fp "+define+$d"
    }

    puts $fp ""

    # write list of files to file
    foreach f $file_list {
	puts $fp "$f"
    }
}

if {[info exists argv0] && [
    file dirname [file normalize [info script]/...]] eq [
    file dirname [file normalize $argv0/...]]} {

    if {$::argc > 0} {
	foreach arg $::argv {
	    source $arg
	}
    } else {
	puts "When called directly you need to set scripts to include"
    }
}
