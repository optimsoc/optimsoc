OR32_CFLAGS = $(CFLAGS) -std=gnu99 -O2 -Wall -I ../include -I ../../libbaremetal/include
	
all: ../libmcapi-bare-simple.a

DEPFILE=.depend

OBJECTS_COMMON=mcapi.o
OBJECTS_SIMPLE=mcapi_trans_simple/mcapi_trans_simple.o

OBJECTS=$(OBJECTS_COMMON) $(OBJECTS_SIMPLE)

SRCS=$(OBJECTS:%.o=../src/%.c)

dep:
	or32-elf-gcc -I ../include -MM $(SRCS) > $(DEPFILE)

../libmcapi-bare-simple.a: $(OBJECTS)
	or32-elf-ar cru $@ $^
	or32-elf-ranlib $@

$(OBJECTS): %.o: ../src/%.c
	or32-elf-gcc $(OR32_CFLAGS) -c $< -o $@

clean:
	rm -f ../*.a *.o ../*.o */*.o 

-include $(DEPFILE)
