OR32_CFLAGS = $(CFLAGS) -std=gnu99 -O2 -Wall -I ../include -I ../src -I ../../libbaremetal/include -I $(OPTIMSOC_NEWLIB)/include
	
all: ../libruntime.a

DEPFILE=.depend

OBJECTS_C=scheduler.o  \
          runtime.o    \
          list.o       \
          thread.o
                              
OBJECTS_ASM=ctx_replace.o \
            yield_switchctx.o

OBJECTS=$(OBJECTS_C) $(OBJECTS_ASM)

SRCS_C=$(OBJECTS_C:%.o=../src/%.c)
SRCS_LIBC=$(OBJECTS_LIBC:%.o=../src/libc/%.c)
SRCS_ASM=$(OBJECTS_ASM:%.o=../src/%.S)

dep:
	or32-elf-gcc -I ../src -I $(OPTIMSOC_NEWLIB)/include -MM $(SRCS_C) $(SRCS_ASM) > $(DEPFILE)

../libruntime.a: $(OBJECTS)
	or32-elf-ar cru $@ $^
	or32-elf-ranlib $@

%.o: ../src/%.S
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

%.o: ../src/%.c ../src/%.h
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@
	
$(OBJECTS_C): %.o: ../src/%.c
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

clean:
	rm -f ../*.a *.o ../*.o libc/*.o 

-include $(DEPFILE)
