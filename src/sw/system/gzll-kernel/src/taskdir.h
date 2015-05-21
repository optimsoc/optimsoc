#ifndef __TASKDIR_H__
#define __TASKDIR_H__

//TODO define error codes & return values

extern uint32_t *taskdir_table[];

struct gzll_app_node {
    uint32_t rank;
    uint32_t nodeid;
};

struct gzll_app_taskdir {
    unsigned int size;
    struct gzll_app_node *tasks;
};

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
                          uint32_t tile, uint32_t node);

/**
 * Remap a task to a new tile
 *
 */
int taskdir_task_remap(struct gzll_app_taskdir *dir, uint32_t taskid,
                       uint32_t old_tileid, uint32_t new_tileid,
                       uint32_t old_nodeid, uint32_t new_nodeid);

/**
 * Query the location of the task
 * @param taskid the taskid of the task
 * @param *tileid a pointer to return the tile ID
 * @return error code
 */
int taskdir_tileid_lookup(struct gzll_app_taskdir *dir, uint32_t taskid,
                          uint32_t *tileid, uint32_t nodeid);

#endif //__TASKDIR_H__
