include ../Makefile.inc

OBJS=main.o

all: app1bin.o app1.dis

app1bin.o: app1.bin
	$(OR32_PREFIX)-objcopy -I binary -O elf32-or32 app1.bin app1bin.o
	./patchmachine.py app1bin.o

app1.dis: app1.elf
	$(OR32_PREFIX)-objdump -d app1.elf > app1.dis

app1.bin: app1.elf
	$(OR32_PREFIX)-objcopy -I elf32-or32 -O binary --set-section-flags .bss=alloc,load,contents app1.elf app1.bin

app1.elf: $(OBJS) ../../librts/librts.a
	$(OR32_PREFIX)-gcc -nostdlib -L ../../librts/ -lrts $^ -T../../librts/linker.ld -o app1.elf
	
clean:
	rm -f *.o *.a *.elf
