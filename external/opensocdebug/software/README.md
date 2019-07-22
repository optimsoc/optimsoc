[![Run Status](https://api.shippable.com/projects/5a212a1b7cc9a8070082fa14/badge?branch=master)](https://app.shippable.com/github/opensocdebug/osd-sw) 
[![codecov](https://codecov.io/gh/opensocdebug/osd-sw/branch/master/graph/badge.svg)](https://codecov.io/gh/opensocdebug/osd-sw)

# Open SoC Debug Software

This is the reference implementation of the Open SoC Debug host software.

## Documentation
- [Implementation Documentation](https://opensocdebug.readthedocs.io/projects/osd-sw)
- [OSD Specification and overview](https://opensocdebug.readthedocs.io/) (good first read if you're new to OSD)

## Build Dependencies
```sh
./install-build-deps.sh
```

## Build
```sh
./autogen.sh
mkdir build
cd build
../configure
make
make install
```

## License
This software is licensed under the Apache 2 license.
See the [LICENSE](LICENSE) file for the full license text.

The included command line tools use [argtable3](https://www.argtable.org/), which is licensed under a BSD license.
Pleaes find the full license and attribution notice in [LICENSE.argtable](LICENSE.argtable).
