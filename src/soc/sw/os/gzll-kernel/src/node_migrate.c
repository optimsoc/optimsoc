#include <gzll.h>

#include "app.h"
#include "messages.h"
#include "taskdir.h"

int gzll_node_migrate(uint32_t appid, uint32_t nodeid, uint32_t dest_rank)
{

    uint32_t curr_rank;
    uint32_t node_id;

    struct gzll_app *app;
    app = gzll_app_get(appid);

    if (app != NULL) {
        if (taskdir_mapping_lookup(app->task_dir, nodeid, &curr_rank, &node_id)
            == 0) {

            if (curr_rank != gzll_rank) {
                message_send_node_migrate(appid, nodeid, curr_rank, dest_rank);
                return 0;
            } else {
                struct gzll_node *node = gzll_node_find(node_id);
                assert(node != NULL);

                gzll_node_suspend(node);

                message_send_node_fetch(dest_rank, node);
                return 0;
            }
        }
    }

    return -1;
}
