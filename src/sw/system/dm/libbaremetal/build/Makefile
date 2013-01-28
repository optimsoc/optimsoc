OR32_CFLAGS = $(CFLAGS) -std=gnu99 -O2 -Wall -I ../include
	
all: ../libbaremetal.a ../libbaremetal-paging.a

DEPFILE=.depend

OBJECTS_C=exceptions.o \
		  utils.o      \
          int.o        \
          cas.o        \
          mutex.o    \
          sbrk.o     \
          dma.o \
          mp_simple.o \
          lcd.o \
          optimsoc.o
                              
OBJECTS=$(OBJECTS_C)

SRCS_C=$(OBJECTS_C:%.o=../src/%.c)

dep:
	or32-elf-gcc -I ../include -MM $(SRCS_C) > $(DEPFILE)

../libbaremetal.a: $(OBJECTS) ../crt0.o boot.o ../vectors.o
	or32-elf-ar cru $@ $^
	or32-elf-ranlib $@

../libbaremetal-paging.a: $(OBJECTS) ../crt0-paging.o boot-paging.o ../vectors.o
	or32-elf-ar cru $@ $^
	or32-elf-ranlib $@

../vectors.o: ../src/vectors.S	
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

../crt0.o: ../src/crt0.S	
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

../crt0-paging.o: ../src/crt0.S	
	or32-elf-gcc $(OR32_CFLAGS) -DPAGING -c $< -o $@

%.o: ../src/%.S
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

%.o: ../src/%.c ../src/%.h
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@
	
boot.o: ../src/boot.c
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

boot-paging.o: ../src/boot.c
	or32-elf-gcc $(OR32_CFLAGS) -DPAGING -c $< -o $@
	
$(OBJECTS_C): %.o: ../src/%.c
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

clean:
	rm -f ../*.a *.o ../*.o $(DEPFILE)

-include $(DEPFILE)

