include ../Makefile.inc

OBJS=main.o

all: app0bin.o app0.dis

app0bin.o: app0.bin
	$(OR32_PREFIX)-objcopy -I binary -O elf32-or32 app0.bin app0bin.o
	./patchmachine.py app0bin.o

app0.dis: app0.elf
	$(OR32_PREFIX)-objdump -d app0.elf > app0.dis

app0.bin: app0.elf
	$(OR32_PREFIX)-objcopy -I elf32-or32 -O binary --set-section-flags .bss=alloc,load,contents app0.elf app0.bin

app0.elf: $(OBJS) ../../librts/librts.a
	$(OR32_PREFIX)-gcc -nostdlib -L ../../librts/ -lrts $^ -T../../librts/linker.ld -o app0.elf
	
clean:
	rm -f *.o *.a *.elf
