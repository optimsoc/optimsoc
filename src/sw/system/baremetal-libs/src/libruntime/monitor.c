#include "monitor.h"
#include "communication.h"
#include "runtime.h"

#include "trace.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

uint32_t *monitor_load_array;

uint32_t load_history[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint32_t load_current;

unsigned int update_counter;

const unsigned int update_counter_interval = 1;

void monitor_init() {
    monitor_load_array = malloc(sizeof(uint32_t) * runtime_get_num_instances());
    update_counter = 0;
    load_current = 0;
}


uint32_t *monitor_get_load() {
    return monitor_load_array;
}

void monitor_receive_data(uint32_t source, uint32_t *message, uint32_t len) {
    // This function must return and itself not send a message!
    assert(source < runtime_get_num_instances());
    monitor_load_array[source] = message[0];
}

void monitor_schedule_run(uint32_t runnable) {
    load_current -= load_history[15];

    // Determine load
    for (int l = 15; l > 0; l--) {
        load_history[l] = load_history[l-1];
    }

    load_history[0] = runnable;

    load_current += load_history[0];

    runtime_trace_load(load_current);

    monitor_load_array[runtime_get_instance_id()] = load_current;

/*    if (scheduler_needs_rebalance() != 0) {
        scheduler_rebalance();
    }*/

    update_counter++;

    if (update_counter == update_counter_interval) {
        if (communication_ready()) {
            for (int t = 0; t < runtime_get_num_instances(); t++) {
                if (t != runtime_get_instance_id()) {
                    printf("Send load to %u\n", t);
                    communication_send(t, SYSTEM_MESSAGE_MONITOR, &load_current, 4);
                }
            }
        }
        update_counter = 0;
    }

}
