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
import xml.etree.ElementTree as ET
import itertools as it
import copy
import multiprocessing

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


class CocotbTest:
    def __init__(self, manifest_path, manifest):
        self.objdir = None
        self.manifest_path = manifest_path
        self.manifest = manifest

    def set_objdir(self, base_dir):
        sub_dir = ""
        if "parameters" in self.manifest:
            param_names = sorted(self.manifest["parameters"])
            for p in param_names:
                sub_dir += "{}_{}__".format(p, self.manifest["parameters"][p])
            sub_dir = sub_dir[:-2].lower()
        else:
            sub_dir = "no_parameters"
        self.objdir = os.path.join(base_dir, self.manifest['toplevel'], sub_dir)

    def _get_makefile_vcs(self):
        pythonpath = os.path.abspath(os.path.dirname(__file__))

        """
        Generate Makefile for running cocotb with Synopsys VCS
        """
        makefile = "# Auto-generated Makefile by cocotb_testrunner for Synopsys VCS\n"
        makefile += "PYTHONPATH := " + pythonpath + ":" + self.manifest['manifest_dir'] + "$(PYTHONPATH)\n"
        makefile += "SIM=vcs\n"
        makefile += "VERILOG_SOURCES=" + " \\\n\t".join(self.manifest["sources"]) + "\n"
        makefile += "TOPLEVEL=" + self.manifest["toplevel"] + "\n"
        makefile += "MODULE=" + self.manifest["module"] + "\n"

        # HDL parameters (passed to toplevel module in the design)
        args_hdl_params = []
        if "parameters" in self.manifest:
            for name, value in self.manifest["parameters"].items():
                args_hdl_params.append("-pvalue+{}={}".format(self.manifest["toplevel"]+'.'+name, value))

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

    def _collect_results(self):
        results_xml = os.path.join(self.objdir, 'results.xml')
        try:
            tree = ET.parse(results_xml)
        except:
            res = {'failed': True, 'name': self.manifest['module'] + ":" + os.path.basename(self.objdir),
                   'failuremsg': 'No results.xml generated. Compilation error?'}
            return {'any_failed': True, 'results': [res]}

        root = tree.getroot()

        results = []
        any_failed = False
        for tc in root.find("testsuite").findall("testcase"):
            tc_result = {}
            tc_result['failed'] = False
            tc_result['failuremsg'] = ''
            tc_result['name'] = tc.get('classname') + '.' + tc.get('name') +\
            ":" + os.path.basename(self.objdir)

            for failure in tc.iter("failure"):
                tc_result['failuremsg'] += failure.get('stdout')
                tc_result['failed'] = True
                any_failed = True

            results.append(tc_result)

        return {'any_failed': any_failed, 'results': results}

    def get_call_str(self, gui, loglevel='INFO', seed=None, testcase=None):
        self._prepare_objdir()

        make_args = []
        sim_args = ""
        if gui:
            sim_args = "-gui"

        make_args.append("SIM_ARGS=%s" % sim_args)

        if seed:
            make_args.append("RANDOM_SEED=%d" % seed)

        return "@make -C {} {}".format(self.objdir, ' '.join(make_args))

class CocotbTestRunner:
    def __init__(self, objdir=None):
        self.objdir = objdir
        self.tests = []

    def _create_makefile(self, gui=False, loglevel='INFO', seed=None, testcase=None, jobs=False):
        call_strings = []
        for t in self.tests:
            t.set_objdir(base_dir=self.objdir)
            ensure_directory(t.objdir, recursive=True)
            call_strings.append(t.get_call_str(gui, loglevel, seed, testcase))

        makefile_contents = "all: "
        for i in range(0,len(call_strings)):
            makefile_contents += "run_test_{} ".format(i+1)

        for i in range(0,len(call_strings)):
            runtest = "run_test_{}".format(i+1)
            logfile = " > {}/run.log 2>&1".format(call_strings[i].split(' ')[2]) if jobs else ""
            pre_run_message = "\n\t@echo running test {}...".format(i+1) if jobs else ""
            post_run_message = "\n\t@echo ...test {} finished.".format(i+1) if jobs else ""
            makefile_contents += "\n\n{}:{}".format(runtest, pre_run_message)
            makefile_contents += "\n\t{}{}{}".format(call_strings[i], logfile, post_run_message)

        with open("{}/Makefile".format(self.objdir), "w") as fp_makefile:
            fp_makefile.write(makefile_contents)

    def run_tests(self, gui=False, loglevel='INFO', seed=None, testcase=None, jobs=False):
        """
        Run all discovered tests

        Return True if all tests were successful, False otherwise
        """
        if (gui or testcase) and len(self.tests) > 1:
            print("Running multiple tests at once is not possible with "
                  "-g/--gui or -t/--testcase.\nPlease run again with a single "
                  "test.")
            exit(1)

        all_tests_successful = True
        all_results = []

        # create makefile to run all tests
        self._create_makefile(gui, loglevel, seed, testcase, jobs)

        env = os.environ
        env['COCOTB_LOG_LEVEL'] = loglevel
        if testcase:
            env['TESTCASE'] = testcase

        make_args = []
        if jobs:
            maxjobs = multiprocessing.cpu_count() * 2
            ntests = len(self.tests)
            njobs = maxjobs if ntests >= maxjobs else ntests
            print("Running {} tests using {} jobs...".format(ntests, njobs))
            make_args.append("-j{}".format(njobs))

        # run tests
        subprocess.run(["make"] + make_args, cwd=self.objdir, env=env)

        # collect results
        for t in self.tests:
            r = t._collect_results()
            results = r['results']
            if r['any_failed']:
                all_tests_successful = False
            all_results.append(results)

        print("\n\n=== TEST RESULT SUMMARY ===")
        for result in all_results:
            for tc in result:
                if tc['failed']:
                    print("\033[91mFAIL {name}: {errormsg}\033[0m".format(
                        name=tc['name'], errormsg=tc['failuremsg']))
                else:
                    print("\033[92mPASS {name}\033[0m".format(name=tc['name']))

        return all_tests_successful

    def _abspath(self, path, basedir):
        """
        Return the absolute path for |path| relative to |basedir| with all
        environment variables expanded.
        """
        path = os.path.expandvars(path)

        if path.startswith('/'):
            return path
        return os.path.abspath(os.path.join(basedir, path))

    def parse_manifest(self, manifest_path, all_comb):
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

        # read all HDL parameters (passed to toplevel module in the design)
        test_combinations = []
        if "parameters" in manifest and (all_comb or "parameter_includes" not in manifest):
            parameters = {}
            for name, value in manifest["parameters"].items():
                parameters[name] = value if type(value) is list else [value]
            # create a list with all combinations of the parameters
            param_names = sorted(parameters)
            test_combinations = [dict(zip(param_names, prod)) for prod in it.product(*(parameters[param_name] for param_name in param_names))]
        elif "parameter_includes" in manifest:
            for d in manifest["parameter_includes"]:
                parameters = {}
                for name, value in d.items():
                    parameters[name] = value
                test_combinations.append(parameters)

        return manifest, test_combinations

    def discover_tests(self, search_base, all_comb):
        self.tests = []
        if os.path.isfile(search_base):
            test_manifests = [ search_base ]
        else:
            search_expr = '{}/**/*.manifest.yaml'.format(search_base)
            test_manifests = glob.iglob(search_expr, recursive=True)

        for f in test_manifests:
            manifest, test_combinations = self.parse_manifest(manifest_path=f, all_comb=all_comb)
            if len(test_combinations) > 0:
                for t in test_combinations:
                    manifest["parameters"] = t
                    cocotb_test = CocotbTest(manifest_path=f, manifest=copy.deepcopy(manifest))
                    self.tests.append(cocotb_test)
            else:
                cocotb_test = CocotbTest(manifest_path=f, manifest=copy.deepcopy(manifest))
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
    parser.add_argument("-a", "--all", action='store_true',
                        help="run tests for all possible parameter combinations")
    parser.add_argument("-j", "--jobs", action='store_true',
                        help="run tests in parallel. The number of jobs is twice "
                        "the number of the processors available. Ignored if only "
                        "one test is specified.")
    parser.add_argument("--seed", type=int, required=False,
                        help="set a fixed seed for the test run")
    parser.add_argument("-t", "--testcase", required=False,
                        help="only run the specified testcases. Expects a "
                        "comma-separated list of test function names, e.g. "
                        "'test_basic,test_advanced'.")
    parser.add_argument('dir_file', nargs='?', default=os.getcwd())

    args = parser.parse_args()

    testrunner = CocotbTestRunner(objdir=args.objdir)
    testrunner.discover_tests(args.dir_file, args.all)

    if len(testrunner.tests) == 0:
        print("No test manifests (*.manifest.yaml) found in " + args.dir_file)
        exit(1)

    all_tests_successful = testrunner.run_tests(gui=args.gui,
                                                loglevel=args.loglevel,
                                                seed=args.seed,
                                                testcase=args.testcase,
                                                jobs=args.jobs)

    if all_tests_successful:
        print("\nAll tests successful.")
        exit(0)
    else:
        print("\nAt least one test failed.")
        exit(1)
