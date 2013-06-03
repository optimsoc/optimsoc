#!/bin/bash
#
# Check if a .bin file is not larger than the compute tile memory
# as specified inside optimsoc-sysconfig.c
# To avoid parsing the C source file we read the constant from
# the compiled object code of optimsoc-sysconfig.o.
#
# The environment variable OR32_OBJDUMP needs to be set to the OpenRISC
# toolchain objdump tool.
#
# Author: Philipp Wagner, philipp.wagner@tum.de
#

optimsoc_sysconfig_o=$1
binfile=$2

programsize=$(ls -l $binfile | cut -d' ' -f5)
memsize_hex=$($OR32_OBJDUMP -D $optimsoc_sysconfig_o | \
            grep _optimsoc_compute_tile_memsize -A1 | \
            tail -n1 | cut -d':' -f2 | cut -d' ' -f1-4 | tr -d '\t ')
memsize=$((0x$memsize_hex))
echo -n "Program image is $programsize bytes, memory is $memsize bytes: "
if [ $programsize -gt $memsize ]; then
  echo "Program too large!"
  exit 1
else
  echo "OK"
  exit 0
fi

