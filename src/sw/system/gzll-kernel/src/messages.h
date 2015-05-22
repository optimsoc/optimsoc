#ifndef SRC_MESSAGES_H_
#define SRC_MESSAGES_H_

void message_send_node_new(uint32_t appid, uint32_t app_nodeid, uint32_t nodeid,
                           const char *nodename);

#define GZLL_NUM_MESSAGE_TYPES 1

enum gzll_message_types {
    GZLL_NODE_NEW
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

void gzll_message_node_new_handler(struct gzll_message *msg);

#endif /* SRC_MESSAGES_H_ */
