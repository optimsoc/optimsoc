#ifndef GROUP_H_
#define GROUP_H_

#include "list.h"

#include <stdint.h>

struct group {
    uint32_t    id;
    char        *name;
    struct list *tasks;
};

void group_init();
void group_create(char *name);
void group_add(struct group *g);
struct group *group_find_byname(char *name);
struct group *group_find_byid(uint32_t id);

#endif /* GROUP_H_ */
