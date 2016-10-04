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
    if not report.failed:
        return
    if not 'tmpdir' in node.funcargs:
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
    logging.getLogger('__name__').info('Reading configuration from ' + localconf_yaml_file)

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
