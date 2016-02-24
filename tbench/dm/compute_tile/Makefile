all:
	@echo "Run make with the following targets:"
	@echo "  build-verilator    Build verilator simulation"

TBENCH=tb_compute_tile

buildtools=$(shell pkg-config --variable=buildtools optimsoc-sim)

include $(buildtools)/Makefile.verilator.inc
