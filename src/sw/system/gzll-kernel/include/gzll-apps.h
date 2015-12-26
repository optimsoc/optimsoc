#ifndef __GZLL_TASKS_H__
#define __GZLL_TASKS_H__

#include <stdint.h>

#define GZLL_BOOT_EVERYWHERE (uint32_t) -2
#define GZLL_BOOT_ANYWHERE   (uint32_t) -1

struct gzll_task_descriptor {
    const char *identifier;
    void *obj_start;
    void *obj_end;
};

struct gzll_task_descriptor_list {
    uint32_t len;
    struct gzll_task_descriptor tasks[];
};

struct gzll_application_descriptor {
    const char *identifier;
    struct gzll_task_descriptor_list *tasks;
};

struct gzll_application_descriptor_table {
    uint32_t len;
    struct gzll_application_descriptor applications[];
};

extern struct gzll_application_descriptor_table gzll_application_table;

struct gzll_boot_mappings {
    uint32_t len;
    struct {
        int32_t rank;
        struct gzll_task_descriptor *task;
        uint32_t instidx;;
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
