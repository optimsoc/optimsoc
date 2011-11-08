include Makefile.inc
include arch/$(ARCH)/Makefile.inc

all: librts.a

OBJS = init.o malloc.o sbrk.o mmap.o util.o write.o dma.o thread.o services.o
REL_ARCH_OBJS = $(ARCH_OBJS:%.o=arch/$(ARCH)/%.o)

KLIBC_OBJS_GCC = klibc/libgcc/__ashldi3.o klibc/libgcc/__udivdi3.o klibc/libgcc/__umoddi3.o klibc/libgcc/__udivmoddi4.o
KLIBC_OBJS = $(KLIBC_OBJS_GCC) klibc/fputs.o klibc/fwrite.o klibc/puts.o klibc/printf.o  klibc/strlen.o klibc/vfprintf.o klibc/vsnprintf.o

librts.a: $(OBJS) $(REL_ARCH_OBJS) $(KLIBC_OBJS)
	$(OR32_PREFIX)-ar cru $@ $^
	$(OR32_PREFIX)-ranlib $@

clean:
	rm -f $(OBJS) $(REL_ARCH_OBJS) $(KLIBC_OBJS) librts.a