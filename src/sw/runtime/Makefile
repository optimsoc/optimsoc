include kernel/Makefile.inc
include kernel/arch/$(ARCH)/Makefile.inc

all: final.vmem final.dis

final.vmem: final.bin
	bin2vmem final.bin > final.vmem
	ln -fs final.vmem sram.vmem

final.bin: final.elf 
	or32-elf-objcopy -O binary final.elf final.bin

final.dis: final.elf
	or32-elf-objdump -d final.elf > final.dis
	or32-elf-objdump -t final.elf > final.sym

final.elf: kernel/libkernel.a apps/app0/app0bin.o apps/app1/app1bin.o appinit.o
	or32-elf-gcc -nostartfiles -mor32-newlib -lgcc -lc kernel/arch/$(ARCH)/vectors.o kernel/libkernel.a apps/app0/app0bin.o apps/app1/app1bin.o appinit.o -Tlink.ld -e 256 -o final.elf 

kernel/libkernel.a:
	make -C kernel libkernel.a

librts/librts.a:
	make -C librts librts.a

apps/app0/app0bin.o: librts/librts.a
	make -C apps/app0 all

apps/app1/app1bin.o: librts/librts.a
	make -C apps/app1 all

.PHONY: kernel/libkernel.a librts/librts.a apps/app0/app0bin.o

clean:
	make -C kernel clean
	make -C librts clean
	make -C apps/app0 clean
	make -C apps/app1 clean
	rm -f *.elf *.o *.vmem *.bin *.dis

