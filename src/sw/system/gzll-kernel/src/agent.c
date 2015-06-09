#include <optimsoc-baremetal.h>
#include <optimsoc-runtime.h>
#include <gzll.h>

#include <stdio.h>

extern uint32_t gzll_rank;
optimsoc_thread_t _gzll_agent_thread;

void agent_thread()
{
    printf("agent start\n");
    optimsoc_timer_wait_ticks(2);

    printf("agent continue\n");

    /* TODO trigger task migration */

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
