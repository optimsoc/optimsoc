#ifndef __APP_H__
#define __APP_H__

struct gzll_app;

#include "node.h"
#include "taskdir.h"

struct gzll_app {
    uint32_t id;
    char *name;
    struct gzll_app_taskdir *task_dir;
};

void gzll_app_new(uint32_t id, const char* name);
struct gzll_app *gzll_app_get(uint32_t id);

#endif
