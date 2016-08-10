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
# Author(s):
#   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
#   Philipp Wagner <philipp.wagner@tum.de>

all: fw-213.ihx fw-115.ihx

fw-213.ihx: fw.c
	bmpsdcc.sh fw.c "-D ZTEX_213 -I $(ZTEX_INCLUDE)"
	mv fw.ihx fw-213.ihx

fw-115.ihx: fw.c
	bmpsdcc.sh fw.c "-D ZTEX_115 -I $(ZTEX_INCLUDE)"
	mv fw.ihx fw-115.ihx

clean:
	rm -f fw.{asm,ihx,lnk,lst,map,mem,rel,rst,sym,tmp.c}
	rm -f fw-213.ihx fw-115.ihx

flash-213: fw-213.ihx
	FWLoader -c -uu fw-213.ihx -ue fw-213.ihx
	FWLoader -p

flash-115: fw-115.ihx
	FWLoader -c -uu fw-115.ihx -ue fw-115.ihx
	FWLoader -p


