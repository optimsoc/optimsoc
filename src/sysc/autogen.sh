#!/bin/sh -e

autoreconf --install --symlink

args="--prefix=\`pwd\`/.. --with-systemc-inc-dir=\${SYSTEMC}/include \
--with-systemc-lib-dir=\${SYSTEMC}/lib-linux"

echo
echo "----------------------------------------------------------------"
echo "Initialized build system. For a common configuration please run:"
echo "----------------------------------------------------------------"
echo
echo "mkdir build; cd build"
echo "../configure CFLAGS='-g -O0' $args"
echo "make"
echo "make install"
echo
