OR32_PREFIX = or32-elf
OR32_CFLAGS = $(CFLAGS) -nostartfiles -std=gnu99 -O2 -Wall
OR32_LDFLAGS = -mor32-newlib -lgcc -lc -Tarch/mapco/link.ld -e 256

%.o: %.S
	$(OR32_PREFIX)-gcc $(OR32_CFLAGS) -c $< -o $@

%.o: %.c %.h
	$(OR32_PREFIX)-gcc $(OR32_CFLAGS) -c $< -o $@

%.o: %.c
	$(OR32_PREFIX)-gcc $(OR32_CFLAGS) -c $< -o $@
	
	
ARCH_OBJS=crt0.o vectors.o utils.o uart.o printf.o context.o mm.o cas.o