all: build

build: 
	vsim -c -do modelsim_compile.tcl

sim:
	vsim -voptargs=+acc -c -do run.do work.tb_compute_tile

sim-dualcore:
	vsim -voptargs=+acc -c -do run.do work.tb_compute_tile -G NUMCORES=2

clean:
	rm -rf work
