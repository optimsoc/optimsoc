all: tb_noc_router

tb_noc_router:
	@if [ ! -e work ]; then vlib work; fi
	@vlog -f tb_noc_router.vc

tb_noc_router-sim:
	vsim -voptargs=+acc -c -do run.do work.tb_noc_router
