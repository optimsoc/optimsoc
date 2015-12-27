#!/usr/bin/python

from optparse import OptionParser
import os
import subprocess
import shutil

def info(msg):
    print("(I) {}".format(msg))

def fatal(msg):
    print("(E) {}".format(msg))
    exit(1)

def check_base_permission(base):
    return os.access(base, os.F_OK | os.R_OK | os.W_OK | os.X_OK)

def check_program(program):
    try:
        subprocess.check_output("{} --version".format(program), stderr=subprocess.STDOUT, shell=True)
    except subprocess.CalledProcessError:
        fatal("'{}' not found".format(program))
    
def check_make():
    check_program("make")

def check_or1kelf():
    check_program("or1k-elf-gcc")

def check_autotools():
    check_program("autoconf")
    check_program("automake")
    check_program("autoreconf")

def run_command(cmd, **kwargs):
    try:
        subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True, **kwargs)
    except subprocess.CalledProcessError as e:
        fatal("make error: {}\n{}".format(e, e.output))
    

def create_base(options):
    base = options.dest
    force = options.force
    
    if not os.access(base, os.F_OK):
        # The base folder does not exist
        try:
            # Try to create it
            os.makedirs(base)
        except os.error as e:
            fatal("Cannot create base: {}".format(e))
    else:
        # The base folder exists, check permission
        if not check_base_permission(base):
            fatal("Cannot write to '{}'."
                  "You need full permissions.".format(base))
        listdir = os.listdir(base)
        if listdir:
            # The folder is not empty
            if not force:
                fatal("The destination '{}' is not empty. Use -f to force install.".format(base))
            info("Destination not empty. Force clean.")
            for d in listdir:
                shutil.rmtree(os.path.join(base, d))

def install_tools(options):
    src = options.src
    dest = options.dest
    
    info("Install tools")
    
    check_make()
    
    toolsinstdir = os.path.join(dest,"tools")
    try:
        os.mkdir(toolsinstdir)
    except Exception as e:
        fatal(e)
    
    info(" + tcl")
    tclsrcdir = os.path.join(src,"tools","tcl")
    tcldestdir = os.path.join(dest,"tools","tcl")
    
    try:
        shutil.copytree(tclsrcdir, tcldestdir)
    except Exception as e:
        fatal(e)
        
    info(" + utils")
    utilssrcdir = os.path.join(src, "tools", "utils")
    utilsdestdir = os.path.join(dest, "tools", "utils")

    os.mkdir(utilsdestdir)

    try:
        info("  + Build")
        makecmd = "make -C {}".format(utilssrcdir)
        run_command(makecmd)
    except Exception as e:
        fatal(e)
    
    info("  + Install")
    utilsfiles = ['bin2vmem', 'optimsoc-pgas-binary']
    for f in utilsfiles:
        srcf = os.path.join(utilssrcdir, f) 
        destf = os.path.join(utilsdestdir, f) 
        shutil.copy(srcf, destf)

def install_soc_software(options):
    src = options.src
    dest = options.dest
    
    info("Install SoC software")
    check_autotools()
    check_or1kelf()
    
    libs = {}
    libs["baremetal-libs"] = {}
    
    libssrc = os.path.join(src, "src", "sw", "system")
    libsdest = os.path.join(dest, "sw")
    
    for lib in libs:
        info(" + {}".format(lib))
        info("  + autogen")
        cmd = "./autogen.sh"
        cwd = os.path.join(libssrc, lib)
        run_command(cmd, cwd=cwd)
        
        info("  + Configure")
        cwdbuild = os.path.join(cwd, "build")
        try:
            os.mkdir(cwdbuild)
        except OSError as e:
            shutil.rmtree(cwdbuild)
            os.mkdir(cwdbuild)
        
        cmd = "../configure --prefix={} --host=or1k-elf".format(libsdest)
        run_command(cmd, cwd=cwdbuild)

        info("  + Build and Install")
        cmd = "make install"
        run_command(cmd, cwd=cwdbuild)

def install_systemc_library(options):
    src = options.src
    dest = options.dest

    systemcsrc = os.path.join(src, "src", "sysc")
    systemcdest = dest
    
    info("Install SystemC libs")
    check_autotools()
    check_make()
    
    info(" + autogen")
    cmd = "./autogen.sh"
    run_command(cmd, cwd=systemcsrc)
        
    info("  + Configure")
    cwdbuild = os.path.join(systemcsrc, "build")
    try:
        os.mkdir(cwdbuild)
    except OSError as e:
        shutil.rmtree(cwdbuild)
        os.mkdir(cwdbuild)
        
    cmd = "../configure --prefix={}".format(systemcdest)
    run_command(cmd, cwd=cwdbuild)

    info("  + Build and Install")
    cmd = "make install"
    run_command(cmd, cwd=cwdbuild)

if __name__ == '__main__':
    scriptname = os.path.realpath(__file__)
    mysrcdir = os.path.dirname(os.path.dirname(scriptname))
    
    parser = OptionParser()
    parser.add_option("-d", "--destination", dest="dest",
                      help="destination folder", default="/opt/optimsoc")
    parser.add_option("-s", "--source", dest="src",
                      help="source folder", default=mysrcdir)
    parser.add_option("-f", "--force-install", dest="force", action="store_true",
                      help="force installation (removes old)", default=False)

    (options, args) = parser.parse_args()

    info("Install OpTiMSoC")
    info(" source: {}".format(options.src))
    info(" destination: {}".format(options.dest))

    create_base(options)

    install_tools(options)
    
    install_soc_software(options)
    
    install_systemc_library(options)
    
    info("Done")
