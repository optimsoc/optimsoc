#!/usr/bin/python

from optparse import OptionParser
import os
import subprocess
import shutil
import errno

def info(msg):
    print("(I) {}".format(msg))

def fatal(msg):
    print("(E) {}".format(msg))
    exit(1)

def check_base_permission(base):
    return os.access(base, os.F_OK | os.R_OK | os.W_OK | os.X_OK)

def check_make():
    try:
        subprocess.check_output("make --version", stderr=subprocess.STDOUT, shell=True)
    except subprocess.CalledProcessError as e:
        fatal("'make' not found")

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
        info("   + make")
        makecmd = "make -C {}".format(utilssrcdir)
        try:
            subprocess.check_output(makecmd, stderr=subprocess.STDOUT, shell=True)
        except subprocess.CalledProcessError as e:
            fatal("make error: {}\n{}".format(e, e.output))
    except Exception as e:
        fatal(e)
    
    info("   + copy")
    utilsfiles = ['bin2vmem', 'optimsoc-pgas-binary']
    for f in utilsfiles:
        srcf = os.path.join(utilssrcdir, f) 
        destf = os.path.join(utilsdestdir, f) 
        shutil.copy(srcf, destf)

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