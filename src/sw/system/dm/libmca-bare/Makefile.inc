MCAPI_BASE=$(MCA_BASE)/mcapi
MTAPI_BASE=$(MCA_BASE)/mtapi

CFLAGS=-std=gnu99 -Wall -O0 -I $(MCA_BASE) -I $(MCAPI_BASE)/include -I $(MCAPI_BASE)/src/mcapi_trans/mcapi_trans_simple -I $(MCA_BASE)/common -I $(MCA_BASE)/../libbaremetal -I $(MCA_BASE)/../libruntime/include -I $(MCA_BASE)/../libsupport -I $(MTAPI_BASE)/include
CC=or32-elf-gcc

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
