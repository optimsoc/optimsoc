#include <stdio.h>
#include <assert.h>

#include <optimsoc-runtime.h>

#include "gzll-apps.h"
#include "gzll.h"

#include "app.h"

struct optimsoc_list_t *gzll_app_list;

void gzll_app_new(uint32_t id, const char* name) {

    struct gzll_app *app = malloc(sizeof(struct gzll_app));
    assert(app);

    app->id = id;
    app->name = strdup(name);
    app->task_dir = malloc(sizeof(struct gzll_app_taskdir));
    assert(app->task_dir);

    taskdir_initialize(app->task_dir);

    if (!gzll_app_list) {
        gzll_app_list = optimsoc_list_init(app);
    } else {
        optimsoc_list_add_tail(gzll_app_list, app);
    }

}

struct gzll_app *gzll_app_get(uint32_t id) {

    return optimsoc_list_find_content_header(gzll_app_list, id);
}

void _gzll_app_bootstrap(uint32_t appid, char *appname,
                         struct gzll_boot_mappings *map) {
    // We will enumerate all tasks started at boot, globally unique.
    // Essentially this is the index in the mapping, but when there is a
    // mapping for everywhere, we need to be sure to account for those numct()
    // many nodes. This is ensured with accumulating the extra node ids with
    // this offset.
    uint32_t nodeid_offset = 0;

    // Create new app entry
    gzll_app_new(appid, appname);

    // Iterate all boot mappings
    for (uint32_t idx = 0; idx < map->len; idx++) {
        if ((map->mappings[idx].rank == gzll_rank) ||
                (map->mappings[idx].rank == GZLL_BOOT_EVERYWHERE)) {
            // Start this task here or everywhere
            struct gzll_task_descriptor *task = map->mappings[idx].task;

            // Generate taskname unique for app
            char tname[33];
            uint32_t instidx = map->mappings[idx].instidx;
            uint32_t nodeid = idx + nodeid_offset;

            // Special handling of everywhere mappings
            if (map->mappings[idx].rank == GZLL_BOOT_EVERYWHERE) {
                // The index id is the rank
                instidx = gzll_rank;
                // The nodeid gets the rank added
                nodeid += gzll_rank;
                // .. and we need to account for these extra node ids
                nodeid_offset += optimsoc_get_numct() - 1;
            }
            snprintf(tname, 32, "%s-%d", task->identifier, instidx);

            // The fullname includes the application
            char fullname[65];
            snprintf(fullname, 256, "%s.%s", appname, tname);

            gzll_node_start(appid, appname, nodeid, tname, task);

        } else if ((map->mappings[idx].rank == 0) ||
                (map->mappings[idx].rank == GZLL_BOOT_ANYWHERE)) {
            /* TODO: dynamic task allocation */
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
        _gzll_app_bootstrap(appidx, name, map);
    }
}
