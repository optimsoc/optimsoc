all: build

work:
	if [ ! -d work ]; then vlib work; fi

build: work
	vlog -f tb_compute_tile.vc

sim:
	vsim -voptargs=+acc -c -do run.do work.tb_compute_tile

clean:
	rm -rf work
