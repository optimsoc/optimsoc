
all: $(PROGRAM).elf $(PROGRAM).dis $(PROGRAM).bin $(PROGRAM).vmem \
     $(PROGRAM)-sim.elf $(PROGRAM)-sim.dis $(PROGRAM)-sim.bin $(PROGRAM)-sim.vmem
     
$(PROGRAM).elf: $(PROGRAM).o ../sysconfig.o ../runtimeconfig.o
	or32-elf-gcc -Wall $^ -o $@ $(BASE)/libruntime/crt0.o $(BASE)/libruntime/vectors.o -Wl,--start-group $(BASE)/libruntime/libruntime.a $(BASE)/libmca-bare/libmca.a -Wl,--end-group  -nostartfiles -mor32-newlib -lc -lgcc  -T../link.ld -e 256 -Wl,-M

$(PROGRAM)-sim.elf: $(PROGRAM).o ../sysconfig.o ../runtimeconfig.o
	or32-elf-gcc -Wall $^ -o $@ $(BASE)/libruntime/crt0-sim.o $(BASE)/libruntime/vectors.o -Wl,--start-group $(BASE)/libruntime/libruntime-sim.a $(BASE)/libmca-bare/libmca.a -Wl,--end-group -nostartfiles -mor32-newlib -lc -lgcc -T../link.ld -e 256

%.dis: %.elf
	or32-elf-objdump -D $^ > $@

%.vmem: %.bin 
	bin2vmem $^ > $@

%.bin: %.elf 
	or32-elf-objcopy -O binary $^ $@

%.o: %.c
	or32-elf-gcc -O0 -std=gnu99 -c -I $(BASE)/libmca-bare -I $(BASE)/libmca-bare/mcapi/include -I $(BASE)/libmca-bare/mcapi/src/mcapi_trans/mcapi_trans_simple -I $(BASE)/libmca-bare/mtapi/include -I $(BASE)/libruntime/include -o $@ $^

%.o: %.S
	or32-elf-gcc -O0 -Wall -o $@ -c $^ 

clean:
	rm -f *.o *.elf *.bin *.vmem *.dis
