all: build

build: 
	vsim -c -do modelsim_compile.tcl

sim:
	vsim -voptargs=+acc -c -do run.do work.tb_compute_tile_dual

clean:
	rm -rf work
