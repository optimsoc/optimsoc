#!/bin/sh -e

autoreconf --install --symlink

args="--prefix=\${OPTIMSOC}/host"

echo
echo "----------------------------------------------------------------"
echo "Initialized build system. For a common configuration please run:"
echo "----------------------------------------------------------------"
echo
echo "mkdir build; cd build"
echo "../configure CFLAGS='-g -O0' $args"
echo "make"
echo "sudo make install"
echo
