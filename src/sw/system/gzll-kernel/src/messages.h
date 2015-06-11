#ifndef SRC_MESSAGES_H_
#define SRC_MESSAGES_H_

void message_send_node_new(uint32_t appid, uint32_t app_nodeid, uint32_t nodeid,
                           const char *nodename);
void message_send_node_migrate(uint32_t appid, uint32_t taskid,
                                 uint32_t curr_rank, uint32_t new_rank);
void message_send_node_fetch(uint32_t dest_rank, void *node_addr);


#define GZLL_NUM_MESSAGE_TYPES 3

enum gzll_message_types {
    GZLL_NODE_NEW,
    GZLL_NODE_MIGRATE,
    GZLL_NODE_FETCH
};


struct gzll_message {
    uint32_t type;
    uint32_t source_rank;
    uint32_t len;
    uint8_t data[0];
};

struct gzll_message_node_new {
    uint32_t app_id;
    uint32_t app_nodeid;
    uint32_t rank_nodeid;
    char     app_nodename[0];
};

struct gzll_message_node_migrate {
    uint32_t app_id;
    uint32_t node_id;
    uint32_t dest;
};

struct gzll_message_node_fetch {
    void* node_addr;
};

void gzll_message_node_new_handler(struct gzll_message *msg);
void gzll_message_node_migrate_handler(struct gzll_message *msg);
void gzll_message_node_fetch_handler(struct gzll_message *msg);


#endif /* SRC_MESSAGES_H_ */
