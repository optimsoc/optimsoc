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
        print("vvvvvvvvvvvvvvvvvvvv {} vvvvvvvvvvvvvvvvvvvv".format(f))
        with open(os.path.join(tmpdir, f), 'r') as fp:
            print(fp.read())
        print("^^^^^^^^^^^^^^^^^^^^ {} ^^^^^^^^^^^^^^^^^^^^\n\n".format(f))
