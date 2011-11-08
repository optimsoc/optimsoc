#!/usr/bin/python

import mmap
from sys import argv

if (len(argv) == 1):
	print "Provide file to patch as parameter"
	exit(1)

f = open(argv[1],"r+b")
map = mmap.mmap(f.fileno(),20)
map[19] = '\x5c'
map.close()
