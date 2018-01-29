# Copyright 2017-2018 The Open SoC Debug Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from setuptools import setup
from setuptools.extension import Extension
import os
import sys
import subprocess

if '--use-cython' in sys.argv:
    USE_CYTHON = True
    sys.argv.remove('--use-cython')
else:
    USE_CYTHON = False
ext = '.pyx' if USE_CYTHON else '.c'

def pkgconfig(*packages, **kw):
    config = kw.setdefault('config', {})
    optional_args = kw.setdefault('optional', '')

    flag_map = {'include_dirs': ['--cflags-only-I', 2],
                'library_dirs': ['--libs-only-L', 2],
                'libraries': ['--libs-only-l', 2],
                'extra_compile_args': ['--cflags-only-other', 0],
                'extra_link_args': ['--libs-only-other', 0],
                }
    for package in packages:
        for distutils_key, (pkg_option, n) in flag_map.items():
            try:
                cmd = ['pkg-config', optional_args, pkg_option, package]
                items = subprocess.check_output(cmd).decode('utf8').split()
                config.setdefault(distutils_key, []).extend([i[n:] for i in items])
            except subprocess.CalledProcessError:
                return config

    return config

extensions = [
    Extension('osd',
              ['src/osd'+ext],
              **pkgconfig('osd', 'libglip'))
]

if USE_CYTHON:
    from Cython.Build import cythonize
    extensions = cythonize(extensions,
                           compiler_directives={'language_level': 3,
                                                'embedsignature': True})

with open("README.md", "r", encoding='utf-8') as readme:
    long_description = readme.read()

version_file = os.path.join(os.path.dirname(os.path.realpath(__file__)),
                            "src/version.py")

setup(
    name = "opensocdebug",
    ext_modules = extensions,
    use_scm_version = {
        "root": "../..",
        "relative_to": __file__,
        "write_to": version_file,
    },
    author = "Philipp Wagner",
    author_email = "mail@philipp-wagner.com",
    description = ("Open SoC Debug is a way to interact with and obtain "
                   "information from a System-on-Chip (SoC)."),
    long_description = long_description,
    url = "http://www.opensocdebug.org",
    license = 'Apache License, Version 2.0',
    classifiers = [
        "Development Status :: 3 - Alpha",
        "Topic :: Utilities",
        "Topic :: Software Development :: Debuggers",
        "Topic :: Software Development :: Libraries",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Cython",
    ],
    setup_requires = [
        'setuptools_scm',
        'pytest-runner',
    ],
    tests_require = [
        'pytest',
    ],
)

