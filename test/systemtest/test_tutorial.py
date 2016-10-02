import os
import subprocess
import pytest
import filecmp
import inspect

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

    @pytest.fixture(scope="module")
    def baremetal_apps(self, tmpdir_factory):
        """
        Get baremetal-apps from git
        """
        src_baremetal_apps = tmpdir_factory.mktemp('baremetal-apps')

        cmd = ['/usr/bin/git',
               'clone',
               'https://github.com/optimsoc/baremetal-apps',
               str(src_baremetal_apps)]
        subprocess.check_output(cmd)

        return src_baremetal_apps

    @pytest.fixture(scope="module")
    def baremetal_apps_hello(self, baremetal_apps):
        """
        Module-scoped fixture: download and build the hello world example
        from baremetal-apps
        """

        src_baremetal_apps_hello = baremetal_apps.join('hello')
        cmd = ['make', '-C', str(src_baremetal_apps_hello)]
        subprocess.check_output(cmd)

        return src_baremetal_apps_hello

    @pytest.fixture(scope="module")
    def baremetal_apps_hello_mpsimple(self, baremetal_apps):
        """
        Module-scoped fixture: download and build the hello world example
        from baremetal-apps
        """

        src_baremetal_apps_hello_mpsimple = baremetal_apps.join('hello_mpsimple')
        cmd = ['make', '-C', str(src_baremetal_apps_hello_mpsimple)]
        subprocess.check_output(cmd)

        return src_baremetal_apps_hello_mpsimple

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
