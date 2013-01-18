#ifndef __MP_SIMPLE_H__
#define __MP_SIMPLE_H__

#define E_SIZE -12

extern void optimsoc_mp_simple_init();
extern void optimsoc_mp_simple_send(unsigned int size,unsigned int* buf);
void optimsoc_mp_simple_inth(void* arg);
void optimsoc_mp_simple_addhandler(unsigned int class,void (*hnd)(unsigned int*,int));

#endif
