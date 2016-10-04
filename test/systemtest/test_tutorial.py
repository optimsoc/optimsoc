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

import os
import subprocess
import pytest
import filecmp
import inspect
import shlex
import logging
import time
import re

logging.basicConfig(level=logging.DEBUG)


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

class TestTutorial:
    """
    Test all examples in the User Guide Tutorial

    The goal of this test case is to ensure that the tutorials as described in
    the user guide continue to work after changes to OpTiMSoC. It therefore
    tests only the examples as given in the tutorial, and does not aim for
    full coverage of the used parts of OpTiMSoC.
    """

    def matches_golden_reference(self, basedir, testfile):
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
        return filecmp.cmp(path_ref, path_test)


    @pytest.fixture
    def sim_system_2x2_cccc_sim_dualcore_debug(self, tmpdir):
        cmd_sim = '{}/examples/sim/system_2x2_cccc/system_2x2_cccc_sim_dualcore_debug'.format(os.environ['OPTIMSOC'])
        p_sim = Process(cmd_sim, logdir=str(tmpdir), cwd=str(tmpdir),
                        startup_done_expect='Glip TCP DPI listening',
                        startup_timeout=10)
        p_sim.run()

        yield p_sim

        p_sim.terminate()

    @pytest.fixture
    def opensocdebugd_tcp(self, tmpdir):
        cmd_opensocdebugd = ['opensocdebugd', 'tcp']
        p_opensocdebugd = Process(cmd_opensocdebugd, logdir=str(tmpdir),
                                  cwd=str(tmpdir),
                                  startup_done_expect="Wait for connection",
                                  startup_timeout=30)
        p_opensocdebugd.run()

        yield p_opensocdebugd

        p_opensocdebugd.terminate()

    def test_baremetal_hello(self, baremetal_apps_hello):
        """
        Ensure that all files which are mentioned to result from compiling the
        hello world example are actually being generated.
        """
        assert baremetal_apps_hello.join('hello.elf').isfile()
        assert baremetal_apps_hello.join('hello.dis').isfile()
        assert baremetal_apps_hello.join('hello.bin').isfile()
        assert baremetal_apps_hello.join('hello.vmem').isfile()


    def test_tutorial1(self, baremetal_apps_hello, tmpdir):
        """
        Tutorial 1: Run hello world on compute_tile_dm with a single core
        """

        # run simulation
        cmd = ['{}/examples/sim/compute_tile/compute_tile_sim_singlecore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello.join('hello.vmem')))]
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # check if output files exist
        assert tmpdir.join('stdout.000').isfile()

        # compare output to golden reference
        assert self.matches_golden_reference(str(tmpdir), 'stdout.000')

    def test_tutorial2(self, baremetal_apps_hello, tmpdir):
        """
        Tutorial 2: generate a VCD file
        """

        cmd = ['{}/examples/sim/compute_tile/compute_tile_sim_singlecore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello.join('hello.vmem'))),
               '--vcd']
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # XXX: We currently only check if the VCD file is written, not if it
        #      contains meaningful content.
        assert tmpdir.join('sim.vcd').isfile()

    def test_tutorial3_dualcore(self, baremetal_apps_hello, tmpdir):
        """
        Tutorial 3: like tutorial 1, just using two cores
        """

        # run simulation
        cmd = ['{}/examples/sim/compute_tile/compute_tile_sim_dualcore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello.join('hello.vmem')))]
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # check all output files
        for f in ['stdout.000', 'stdout.001']:
            assert tmpdir.join(f).isfile()
            assert self.matches_golden_reference(str(tmpdir), f)

    def test_tutorial3_quadcore(self, baremetal_apps_hello, tmpdir):
        """
        Tutorial 3: like tutorial 1, just using four cores
        """

        # run simulation
        cmd = ['{}/examples/sim/compute_tile/compute_tile_sim_quadcore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello.join('hello.vmem')))]
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # check all output files
        for f in ['stdout.000', 'stdout.001', 'stdout.002', 'stdout.003']:
            assert tmpdir.join(f).isfile()
            assert self.matches_golden_reference(str(tmpdir), f)

    def test_tutorial4_hello(self, baremetal_apps_hello, tmpdir):
        # run simulation
        cmd = ['{}/examples/sim/system_2x2_cccc/system_2x2_cccc_sim_dualcore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello.join('hello.vmem')))]
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # check all output files
        for i in range(0, 7):
            f = "stdout.{:03d}".format(i)
            assert tmpdir.join(f).isfile()
            assert self.matches_golden_reference(str(tmpdir), f)

    def test_tutorial4_hello_mpsimple(self, baremetal_apps_hello_mpsimple, tmpdir):
        # run simulation
        cmd = ['{}/examples/sim/system_2x2_cccc/system_2x2_cccc_sim_dualcore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello_mpsimple.join('hello_mpsimple.vmem')))]
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # check all output files
        for i in range(0, 7):
            f = "stdout.{:03d}".format(i)
            assert tmpdir.join(f).isfile()
            assert self.matches_golden_reference(str(tmpdir), f)

    def test_tutorial5(self, tmpdir, baremetal_apps_hello,
                       sim_system_2x2_cccc_sim_dualcore_debug,
                       opensocdebugd_tcp):
        """
        Tutorial 5: Verilator with debug system and interactive osd-cli
        """

        cmd_osdcli = 'osd-cli'
        p_osdcli = Process(cmd_osdcli, logdir=str(tmpdir),
                           cwd=str(tmpdir),
                           startup_done_expect="osd>",
                           startup_timeout=10)
        p_osdcli.run()

        assert p_osdcli.expect(stdin_data="help\n",
                               pattern="Available commands:")
        time.sleep(0.5)  # XXX: match end of the output to avoid sleep

        assert p_osdcli.expect(stdin_data="mem help\n",
                               pattern="Available subcommands:")
        time.sleep(0.5)  # XXX: match end of the output to avoid sleep

        assert p_osdcli.expect(stdin_data="reset -halt\n", pattern="osd>")

        hello_elf = str(baremetal_apps_hello.join('hello.elf'))
        assert p_osdcli.expect(stdin_data="mem loadelf {} 2 -verify\n".format(hello_elf),
                               pattern="Verify program header 1")

        assert p_osdcli.expect(stdin_data="stm log stm000.log 3\n",
                               pattern="osd>")

        assert p_osdcli.expect(stdin_data="ctm log ctm000.log 4 {}\n".format(hello_elf),
                               pattern="osd>")

        assert p_osdcli.expect(stdin_data="start\n",
                               pattern=re.compile(r"\[STM 003\] [a-f0-9]{8}  rank 3 is tile 3"),
                               timeout=60)

        assert p_osdcli.expect(stdin_data="quit\n")

        # wait up to 2 seconds for osd-cli to terminate, then kill
        try:
            p_osdcli.proc.wait(timeout=2)
        except subprocess.TimeoutExpired:
            p_osdcli.proc.kill()

        # XXX: also check the contents of the STM/CTM logs
        assert tmpdir.join('stm000.log').isfile()
        assert tmpdir.join('ctm000.log').isfile()

        return

    def test_tutorial6(self, tmpdir, baremetal_apps_hello,
                       sim_system_2x2_cccc_sim_dualcore_debug,
                       opensocdebugd_tcp):
        """
        Tutorial 6: Same as tutorial 5, but using the python interface of
        osd-cli instead of manually typing commands
        """

        hello_elf = str(baremetal_apps_hello.join('hello.elf'))
        cmd_runelf = ['python2',
                      '{}/host/share/opensocdebug/examples/runelf.py'.format(os.environ['OPTIMSOC']),
                      hello_elf]
        p_runelf = Process(cmd_runelf, logdir=str(tmpdir),
                           cwd=str(tmpdir))
        p_runelf.run()

        logging.getLogger(__name__).info("Waiting 10 minutes for process to finish")
        p_runelf.proc.wait(timeout=600)
        assert p_runelf.proc.returncode == 0

        try:
            p_runelf.terminate()
        except ProcessLookupError:
            # process is already dead
            pass

class TestTutorialFpga:
    def test_tutorial7(self, tmpdir, localconf, baremetal_apps_hello):
        """
        Tutorial 7: Program a 2x2 CCCC system to a Nexys4 DDR board and run
        hello world on it.
        """

        # program FPGA with bitstream
        bitstream = "{}/examples/fpga/nexys4ddr/system_2x2_cccc/system_2x2_cccc_nexys4ddr.bit".format(os.environ['OPTIMSOC'])
        cmd_pgm = ['optimsoc-pgm-fpga', bitstream, 'xc7a100t_0']
        p_pgm = Process(cmd_pgm, logdir=str(tmpdir), cwd=str(tmpdir))
        p_pgm.run()
        p_pgm.proc.wait(timeout=60)
        assert p_pgm.proc.returncode == 0

        time.sleep(2)

        # connect to board with opensocdebugd
        nexys4ddr_device = localconf['boards']['nexys4ddr']['device']
        logging.getLogger(__name__).info("Using Nexys 4 board connected to " + nexys4ddr_device)
        cmd_opensocdebugd = ['opensocdebugd', 'uart',
                             'device=' + nexys4ddr_device,
                             'speed=12000000' ]
        p_opensocdebugd = Process(cmd_opensocdebugd, logdir=str(tmpdir),
                                  cwd=str(tmpdir),
                                  startup_done_expect="Wait for connection",
                                  startup_timeout=30)
        p_opensocdebugd.run()

        # run hello world on FPGA
        hello_elf = str(baremetal_apps_hello.join('hello.elf'))
        cmd_runelf = ['python2',
                      '{}/host/share/opensocdebug/examples/runelf.py'.format(os.environ['OPTIMSOC']),
                      hello_elf]
        p_runelf = Process(cmd_runelf, logdir=str(tmpdir),
                           cwd=str(tmpdir))
        p_runelf.run()

        logging.getLogger(__name__).info("Waiting 10 minutes for process to finish")
        p_runelf.proc.wait(timeout=600)
        assert p_runelf.proc.returncode == 0

        try:
            p_runelf.terminate()
        except ProcessLookupError:
            # process is already dead
            pass

        p_opensocdebugd.terminate()

