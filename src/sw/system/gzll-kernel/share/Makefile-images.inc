
CC_FOR_TARGET ?= or1k-elf-gcc
OBJDUMP_FOR_TARGET ?= or1k-elf-objdump
OBJCOPY_FOR_TARGET ?= or1k-elf-objcopy

LDSCRIPT = $(shell pkg-config --variable=buildscriptdir optimsoc-gzll)/gzll-image.ld
KERNEL = $(shell pkg-config --variable=libdir optimsoc-gzll)/gzll-kernel/libgzll-kernel.a

LIBS = $(shell pkg-config --libs --static gzll-kernel)
INCS = $(shell pkg-config --cflags gzll-kernel)
LIBS += $(shell pkg-config --libs --static optimsoc-baremetal-mp-runtime)
INCS += $(shell pkg-config --cflags optimsoc-baremetal-mp-runtime)
LIBS += $(shell pkg-config --libs --static optimsoc-baremetal-runtime)
INCS += $(shell pkg-config --cflags optimsoc-baremetal-runtime)

image.elf: ${APP_BINARY_OBJECTS} apps.o
	or1k-elf-gcc -mboard=optimsoc -Wall -T $(LDSCRIPT) $^ $(LIBS) -o image.elf

image.elf: $(KERNEL)

image.bin: image.elf
	or1k-elf-objcopy -O binary $^ $@

image.vmem: image.bin
	bin2vmem $^ > $@

%.o: %.c
	$(CC_FOR_TARGET) -O0 -std=gnu99 -c $(INCS) -o $@ $^

%_binary.o: %.bin
	$(OBJCOPY_FOR_TARGET) --rename-section .data=.app_data -I binary -B or1k -O elf32-or1k $^ $@

clean:
	rm -f *.o *.elf *.bin *.vmem
