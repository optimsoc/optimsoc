# Copyright (c) 2016 by the author(s)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author(s):
#   Philipp Wagner <philipp.wagner@tum.de>

import filecmp
import inspect
import os
import re
import os
import subprocess
import shlex
import logging
import time


def matches_golden_reference(basedir, testfile, filter_func=None):
    """
    Check if the given file matches a golden reference
    """

    stack = inspect.stack()
    # test_class = stack[1][0].f_locals["self"].__class__.__name__
    this_test_file = os.path.splitext(os.path.basename(str(stack[1][0].f_code.co_filename)))[0]
    this_test_name = str(stack[1][0].f_code.co_name)

    path_test = os.path.join(basedir, testfile);
    path_ref = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                            this_test_file + '.data',
                            this_test_name, testfile)

    if filter_func:
        # filter reference
        ref_lines = open(path_ref, 'U').readlines()
        ref_lines_filtered = filter_func(ref_lines)
        path_ref_filtered = os.path.join(basedir,
                                          testfile+'.reference.filtered');
        f = open(path_ref_filtered, 'w')
        f.write(''.join(ref_lines_filtered))
        f.close()

        # filter test output
        test_lines = open(path_test, 'U').readlines()
        test_lines_filtered = filter_func(test_lines)
        path_test_filtered = path_test+'.filtered'
        f = open(path_test_filtered, 'w')
        f.write(''.join(test_lines_filtered))
        f.close()

        path_ref = path_ref_filtered
        path_test = path_test_filtered

    result = filecmp.cmp(path_ref, path_test, shallow=False)

    if not result:
        logger = logging.getLogger(__name__)
        logger.error("golden reference test: "+
            "test output {} does not match golden reference {}"
            .format(path_test, path_ref))
        ref_lines = open(path_ref, 'U').readlines()
        test_lines = open(path_test, 'U').readlines()
        diff = difflib.unified_diff(ref_lines, test_lines,
                                    fromfile='reference',
                                    tofile='test_output')
        logger.error(''.join(list(diff)))

    return result

def filter_timestamps(in_str_list):
    """
    Filter out timestamps and core IDs in OpTiMSoC STDOUT/STM/CTM log files

    The timestamps depend at least on the compiler, but also on other
    variables. For functional tests we are only interested in the output,
    not the timing of the output.
    """
    filter_expr = re.compile(r'^\[\s*\d+, \d+\] ', flags=re.MULTILINE)
    return [filter_expr.sub(repl='', string=l) for l in in_str_list]

def filter_stm_printf(in_str_list):
    """
    Process STM log file to contain only the printf() outputs from software
    """
    filter_expr = re.compile(r'^[a-z0-9]{8} \d{4} [a-z0-9]{8}$', flags=re.MULTILINE)
    str_filtered = [l for l in in_str_list if filter_expr.match(string=l)]


    remove_ts_expr = re.compile(r'^[a-z0-9]{8} ', flags=re.MULTILINE)
    return [remove_ts_expr.sub(repl='', string=l) for l in str_filtered]


class Process:
    def __init__(self, cmd, logdir, cwd=None, startup_done_expect=None,
               startup_timeout=None):

        if isinstance(cmd, str):
            self.cmd = shlex.split(cmd)
        else:
            self.cmd = cmd
        self.logdir = logdir
        self.cwd = cwd
        self.startup_done_expect = startup_done_expect
        self.startup_timeout = startup_timeout
        self.proc = None
        self.logger = logging.getLogger(__name__)

        self._f_stdout = None
        self._f_stderr = None
        self._f_stdout_r = None

    def __del__(self):
        try:
            self.proc.kill()
            self._f_stdout.close()
            self._f_stderr.close()
            self._f_stdout_r.close()
        except:
            pass

    def run(self):
        cmd_name = os.path.basename(self.cmd[0])

        # enforce line-buffered STDOUT even when sending STDOUT/STDERR to file
        # If applications don't fflush() STDOUT manually, STDOUT goes through
        # a 4kB buffer before we see any output, which prevents searching in the
        # command output for the string indicating a successful startup.
        # see discussion at http://www.pixelbeat.org/programming/stdio_buffering/
        cmd = ['stdbuf', '-oL'] + self.cmd
        self.logger.info("Running command " + ' '.join(cmd))

        logfile_stdout = os.path.join(self.logdir, "{}.stdout".format(cmd_name))
        logfile_stderr = os.path.join(self.logdir, "{}.stderr".format(cmd_name))
        self.logger.debug("Capturing STDOUT at " + logfile_stdout)
        self.logger.debug("Capturing STDERR at " + logfile_stderr)

        self._f_stdout = open(logfile_stdout, 'w')
        self._f_stderr = open(logfile_stderr, 'w')
        self.proc = subprocess.Popen(cmd,
                                     cwd=self.cwd,
                                     universal_newlines=True,
                                     bufsize=1,
                                     stdin=subprocess.PIPE,
                                     stdout=self._f_stdout,
                                     stderr=self._f_stderr)

        self._f_stdout_r = open(logfile_stdout, 'r')

        # no startup match pattern given => startup done!
        if self.startup_done_expect == None:
            return True

        # check if the string indicating a successful startup appears in STDOUT
        init_done = self._find_in_stdout(pattern=self.startup_done_expect,
                                         timeout=self.startup_timeout)

        if not init_done:
            raise subprocess.TimeoutExpired

        self.logger.info("Startup sequence matched, startup done.")

        return True

    def terminate(self):
        self.proc.terminate()

    def expect(self, stdin_data=None, pattern=None, timeout=None):
        """
        Write send to STDIN and check if the output is as expected
        """
        # we don't get STDOUT/STDERR from subprocess.communicate() as it's
        # redirected to file. We need to read the files instead.

        # XXX: races (false positives) can happen here if output is generated
        # before the input is sent to the process.
        if pattern == None:
            self._f_stdout_r.seek(0, 2)

        self.proc.stdin.write(stdin_data)
        self.proc.stdin.flush()

        if pattern == None:
            return True

        return self._find_in_stdout(pattern, timeout)

    def _find_in_stdout(self, pattern, timeout):
        """
        read STDOUT from file to find an expected pattern within timeout seconds
        """
        found = False

        if timeout != None:
            t_end = time.time() + timeout

        while True:
            # check program output as long as there is one
            i = 0
            for line in self._f_stdout_r:
                i += 1
                if hasattr(pattern, "match"):
                    if pattern.match(line):
                        found = True
                else:
                    if line.startswith(pattern):
                        found = True
                        break

                # check if we exceed the timeout while reading from STDOUT
                # do so only every 100 lines to reduce the performance impact
                if timeout != None:
                    if i % 100 == 99 and time.time() >= t_end:
                        break

            if found:
                break

            # wait for 200ms for new output
            if timeout != None:
                try:
                    self.proc.wait(timeout=0.2)
                except subprocess.TimeoutExpired:
                    pass

        return found
