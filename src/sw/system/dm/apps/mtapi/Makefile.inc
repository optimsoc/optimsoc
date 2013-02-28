D_LIBMCAPI := $(BASE)/libmcapi
D_LIBRUN := $(BASE)/libruntime
D_LIBBARE := $(BASE)/libbaremetal
D_LIBMTAPI := $(BASE)/libmtapi

all: $(PROGRAM).elf $(PROGRAM).dis $(PROGRAM).bin $(PROGRAM).vmem

$(PROGRAM).elf: $(PROGRAM).o ../sysconfig.o ../runtimeconfig.o link.ld
	or32-elf-gcc -Wall \
		-nostartfiles -mor32-newlib -lc -lgcc \
		-o $@ \
		$(PROGRAM).o ../sysconfig.o ../runtimeconfig.o \
		$(D_LIBBARE)/crt0.o $(D_LIBBARE)/vectors.o \
		-Wl,--start-group \
		$(D_LIBBARE)/libbaremetal.a \
		$(D_LIBRUN)/libruntime.a \
		$(D_LIBMCAPI)/libmcapi-runtime-simple.a \
		$(D_LIBMTAPI)/build/libmtapi.a \
		-Wl,--end-group \
		-Tlink.ld -e 256

sysconfig.o: ../sysconfig.c
	or32-elf-gcc -O0 -std=gnu99 -c -o $@ $^

link.ld: $(BASE)/apps/mtapi/link.ld.in sysconfig.o
	@echo "Generate linker script"
	@BASE=$(BASE) $(BASE)/apps/mtapi/create_linker_script.sh

%.dis: %.elf
	or32-elf-objdump -D $^ > $@

%.vmem: %.bin
	bin2vmem $^ > $@

%.bin: %.elf
	or32-elf-objcopy -O binary $^ $@

%.o: %.c
	or32-elf-gcc -O0 -std=gnu99 -c -o $@ $^ \
		-I$(D_LIBRUN)/include \
		-I$(D_LIBBARE)/include \
		-I$(D_LIBMCAPI)/include \
		-I$(D_LIBMTAPI)/include \

%.o: %.S
	or32-elf-gcc -O0 -Wall -o $@ -c $^ 

clean:
	rm -f *.o *.elf *.bin *.vmem *.dis
