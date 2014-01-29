
#ifndef MCAPI_SYSCALLS_H
#define MCAPI_SYSCALLS_H

#include "mcapi.h"

enum mcapi_syscall_id {
	MCAPI_TRANS_ENDPOINT_CREATE = 0,
	MCAPI_TRANS_ENDPOINT_EXISTS,
	MCAPI_TRANS_ENDPOINT_GET,
	MCAPI_TRANS_GET_DOMAIN_NUM,
	MCAPI_TRANS_GET_NODE_NUM,
	MCAPI_TRANS_INITIALIZE,
	MCAPI_TRANS_INITIALIZED,
	MCAPI_TRANS_NUM_ENDPOINTS,
	MCAPI_TRANS_MSG_AVAILABLE,
	MCAPI_TRANS_MSG_RECV,
	MCAPI_TRANS_MSG_SEND,
	MCAPI_TRANS_SET_NODE_NUM,
	MCAPI_TRANS_VALID_ENDPOINT,
	MCAPI_TRANS_VALID_ENDPOINTS,
	MCAPI_TRANS_VALID_NODE,
	MCAPI_TRANS_VALID_PORT
};


struct mcapi_trans_endpoint_create {
	mcapi_endpoint_t *endpoint;
	mcapi_uint_t port_num;
	mcapi_boolean_t anonymous;
	mcapi_boolean_t ret;
};

struct mcapi_trans_endpoint_exists {
	mcapi_domain_t domain_id;
	uint32_t port_num;
	mcapi_boolean_t ret;
};

struct mcapi_trans_endpoint_get {
	mcapi_endpoint_t *endpoint;
	mcapi_domain_t domain_id;
	mcapi_uint_t node_num;
	mcapi_uint_t port_num;
};

struct mcapi_trans_get_domain_num {
	mcapi_node_t *domain_num;
	mcapi_boolean_t ret;
};

struct mcapi_trans_get_node_num {
	mcapi_node_t *node_num;
	mcapi_boolean_t ret;
};

struct mcapi_trans_initialize {
	mca_domain_t domain_id;
	mca_node_t node_num;
	mcapi_node_attributes_t *node_attrs;
	mcapi_boolean_t ret;
};

struct mcapi_trans_initialized {
	mca_domain_t domain_id;
	mca_node_t node_id;
	mcapi_boolean_t ret;
};

struct mcapi_trans_num_endpoints {
	mcapi_uint32_t ret;
};

struct mcapi_trans_msg_available {
	mcapi_endpoint_t receive_endpoint;
	mcapi_uint_t ret;
};

struct mcapi_trans_msg_recv {
	mcapi_endpoint_t receive_endpoint;
	char *buffer;
	size_t buffer_size;
	size_t *received_size;
	mcapi_boolean_t ret;
};

struct mcapi_trans_msg_send {
	mcapi_endpoint_t send_endpoint;
	mcapi_endpoint_t receive_endpoint;
	char *buffer;
	size_t buffer_size;
	mcapi_boolean_t ret;
};

struct mcapi_trans_set_node_num {
	mcapi_node_t node_num;
	mcapi_boolean_t ret;
};

struct mcapi_trans_valid_endpoint {
	mcapi_endpoint_t endpoint;
	mcapi_boolean_t ret;
};

struct mcapi_trans_valid_endpoints {
	mcapi_endpoint_t endpoint1;
	mcapi_endpoint_t endpoint2;
	mcapi_boolean_t ret;
};

struct mcapi_trans_valid_node {
	mcapi_node_t node_num;
	mcapi_boolean_t ret;
};

struct mcapi_trans_valid_port {
	mcapi_uint_t port_num;
	mcapi_boolean_t ret;
};
#endif // MCAPI_SYSCALLS_H
