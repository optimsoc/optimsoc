all:
	@echo "Run make with the following targets:"
	@echo "  build-verilator    Build verilator simulation"

TBENCH=tb_system_2x2_cccc
TBENCH_CFLAGS?=-DNUMCORES=4

buildtools=$(shell pkg-config --variable=buildtools optimsoc-sim)

include $(buildtools)/Makefile.verilator.inc
