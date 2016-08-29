#!/usr/bin/python3

import re
from struct import pack

stm_in = open('stm006.log', 'r')
img_out = open('image.rgb565.raw','bw')

for line in stm_in:
    m = re.search('^[0-9a-f]+ 0200 ([0-9a-f]+)$', line)
    if not m:
        continue;

    d = int(m.group(1), base=16)
    img_out.write(pack("<I", d))

img_out.close()
