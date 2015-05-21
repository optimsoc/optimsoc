#ifndef __TASKDIR_H__
#define __TASKDIR_H__

//TODO define error codes & return values

extern uint32_t *taskdir_table[];

/**
 * Initialize
 */
void taskdir_initialize();

/**
 * Delete an task
 * @param taskid the ID of the task
 * @return error code
 */
int taskdir_task_delete(uint32_t taskid);

/**
 * Register a new remote task
 * @param taskid the ID of the task
 * @return error code
 */
int taskdir_task_register(uint32_t taskid, uint32_t tile);

/**
 * Remap a task to a new tile
 *
 */
int taskdir_task_remap(uint32_t taskid, uint32_t old_tileid, uint32_t new_tileid);

/**
 * Checks if an task ID is not locally assigned
 * @param taskid the ID of the task
 * @return error code
 */
int taskdir_taskid_available(uint32_t taskid);

/**
 * Reserve the task ID entry
 * @param taskid the ID of the task
 * @return error code
 */
int taskdir_taskid_reserve(uint32_t taskid);

/**
 * Query the location of the task
 * @param taskid the taskid of the task
 * @param *tileid a pointer to return the tile ID
 * @return error code
 */
int taskdir_tileid_lookup(uint32_t taskid, uint32_t *tileid);

#endif //__TASKDIR_H__
