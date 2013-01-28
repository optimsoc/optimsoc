all: $(PROGRAM).elf $(PROGRAM).dis $(PROGRAM).bin $(PROGRAM).vmem \
     $(PROGRAM)-paging.elf $(PROGRAM)-paging.dis $(PROGRAM)-paging.bin $(PROGRAM)-paging.vmem
     
$(PROGRAM).elf: $(PROGRAM).o ../sysconfig.o ../runtimeconfig.o
	or32-elf-gcc -Wall $^ -o $@ $(BASE)/libbaremetal/crt0.o $(BASE)/libbaremetal/vectors.o -Wl,--start-group $(BASE)/libruntime/libruntime.a  $(BASE)/libbaremetal/libbaremetal.a -Wl,--end-group -nostartfiles -mor32-newlib -lc -lgcc -T../link.ld -e 256

$(PROGRAM)-paging.elf: $(PROGRAM).o ../sysconfig.o ../runtimeconfig.o
	or32-elf-gcc -Wall $^ -o $@ $(BASE)/libbaremetal/crt0-paging.o $(BASE)/libbaremetal/vectors.o -Wl,--start-group $(BASE)/libruntime/libruntime.a  $(BASE)/libbaremetal/libbaremetal-paging.a -Wl,--end-group -nostartfiles -mor32-newlib -lc -lgcc -T../link.ld -e 256
	
%.dis: %.elf
	or32-elf-objdump -D $^ > $@

%.vmem: %.bin 
	bin2vmem $^ > $@

%.bin: %.elf 
	or32-elf-objcopy -O binary $^ $@

%.o: %.c
	or32-elf-gcc -O0 -std=gnu99 -c -I $(BASE)/libruntime/include -o $@ $^

%.o: %.S
	or32-elf-gcc -O0 -Wall -o $@ -c $^ 

clean:
	rm -f *.o *.elf *.bin *.vmem *.dis
