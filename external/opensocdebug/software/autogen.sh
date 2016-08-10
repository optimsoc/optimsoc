#!/bin/sh -e

autoreconf --install --symlink

echo
echo "Configuration done. Before proceeding you need to decide for an installation"
echo "Usual build procedure:"
echo
echo "    mkdir build; cd build;"
echo "    ../configure"
echo "    make"
echo "    sudo make install"
echo 
