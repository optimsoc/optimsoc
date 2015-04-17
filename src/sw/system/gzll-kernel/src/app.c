#include <stdio.h>

#include "gzll-apps.h"
#include "gzll.h"

void _gzll_app_bootstrap(char *name, struct gzll_boot_mappings *map) {
    for (uint32_t idx = 0; idx < map->len; idx++) {
        if (map->mappings[idx].rank == gzll_rank) {
            struct gzll_task *task = map->mappings[idx].task;
            char *tname = task->identifier;
            char fullname[256];
            snprintf(fullname, 256, "%s.%s", name, tname);
            gzll_task_start(fullname, task);
        }
    }
}

void gzll_apps_bootstrap() {
    printf("Bootstrap applications\n");
    for (uint32_t appidx = 0; appidx < gzll_boot_apps.len; appidx++) {
        char *name;
        struct gzll_boot_mappings *map;

        name = gzll_boot_apps.instances[appidx].name;
        map = gzll_boot_apps.instances[appidx].mappings;
        _gzll_app_bootstrap(name, map);
    }
}
