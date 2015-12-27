
VERILATOR_INCLUDE ?= $(shell pkg-config --variable=includedir verilator)

all: tutorial_01

obj_dir/Vtutorial_01.mk: tutorial_01.v
	verilator -sv -cc --trace tutorial_01.v

verilated.o:
	g++ -c -o verilated.o $(VERILATOR_INCLUDE)/verilated.cpp

verilated_vcd_c.o:
	g++ -c -o verilated_vcd_c.o $(VERILATOR_INCLUDE)/verilated_vcd_c.cpp

obj_dir/Vtutorial_01__ALL.a: obj_dir/Vtutorial_01.mk
	make -C obj_dir -f Vtutorial_01.mk

tutorial_01: obj_dir/Vtutorial_01__ALL.a verilated.o verilated_vcd_c.o
	g++ -o tutorial_01 -I $(VERILATOR_INCLUDE) tutorial_01.cpp verilated.o verilated_vcd_c.o obj_dir/Vtutorial_01__ALL.a

clean:
	rm -rf obj_dir verilated.o verilated_vcd_c.o tutorial_01
