# -*- makefile -*- vim:ft=make

VSIM ?= vsim
VLIB ?= vlib
SCCOM ?= sccom
# XXX: The multilib include is a workaround, this should be fixed by ModelSim
SCCOM_CFLAGS ?= -Wall -I /usr/include/x86_64-linux-gnu

TBENCH_SYSC_SRCS ?=
TBENCH_VSIM_SIMFLAGS ?=
TBENCH_VSIM_CFLAGS ?=

SYSC_OBJ = $(TBENCH_SYSC_SRCS:%.cpp=%.o)

build-modelsim: $(TBENCH)

run-modelsim: $(TBENCH)-sim

run-modelsim-gui: $(TBENCH)-sim-gui

work:
	if [ ! -d work ]; then $(VLIB) work; fi

%.o: %.cpp
	$(SCCOM) $(SCCOM_CFLAGS) $<

sysc: work $(SYSC_OBJ)
	$(SCCOM) -link

$(TBENCH): work sysc
	$(VSIM) -c $(TBENCH_VSIM_CFLAGS) -do "do $(OPTIMSOC_TCL)/optimsoc_modelsim.tcl build.tcl; exit"

$(TBENCH)-sim: $(TBENCH)
	$(VSIM) -c -voptargs=+acc work.$(TBENCH) $(TBENCH_VSIM_SIMFLAGS) -do "run -all"

$(TBENCH)-sim-gui: $(TBENCH)
	$(VSIM) -voptargs=+acc work.$(TBENCH) $(TBENCH_VSIM_SIMFLAGS)

clean:
	rm -rf work

.PHONY: $(TBENCH)

