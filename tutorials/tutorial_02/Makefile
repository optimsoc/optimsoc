
VERILATOR_INCLUDE ?= $(shell pkg-config --variable=includedir verilator)

all: tutorial_02

obj_dir/Vtutorial_02.mk: tutorial_02.v
	verilator -sv -cc --trace tutorial_02.v

verilated.o:
	g++ -c -o verilated.o $(VERILATOR_INCLUDE)/verilated.cpp

verilated_vcd_c.o:
	g++ -c -o verilated_vcd_c.o $(VERILATOR_INCLUDE)/verilated_vcd_c.cpp

obj_dir/Vtutorial_02__ALL.a: obj_dir/Vtutorial_02.mk
	make -C obj_dir -f Vtutorial_02.mk

tutorial_02: obj_dir/Vtutorial_02__ALL.a verilated.o verilated_vcd_c.o
	g++ -o tutorial_02 -I $(VERILATOR_INCLUDE) tutorial_02.cpp verilated.o verilated_vcd_c.o obj_dir/Vtutorial_02__ALL.a

clean:
	rm -rf obj_dir verilated.o verilated_vcd_c.o tutorial_02
