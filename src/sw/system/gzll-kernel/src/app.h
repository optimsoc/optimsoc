#ifndef __APP_H__
#define __APP_H__

struct gzll_app;

#include "task.h"
#include "taskdir.h"

struct gzll_app {
    char *name;
    struct gzll_app_taskdir *task_dir;
};

void gzll_app_new(uint32_t id, const char* name);
struct gzll_app_taskdir *gzll_app_get_taskdir(uint32_t id);

#endif
