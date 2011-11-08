/*
 * appinit.c
 *
 *  Created on: Nov 28, 2010
 *      Author: wallento
 */

#include "appinit.h"

extern void *_binary_app0_bin_start, *_binary_app0_bin_end;
extern void *_binary_app1_bin_start, *_binary_app1_bin_end;

app_t app_init[2] = { { &_binary_app0_bin_start,&_binary_app0_bin_end },
                      { &_binary_app1_bin_start,&_binary_app1_bin_end }};
unsigned int app_init_size = 2;
