all: tb_compute_tile tb_compute_tile-vcd

CXX ?= g++
CFLAGS ?= -g -Wall
VERILATOR ?= verilator

INC=$(shell pkg-config --cflags optimsoc-sysc)
LIB=$(shell pkg-config --libs optimsoc-sysc)

main.o: main.cpp obj_dir/Vtb_compute_tile.a
	$(CXX) $(CFLAGS) -I$(SYSTEMC)/include -I$(VERILATOR_ROOT)/include \
		-I$(VERILATOR_ROOT)/include/vltstd $(INC) \
		-o main.o -c main.cpp

main-vcd.o: main.cpp obj_dir/Vtb_compute_tile.a
	$(CXX) $(CFLAGS) -I$(SYSTEMC)/include -I$(VERILATOR_ROOT)/include \
		-I$(VERILATOR_ROOT)/include/vltstd $(INC) \
		-DVCD_TRACE -o main-vcd.o -c main.cpp

verilator.vc:
	tclsh verilator.tcl

obj_dir/Vtb_compute_tile.a: verilator.vc
	$(VERILATOR) -sc -Wno-width -Wno-unoptflat -f verilator.vc -trace
	make -C obj_dir -f Vtb_compute_tile.mk

verilated.o:
	$(CXX) $(CFLAGS) -o verilated.o -c $(VERILATOR_ROOT)/include/verilated.cpp

verilated_vcd_c.o:
	$(CXX) $(CFLAGS) -o verilated_vcd_c.o -c \
		$(VERILATOR_ROOT)/include/verilated_vcd_c.cpp

tb_compute_tile: obj_dir/Vtb_compute_tile.a main.o verilated.o verilated_vcd_c.o
	$(CXX) $(CFLAGS) -o tb_compute_tile -pthread main.o \
		obj_dir/Vtb_compute_tile__ALL.a verilated.o \
		$(LIB) \
		$(SYSTEMC)/lib-linux/libsystemc.a verilated_vcd_c.o

tb_compute_tile-vcd: obj_dir/Vtb_compute_tile.a main-vcd.o verilated.o verilated_vcd_c.o
	$(CXX) $(CFLAGS) -o tb_compute_tile-vcd -pthread main-vcd.o \
		obj_dir/Vtb_compute_tile__ALL.a verilated.o \
		$(LIB) \
		$(SYSTEMC)/lib-linux/libsystemc.a verilated_vcd_c.o

clean:
	rm -rf obj_dir *.o tb_compute_tile tb_compute_tile-vcd

.PHONY: verilator.vc
