#include "group.h"

#include "runtime.h"

struct list_t *group_list;
uint16_t group_nextid;

void group_init() {
    // Initialize empty
    group_list = 0;
    group_nextid = 0;

    // Create kernel group, will be id 0
    group_create("kernel");
}

void group_create(char *name) {
    // Allocate memory
    struct group *g = malloc(sizeof(struct group));

    // Set id and increment
    // For uniqueness the creating instance sets its ID in the upper 16 bits
    g->id = (runtime_get_instance_id() << 16) | group_nextid;

    // Increment for next identifier
    group_nextid++;

    // Set name, but duplicate!
    g->name = strdup(name);

    // Empty list of tasks
    g->tasks = 0;

    // Append to group or initialize if empty
    if (!group_list) {
        group_list = list_init((void*) g);
    } else {
        list_add_tail(group_list, (void*) g);
    }
}

void group_add(struct group *g) {
    list_add_tail(group_list, (void*) g);
}

struct group *group_find_byname(char *name) {
    // Get head of list
    struct list_entry_t *entry = group_list->head;

    // Iterate list
    while (entry && entry->next) {
        // Cast abstract data to group
        struct group *g = (struct group*) entry->data;

        // If name matches (strcmp): found
        if (strcmp(g->name, name) == 0) {
            return g;
        }
    }

    // If we reach here: nothing found
    return 0;
}

struct group *group_find_byid(uint32_t id) {
    // Get head of list
    struct list_entry_t *entry = group_list->head;

    // Iterate list
    while (entry && entry->next) {
        // Cast abstract data to group
        struct group *g = (struct group*) entry->data;

        // If id matches: found
        if (g->id == id) {
            return g;
        }
    }

    // If we reach here: nothing found
    return 0;
}
