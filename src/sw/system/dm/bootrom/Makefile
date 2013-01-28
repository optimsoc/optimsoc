all: bootrom.hex bootrom_code.v

bootrom.hex: bootrom.bin
	bin2vmem bootrom.bin > bootrom.hex

bootrom_code.v: bootrom.bin
	cat bootrom.bin | ./bin2vlogarray > bootrom_code.v

bootrom.bin: a.out
	or32-elf-objcopy -O binary a.out bootrom.bin

bootrom.dis: a.out
	or32-elf-objdump -d a.out > bootrom.dis

a.out: boot.s
	or32-elf-as boot.s

install:
	cp -i bootrom_code.v ../../rtl/bootrom/verilog/

