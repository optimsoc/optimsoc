#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <stdint.h>

void monitor_init();
void monitor_receive_data(uint32_t source, uint32_t *message, uint32_t len);

void monitor_schedule_run(uint32_t runnable);

#endif
