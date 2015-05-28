#ifndef __TASK_H__
#define __TASK_H__

struct gzll_node;

#include "gzll.h"
#include "app.h"

struct gzll_node {
    uint32_t id;
    char *identifier;
    struct gzll_app *app;
    uint32_t taskid;
};

#endif
