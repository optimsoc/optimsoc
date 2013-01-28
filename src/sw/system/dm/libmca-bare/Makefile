MCA_BASE=.
include Makefile.inc

OBJECTS=mcapi/src/mcapi.o \
	mcapi/src/mcapi_trans/mcapi_trans_simple/mcapi_trans_simple.o \
	mtapi/src/mtapi.o \
	mtapi/src/mtapi_impl_spec.o \
	mtapi/src/mtapi_optimsoc_spec.o \
	mtapi/src/mtapi_runtime.o \
	common/mca_utils.o

all: libmca.a

libmca.a: $(OBJECTS)
	or32-elf-ar cru $@ $(OBJECTS)
	or32-elf-ranlib $@

clean:
	rm $(OBJECTS)
