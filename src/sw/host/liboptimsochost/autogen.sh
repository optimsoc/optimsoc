#!/bin/sh -e

autoreconf --install --symlink

echo
echo "Configuration done. Before proceeding you need to decide for an installation"
echo "path (the documentation assumes /opt/optimsoc/)."
echo
echo "Usual build procedure:"
echo
echo "    mkdir build; cd build;"
echo "    ../configure CFLAGS='-g -O0' --prefix=/opt/optimsoc/host --enable-python-interface"
echo "    make"
echo "    make install"
echo 