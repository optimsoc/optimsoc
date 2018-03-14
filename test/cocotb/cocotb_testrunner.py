#!/usr/bin/python3
"""
    cocotb_testrunner
    ~~~~~~~~~~~~~~~~~

    Run unit tests using cocotb

    :copyright: Copyright 2017 by the Open SoC Debug team
    :license: MIT, see LICENSE for details.
"""

import os
import yaml
import glob
import argparse
import subprocess

tests = []
objdir = None


"""Check if user can access the given directory for reading and writing

Check the directories permissions and return if exists and rwx
"""
def check_dir_accessible_rw(directory):
    return os.access(directory, os.F_OK | os.R_OK | os.W_OK | os.X_OK)

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
            print("Cannot write to '{}'."
                  "You need full permissions.".format(dirpath))
            exit(1)
        return

    # The folder does not exist
    try:
        # Try to create it
        if recursive:
            print("Recursively creating directory {}".format(dirpath))
            os.makedirs(dirpath)
        else:
            print("Creating directory {}").format(dirpath)
            os.mkdir(dirpath)
    except os.error as e:
        print("Cannot create directory {}: {}".format(dirpath, e))
        exit(1)


class CocotbTest():
    def __init__(self, manifest_path):
        self._manifest_path = None
        self.manifest = None
        self.objdir = None

        self.manifest_path = manifest_path

    def _abspath(self, path, basedir):
        """
        Return the absolute path for |path| relative to |basedir| with all
        environment variables expanded.
        """
        path = os.path.expandvars(path)

        if path.startswith('/'):
            return path
        return os.path.abspath(os.path.join(basedir, path))

    def _parse_manifest(self, manifest_path):
        with open(manifest_path, 'r') as fp_manifest:
            try:
                manifest = yaml.safe_load(fp_manifest)
            except yaml.YAMLError as e:
                print(e)
                return False

        # ensure absolute paths in the list of sources
        manifest_dir = os.path.dirname(manifest_path)
        manifest['sources'][:] = map(lambda x: self._abspath(x, manifest_dir),
                                     manifest['sources'])
        if 'include_dirs' in manifest:
            manifest['include_dirs'][:] =\
                map(lambda x: self._abspath(x, manifest_dir),
                    manifest['include_dirs'])

        manifest['manifest_dir'] = os.path.abspath(manifest_dir)
        return manifest

    @property
    def manifest_path(self):
        return self._manifest_path

    @manifest_path.setter
    def manifest_path(self, manifest_path):
        self.manifest = self._parse_manifest(manifest_path)
        self._manifest_path = manifest_path

    def _get_makefile_vcs(self):
        pythonpath = os.path.abspath(os.path.dirname(__file__))

        """
        Generate Makefile for running cocotb with Synopsys VCS
        """
        makefile = "# Auto-generated Makefile by cocotb_testrunner for Synopsys VCS\n"
        makefile += "PYTHONPATH := " + pythonpath + ":$(PYTHONPATH)\n"
        makefile += "SIM=vcs\n"
        makefile += "VERILOG_SOURCES=" + " \\\n\t".join(self.manifest["sources"]) + "\n"
        makefile += "TOPLEVEL=" + self.manifest["toplevel"] + "\n"
        makefile += "MODULE=" + self.manifest["module"] + "\n"

        # HDL parameters (passed to toplevel module in the design)
        args_hdl_params = []
        if "parameters" in self.manifest:
            for name, value in self.manifest["parameters"].items():
                args_hdl_params.append("-pvalue+{}={}".format(name, value))

        args_incdirs = []
        if "include_dirs" in self.manifest:
            for incdir in self.manifest["include_dirs"]:
                args_incdirs.append("+incdir+{}".format(incdir))

        sim_args = "+lint=all"
        compile_args = "+lint=all -timescale=1ns/10ps " +\
            ' '.join(args_hdl_params) + ' ' +\
            ' '.join(args_incdirs)

        makefile += "SIM_ARGS=" + sim_args + "\n"
        makefile += "COMPILE_ARGS=" + compile_args + "\n"

        makefile += "include $(COCOTB)/makefiles/Makefile.inc\n"
        makefile += "include $(COCOTB)/makefiles/Makefile.sim\n"

        return makefile

    def _prepare_objdir(self):
        # create Makefile
        makefile_contents = self._get_makefile_vcs()
        with open("{}/Makefile".format(self.objdir), "w") as fp_makefile:
            fp_makefile.write(makefile_contents)

    def run(self, gui, loglevel='INFO', seed=None, testcase=None):
        self._prepare_objdir()

        env = os.environ
        env['PYTHONPATH'] = self.manifest['manifest_dir']
        env['COCOTB_LOG_LEVEL'] = loglevel

        if testcase:
            env['TESTCASE'] = testcase

        make_args = []
        sim_args = ""
        if gui:
            sim_args = "-gui"

        make_args.append("SIM_ARGS=%s" % sim_args)

        if seed:
            make_args.append("RANDOM_SEED=%d" % seed)

        subprocess.run(["make"] + make_args, cwd=self.objdir, env=env)

class CocotbTestRunner:
    def __init__(self, objdir=None):
        self.objdir = objdir
        self.tests = []

    def run_tests(self, gui=False, loglevel='INFO', seed=None, testcase=None):
        """
        Run all discovered tests
        """
        if (gui or testcase) and len(self.tests) > 1:
            print("Running multiple tests at once is not possible with "
                  "-g/--gui or -t/--testcase.\nPlease run again with a single "
                  "test.")
            exit(1)

        for t in self.tests:
            t.objdir = os.path.join(self.objdir, t.manifest['toplevel'])
            ensure_directory(t.objdir, recursive=True)
            t.run(gui, loglevel, seed, testcase)

    def discover_tests(self, search_base):
        self.tests = []
        if os.path.isfile(search_base):
            test_manifests = [ search_base ]
        else:
            search_expr = '{}/**/*.manifest.yaml'.format(search_base)
            test_manifests = glob.iglob(search_expr, recursive=True)

        for f in test_manifests:
            cocotb_test = CocotbTest(manifest_path=f)
            self.tests.append(cocotb_test)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Testrunner for cocotb testbenches')
    parser.add_argument("-o", "--objdir",
                        help="object directory [default: %(default)s]",
                        default=os.path.join(os.getcwd(), "objdir"))
    parser.add_argument("-l", "--loglevel",
                        help="cocotb log level [default: %(default)s]",
                        default='INFO')
    parser.add_argument("-g", "--gui", action='store_true',
                        help="show GUI[default: %(default)s]")
    parser.add_argument("--seed", type=int, required=False,
                        help="set a fixed seed for the test run")
    parser.add_argument("-t", "--testcase", required=False,
                        help="only run the specified testcases. Expects a "
                        "comma-separated list of test function names, e.g. "
                        "'test_basic,test_advanced'.")
    parser.add_argument('dir_file', nargs='?', default=os.getcwd())

    args = parser.parse_args()

    testrunner = CocotbTestRunner(objdir=args.objdir)
    testrunner.discover_tests(args.dir_file)

    if len(testrunner.tests) == 0:
        print("No test manifests (*.manifest.yaml) found in " + args.dir_file)
        exit(1)

    testrunner.run_tests(gui=args.gui, loglevel=args.loglevel, seed=args.seed,
                         testcase=args.testcase)
