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
import pytest
import subprocess
import yaml
import logging

@pytest.hookimpl(tryfirst=True)
def pytest_exception_interact(node, call, report):
    """Dump all log files in case of a test failure"""
    try:
        if not report.failed:
            return
        if not 'tmpdir' in node.funcargs:
            return
    except:
        return

    tmpdir = str(node.funcargs['tmpdir'])
    print("\n\n")
    print("================= DUMP OF ALL TEMPORARY FILES =================")

    for f in os.listdir(tmpdir):
        f_abs = os.path.join(tmpdir, f)
        if not os.path.isfile(f_abs):
            continue
        print("vvvvvvvvvvvvvvvvvvvv {} vvvvvvvvvvvvvvvvvvvv".format(f))
        with open(f_abs, 'r') as fp:
            print(fp.read())
        print("^^^^^^^^^^^^^^^^^^^^ {} ^^^^^^^^^^^^^^^^^^^^\n\n".format(f))

@pytest.fixture(scope="session")
def localconf(request):
    """
    Host-local configuration
    """

    if os.getenv('OPTIMSOC_TEST_LOCALCONF') and os.path.isfile(os.environ['OPTIMSOC_TEST_LOCALCONF']):
        localconf_yaml_file = os.environ['OPTIMSOC_TEST_LOCALCONF']
    else:
        XDG_CONFIG_HOME = os.getenv('XDG_CONFIG_HOME',
                                    os.path.join(os.environ['HOME'], '.config'))
        localconf_yaml_file = os.path.join(XDG_CONFIG_HOME, 'optimsoc', 'test-localconf.yaml')
    logging.getLogger(__name__).info('Reading configuration from ' + localconf_yaml_file)

    with open(str(localconf_yaml_file), 'r') as fp:
        y = yaml.load(fp)

    return y

@pytest.fixture(scope="module")
def baremetal_apps(tmpdir_factory):
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
def baremetal_apps_hello(baremetal_apps):
    """
    Module-scoped fixture: download and build the hello world example
    from baremetal-apps
    """

    src_baremetal_apps_hello = baremetal_apps.join('hello')
    cmd = ['make', '-C', str(src_baremetal_apps_hello)]
    subprocess.check_output(cmd)

    return src_baremetal_apps_hello

@pytest.fixture(scope="module")
def baremetal_apps_hello_mpsimple(baremetal_apps):
    """
    Module-scoped fixture: download and build the hello world example
    from baremetal-apps
    """

    src_baremetal_apps_hello_mpsimple = baremetal_apps.join('hello_mpsimple')
    cmd = ['make', '-C', str(src_baremetal_apps_hello_mpsimple)]
    subprocess.check_output(cmd)

    return src_baremetal_apps_hello_mpsimple

@pytest.fixture(scope="module")
def baremetal_apps_hello_mp(baremetal_apps):
    """
    Module-scoped fixture: download and build the hello world mp example
    from baremetal-apps
    """

    src_baremetal_apps_hello_mp = baremetal_apps.join('hello_mp')
    cmd = ['make', '-C', str(src_baremetal_apps_hello_mp)]
    subprocess.check_output(cmd)

    return src_baremetal_apps_hello_mp

def _git_ensure_checkout(repo_url, revision, target_dir):
    """
    Ensure that target_dir contains a up-to-date checkout of the repo_url

    This does a clone if the repository doesn't exist yet, and otherwise updates
    the repository and checks out the specified revision.
    """

    if not target_dir.check(dir = True):
        cmd = ['/usr/bin/git',
              'clone',
              repo_url,
              str(target_dir)]
        subprocess.check_output(cmd)
    else:
        cmd = ['/usr/bin/git',
               'fetch',
               '--tags',
               '--prune']
        subprocess.check_output(cmd, cwd = str(target_dir))

    cmd = ['/usr/bin/git',
           'reset',
           '--hard',
           revision]
    subprocess.check_output(cmd, cwd = str(target_dir))


@pytest.fixture(scope="module")
def optimsoc_buildroot(request):
    """
    Get buildroot and optimsoc-buildroot from git

    This fixture uses the pytest cache to avoid cloning the git repositories
    over and over again. If you notice inconsistencies run pytest --clearcache
    to clear the cache. (It's probably a good idea to always run that in CI.)
    """

    src_buildroot_base = request.config.cache.makedir('optimsoc-buildroot-base')

    src_buildroot = src_buildroot_base.join('buildroot')
    src_optimsoc_buildroot = src_buildroot_base.join('optimsoc-buildroot')

    # prepare optimsoc-buildroot (the OpTiMSoC br2-external buildroot tree)
    _git_ensure_checkout('https://github.com/optimsoc/optimsoc-buildroot',
                         'master', src_optimsoc_buildroot)

    # figure out which version of buildroot is supported by optimsoc-buildroot
    optimsoc_buildroot_version = None
    with open(str(src_optimsoc_buildroot.join('buildroot_version'))) as f:
        optimsoc_buildroot_version = f.readlines()
    optimsoc_buildroot_version = optimsoc_buildroot_version[0].rstrip()
    logging.getLogger(__name__).info('Using buildroot version %s'
                                     % optimsoc_buildroot_version)

    # get upstream buildroot
    # We're using the github mirror instead of the official repository at
    # https://git.busybox.net/buildroot since the official mirror doesn't
    # support the "advanced" HTTP protocol requried for shallow clones.
    _git_ensure_checkout('https://github.com/buildroot/buildroot.git',
                         optimsoc_buildroot_version, src_buildroot)

    return src_buildroot_base

@pytest.fixture(scope="module")
def linux_compute_tile_singlecore(optimsoc_buildroot):
    """
    Module-scoped fixture: build a Linux image for a single-core compute tile
    """

    # Get the buildroot base directory from the optimsoc_buildroot() fixture.
    # Note that this directory is cached between pytest runs. Make sure the
    # commands executed as part of this test can deal with that and rebuild
    # artifacts as needed.
    src_optimsoc_buildroot = optimsoc_buildroot.join('optimsoc-buildroot')
    src_buildroot = optimsoc_buildroot.join('buildroot')

    config = 'optimsoc_computetile_singlecore_defconfig'

    # buildroot doesn't like our OpTiMSoC compiler being in the path. Error is:
    # ---
    # You seem to have the current working directory in your
    # LD_LIBRARY_PATH environment variable. This doesn't work.
    # support/dependencies/dependencies.mk:21: recipe for target 'dependencies' failed
    # ---
    env = dict(os.environ, LD_LIBRARY_PATH='', PATH='/bin:/usr/bin:/usr/local/bin')

    cmd = ['make',
           '-C', str(src_buildroot),
           'BR2_EXTERNAL='+str(src_optimsoc_buildroot),
           config]
    subprocess.check_output(cmd, env=env, stderr=subprocess.STDOUT,
                            universal_newlines=True)

    cmd = ['make',
           '-C', str(src_buildroot)]
    env = dict(os.environ, LD_LIBRARY_PATH='')
    subprocess.check_output(cmd, env=env, stderr=subprocess.STDOUT,
                            universal_newlines=True)

    linux_img = src_buildroot.join('output/images/vmlinux')

    return linux_img
