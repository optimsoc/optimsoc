#!/usr/bin/python
#
# Build, package and install the OpTiMSoC distribution
#
# This script controls the build process of the various components making up the
# OpTiMSoC framework. Its output is a "distribution directory," which can then
# be copied to a installation location, or zipped and released as binary
# distribution.
#
# We try hard not to leave any intermediate build artifacts inside the source
# tree. All build steps are done inside the "object directory", short objdir.
# The build is then "installed" into the distribution directory.
# One notable exception where files are modified in the source directory are
# the autotools-generated scripts, mostly the configure script.
#

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

    info(" + utils")
    utilssrcdir = os.path.join(src, "tools", "utils")
    utilsobjdir = os.path.join(objdir, "tools", "utils")
    bindistdir = os.path.join(dist, "host", "bin")

    # Build the utils (bin2vmem currently)
    info("  + Build")
    ensure_directory(utilsobjdir)
    makecmd = "make -C {} OBJDIR={}".format(utilssrcdir, utilsobjdir)
    run_command(makecmd, cwd=utilsobjdir)

    # Copy build artifacts
    info("  + Copy build artifacts")
    ensure_directory(bindistdir)
    utilsfiles = ['bin2vmem', 'optimsoc-pgas-binary', 'pkg-config']
    for f in utilsfiles:
        srcf = os.path.join(utilsobjdir, f)
        destf = os.path.join(bindistdir, f)
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

"""Build the hardware modules
"""
def build_hw_modules(options):
    dist = os.path.join(options.objdir, "dist")
    src = options.src

    info("Build hardware modules")

    modsrcdir = os.path.join(src, "src", "soc", "hw")
    moddistdir = os.path.join(dist, "soc", "hw")

    file_copytree(modsrcdir, moddistdir)

"""Build and install the examples
"""
def build_examples(options, env):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Build examples (verilator based)")

    exsrc = os.path.join(src, "examples")
    exobjdir = os.path.join(objdir, "examples")
    exdist = os.path.join(dist, "examples")

    examples = [
      { "name": "compute_tile_simonly",
        "files": [ "build/optimsoc_examples_compute_tile_simonly/sim-verilator/obj_dir/Vtb_compute_tile" ] },
    ]

    for ex in examples:
        info(" + {}".format(ex["name"]))
        buildsrcdir = os.path.join(exsrc, ex["name"])
        buildobjdir = os.path.join(exobjdir, ex["name"])
        builddist = os.path.join(exdist, ex["name"])

        info("  + Build")
        ensure_directory(buildobjdir)
        cmd = "optimsoc-fusesoc --verbose --cores-root {} sim --build-only optimsoc:examples:{}".format(buildsrcdir, ex["name"])
        run_command(cmd, cwd=buildobjdir, env=env)

        info("  + Install build artifacts")
        ensure_directory(builddist)
        for f in ex["files"]:
            srcf = os.path.join(buildobjdir, f)
            destf = os.path.join(builddist, ex["name"])
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

""" Copy lisnoc
"""
def build_externals_lisnoc(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Copy lisnoc")
    srcdir = os.path.join(src, "external", "lisnoc")
    distdir = os.path.join(dist, "external", "lisnoc")

    file_copytree(srcdir, distdir)

""" Copy extra_cores
"""
def build_externals_extra_cores(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Copy extra_cores hardware modules")
    srcdir = os.path.join(src, "external", "extra_cores")
    distdir = os.path.join(dist, "external", "extra_cores")

    file_copytree(srcdir, distdir)

""" Copy GLIP hardware modules
"""
def build_externals_glip(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Copy GLIP hardware modules")
    # XXX: Only copy hardware, not the full tree.
    srcdir = os.path.join(src, "external", "glip")
    distdir = os.path.join(dist, "external", "glip")

    file_copytree(srcdir, distdir)

""" Build and install our private copy of the Open SoC Debug host software
"""
def build_externals_glip_software(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    src = os.path.join(src, "external", "glip")
    objdir = os.path.join(objdir, "external", "opensocdebug")
    dist = os.path.join(dist, "host")
    # magic escaping to ultimately get prefix=${OPTIMSOC}/host into the
    # pkg-config file
    prefix = os.path.join("\$\$\\{OPTIMSOC\\}", "host")

    info("Build GLIP (for host)")
    check_autotools()
    check_make()

    info(" + autogen")
    cmd = "./autogen.sh"
    run_command(cmd, cwd=src)

    info(" + Configure")
    ensure_directory(objdir)

    cmd = "{}/configure --prefix={} --enable-cypressfx2 --enable-jtag --enable-tcp".format(src, prefix)
    run_command(cmd, cwd=objdir)

    info(" + Build")
    cmd = "make"
    run_command(cmd, cwd=objdir)

    info(" + Install build artifacts")
    cmd = "make install prefix={}".format(dist)
    run_command(cmd, cwd=objdir)

""" Build and install the Open SoC Debug hardware components
"""
def build_externals_opensocdebug_hardware(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    srcdir = os.path.join(src, "external", "opensocdebug", "hardware")
    distdir = os.path.join(dist, "external", "opensocdebug", "hardware")

    file_copytree(srcdir, distdir)

""" Build and install our private copy of the Open SoC Debug host software
"""
def build_externals_opensocdebug_software(options, env):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    src = os.path.join(src, "external", "opensocdebug", "software")
    objdir = os.path.join(objdir, "external", "opensocdebug", "software")
    dist = os.path.join(dist, "host")
    # magic escaping to ultimately get prefix=${OPTIMSOC}/host into the
    # pkg-config file
    prefix = os.path.join("\$\$\\{OPTIMSOC\\}", "host")

    info("Build opensocdebug host software")
    check_autotools()
    check_make()

    info(" + autogen")
    cmd = "./autogen.sh"
    run_command(cmd, cwd=src, env=env)

    info(" + Configure")
    ensure_directory(objdir)

    cmd = "{}/configure --prefix={}".format(src, prefix)
    run_command(cmd, cwd=objdir, env=env)

    info(" + Build")
    cmd = "make"
    run_command(cmd, cwd=objdir, env=env)

    info(" + Install build artifacts")
    cmd = "make install prefix={}".format(dist)
    run_command(cmd, cwd=objdir, env=env)


""" Build and install our private copy of FuseSoC

We don't "build" or "install" fusesoc using setuptools, pip or any other of the
numerous options in python to install things. All those methods output
directories which depend on a specific python version, in addition to a specific
OS and architecture.
Instead, just copying the fusesoc sources and calling them directly works just
fine.
"""
def build_externals_fusesoc(options):
    src = options.src
    objdir = options.objdir
    dist = os.path.join(objdir, "dist")

    info("Build and install our private copy of FuseSoC")

    info(" + Copy sources")
    srcdir = os.path.join(src, "external", "fusesoc", "fusesoc")
    distdir = os.path.join(dist, "tools", "fusesoc")
    file_copytree(srcdir, distdir)


    info(" + Copy ipyxact module as dependency into fusesoc")
    srcdir = os.path.join(src, "external", "fusesoc-ipyxact", "ipyxact")
    distdir = os.path.join(dist, "tools", "fusesoc", "ipyxact")
    file_copytree(srcdir, distdir)

    info(" + Create optimsoc-fusesoc wrapper script")
    bindistdir  = os.path.join(dist, "host", "bin")
    ensure_directory(bindistdir)

    fusesoc_wrapper_file = "{}/optimsoc-fusesoc".format(bindistdir)

    fusesoc_wrapper = open(fusesoc_wrapper_file, "w")
    fusesoc_wrapper.write("""#!/bin/sh
test -z "$OPTIMSOC" && (echo 'The environment variable $OPTIMSOC must be set.' >&2; exit 1)
exec python3 $OPTIMSOC/tools/fusesoc/main.py $@
""")
    os.chmod(fusesoc_wrapper_file, 0755)


"""Setup the OpTiMSoC environment variables pointing towards the dist directory

This environment can be used to build software depending on an OpTiMSoC
installation.
"""
def set_environment(options, env):
    dist = os.path.join(options.objdir, "dist")

    env['OPTIMSOC'] = dist
    env['OPTIMSOC_RTL'] = "{}/soc/hw".format(dist)
    env['LISNOC'] = "{}/external/lisnoc".format(dist)
    env['LISNOC_RTL'] = "{}/external/lisnoc/rtl".format(dist)

    env['FUSESOC_CORES'] = (
        "{dist}/soc/hw:"
        "{dist}/external/lisnoc:"
        "{dist}/external/opensocdebug/hardware:"
        "{dist}/external/extra_cores:"
        "{dist}/external/glip".format(dist=dist));

    pkgconfig = (
        "{dist}/host/share/pkgconfig:"
        "{dist}/host/lib/pkgconfig:"
        "{dist}/soc/sw/share/pkgconfig".format(dist=dist))
    if 'PKG_CONFIG_PATH' in env:
        env['PKG_CONFIG_PATH'] = "{}:{}".format(pkgconfig, env['PKG_CONFIG_PATH'])
    else:
        env['PKG_CONFIG_PATH'] = pkgconfig

    env['PATH'] =  "{dist}/host/bin:{existing_path}".format(dist=dist, existing_path=env['PATH'])

    ldlibrary = "{dist}/host/lib".format(dist=dist)
    if 'LD_LIBRARY_PATH' in env:
        env['LD_LIBRARY_PATH'] = "{}:{}".format(ldlibrary, env['LD_LIBRARY_PATH'])
    else:
        env['LD_LIBRARY_PATH'] = ldlibrary

"""Write the OpTiMSoC environment setup file (optimsoc-environment.sh)
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

# Check if we were able to determine the optimsoc installation path properly.
# It is known to fail with some shells if sourced not from the installation
# directory itself.
if [ ! -f "$OPTIMSOC/optimsoc-environment.sh" ]; then
  echo "Unable to determine OpTiMSoC path from shell." >&2
  echo "Source this file directly from the installation directory." >&2
  echo "$> cd optimsoc/installation/directory" >&2
  echo "$> source optimsoc-environment.sh" >&2
  return
fi

export OPTIMSOC
export OPTIMSOC_RTL=$OPTIMSOC/soc/hw
export OPTIMSOC_VERSION={}
export LISNOC=$OPTIMSOC/external/lisnoc
export LISNOC_RTL=$LISNOC/rtl

export FUSESOC_CORES=$OPTIMSOC/soc/hw:$OPTIMSOC/external/lisnoc:$OPTIMSOC/external/opensocdebug/hardware:$OPTIMSOC/external/extra_cores:$OPTIMSOC/external/glip:$FUSESOC_CORES

export PKG_CONFIG_PATH=$OPTIMSOC/host/share/pkgconfig:$OPTIMSOC/host/lib/pkgconfig:$OPTIMSOC/soc/sw/share/pkgconfig:$PKG_CONFIG_PATH
export PATH=$OPTIMSOC/host/bin:$PATH
export LD_LIBRARY_PATH=$OPTIMSOC/host/lib:$LD_LIBRARY_PATH
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
    parser.add_option("--no-examples", dest="noexamples", action="store_true",
                      help="Skip building of examples [default: %default]",
                      default=False)
    parser.add_option("--verbose", dest="verbose", action="store_true",
                      help="Enable verbose logging output [default: %default]",
                      default=False)

    (options, args) = parser.parse_args()

    logging_verbose = options.verbose

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

        build_hw_modules(options)

        if not options.nodoc:
            build_docs(options)

        # External dependencies
        build_externals_fusesoc(options)

        # Additional hardware
        build_externals_lisnoc(options)
        build_externals_opensocdebug_hardware(options)
        build_externals_extra_cores(options)

        # GLIP
        build_externals_glip(options)
        build_externals_glip_software(options)

        # OSD (uses GLIP; pass the previously built version in the environment)
        env = os.environ
        set_environment(options, env)
        build_externals_opensocdebug_software(options, env)

        # write out optimsoc-environment.sh for our users
        write_environment_file(options)

        # Examples
        if not options.noexamples:
            # From here on we will need our new environment
            env = os.environ
            set_environment(options, env)

            build_examples(options, env)
    except Exception as e:
        if logging_verbose:
            raise
        else:
            fatal(e)


    info("Build finished.")
    distdir = os.path.join(options.objdir, "dist")
    info("All build artifacts are available at {}".format(distdir))
