#include "taskdir.h"

#include <assert.h>

void taskdir_initialize(struct gzll_app_taskdir *dir) {

    optimsoc_mutex_init(&dir->lock);

    dir->size = 1;
    dir->tasks = NULL;
}

int taskdir_task_delete(struct gzll_app_taskdir *dir, uint32_t taskid) {

    optimsoc_mutex_lock(&dir->lock);

    if(taskid < dir->size) {

	struct gzll_app_node *entry = dir->tasks + taskid;

	if(entry->rank != TASKDIR_INVALID_RANK
	   && entry->nodeid != TASKDIR_INVALID_NODEID) {

	    /* there is a valid entry for this task id - delete it */

	    entry->rank = TASKDIR_INVALID_RANK;
	    entry->nodeid = TASKDIR_INVALID_NODEID;

	    optimsoc_mutex_unlock(&dir->lock);
	    return 0; /*success*/
	}

    }

    optimsoc_mutex_unlock(&dir->lock);
    return -1; /*failed*/
}

int taskdir_task_register(struct gzll_app_taskdir *dir, uint32_t taskid,
                          uint32_t rank, uint32_t nodeid) {

    optimsoc_mutex_lock(&dir->lock);
    struct gzll_app_node *entry;

    if(taskid >= dir->size) {
	/* table must be expanded */

	uint32_t new_size = taskid + 1;

	dir->tasks = realloc(dir->tasks, new_size);
	assert(dir->tasks != NULL);

	/* invalidate unused spare entries between the current last entry and the new end*/
	for(entry = dir->tasks + dir->size; entry < dir->tasks + new_size; ++entry) {
	    entry->rank = TASKDIR_INVALID_RANK;
	    entry->nodeid = TASKDIR_INVALID_NODEID;
	}
    }

    entry = dir->tasks + taskid;

    /* entry must be unused */
    if(entry->rank == TASKDIR_INVALID_RANK && entry->nodeid == TASKDIR_INVALID_NODEID) {

	entry->rank = rank;
	entry->nodeid = nodeid;

	optimsoc_mutex_unlock(&dir->lock);

	return 0; /*success*/
    } else {

	optimsoc_mutex_unlock(&dir->lock);

	return -1; /*failed*/
    }

}

int taskdir_task_remap(struct gzll_app_taskdir *dir, uint32_t taskid,
                       uint32_t old_rank, uint32_t new_rank,
                       uint32_t old_nodeid, uint32_t new_nodeid) {

    optimsoc_mutex_lock(&dir->lock);

    if(taskid < dir->size) {

	struct gzll_app_node *entry = dir->tasks + taskid;

	if(entry->rank == old_rank
	   && entry->nodeid == old_nodeid) {

	    /* there is a valid entry for this task id - delete it */

	    entry->rank = new_rank;
	    entry->nodeid = new_nodeid;

	    optimsoc_mutex_unlock(&dir->lock);
	    return 0; /*success*/
	}

    }

    optimsoc_mutex_unlock(&dir->lock);
    return -1; /*failed*/

}


int taskdir_rank_lookup(struct gzll_app_taskdir *dir, uint32_t taskid,
			uint32_t *tileid) {

    assert(tileid != NULL);

    optimsoc_mutex_lock(&dir->lock);

    if(taskid < dir->size) {

	struct gzll_app_node *entry = dir->tasks + taskid;

	if(entry->rank != TASKDIR_INVALID_RANK
	   && entry->nodeid != TASKDIR_INVALID_NODEID) {

	    /* there is a valid entry for this task id - delete it */

	    *tileid = entry->rank;
	    optimsoc_mutex_unlock(&dir->lock);
	    return 0; /*success*/
	}

    }

    optimsoc_mutex_unlock(&dir->lock);
    return -1; /*failed*/

}