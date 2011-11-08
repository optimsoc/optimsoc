include Makefile.inc

all: libarch.a

libarch.a: $(ARCH_OBJS)
	$(OR32_PREFIX)-ar cru $@ $^
	$(OR32_PREFIX)-ranlib $@

clean:
	rm -f *.a *.o
