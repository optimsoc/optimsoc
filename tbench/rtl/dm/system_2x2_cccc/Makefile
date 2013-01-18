all: build

work:
	if [ ! -d work ]; then vlib work; fi

build: work
	vlog -f tb_system_2x2_cccc.vc

sim:
	vsim -voptargs=+acc -c work.tb_system_2x2_cccc -do "run -all"

clean:
	rm -rf work
