#include "task.h"
#include "taskdir.h"

struct gzll_app {
    struct gzll_task_dir *task_dir;
};

void gzll_app_new(uint32_t id, const char* name);
struct gzll_task_dir *gzll_app_get_taskdir(struct gzll_app *app);
