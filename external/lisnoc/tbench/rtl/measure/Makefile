all: build measure-vc

build:
	@if [ ! -e work ]; then vlib work; fi
	@vlog -f measure.vc

measure-vc:
	./measure_vc.py
