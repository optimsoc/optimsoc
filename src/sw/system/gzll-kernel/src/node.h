#ifndef __NODE_H__
#define __NODE_H__

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
        GZLL_NODE_ACTIVE = 0,
        GZLL_NODE_SUSPENDED = 0
    } state;
    optimsoc_page_dir_t pagedir;
    /* at the moment only one thread per task */
    optimsoc_thread_t thread;
};

void gzll_node_add(struct gzll_node *node);
int gzll_node_remove(struct gzll_node *node);
struct gzll_node *gzll_node_find(uint32_t id);

void gzll_node_suspend(struct gzll_node *node);
void gzll_node_resume(struct gzll_node *node);
struct gzll_node *gzll_node_fetch(uint32_t remote_tile, void *remote_addr);

#endif
