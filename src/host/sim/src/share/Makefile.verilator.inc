# -*- makefile -*- vim:ft=make

CXX ?= g++
CFLAGS ?= -g -Wall
VERILATOR ?= verilator

INC=$(shell pkg-config --cflags optimsoc-sim)
LIB=$(shell pkg-config --libs optimsoc-sim)
INC+=$(shell pkg-config --cflags verilator)
LIB+=$(shell pkg-config --libs verilator)

VERILATOR_INCDIR=$(shell pkg-config --variable=includedir verilator)

build-verilator: $(TBENCH)

flags = -CFLAGS "$(INC)" -LDFLAGS "$(LIB) -pthread" --exe --trace --sv --unroll-count 256 --Wno-fatal

$(TBENCH): verilator.vc
	$(VERILATOR) --cc -f verilator.vc $(TBENCH).cpp $(flags) -o ../$(TBENCH)
	make -C obj_dir -f V$(TBENCH).mk

verilator.vc:
	tclsh $(OPTIMSOC_TCL)/optimsoc_verilator.tcl build.tcl

clean:
	rm -rf obj_dir *.o

.PHONY: verilator.vc
