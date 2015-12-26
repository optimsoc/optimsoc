#ifndef __GZLL_H__
#define __GZLL_H__

#include <optimsoc-runtime.h>
#include <stdint.h>

#include "gzll-apps.h"

void gzll_syscall_handler(struct optimsoc_syscall *sysc);

void gzll_init();
void init();
void communication_thread();
void communication_init();
void gzll_agent_init();

typedef uint32_t gzll_node_id;
extern gzll_node_id gzll_node_nxtid;

extern uint32_t gzll_rank;

struct _gzll_image_layout {
    char gzll[4];
    void *kernel_end;
    void *apps_start;
    void *apps_end;
};
extern struct _gzll_image_layout _gzll_image_layout;


uint32_t gzll_swapping();

struct gzll_page_t {
    enum PAGE_STATUS { UNUSED = 0, KERNEL = 1, USER = 2 } status;
    enum PAGE_PLACE { LOCAL = 0, GLOBAL = 1, BOTH = 2 } place;
    uint32_t page_local;
    uint32_t page_global;
    // TODO: task
};

void gzll_paging_init();
void gzll_paging_dpage_fault(uint32_t vaddr);
void gzll_paging_ipage_fault(uint32_t vaddr);

unsigned int gzll_page_alloc();

void gzll_apps_bootstrap();

void gzll_node_start(uint32_t app_id, char* app_name, uint32_t app_nodeid,
                     char *taskname, struct gzll_task_descriptor *task);


#endif /* SRC_GZLL_H_ */
