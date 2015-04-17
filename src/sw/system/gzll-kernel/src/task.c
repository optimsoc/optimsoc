#include "gzll.h"
#include "gzll-apps.h"



/*void _gzll_task_bootstrap_instance(uint32_t id,
                              struct gzll_boot_app_instance *bootapp) {
    struct gzll_application *app = bootapp->app;
    printf("Boot app %s\n", app->identifier);

    for (uint32_t m = 0; m < bootapp->num_mappings; m++) {
        if (bootapp->mappings[m].rank == _gzll_rank) {
            struct gzll_task *task = bootapp->mappings[m].task;
            printf(" -> Start task %s here\n", task->identifier);
        }
    }
}*/

