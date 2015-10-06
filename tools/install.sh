#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: ./install.sh <path>"
    exit 1
fi

echo "++ Install OpTiMSoC tools to $1"

mkdir -p $1/tools
cp -r $OPTIMSOC/tools/tcl $1/tools
make -C $OPTIMSOC/tools/utils > /dev/null
mkdir -p $1/tools/utils
cp $OPTIMSOC/tools/utils/bin2vmem $1/tools/utils
cp $OPTIMSOC/tools/utils/optimsoc-pgas-binary $1/tools/utils

echo "++ Done. Be sure to set the following environment variables"
echo "++  OPTIMSOC_TCL=$1/tools/tcl"
echo "++  PATH=\$PATH:$1/tools/utils"

