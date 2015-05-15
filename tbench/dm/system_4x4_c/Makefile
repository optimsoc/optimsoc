all: build

work:
	if [ ! -d work ]; then vlib work; fi

build: work
	vsim -c -do modelsim_compile.tcl

sim:
	vsim -voptargs=+acc -c work.tb_system_4x4_c -do "run -all"

clean:
	rm -rf work
