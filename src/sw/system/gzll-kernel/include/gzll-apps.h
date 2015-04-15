#ifndef __GZLL_TASKS_H__
#define __GZLL_TASKS_H__

#include <stdint.h>

struct gzll_task {
    const char *identifier;
    void *obj_start;
    void *obj_end;
};

struct gzll_application {
    const char *identifier;
    struct gzll_task *tasks;
};

extern struct gzll_application gzll_application_table[];

#endif
