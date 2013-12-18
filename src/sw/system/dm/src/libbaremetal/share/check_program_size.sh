#!/bin/bash
# Copyright (c) 2012-2013 by the author(s)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# =============================================================================
#
# Check if a .bin file is not larger than the compute tile memory
# as specified inside optimsoc-sysconfig.c
# To avoid parsing the C source file we read the constant from
# the compiled object code of optimsoc-sysconfig.o.
#
# The environment variable OR32_OBJDUMP needs to be set to the OpenRISC
# toolchain objdump tool.
#
# Author(s):
#   Philipp Wagner <philipp.wagner@tum.de>

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

