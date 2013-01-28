BASE=../../..

all: blowfish.elf blowfish.dis blowfish.bin 
     
blowfish.elf: blowfish.o
	or32-elf-gcc -Wall -nostartfiles -nostdlib $^ -o $@ $(BASE)/libbaremetal/crt0.o $(BASE)/libbaremetal/vectors.o $(BASE)/libbaremetal/libbaremetal.a -Tlink.ld -e 256


%.dis: %.elf
	or32-elf-objdump -D $^ > $@

%.vmem: %.bin 
	bin2vmem $^ > $@

%.bin: %.elf 
	or32-elf-objcopy -O binary $^ $@

%.o: %.c
	or32-elf-gcc -O0 -std=gnu99 -c -I $(BASE)/libbaremetal/ -I $(BASE)/liboptimsoc/ -o $@ $^

%.o: %.S
	or32-elf-gcc -O0 -I $(BASE)/libsupport/ -Wall -o $@ -c $^ 

clean:
	rm -f *.o *.elf *.bin *.vmem *.dis
