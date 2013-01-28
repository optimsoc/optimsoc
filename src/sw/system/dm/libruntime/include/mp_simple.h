#ifndef __MP_SIMPLE_H__
#define __MP_SIMPLE_H__

#define E_SIZE -12

#define OPTIMSOC_DEST_MSB 31
#define OPTIMSOC_DEST_LSB 27
#define OPTIMSOC_CLASS_MSB 26
#define OPTIMSOC_CLASS_LSB 24
#define OPTIMSOC_CLASS_NUM 8
#define OPTIMSOC_SRC_MSB 23
#define OPTIMSOC_SRC_LSB 19

extern void optimsoc_mp_simple_init();
extern void optimsoc_mp_simple_send(unsigned int size,unsigned int* buf);
void optimsoc_mp_simple_inth(void* arg);
void optimsoc_mp_simple_addhandler(unsigned int class,void (*hnd)(unsigned int*,int));


#endif
