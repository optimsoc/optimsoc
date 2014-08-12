all: tb_system_2x2_cccm tb_system_2x2_cccm-vcd

CXX ?= g++
CFLAGS ?= -g -Wall
VERILATOR ?= verilator

INC=$(shell pkg-config --cflags optimsoc-sysc)
LIB=$(shell pkg-config --libs optimsoc-sysc)

main.o: main.cpp obj_dir/Vtb_system_2x2_cccm.a
	$(CXX) $(CFLAGS) -I$(SYSTEMC)/include -I$(VERILATOR_ROOT)/include \
		-I$(VERILATOR_ROOT)/include/vltstd $(INC)  \
		-o main.o -c main.cpp

main-vcd.o: main.cpp obj_dir/Vtb_system_2x2_cccm.a
	$(CXX) $(CFLAGS) -I$(SYSTEMC)/include -I$(VERILATOR_ROOT)/include \
		-I$(VERILATOR_ROOT)/include/vltstd $(INC) \
		-DVCD_TRACE -o main-vcd.o -c main.cpp

verilator.vc:
	tclsh verilator.tcl

obj_dir/Vtb_system_2x2_cccm.a: verilator.vc
	$(VERILATOR) -sc -Wno-width -Wno-unoptflat -f verilator.vc -trace
	make -C obj_dir -f Vtb_system_2x2_cccm.mk

verilated.o:
	$(CXX) $(CFLAGS) -o verilated.o -c $(VERILATOR_ROOT)/include/verilated.cpp

verilated_vcd_c.o:
	$(CXX) $(CFLAGS) -o verilated_vcd_c.o -c \
		$(VERILATOR_ROOT)/include/verilated_vcd_c.cpp

tb_system_2x2_cccm: obj_dir/Vtb_system_2x2_cccm.a main.o verilated.o verilated_vcd_c.o
	$(CXX) $(CFLAGS) -o tb_system_2x2_cccm -pthread main.o \
		obj_dir/Vtb_system_2x2_cccm__ALL.a verilated.o \
		$(LIB) \
		$(SYSTEMC)/lib-linux/libsystemc.a verilated_vcd_c.o

tb_system_2x2_cccm-vcd: obj_dir/Vtb_system_2x2_cccm.a main-vcd.o verilated.o verilated_vcd_c.o
	$(CXX) $(CFLAGS) -o tb_system_2x2_cccm-vcd -pthread main-vcd.o \
		obj_dir/Vtb_system_2x2_cccm__ALL.a verilated.o \
		$(LIB) \
		$(SYSTEMC)/lib-linux/libsystemc.a verilated_vcd_c.o

clean:
	rm -rf obj_dir *.o tb_system_2x2_cccm tb_system_2x2_cccm-vcd

.PHONY: verilator.vc