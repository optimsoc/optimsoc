
all: $(PROGRAM).elf $(PROGRAM).dis $(PROGRAM).bin $(PROGRAM).vmem
     
$(PROGRAM).elf: $(PROGRAM).o sysconfig.o runtimeconfig.o link.ld
	or32-elf-gcc -Wall -nostartfiles -nostdlib \
	$(OPTIMSOC_NEWLIB)/lib/crt0.o \
	$(PROGRAM).o sysconfig.o runtimeconfig.o -o $@ \
	$(BASE)/libbaremetal/libbaremetal.a \
	$(BASE)/libruntime/libruntime.a \
	-L $(OPTIMSOC_NEWLIB)/lib/ -lc -loptimsoc -lc -lgcc \
	-T link.ld -e 256

sysconfig.o: ../sysconfig.c
	or32-elf-gcc -O0 -std=gnu99 -c -o $@ $^

runtimeconfig.o: ../runtimeconfig.c
	or32-elf-gcc -O0 -std=gnu99 -c -o $@ $^

link.ld: $(BASE)/apps/runtime/link.ld.in sysconfig.o
	@echo "Generate linker script"
	@BASE=$(BASE) $(BASE)/apps/runtime/create_linker_script.sh	

%.dis: %.elf
	or32-elf-objdump -D $^ > $@

%.vmem: %.bin 
	bin2vmem $^ > $@

%.bin: %.elf 
	or32-elf-objcopy -O binary $^ $@

%.o: %.c
	or32-elf-gcc -O0 -std=gnu99 -c -I $(BASE)/libbaremetal/include -I $(BASE)/libruntime/include -o $@ $^

clean:
	rm -f *.o *.elf *.bin *.vmem *.dis link.ld
