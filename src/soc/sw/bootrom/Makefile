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

all: bootrom.hex bootrom_code.v

bootrom.hex: bootrom.bin
	bin2vmem bootrom.bin > bootrom.hex

bootrom_code.v: bootrom.bin bin2vlogarray
	cat bootrom_code.v.header > bootrom_code.v
	cat bootrom.bin | ./bin2vlogarray >> bootrom_code.v

bootrom.bin: a.out
	or1k-elf-objcopy -O binary a.out bootrom.bin

bootrom.dis: a.out
	or1k-elf-objdump -d a.out > bootrom.dis

a.out: boot.s
	or1k-elf-as boot.s

bin2vlogarray: bin2vlogarray.c
	gcc -Wall -o bin2vlogarray bin2vlogarray.c

install:
	cp -i bootrom_code.v ../../rtl/bootrom/verilog/

clean:
	rm -f a.out bootrom.bin bootrom.hex bootrom_code.v bin2vlogarray

