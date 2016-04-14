#!/usr/bin/python

from optparse import OptionParser
import os
import subprocess
import shutil

###############################################################################
# Logging

"""Print debug message

If verbose logging is enabled, prints an debug message prepended by (D)
on the command line. Otherwise nothing is done.
"""
def dbg(msg):
    if logging_debug:
        print("(D) {}".format(msg))

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

"""Check if user can access the given directory for reading and writing

Check the directories permissions and return if exists and rwx
"""
def check_dir_accessible_rw(directory):
    return os.access(directory, os.F_OK | os.R_OK | os.W_OK | os.X_OK)

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

"""Copy a file tree
"""
def file_copy(src, dest):
    dbg("Copying {} -> {}".format(src, dest))
    try:
      shutil.copy(src, dest)
    except IOError as e:
      fatal("Copying file {} to {} failed: {}".format(src, dest, e))


def file_copytree(srcdir, destdir):
    dbg("Copying file tree {} -> {}".format(srcdir, destdir))
    try:
        shutil.copytree(srcdir, destdir)
    except IOError as e:
      fatal("Copying file tree {} to {} failed: {}".format(srcdir, destdir, e))


"""Run a command

Runs a command and fails if it returns with a code other than 0. Prints the
commands output and exits then.
"""
def run_command(cmd, **kwargs):
    try:
        dbg("Executing command:\n{}".format(cmd))
        subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True, **kwargs)
    except subprocess.CalledProcessError as e:
        fatal("Error {}\n{}".format(e, e.output))

"""Ensure a directory exists

Ensures that a directory exists and that the script is able to write into it.
If the directory does not exist yet, it is created.
If the recursive parameter is set to True, the creation is done recursively,
i.e. all required parents of the target folder are created as well.
"""
def ensure_directory(dirpath, recursive=True):
    if os.access(dirpath, os.F_OK):
        # Directory exists, check for correct permissions
        if not check_dir_accessible_rw(dirpath):
            fatal("Cannot write to '{}'."
                  "You need full permissions.".format(dirpath))
        return

    # The folder does not exist
    try:
        # Try to create it
        if recursive:
            dbg("Recursively creating directory {}".format(dirpath))
            os.makedirs(dirpath)
        else:
            dbg("Creating directory {}").format(dirpath)
            os.mkdir(dirpath)
    except os.error as e:
        fatal("Cannot create directory {}: {}".format(dirpath, e))


"""Prepare the build output directory (distdir)

The distdir contains the staged build artifacts, ready to be either copied
to another location (installed), or to be packaged into a distribution bundle.
"""
def prepare_distdir(options):
    distdir = os.path.join(options.objdir, "dist")

    if not os.access(distdir, os.F_OK):
        # The folder does not exist
        try:
            # Try to create it
            os.makedirs(distdir)
        except os.error as e:
            fatal("Cannot create distdir: {}".format(e))
        return

    # dir exists: remove all contents
    listdir = os.listdir(distdir)
    if listdir:
        for d in listdir:
            p = os.path.join(distdir, d)
            if os.path.isdir(p):
                shutil.rmtree(p)
            else:
                os.remove(p)


"""Build the OpTiMSoC tools

Build host tools (small utilities).
"""
def build_tools(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Build host tools")

    check_make()

    # Create tools folder in dist
    toolsdistdir = os.path.join(dist, "tools")
    ensure_directory(toolsdistdir)

    info(" + tcl")
    tclsrcdir = os.path.join(src, "tools", "tcl")
    tcldistdir = os.path.join(dist, "tools", "tcl")

    # Simply copy tcl sources
    file_copytree(tclsrcdir, tcldistdir)

    info(" + utils")
    utilssrcdir = os.path.join(src, "tools", "utils")
    utilsobjdir = os.path.join(objdir, "tools", "utils")
    utilsdistdir = os.path.join(dist, "tools", "utils")

    # Build the utils (bin2vmem currently)
    info("  + Build")
    ensure_directory(utilsobjdir)
    makecmd = "make -C {} OBJDIR={}".format(utilssrcdir, utilsobjdir)
    run_command(makecmd, cwd=utilsobjdir)

    # Copy build artifacts
    info("  + Copy build artifacts")
    ensure_directory(utilsdistdir)
    utilsfiles = ['bin2vmem', 'optimsoc-pgas-binary', 'pkg-config']
    for f in utilsfiles:
        srcf = os.path.join(utilsobjdir, f)
        destf = os.path.join(utilsdistdir, f)
        file_copy(srcf, destf)

"""Build the SoC software

Builds the SoC software.
"""
def build_soc_software(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Build SoC software")
    check_autotools()
    check_or1kelf()

    # List of libraries to build
    libs = {}
    libs["baremetal-libs"] = {}

    libssrc = os.path.join(src, "src", "soc", "sw")
    libsobjdir = os.path.join(objdir, "soc", "sw")
    libsdist = os.path.join(dist, "soc", "sw")
    # magic escaping to ultimately get prefix=${OPTIMSOC}/soc/sw into the
    # pkg-config file
    libsprefix = os.path.join("\$\$\\{OPTIMSOC\\}", "soc", "sw")

    # Build each lib and install
    for lib in libs:
        libsrc = os.path.join(libssrc, lib)
        libobjdir = os.path.join(libsobjdir, lib)

        info(" + {}".format(lib))
        info("  + autogen")
        cmd = "./autogen.sh"
        run_command(cmd, cwd=libsrc)

        info("  + Create object directory")
        ensure_directory(libobjdir)

        info("  + Configure")
        cmd = "{}/configure --prefix={} --host=or1k-elf".format(libsrc, libsprefix)
        run_command(cmd, cwd=libobjdir)

        info("  + Build")
        cmd = "make"
        run_command(cmd, cwd=libobjdir)

        info("  + Install build artifacts")
        cmd = "make install prefix={}".format(libsdist)
        run_command(cmd, cwd=libobjdir)

"""Build simulation library
"""
def build_sim_library(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    simsrc = os.path.join(src, "src", "host", "sim")
    simobjdir = os.path.join(objdir, "host", "sim")
    simdist = os.path.join(dist, "host")
    # magic escaping to ultimately get prefix=${OPTIMSOC}/sim into the
    # pkg-config file
    simprefix = os.path.join("\$\$\\{OPTIMSOC\\}", "host")

    info("Build simulation libs")
    check_autotools()
    check_make()

    info(" + autogen")
    cmd = "./autogen.sh"
    run_command(cmd, cwd=simsrc)

    info(" + Configure")
    ensure_directory(simobjdir)

    cmd = "{}/configure --prefix={}".format(simsrc, simprefix)
    run_command(cmd, cwd=simobjdir)

    info(" + Build")
    cmd = "make"
    run_command(cmd, cwd=simobjdir)

    info(" + Install build artifacts")
    cmd = "make install prefix={}".format(simdist)
    run_command(cmd, cwd=simobjdir)

"""Build the hardware modules
"""
def build_hw_modules(options):
    dist = os.path.join(options.objdir, "dist")
    src = options.src

    info("Build hardware modules")

    modsrcdir = os.path.join(src, "src", "soc", "hw")
    moddistdir = os.path.join(dist, "soc", "hw")

    file_copytree(modsrcdir, moddistdir)

    modsrcdir = os.path.join(src, "external", "lisnoc")
    moddistdir = os.path.join(dist, "external", "lisnoc")

    file_copytree(modsrcdir, moddistdir)

"""Build and install the examples
"""
def build_examples(options, env):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Build examples")

    exsrc = os.path.join(src, "tbench")
    exobjdir = os.path.join(objdir, "examples")
    exdist = os.path.join(dist, "examples")

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
        buildsrcdir = os.path.join(exsrc, ex["path"])
        buildobjdir = os.path.join(exobjdir, ex["path"])
        builddist = os.path.join(exdist, ex["path"])

        info("  + Build")
        ensure_directory(buildobjdir)
        cmd = "make build-verilator OBJDIR={}".format(buildobjdir)
        run_command(cmd, cwd=buildsrcdir, env=env)

        info("  + Install build artifacts")
        ensure_directory(builddist)
        for f in ex["files"]:
            srcf = os.path.join(buildobjdir, f)
            destf = os.path.join(builddist, f)
            file_copy(srcf, destf)

"""Build and install the documentation
"""
def build_docs(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Build and install documentation")


    check_program("pdflatex")

    user_guide_srcdir = os.path.join(src, "doc", "user_guide")
    user_guide_objdir = os.path.join(objdir, "doc", "user_guide")
    ensure_directory(user_guide_objdir)

    info(" + Build")
    cmd="make pdf OBJDIR={}".format(user_guide_objdir)
    run_command(cmd, cwd=user_guide_srcdir)

    info(" + Install build artifacts")
    distdoc = os.path.join(dist, "doc")
    ensure_directory(distdoc)

    srcuser_guide = os.path.join(user_guide_objdir, "user_guide.pdf")
    distuser_guide = os.path.join(distdoc, "user_guide.pdf")
    file_copy(srcuser_guide, distuser_guide)

    srcreadme = os.path.join(src, "README.installed")
    distreadme = os.path.join(dist, "README")
    file_copy(srcreadme, distreadme)

    srccopying = os.path.join(src, "COPYING")
    distcopying = os.path.join(dist, "COPYING")
    file_copy(srccopying, distcopying)

"""Set an environment
"""
def set_environment(options, env):
    dist = os.path.join(options.objdir, "dist")

    env['OPTIMSOC'] = dist
    env['OPTIMSOC_RTL'] = "{}/soc/hw".format(dist)
    env['OPTIMSOC_TCL'] = "{}/tools/tcl".format(dist)
    env['LISNOC'] = "{}/external/lisnoc".format(dist)
    env['LISNOC_RTL'] = "{}/external/lisnoc/rtl".format(dist)

    pkgconfig = "{dist}/host/share/pkgconfig:{dist}/soc/sw/share/pkgconfig".format(dist=dist)
    if 'PKG_CONFIG_PATH' in env:
        env['PKG_CONFIG_PATH'] = "{}:{}".format(pkgconfig, env['PKG_CONFIG_PATH'])
    else:
        env['PKG_CONFIG_PATH'] = pkgconfig

    env['PATH'] =  "{dist}/tools/utils:{existing_path}".format(dist=dist, existing_path=env['PATH'])

    ldlibrary = "{dist}/host/lib".format(dist=dist)
    if 'LD_LIBRARY_PATH' in env:
        env['LD_LIBRARY_PATH'] = "{}:{}".format(ldlibrary, env['LD_LIBRARY_PATH'])
    else:
        env['LD_LIBRARY_PATH'] = ldlibrary

"""Write the OpTiMSoC environment setup file
"""
def write_environment_file(options):
    dist = os.path.join(options.objdir, "dist")
    environment_sh = open("{}/optimsoc-environment.sh".format(dist), "w")

    info("Write OpTiMSoC environment file")

    environment_sh.write("""
# This file sets all environment variables necessary to run the installed
# OpTiMSoC framework distribution contained in this folder.
# Source this file to work with OpTiMSoC, i.e. run in your console
# $> source optimsoc-environment.sh

if [ -n "$BASH_SOURCE" ]; then
   OPTIMSOC="`cd "$( dirname "${{BASH_SOURCE[0]}}" )" && pwd `"
elif [ -n "$ZSH_NAME" ]; then
   OPTIMSOC="`dirname $0`"
else
   OPTIMSOC="`pwd`"
fi

export OPTIMSOC
export OPTIMSOC_RTL=$OPTIMSOC/soc/hw
export OPTIMSOC_TCL=$OPTIMSOC/tools/tcl
export OPTIMSOC_VERSION={}
export LISNOC=$OPTIMSOC/external/lisnoc
export LISNOC_RTL=$LISNOC/rtl

export PKG_CONFIG_PATH=$OPTIMSOC/host/share/pkgconfig:$OPTIMSOC/soc/sw/share/pkgconfig:$PKG_CONFIG_PATH
export PATH=$OPTIMSOC/tools/utils:$PATH
export LD_LIBRARY_PATH=$OPTIMSOC/lib:$OPTIMSOC/host/lib:$LD_LIBRARY_PATH
""".format(options.version))

"""Get the version number from the source code
"""
def get_version(src):
    srctools = os.path.join(src, "tools")

    get_version_tool = os.path.join(srctools, "get-version.sh")

    proc = subprocess.Popen(get_version_tool, stdout=subprocess.PIPE,
                            shell=True, cwd=srctools)
    return proc.stdout.read().split("\n", 1)[0]


if __name__ == '__main__':
    scriptname = os.path.realpath(__file__)
    mysrcdir = os.path.dirname(os.path.dirname(scriptname))

    parser = OptionParser()
    parser.add_option("-o", "--objdir", dest="objdir",
                      help="object directory [default: %default]",
                      default=os.path.join(mysrcdir, "objdir"))
    parser.add_option("-s", "--source", dest="src",
                      help="source folder [default: %default]",
                      default=mysrcdir)
    parser.add_option("-v", "--set-version", dest="version",
                      help="set the version number to the given value, "
                           "overriding the detected version.")
    parser.add_option("--no-doc", dest="nodoc", action="store_true",
                      help="Skip building of documentation [default: %default]",
                      default=False)
    parser.add_option("-d", "--debug", dest="debug", action="store_true",
                      help="Enable verbose debug logging output [default: %default]",
                      default=False)

    (options, args) = parser.parse_args()

    logging_debug = options.debug

    if not options.version:
        options.version = get_version(mysrcdir)

    # ensure absolute paths for source and object directories
    options.src = os.path.abspath(options.src)
    options.objdir = os.path.abspath(options.objdir)

    info("Building OpTiMSoC")
    info(" version: {}".format(options.version))
    info(" source: {}".format(options.src))
    info(" objdir: {}".format(options.objdir))


    try:
        # Prepare build output directory structure
        ensure_directory(options.objdir)
        prepare_distdir(options)

        # Build an install into objdir/dist all parts of OpTiMSoC
        build_tools(options)

        build_soc_software(options)

        build_sim_library(options)

        build_hw_modules(options)

        if not options.nodoc:
            build_docs(options)

        write_environment_file(options)

        # From here on we will need our new environment
        env = os.environ
        set_environment(options, env)

        build_examples(options, env)
    except Exception as e:
        if logging_debug:
            raise
        else:
            fatal(e)


    info("Build finished.")
    distdir = os.path.join(options.objdir, "dist")
    info("All build artifacts are available at {}".format(distdir))
