#ifndef __TASK_H__
#define __TASK_H__

struct gzll_task;

#include "app.h"

struct gzll_task {
    char *identifier;
    struct gzll_app *app;
};

#endif
