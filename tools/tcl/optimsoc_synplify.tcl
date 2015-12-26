
puts {
  OpTiMSoC compile script
}

set file_list { }
set inc_dirs { }
set defines { }
set build_defines { }

set OPTIMSOC_RTL $env(OPTIMSOC_RTL)
set OPTIMSOC $env(OPTIMSOC)
set LISNOC_RTL $env(LISNOC_RTL)

# The environment variable OPTIMSOC_SYN_OUTDIR lets a user specify
# the working/output directory of a synthesis. This allows for the
# synthesis results to be outside the source tree.
# If this environment variable is not set, the synthesis is done
# in the directory where it was started.
if { [info exists env(OPTIMSOC_SYN_OUTDIR)] } {
    set OPTIMSOC_SYN_OUTDIR $env(OPTIMSOC_SYN_OUTDIR)
} else {
    set OPTIMSOC_SYN_OUTDIR "."
}

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
	    add_file $filename
	}
    } else {
	set fname $OPTIMSOC_RTL/$optimsoc_current_module/verilog/$filename
	if { [lsearch $file_list $fname] == -1 } {
	    lappend file_list $fname
	    add_file $fname
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
	    add_file $filename
	}
    } else {
	if { [lsearch $file_list $LISNOC_RTL/$filename] == -1 } {
	    lappend file_list $LISNOC_RTL/$filename
	    add_file $LISNOC_RTL/$filename
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

proc optimsoc_get_incdirs {} {
    global inc_dirs
    puts [join $inc_dirs ";"]
    return [join $inc_dirs ";"]
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
