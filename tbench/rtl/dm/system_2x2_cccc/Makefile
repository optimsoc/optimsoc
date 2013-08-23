all: build

build:
	vsim -c -do modelsim_compile.tcl

sim:
	vsim -voptargs=+acc -c work.tb_system_2x2_cccc -do "run -all"

clean:
	rm -rf work
