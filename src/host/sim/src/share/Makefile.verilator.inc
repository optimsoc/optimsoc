# -*- makefile -*- vim:ft=make

CXX ?= g++
CFLAGS ?= -g -Wall
VERILATOR ?= verilator
OBJDIR := .
SRCDIR := $(PWD)

INC=$(shell pkg-config --cflags optimsoc-sim)
LIB=$(shell pkg-config --libs optimsoc-sim)
INC+=$(shell pkg-config --cflags verilator)
LIB+=$(shell pkg-config --libs verilator)

VERILATOR_INCDIR=$(shell pkg-config --variable=includedir verilator)

build-verilator: $(OBJDIR)/$(TBENCH)

flags = -CFLAGS "$(INC) -I$(OBJDIR)" -LDFLAGS "$(LIB) -pthread" --exe --trace --sv --unroll-count 256 --Wno-fatal

$(OBJDIR)/$(TBENCH): $(OBJDIR)/verilator.vc
	$(VERILATOR) --Mdir $(OBJDIR)/obj_dir --cc  \
		-f $(OBJDIR)/verilator.vc \
		$(SRCDIR)/$(TBENCH).cpp $(flags) -o $(OBJDIR)/$(TBENCH)
	make -C $(OBJDIR)/obj_dir -f V$(TBENCH).mk

$(OBJDIR)/verilator.vc:
	VERILATOR_VC_FILE=$(OBJDIR)/verilator.vc tclsh \
		$(OPTIMSOC_TCL)/optimsoc_verilator.tcl $(SRCDIR)/build.tcl

clean:
	rm -rf $(OBJDIR)/obj_dir $(OBJDIR)/*.o

