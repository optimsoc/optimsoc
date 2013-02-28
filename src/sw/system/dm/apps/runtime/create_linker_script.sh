#!/bin/bash

# Get symbols address from sysconfig
startstr=`or32-elf-objdump -t sysconfig.o | grep optimsoc_compute_tile_memsize | awk '{ print $1 }' | tr '[:lower:]' '[:upper:]'`

if [ -z "$startstr" ]; then
	echo "*** ERROR: Missing optimsoc_compute_tile_memsize symbol in sysconfig.c"
	exit -1
fi

# Convert to decimal
start=`echo "ibase=16;obase=A;$startstr" | bc`

echo " * Start address of symbol: $start"

# Calculate end of this 4 byte symbol
let end=$start+4

echo " * Stop address of symbol: $end"

# Extract value of this symbol
memsizestr=`or32-elf-objdump -s -j .rodata --start-address=$start --stop-address=$end sysconfig.o | awk '{ if($0 ~ "^ [0-9a-fA-f]+ [0-9a-fA-f]+") print $2; }'`

# Convert value to decimal
memsize=`echo "ibase=16;obase=A;$memsizestr" | bc`

# Copy input
cp ${BASE}/apps/runtime/link.ld.in link.ld

# Replace ${MEMSIZE} with actual memory size
sed -i "s/\${MEMSIZE}/$memsize/g" link.ld