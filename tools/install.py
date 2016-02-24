#!/usr/bin/python

from optparse import OptionParser
import os
import subprocess
import shutil

###############################################################################
# Logging

"""Print info message

Prints an info message prepended by (I) on the command line
"""
def info(msg):
    print("(I) {}".format(msg))

"""Print warning message

Prints a warning message prepended by (W) on the command line
"""
def warn(msg):
    print("(W) {}".format(msg))

"""Print error message

Prints an error message prepended by (E) on the command line
"""
def error(msg):
    print("(E) {}".format(msg))

"""Print fatal message

Cause a fatal error, print a message prepended by (E) and exit
"""
def fatal(msg):
    print("(E) {}".format(msg))
    exit(1)

###############################################################################
# Checks

"""Check if user can access the given base directory

Check the base directories permissions and return if exists and rwx
"""
def check_base_permission(base):
    return os.access(base, os.F_OK | os.R_OK | os.W_OK | os.X_OK)

"""Check for existence of program

The program is called with --version. It will also fail if the program does not
have a --version option
"""
def check_program(program):
    try:
        cmd = "{} --version".format(program)
        subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
    except subprocess.CalledProcessError:
        fatal("'{}' not found".format(program))

"""Check for make
"""
def check_make():
    check_program("make")

"""Check for or1k-elf-gcc
"""
def check_or1kelf():
    check_program("or1k-elf-gcc")

"""Check for autotools

Checks for autoconf, automake and autoreconf
"""
def check_autotools():
    check_program("autoconf")
    check_program("automake")
    check_program("autoreconf")

###############################################################################
# Helper functions

"""Run a command

Runs a command and fails if it returns with a code other than 0. Prints the
commands output and exits then.
"""
def run_command(cmd, **kwargs):
    try:
        subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True, **kwargs)
    except subprocess.CalledProcessError as e:
        fatal("make error: {}\n{}".format(e, e.output))

"""Create the base folder

Creates the base folder as given as options.dest. If the folder exists, all the
contents are deleted if options.force is set, fails and exits otherwise. Also
fails and exits of the base cannot be created.
"""
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

        # Check if the folder is empty. We don't delete the entire directory as
        # we otherwise may not be able to recreate it.
        listdir = os.listdir(base)
        if listdir:
            # The folder is not empty
            if not force:
                fatal("The destination '{}' is not empty."
                      "Use -f to force install.".format(base))
            info("Destination not empty. Force clean.")
            for d in listdir:
                p = os.path.join(base, d)
                if os.path.isdir(p):
                    shutil.rmtree(p)
                else:
                    os.remove(p)

"""Install the OpTiMSoC tools

Install host tools (small utilities).
"""
def install_tools(options):
    src = options.src
    dest = options.dest

    info("Install tools")

    check_make()

    # Create folder tools
    toolsinstdir = os.path.join(dest, "tools")
    try:
        os.mkdir(toolsinstdir)
    except Exception as e:
        fatal(e)

    info(" + tcl")
    tclsrcdir = os.path.join(src, "tools", "tcl")
    tcldestdir = os.path.join(dest, "tools", "tcl")

    # Plainly copy tcl sources
    try:
        shutil.copytree(tclsrcdir, tcldestdir)
    except Exception as e:
        fatal(e)

    info(" + utils")
    utilssrcdir = os.path.join(src, "tools", "utils")
    utilsdestdir = os.path.join(dest, "tools", "utils")

    try:
        # Create utils dir
        os.mkdir(utilsdestdir)

        # Build the utils (bin2vmem currently)
        info("  + Build")
        makecmd = "make -C {}".format(utilssrcdir)
        run_command(makecmd)
    except Exception as e:
        fatal(e)

    # Copy the util files
    info("  + Install")
    utilsfiles = ['bin2vmem', 'optimsoc-pgas-binary']
    for f in utilsfiles:
        srcf = os.path.join(utilssrcdir, f)
        destf = os.path.join(utilsdestdir, f)
        shutil.copy(srcf, destf)

"""Build and install SoC software

Builds the SoC software and installs it.
"""
def install_soc_software(options):
    src = options.src
    dest = options.dest

    info("Build and install SoC software")
    check_autotools()
    check_or1kelf()

    # List of libraries to build
    libs = {}
    libs["baremetal-libs"] = {}

    libssrc = os.path.join(src, "src", "soc", "sw")
    libsdest = os.path.join(dest, "soc", "sw")

    # Build each lib and install
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
            if e.errno == 17:
                # Delete the folder if it existed
                shutil.rmtree(cwdbuild)
                os.mkdir(cwdbuild)
            else:
                fatal(e)

        cmd = "../configure --prefix={} --host=or1k-elf".format(libsdest)
        run_command(cmd, cwd=cwdbuild)

        info("  + Build and Install")
        cmd = "make install"
        run_command(cmd, cwd=cwdbuild)

"""Install simulation library
"""
def install_sim_library(options):
    src = options.src
    dest = options.dest

    simsrc = os.path.join(src, "src", "host", "sim")
    simdest = os.path.join(dest, "host")

    info("Install simulation libs")
    check_autotools()
    check_make()

    info(" + autogen")
    cmd = "./autogen.sh"
    run_command(cmd, cwd=simsrc)

    info(" + Configure")
    cwdbuild = os.path.join(simsrc, "build")
    try:
        os.mkdir(cwdbuild)
    except OSError as e:
        if e.errno == 17:
            # Delete the folder if it existed
            shutil.rmtree(cwdbuild)
            os.mkdir(cwdbuild)

    cmd = "../configure --prefix={}".format(simdest)
    run_command(cmd, cwd=cwdbuild)

    info(" + Build and Install")
    cmd = "make install"
    run_command(cmd, cwd=cwdbuild)

def install_hw_modules(options):
    dest = options.dest
    src = options.src

    info("Install hardware modules")

    modsrcdir = os.path.join(src, "src", "soc", "hw")
    moddestdir = os.path.join(dest, "soc", "hw")

    try:
        shutil.copytree(modsrcdir, moddestdir)
    except Exception as e:
        fatal(e)

    modsrcdir = os.path.join(src, "external", "lisnoc")
    moddestdir = os.path.join(dest, "external", "lisnoc")

    try:
        shutil.copytree(modsrcdir, moddestdir)
    except Exception as e:
        fatal(e)

"""Build and install the examples
"""
def install_examples(options, env):
    src = options.src
    dest = options.dest

    info("Install examples")

    exsrc = os.path.join(src, "tbench")
    exdest = os.path.join(dest, "examples")

    examples = [ { "path": "dm/compute_tile",
                   "files": [ "tb_compute_tile" ] },
                 { "path": "dm/compute_tile-dual",
                   "files": [ "tb_compute_tile" ] },
                 { "path": "dm/compute_tile-quad",
                   "files": [ "tb_compute_tile" ] },
                 { "path": "dm/compute_tile-octa",
                   "files": [ "tb_compute_tile" ] },
                 { "path": "dm/system_2x2_cccc",
                   "files": [ "tb_system_2x2_cccc" ] },
                 { "path": "dm/system_2x2_cccc-dual",
                   "files": [ "tb_system_2x2_cccc" ] },
                 { "path": "dm/system_2x2_cccc-quad",
                   "files": [ "tb_system_2x2_cccc" ] }]

    for ex in examples:
        info(" + {}".format(ex["path"]))
        buildcwd = os.path.join(exsrc, ex["path"])
        info("  + Clean")
        cmd = "make clean"
        run_command(cmd, cwd=buildcwd, env=env)
        info("  + Build")
        cmd = "make build-verilator"
        run_command(cmd, cwd=buildcwd, env=env)
        info("  + Install")
        os.makedirs(os.path.join(exdest, ex["path"]))
        for f in ex["files"]:
            srcf = os.path.join(exsrc, ex["path"], f)
            destf = os.path.join(exdest, ex["path"], f)
            shutil.copy(srcf, destf)

"""Build and install the documentation
"""
def install_docs(options):
    src = options.src
    dest = options.dest

    info("Build and install documentation")


    check_program("pdflatex")

    cwd = os.path.join(src, "doc", "user_guide")
    user_guide = os.path.join(cwd, "user_guide.pdf")

    info(" + Clean")
    run_command("make clean", cwd=cwd)

    info(" + Build")
    run_command("make pdf", cwd=cwd)

    destdoc = os.path.join(dest, "doc")
    os.makedirs(destdoc)

    info(" + Install")
    destuser_guide = os.path.join(destdoc, "UserGuide.pdf")
    shutil.copy(user_guide, destuser_guide)

    srcreadme = os.path.join(src, "README.installed")
    destreadme = os.path.join(dest, "README")
    shutil.copy(srcreadme, destreadme)

    srccopying = os.path.join(src, "COPYING")
    destcopying = os.path.join(dest, "COPYING")
    shutil.copy(srccopying, destcopying)

"""Set an environment
"""
def set_environment(options, env):
    dest = options.dest

    env['OPTIMSOC'] = dest
    env['OPTIMSOC_RTL'] = "{}/soc/hw".format(dest)
    env['OPTIMSOC_TCL'] = "{}/tools/tcl".format(dest)
    env['LISNOC'] = "{}/external/lisnoc".format(dest)
    env['LISNOC_RTL'] = "{}/external/lisnoc/rtl".format(dest)

    pkgconfig = "{dest}/share/pkgconfig:{dest}/host/share/pkgconfig:{dest}/sw/sharepkgconfig".format(dest=dest)
    if 'PKG_CONFIG_PATH' in env:
        env['PKG_CONFIG_PATH'] = "{}:{}".format(pkgconfig, env['PKG_CONFIG_PATH'])
    else:
        env['PKG_CONFIG_PATH'] = pkgconfig

    ldlibrary = "{dest}/lib:{dest}/host/lib".format(dest=dest)
    if 'LD_LIBRARY_PATH' in env:
        env['LD_LIBRARY_PATH'] = "{}:{}".format(ldlibrary, env['LD_LIBRARY_PATH'])
    else:
        env['LD_LIBRARY_PATH'] = ldlibrary

"""Write files to setup the environment
"""
def write_setup_files(options):
    environment_sh = open("{}/optimsoc-environment.sh".format(options.dest), "w")

    info("Write setup files")

    environment_sh.write("""
# This file sets all environment variables necessary to run the installed
# OpTiMSoC framework distribution contained in this folder.
# Source this file to work with OpTiMSoC, i.e. run in your console
# $> source optimsoc-environment.sh
export OPTIMSOC={}
export OPTIMSOC_RTL=$OPTIMSOC/soc/hw
export OPTIMSOC_TCL=$OPTIMSOC/tools/tcl
export LISNOC=$OPTIMSOC/external/lisnoc
export LISNOC_RTL=$LISNOC/rtl

export PKG_CONFIG_PATH=$OPTIMSOC/host/share/pkgconfig:$OPTIMSOC/soc/sw/share/pkgconfig:$PKG_CONFIG_PATH
export PATH=$OPTIMSOC/tools/utils:$PATH
export LD_LIBRARY_PATH=$OPTIMSOC/lib:$OPTIMSOC/host/lib:$LD_LIBRARY_PATH
""".format(options.dest.rstrip("/")))

if __name__ == '__main__':
    scriptname = os.path.realpath(__file__)
    mysrcdir = os.path.dirname(os.path.dirname(scriptname))

    parser = OptionParser()
    parser.add_option("-d", "--destination", dest="dest",
                      help="destination folder", default="/opt/optimsoc/current")
    parser.add_option("-s", "--source", dest="src",
                      help="source folder", default=mysrcdir)
    parser.add_option("-f", "--force-install", dest="force", action="store_true",
                      help="force installation (removes old)", default=False)
    parser.add_option("--no-doc", dest="nodoc", action="store_true",
                      help="Skip installing of documentation", default=False)

    (options, args) = parser.parse_args()

    info("Install OpTiMSoC")
    info(" source: {}".format(options.src))
    info(" destination: {}".format(options.dest))

    create_base(options)

    install_tools(options)

    install_soc_software(options)

    install_sim_library(options)

    install_hw_modules(options)

    if not options.nodoc:
        install_docs(options)

    write_setup_files(options)

    # From here on we will need our new environment
    env = os.environ
    set_environment(options, env)

    install_examples(options, env)

    info("Done")
