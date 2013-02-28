OR32_CFLAGS = $(CFLAGS) -std=gnu99 -O2 -Wall -I ../include -I $(OPTIMSOC_NEWLIB)/include
	
all: ../libbaremetal.a 

DEPFILE=.depend

OBJECTS_C=cas.o        \
          mutex.o    \
          dma.o \
          mp_simple.o \
          lcd.o
                              
OBJECTS=$(OBJECTS_C)

SRCS_C=$(OBJECTS_C:%.o=../src/%.c)

dep:
	or32-elf-gcc -I ../include -I $(OPTIMSOC_NEWLIB)/include -MM $(SRCS_C) > $(DEPFILE)

../libbaremetal.a: $(OBJECTS)
	or32-elf-ar cru $@ $^
	or32-elf-ranlib $@

%.o: ../src/%.S
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

%.o: ../src/%.c ../src/%.h
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@
	
$(OBJECTS_C): %.o: ../src/%.c
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

clean:
	rm -f ../*.a *.o ../*.o $(DEPFILE)

-include $(DEPFILE)

