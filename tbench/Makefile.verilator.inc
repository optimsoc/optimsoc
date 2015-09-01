# -*- makefile -*- vim:ft=make

CXX ?= g++
CFLAGS ?= -g -Wall
VERILATOR ?= verilator

INC=$(shell pkg-config --cflags optimsoc-sysc)
LIB=$(shell pkg-config --libs optimsoc-sysc)
INC+=$(shell pkg-config --cflags systemc)
LIB+=$(shell pkg-config --libs systemc)
INC+=$(shell pkg-config --cflags verilator)
LIB+=$(shell pkg-config --libs verilator)

VERILATOR_INCDIR=$(shell pkg-config --variable=includedir verilator)

build-verilator: $(TBENCH) $(TBENCH)-vcd

$(TBENCH): obj_dir/V$(TBENCH)__ALL.a $(TBENCH).o verilated.o verilated_vcd_c.o
	$(CXX) $(CFLAGS) -o $(TBENCH) -pthread $(TBENCH).o \
		obj_dir/V$(TBENCH)__ALL.a verilated.o verilated_vcd_c.o \
		$(LIB)

$(TBENCH)-vcd: obj_dir/V$(TBENCH)__ALL.a $(TBENCH)-vcd.o verilated.o verilated_vcd_c.o
	$(CXX) $(CFLAGS) -o $(TBENCH)-vcd -pthread $(TBENCH)-vcd.o \
		obj_dir/V$(TBENCH)__ALL.a verilated.o verilated_vcd_c.o \
		$(LIB)

$(TBENCH).o: $(TBENCH).cpp
	$(CXX) $(CFLAGS) $(TBENCH_CFLAGS) $(INC) -o $(TBENCH).o -c $(TBENCH).cpp

$(TBENCH)-vcd.o: $(TBENCH).cpp
	$(CXX) $(CFLAGS) $(TBENCH_CFLAGS) $(INC) -DVCD_TRACE -o $(TBENCH)-vcd.o -c $(TBENCH).cpp

obj_dir/V$(TBENCH)__ALL.a: $(TBENCH).sv verilator.vc
	$(VERILATOR) -sc -Wno-width -Wno-unoptflat -f verilator.vc -trace -Wno-fatal
	make -C obj_dir -f V$(TBENCH).mk

verilator.vc:
	tclsh $(OPTIMSOC_TCL)/optimsoc_verilator.tcl build.tcl

verilated.o:
	$(CXX) $(CFLAGS) -o verilated.o -c $(VERILATOR_INCDIR)/verilated.cpp

verilated_vcd_c.o:
	$(CXX) $(CFLAGS) -o verilated_vcd_c.o -c \
		$(VERILATOR_INCDIR)/verilated_vcd_c.cpp

clean:
	rm -rf obj_dir *.o

.PHONY: verilator.vc
