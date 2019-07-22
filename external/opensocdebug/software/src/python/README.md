# Open SoC Debug Python Bindings

`opensocdebug` is a Python 3 library exposing the Open SoC Debug Software Reference Implementation (`libosd`) to Python.

This package is compatible with Python 3 on Linux only.
The source code is available from [GitHub](https://github.com/opensocdebug/osd-sw/tree/master/src/python).
The bindings are implemented using [Cython](https://cython.org/).

## Install from Source

- libosd and libglip must be installed on the system, including its development headers.
  The pkgconfig files (`*.pc`) must be present in the search path of pkgconfig.
  If you installed libosd or libglip into a non-standard location, pass the directories containing the `libosd.pc` and `libglip.pc` files through the environment variable `PKG_CONFIG_PATH`.

```sh
# optional (example, see see above for details)
export PKG_CONFIG_PATH=$HOME/libosd-install/share/pkgconfig:$HOME/libglip-install/lib/pkgconfig:$PKG_CONFIG_PATH

git clone https://github.com/opensocdebug/osd-sw.git
cd osd-sw/src/python
pip3 install --user -e .
```

## Develop

During the development you want to re-build the C extension (in `src/osd.c`) from the corresponding `src/osd.pxd` file using Cython.
To avoid running pip repeatedly you can use the following command (note the `--use-cython` to rebuild the C sources!):

```sh
python3 setup.py build_ext --inplace --use-cython
```

To make use of the library point the `PYTHONPATH` environment variable containing the resulting `.so` file.
If `libosd.so` or `libglip.so` are not in your default library search path, make them available by using `LD_LIBRARY_PATH`.

### Debugging
If you experience a segfault during the execution of a Python script using this package you can get a stacktrace from GDB using the following command:

```sh
gdb -return-child-result -batch -ex r -ex bt --args python3 your_python_code.py
```

## Run the tests

```sh
# install pytest (or take it from your distribution packages)
sudo pip3 install pytest

# run the tests
python3 -m pytest

# or
python3 setup.py test

# or (as run in CI)
tox
```

