#!/bin/sh -e

autoreconf --install --symlink

echo
echo "Configuration done. Before proceeding you need to decide for an installation"
echo "path. In most cases we recommend to use this folder as installation directory"
echo "(documentation assumes this). But you can freely choose the prefix."
echo
echo "Usual build procedure:"
echo
echo "    mkdir build; cd build;"
echo "    ../configure CFLAGS='-g -O0' --prefix=\`pwd\`/.. --enable-python-interface"
echo "    make"
echo "    make install"
echo 