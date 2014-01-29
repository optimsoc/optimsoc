#!/bin/sh
#
# Convert the original MiBench input data into an C compatible include file, 
# which can be used to compile the input data into the program.
#

# small data set
echo '#define ARRAY_COUNT '$(wc -l input_small.dat | cut -f1 -d' ') > input_small.h
echo '#define ARRAY_DATA { \' >> input_small.h
awk '{print "{.qstring = \""$0"\"}, \\"}' input_small.dat >> input_small.h
echo '}' >> input_small.h

