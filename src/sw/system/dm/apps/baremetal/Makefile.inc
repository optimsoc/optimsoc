
all: $(PROGRAM).elf $(PROGRAM).dis $(PROGRAM).bin $(PROGRAM).vmem
     
$(PROGRAM).elf: $(PROGRAM).o sysconfig.o link.ld
	or32-elf-gcc -Wall -nostartfiles -mor32-newlib -lc -lgcc $(PROGRAM).o sysconfig.o -o $@ $(BASE)/libbaremetal/crt0.o $(BASE)/libbaremetal/vectors.o $(BASE)/libbaremetal/libbaremetal.a -T link.ld -e 256

sysconfig.o: ../sysconfig.c
	or32-elf-gcc -O0 -std=gnu99 -c -o $@ $^

link.ld: $(BASE)/apps/baremetal/link.ld.in sysconfig.o
	@echo "Generate linker script"
	@BASE=$(BASE) $(BASE)/apps/baremetal/create_linker_script.sh	

%.dis: %.elf
	or32-elf-objdump -D $^ > $@

%.vmem: %.bin 
	bin2vmem $^ > $@

%.bin: %.elf 
	or32-elf-objcopy -O binary $^ $@

%.o: %.c
	or32-elf-gcc -O0 -std=gnu99 -c -I $(BASE)/libbaremetal/include -o $@ $^

clean:
	rm -f *.o *.elf *.bin *.vmem *.dis link.ld
