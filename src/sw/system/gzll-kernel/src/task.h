#ifndef __TASK_H__
#define __TASK_H__

struct gzll_task;

#include "app.h"

struct gzll_task {
    uint32_t id;
    char *identifier;
    struct gzll_app *app;
    gzll_node_id app_nodeid;
};

#endif
