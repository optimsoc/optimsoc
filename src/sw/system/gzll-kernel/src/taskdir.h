#ifndef __TASKDIR_H__
#define __TASKDIR_H__

#include <optimsoc-baremetal.h>
#include <optimsoc-mp.h>

//TODO define error codes & return values

struct gzll_app_node {
    char *identifier;
    uint32_t rank;
    uint32_t nodeid;
    struct gzll_endpoint_table *endpoints;
};

struct gzll_app_taskdir {
    optimsoc_mutex_t lock;
    unsigned int size;
    struct gzll_app_node *tasks;
};

#define TASKDIR_INVALID_NODEID 0xffffffff
#define TASKDIR_INVALID_RANK   0xffff
/**
 * Initialize
 */
void taskdir_initialize(struct gzll_app_taskdir *dir);

/**
 * Delete an task
 * @param taskid the ID of the task
 * @return error code
 */
int taskdir_task_delete(struct gzll_app_taskdir *dir, uint32_t taskid);

/**
 * Register a new remote task
 * @param taskid the ID of the task
 * @return error code
 */
int taskdir_task_register(struct gzll_app_taskdir *dir, uint32_t taskid,
                          const char* identifier, uint32_t rank,
                          uint32_t nodeid);

/**
 * Remap a task to a new tile
 *
 */
int taskdir_task_remap(struct gzll_app_taskdir *dir, uint32_t taskid,
                       uint32_t old_rank, uint32_t new_rank,
                       uint32_t old_nodeid, uint32_t new_nodeid);

/**
 * Query the location of the task
 * @param taskid the taskid of the task
 * @param *tileid a pointer to return the tile ID
 * @return error code
 */
int taskdir_mapping_lookup(struct gzll_app_taskdir *dir, uint32_t taskid,
                           uint32_t *rankid, uint32_t *nodeid);

/**
 * Do a lookup of the node id for the task identifier
 */
int taskdir_taskid_lookup(struct gzll_app_taskdir *dir, const char* identifier,
                          uint32_t *nodeid);

#endif //__TASKDIR_H__
