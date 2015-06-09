#ifndef __TASK_H__
#define __TASK_H__

struct gzll_node;

#include <optimsoc-runtime.h>
#include "gzll.h"
#include "app.h"

#define GZLL_NODE_IDENTIFIER_LENGTH 64

struct gzll_node {
    uint32_t id;
    char identifier[GZLL_NODE_IDENTIFIER_LENGTH];
    struct gzll_app *app;
    uint32_t taskid;

    enum {
        GZLL_TASK_ACTIVE = 0,
        GZLL_TASK_SUSPENDED = 0
    } state;
    optimsoc_page_dir_t pagedir;
    /* at the moment only one thread per task */
    optimsoc_thread_t thread;
};

#endif
