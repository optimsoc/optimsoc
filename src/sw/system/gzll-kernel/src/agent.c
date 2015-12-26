#include <optimsoc-baremetal.h>
#include <optimsoc-runtime.h>
#include <gzll.h>

#include <stdio.h>

#include "app.h"
#include "taskdir.h"
#include "node_migrate.h"

extern uint32_t gzll_rank;
optimsoc_thread_t _gzll_agent_thread;

void agent_thread()
{
    printf("agent start\n");
    optimsoc_timer_wait_ticks(2);

    printf("agent continue\n");

    /* Find an arbitrary task to migrate */
    /* uint32_t appid = 0; */
    /* uint32_t dest = 2; */

    /* struct gzll_app *app; */
    /* app = gzll_app_get(appid); */
    /* assert(app); */

    /* uint32_t taskid = 0; */
    /* uint32_t rankid = 0; */
    /* uint32_t nodeid = 0; */

    /* taskdir_taskid_lookup(app->task_dir, "receiver-0", &taskid); */

    /* /\* trigger task migration *\/ */

    /* gzll_node_migrate(appid, taskid, 2); */
}

void gzll_agent_init()
{
    if (gzll_rank == 0) {
        struct optimsoc_thread_attr *attr;
        attr = malloc(sizeof(struct optimsoc_thread_attr));
        optimsoc_thread_attr_init(attr);
        attr->identifier = "agent";
        attr->flags |= OPTIMSOC_THREAD_FLAG_KERNEL;
        optimsoc_thread_create(&_gzll_agent_thread,
                               &agent_thread, attr);
    }
}
