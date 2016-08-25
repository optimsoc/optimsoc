#!/bin/sh

set -x 

export PYTHONPATH=$OPTIMSOC/host/lib/python2.7/site-packages/ 
python2 ~/src/optimsoc/external/opensocdebug/software/src/bindings/python/examples/runelf.py --no-ctm --verify-memload ~/src/optimsoc/examples/system_2x2_ccc_cam_nexys4ddr/sw/cam_demo.elf
~/src/optimsoc/examples/system_2x2_ccc_cam_nexys4ddr/sw/convert_to_img.py
avconv -vcodec rawvideo -f rawvideo -pix_fmt rgb565 -s 640x480 -i image.rgb565.raw -f image2 -vcodec png image.png
