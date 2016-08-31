#!/bin/sh

# show bash commands and exit script on first error
set -ex
set -x 

export PYTHONPATH=$OPTIMSOC/host/lib/python2.7/site-packages/ 
python2 ~/src/optimsoc/external/opensocdebug/software/src/bindings/python/examples/runelf.py --no-ctm --verify-memload cam_demo.elf
./convert_to_img.py
./decode/decode image.rgb565.raw image.rgb
convert -depth 8 -size "640x480" image.rgb image.png

#avconv -vcodec rawvideo -f rawvideo -pix_fmt rgb565 -s 640x480 -i image.rgb565.raw -f image2 -vcodec png image.png
