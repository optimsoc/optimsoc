# OpTiMSoC Baremetal Libraries

This repository contains some baremetal libraries for OpTiMSoC:

* `libbaremetal` is the basic runtime support specific to OpTiMSoC.

* `libmp` is the message passing library for OpTiMSoC.

* `libruntime` is basic support for handling threads and virtual
  memory, along with a basic scheduler. It can be used to build your
  own simple runtime system and serves as hardware abstraction layer
  for [gzll](http://github.com/optimsoc/gzll "gzll")

## Build instructions

```
./autogen.sh
mkdir build; cd build
../configure --prefix=/opt/optimsoc/sw --host=or1k-elf
make
sudo make install
```
