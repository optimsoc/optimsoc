#ifndef __GZLL_TASKS_H__
#define __GZLL_TASKS_H__

#include <stdint.h>

struct gzll_task {
    const char *identifier;
    void *obj_start;
    void *obj_end;
};

struct gzll_task_list {
    uint32_t len;
    struct gzll_task tasks[];
};

struct gzll_application {
    const char *identifier;
    struct gzll_task_list *tasks;
};

struct gzll_application_table {
    uint32_t len;
    struct gzll_application applications[];
};

extern struct gzll_application_table gzll_application_table;

struct gzll_boot_mappings {
    uint32_t len;
    struct {
        int32_t rank;
        struct gzll_task *task;
    } mappings[];
};

struct gzll_boot_apps {
    uint32_t len;
    struct {
        char *name;
        struct gzll_boot_mappings *mappings;
    } instances[];
};

extern struct gzll_boot_apps gzll_boot_apps;

#endif
