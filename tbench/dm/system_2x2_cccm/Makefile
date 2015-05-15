all: build

work:
	if [ ! -d work ]; then vlib work; fi

build: work
	vsim -c -do modelsim_compile.tcl

sim:
	vsim -voptargs=+acc -c work.tb_system_2x2_cccm -do "run -all"

sim-dualcore:
	vsim -voptargs=+acc -c work.tb_system_2x2_cccm -G CORES_PER_TILE=2 -do "run -all"

clean:
	rm -rf work
