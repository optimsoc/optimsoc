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
import logging
import time
import re

import util

logging.basicConfig(level=logging.DEBUG)

class TestTutorial:
    """
    Test all simulated examples in the User Guide Tutorial

    The goal of this test case is to ensure that the tutorials as described in
    the user guide continue to work after changes to OpTiMSoC. It therefore
    tests only the examples as given in the tutorial, and does not aim for
    full coverage of the used parts of OpTiMSoC.
    """

    @pytest.fixture
    def sim_system_2x2_cccc_sim_dualcore_debug(self, tmpdir):
        cmd_sim = '{}/examples/sim/system_allct/system_2x2_cccc_sim_dualcore_debug'.format(os.environ['OPTIMSOC'])
        p_sim = util.Process(cmd_sim, logdir=str(tmpdir), cwd=str(tmpdir),
                             startup_done_expect='Glip TCP DPI listening',
                             startup_timeout=10)
        p_sim.run()

        yield p_sim

        p_sim.terminate()

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
        assert util.matches_golden_reference(str(tmpdir), 'stdout.000',
                                             filter_func=util.filter_timestamps)

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
            assert util.matches_golden_reference(str(tmpdir), f,
                                                 filter_func=util.filter_timestamps)

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
            assert util.matches_golden_reference(str(tmpdir), f,
                                                 filter_func=util.filter_timestamps)

    def test_tutorial4_hello(self, baremetal_apps_hello, tmpdir):
        """
        Tutorial 4: Run hello world application on 2x2 CCCC in Verilator
        Memory loading is done through Verilator meminit.
        """
        # run simulation
        cmd = ['{}/examples/sim/system_allct/system_2x2_cccc_sim_dualcore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello.join('hello.vmem')))]
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # check all output files
        for i in range(0, 7):
            f = "stdout.{:03d}".format(i)
            assert tmpdir.join(f).isfile()
            assert util.matches_golden_reference(str(tmpdir), f,
                                                 filter_func=util.filter_timestamps)

    def test_tutorial4_hello_mpsimple(self, baremetal_apps_hello_mpsimple, tmpdir):
        """
        Tutorial 4: Run hello_mpsimple world application on 2x2 CCCC in Verilator
        Memory loading is done through Verilator meminit.
        """
        # run simulation
        cmd = ['{}/examples/sim/system_allct/system_2x2_cccc_sim_dualcore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello_mpsimple.join('hello_mpsimple.vmem')))]
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # check all output files
        for i in range(0, 7):
            f = "stdout.{:03d}".format(i)
            assert tmpdir.join(f).isfile()
            assert util.matches_golden_reference(str(tmpdir), f,
                                                 filter_func=util.filter_timestamps)

    def test_tutorial5_hello_notrace(self, tmpdir, baremetal_apps_hello,
                                     sim_system_2x2_cccc_sim_dualcore_debug):
        """
        Tutorial 5: use osd-target-run and system with debug system to run
        software (no trace enabled)
        """

        hello_elf = str(baremetal_apps_hello.join('hello.elf'))
        cmd_targetrun = ['osd-target-run',
                         '-e', hello_elf,
                         '-vvv']
        p_targetrun = util.Process(cmd_targetrun, logdir=str(tmpdir),
                                   cwd=str(tmpdir))
        p_targetrun.run()

        logging.getLogger(__name__).info("Program should terminate itself; give it up to 5 minutes")
        p_targetrun.proc.wait(timeout=300)
        assert p_targetrun.proc.returncode == 0

        logging.getLogger(__name__).info("Now wait 60 seconds until the program has finished execution")
        time.sleep(60)

        try:
            p_targetrun.terminate()
        except ProcessLookupError:
            # process is already dead
            pass

        # check the simulation-generated stdout files
        for i in range(0, 7):
            f = "stdout.{:03d}".format(i)
            assert tmpdir.join(f).isfile()
            assert util.matches_golden_reference(str(tmpdir), f,
                                                 filter_func=util.filter_timestamps)

    def test_tutorial5_hello_systrace(self, tmpdir, baremetal_apps_hello,
                                      sim_system_2x2_cccc_sim_dualcore_debug):
        """
        Tutorial 5: use osd-target-run and system with debug system to run
        software (with system trace)
        """

        startup_done_string = '[INFO]  osd-target-run: System is now running. Press CTRL-C to end tracing.'
        hello_elf = str(baremetal_apps_hello.join('hello.elf'))
        cmd_targetrun = ['osd-target-run',
                         '-e', hello_elf,
                         '--systrace',
                         '-vvv']
        p_targetrun = util.Process(cmd_targetrun, logdir=str(tmpdir),
                                   cwd=str(tmpdir),
                                   startup_done_expect=startup_done_string,
                                   startup_timeout=300)
        p_targetrun.run()

        logging.getLogger(__name__).info("Record traces for 1 minute")
        time.sleep(60)
        p_targetrun.send_ctrl_c()

        # Give the process some time to clean up
        p_targetrun.proc.wait(timeout=30)
        assert p_targetrun.proc.returncode == 0

        try:
            p_targetrun.terminate()
        except ProcessLookupError:
            # process is already dead
            pass

        # Ensure that the STM logs are written
        stmlogs = [ 'systrace.0002.log', 'systrace.0004.log',
                    'systrace.0007.log', 'systrace.0009.log',
                    'systrace.0012.log', 'systrace.0014.log',
                    'systrace.0017.log', 'systrace.0019.log']
        for f in stmlogs:
            # STM log file exists
            assert tmpdir.join(f).isfile()

            # STM log >= 0 bytes
            # Currently some of the STM logs don't contain a full trace due to
            # overload in the debug system causing dropped packets. Checking for
            # such partial logs cannot be done reliably.
            f_stat = os.stat(str(tmpdir.join(f)))
            assert f_stat.st_size > 0

    def test_tutorial5_hello_coretrace(self, tmpdir, baremetal_apps_hello,
                                       sim_system_2x2_cccc_sim_dualcore_debug):
        """
        Tutorial 5: use osd-target-run and system with debug system to run
        software (with core trace)
        """

        startup_done_string = '[INFO]  osd-target-run: System is now running. Press CTRL-C to end tracing.'
        hello_elf = str(baremetal_apps_hello.join('hello.elf'))
        cmd_targetrun = ['osd-target-run',
                         '-e', hello_elf,
                         '--coretrace',
                         '-vvv']
        p_targetrun = util.Process(cmd_targetrun, logdir=str(tmpdir),
                                   cwd=str(tmpdir),
                                   startup_done_expect=startup_done_string,
                                   startup_timeout=300)
        p_targetrun.run()

        logging.getLogger(__name__).info("Record traces for 1 minute")
        time.sleep(60)
        p_targetrun.send_ctrl_c()

        # Give the process some time to clean up
        p_targetrun.proc.wait(timeout=30)
        assert p_targetrun.proc.returncode == 0

        try:
            p_targetrun.terminate()
        except ProcessLookupError:
            # process is already dead
            pass

        # Ensure that the CTM logs are written
        ctmlogs = [ 'coretrace.0003.log', 'coretrace.0005.log',
                    'coretrace.0008.log', 'coretrace.0010.log',
                    'coretrace.0013.log', 'coretrace.0015.log',
                    'coretrace.0018.log', 'coretrace.0020.log']
        for f in ctmlogs:
            # CTM log file exists
            assert tmpdir.join(f).isfile()

            # CTM log >= 0 bytes
            # Currently some of the CTM logs don't contain a full trace due to
            # overload in the debug system causing dropped packets. Checking for
            # such partial logs cannot be done reliably.
            f_stat = os.stat(str(tmpdir.join(f)))
            assert f_stat.st_size > 0


    def test_tutorial6_hello_mp(self, baremetal_apps_hello_mp, tmpdir):
        """
        Tutorial 8: Run hello_mp world application on 2x2 CCCC in Verilator
        Memory loading is done through Verilator meminit.
        """
        # run simulation
        cmd = ['{}/examples/sim/system_allct/system_2x2_cccc_sim_dualcore'.format(os.environ['OPTIMSOC']),
               '--meminit={}'.format(str(baremetal_apps_hello_mp.join('hello_mp.vmem')))]
        subprocess.check_output(cmd, cwd=str(tmpdir))

        # check all output files
        for i in range(0, 7):
            f = "stdout.{:03d}".format(i)
            assert tmpdir.join(f).isfile()
            assert util.matches_golden_reference(str(tmpdir), f,
                                                 filter_func=util.filter_timestamps)

class TestTutorialFpga:
    """
    Test all FPGA-based tutorial steps in the User Guide

    The tests in this class require the same hardware as used in the User Guide
    to be connected to the node that executes this test. Currently, this is the
    Nexus 4 DDR board.
    """
    def test_tutorial7(self, tmpdir, localconf, baremetal_apps_hello):
        """
        Tutorial 7: Program a 2x2 CCCC system to a Nexys4 DDR board and run
        hello world on it.
        """

        # program FPGA with bitstream
        bitstream = "{}/examples/fpga/nexys4ddr/system_2x2_cccc/system_2x2_cccc_nexys4ddr.bit".format(os.environ['OPTIMSOC'])
        cmd_pgm = ['optimsoc-pgm-fpga', bitstream, 'xc7a100t_0']
        p_pgm = util.Process(cmd_pgm, logdir=str(tmpdir), cwd=str(tmpdir))
        p_pgm.run()
        p_pgm.proc.wait(timeout=300)
        assert p_pgm.proc.returncode == 0

        time.sleep(2)

        # run hello.elf on target board
        nexys4ddr_device = localconf['boards']['nexys4ddr']['device']
        logging.getLogger(__name__).info("Using Nexys 4 board connected to " + nexys4ddr_device)
        glip_backend = 'uart'
        glip_backend_options = 'device=%s,speed=12000000' % nexys4ddr_device

        hello_elf = str(baremetal_apps_hello.join('hello.elf'))
        cmd_targetrun = ['osd-target-run',
                         '-e', hello_elf,
                         '-b', glip_backend,
                         '-o', glip_backend_options,
                         '--coretrace',
                         '--systrace',
                         '--verify',
                         '-vvv']

        startup_done_string = '[INFO]  osd-target-run: System is now running. Press CTRL-C to end tracing.'

        p_targetrun = util.Process(cmd_targetrun, logdir=str(tmpdir), 
                                   cwd=str(tmpdir),
                                   startup_done_expect=startup_done_string,
                                   startup_timeout=300)
        p_targetrun.run()

        logging.getLogger(__name__).info("Record traces for 30 seconds")
        time.sleep(30)
        p_targetrun.send_ctrl_c()

        # Give the process some time to clean up
        p_targetrun.proc.wait(timeout=30)
        assert p_targetrun.proc.returncode == 0

        try:
            p_targetrun.terminate()
        except ProcessLookupError:
            # process is already dead
            pass

        # Ensure that the STM logs are written
        stmlogs = [ 'systrace.0002.log', 'systrace.0005.log',
                    'systrace.0008.log', 'systrace.0011.log' ]
        for f in stmlogs:
            assert tmpdir.join(f).isfile()
            assert util.matches_golden_reference(str(tmpdir), f,
                                                 filter_func=util.filter_osd_trace_timestamps)

        # Ensure that the CTM logs are written
        ctmlogs = [ 'coretrace.0003.log', 'coretrace.0006.log',
                    'coretrace.0009.log', 'coretrace.0012.log' ]
        for f in ctmlogs:
            assert tmpdir.join(f).isfile()
            assert util.matches_golden_reference(str(tmpdir), f,
                                                 filter_func=util.filter_osd_trace_timestamps)
