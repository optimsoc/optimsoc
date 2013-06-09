#!/bin/sh -e

autoreconf --install --symlink

libdir() {
        echo $(cd $1/$(gcc -print-multi-os-directory); pwd)
}

args="--with-systemc-inc-dir=/path/to/systemc-2.3.0/include \
--with-systemc-lib-dir=/path/to/systemc-2.3.0/lib-linux"

echo
echo "----------------------------------------------------------------"
echo "Initialized build system. For a common configuration please run:"
echo "----------------------------------------------------------------"
echo
echo "./configure CFLAGS='-g -O0' $args"
echo
