all: tb_system_2x2_cccc tb_system_2x2_cccc-vcd

CXX ?= g++
CFLAGS ?= -g -Wall
VERILATOR ?= verilator

main.o: main.cpp obj_dir/Vtb_system_2x2_cccc.a
	$(CXX) $(CFLAGS) -I$(SYSTEMC)/include -I$(VERILATOR_ROOT)/include \
		-I$(VERILATOR_ROOT)/include/vltstd -I $(OPTIMSOC_SYSC)/src/debug/ \
		-o main.o -c main.cpp

main-vcd.o: main.cpp obj_dir/Vtb_system_2x2_cccc.a
	$(CXX) $(CFLAGS) -I$(SYSTEMC)/include -I$(VERILATOR_ROOT)/include \
		-I$(VERILATOR_ROOT)/include/vltstd -I$(OPTIMSOC_SYSC)/src/debug/ \
		-DVCD_TRACE -o main-vcd.o -c main.cpp

obj_dir/Vtb_system_2x2_cccc.a:
	$(VERILATOR) -sc -Wno-width -Wno-unoptflat -f tb_system_2x2_cccc.vc -trace
	make -C obj_dir -f Vtb_system_2x2_cccc.mk

verilated.o:
	$(CXX) $(CFLAGS) -o verilated.o -c $(VERILATOR_ROOT)/include/verilated.cpp

verilated_vcd_c.o:
	$(CXX) $(CFLAGS) -o verilated_vcd_c.o -c \
		$(VERILATOR_ROOT)/include/verilated_vcd_c.cpp

tb_system_2x2_cccc: obj_dir/Vtb_system_2x2_cccc.a main.o verilated.o verilated_vcd_c.o
	$(CXX) $(CFLAGS) -o tb_system_2x2_cccc -pthread main.o \
		obj_dir/Vtb_system_2x2_cccc__ALL.a verilated.o \
		$(OPTIMSOC_SYSC)/lib/liboptimsocsysc.a \
		$(SYSTEMC)/lib-linux/libsystemc.a verilated_vcd_c.o

tb_system_2x2_cccc-vcd: obj_dir/Vtb_system_2x2_cccc.a main-vcd.o verilated.o verilated_vcd_c.o
	$(CXX) $(CFLAGS) -o tb_system_2x2_cccc-vcd -pthread main-vcd.o \
		obj_dir/Vtb_system_2x2_cccc__ALL.a verilated.o \
		$(OPTIMSOC_SYSC)/lib/liboptimsocsysc.a \
		$(SYSTEMC)/lib-linux/libsystemc.a verilated_vcd_c.o

clean:
	rm -rf obj_dir *.o tb_system_2x2_cccc tb_system_2x2_cccc-vcd

