include Makefile.inc
include arch/$(ARCH)/Makefile.inc

all: libkernel.a

OBJS = exceptions.o sbrk.o scheduler.o syscall.o thread.o main.o list.o vmm.o process.o mutex.o condition.o barrier.o multicore.o malloc.o dma.o interrupt.o
REL_ARCH_OBJS = $(ARCH_OBJS:%.o=arch/$(ARCH)/%.o)

libkernel.a: $(OBJS) $(REL_ARCH_OBJS)
	$(OR32_PREFIX)-ar cru $@ $^
	$(OR32_PREFIX)-ranlib $@

arch/$(ARCH)/libarch.a:
	make -C arch/$(ARCH) libarch.a
	
.PHONY: kernel/libarch.a

clean:
	make -C arch/$(ARCH) clean
	rm -f *.a *.o
