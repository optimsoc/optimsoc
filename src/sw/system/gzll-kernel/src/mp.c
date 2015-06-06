#include "mp.h"

#include "gzll-syscall.h"
#include "gzll.h"
#include "taskdir.h"

#include <optimsoc-runtime.h>

#include <assert.h>
#include "node.h"

void endpoint_table_init(struct gzll_endpoint_table **table) {
    *table = calloc(sizeof(struct gzll_endpoint_table) + 8 * 4, 1);
    (*table)->len = 8;
}


void endpoint_table_add(struct gzll_endpoint_table *table, uint32_t port,
                        optimsoc_mp_endpoint_handle endpoint) {
    assert(table);
    assert(port < table->len);

    table->ep[port] = endpoint;
}

void gzll_syscall_endpoint_create(struct gzll_syscall *syscall) {
    uint32_t app_nodeid, app_id, nodeid, port;
    optimsoc_endpoint_type type;
    uint32_t buffer_size, max_size;

    optimsoc_thread_t thread;
    struct gzll_node* task;
    thread = optimsoc_thread_current();
    task = (struct gzll_node*) optimsoc_thread_get_extra_data(thread);

    app_nodeid = task->taskid;

    struct gzll_app *app = task->app;
    assert(app);

    app_id = app->id;

    struct gzll_app_taskdir *taskdir = app->task_dir;
    assert(taskdir);

    struct gzll_app_node *app_node = &taskdir->tasks[app_nodeid];

    port = syscall->param[0];
    type = syscall->param[1];

    optimsoc_mp_endpoint_handle eph;

    optimsoc_mp_endpoint_create(&eph, nodeid, port, type, buffer_size, max_size);

    endpoint_table_add(app_node->endpoints, port, eph);

    syscall->output = 0;
}

void gzll_syscall_endpoint_get(struct gzll_syscall *syscall) {
    uint32_t taskid, app_id, nodeid, port, rank;

    taskid = syscall->param[0];
    port = syscall->param[1];

    optimsoc_thread_t thread;
    struct gzll_node* task;
    thread = optimsoc_thread_current();
    task = (struct gzll_node*) optimsoc_thread_get_extra_data(thread);

    struct gzll_app *app = task->app;
    assert(app);

    app_id = app->id;

    struct gzll_app_taskdir *taskdir = app->task_dir;
    assert(taskdir);

    struct gzll_app_node *app_node = &taskdir->tasks[taskid];
    rank = app_node->rank;
    nodeid = app_node->nodeid;

    optimsoc_mp_endpoint_handle eph;

    int rv;
    rv = optimsoc_mp_endpoint_get(&eph, optimsoc_get_ranktile(rank), nodeid,
                                  port);

    syscall->output = (uint32_t) rv;

    if (rv == 0) {
        endpoint_table_add(app_node->endpoints, port, eph);
    }
}

void gzll_syscall_channel_connect(struct gzll_syscall *syscall) {
    // param 0: from task
    // param 1: from port
    // param 2: to task
    // param 3: to port
    // return: success status

    uint32_t fromtask, fromport, totask, toport;

    fromtask = syscall->param[0];
    fromport = syscall->param[1];
    totask = syscall->param[2];
    toport = syscall->param[3];

    optimsoc_thread_t thread;
    struct gzll_node* task;
    thread = optimsoc_thread_current();
    task = (struct gzll_node*) optimsoc_thread_get_extra_data(thread);

    struct gzll_app *app = task->app;
    assert(app);

    struct gzll_app_taskdir *taskdir = app->task_dir;
    assert(taskdir);

    optimsoc_mp_endpoint_handle fromep, toep;

    assert(fromtask < taskdir->size);
    assert(fromport < taskdir->tasks[fromtask].endpoints->len);

    fromep = taskdir->tasks[fromtask].endpoints->ep[fromport];

    assert(totask < taskdir->size);
    assert(toport < taskdir->tasks[totask].endpoints->len);

    toep = taskdir->tasks[totask].endpoints->ep[toport];

    printf("Connect %p->%p\n", fromep, toep);
    syscall->output = optimsoc_mp_channel_connect(fromep, toep);
    printf("Connected\n");

}
