#pragma once

#include <stdint.h>
#include <optimsoc-baremetal.h>


extern uint32_t _mp_trace_config;

enum {
	TRACE_MP_EP_CREATE_ENTER = 0x300,
	TRACE_MP_EP_CREATE_LEAVE = 0x301,
	TRACE_MP_EP_GET_ENTER = 0x302,
	TRACE_MP_EP_GET_LEAVE = 0x303,
	TRACE_MP_MSG_SEND_ENTER = 0x304,
	TRACE_MP_MSG_SEND_LEAVE = 0x305,

	TRACE_MP_LL_EP_CREATE_ENTER = 0x320,
	TRACE_MP_LL_EP_CREATE_LEAVE = 0x321,
	TRACE_MP_LL_EP_DESTROY_ENTER = 0x322,
	TRACE_MP_LL_EP_DESTROY_LEAVE = 0x323,

	TRACE_MP_LL_MGMT_REGISTER_ENTER = 0x330,
	TRACE_MP_LL_MGMT_REGISTER_LEAVE = 0x331,
	TRACE_MP_LL_MGMT_GET_ENTER = 0x332,
	TRACE_MP_LL_MGMT_GET_LEAVE = 0x333,

	TRACE_MP_LL_CTRL_GETEP_ENTER = 0x340,
	TRACE_MP_LL_CTRL_GETEP_LEAVE = 0x341,
	TRACE_MP_LL_CTRL_ALLOC_ENTER = 0x342,
	TRACE_MP_LL_CTRL_ALLOC_LEAVE = 0x343,
	TRACE_MP_LL_CTRL_DATA_ENTER = 0x344,
	TRACE_MP_LL_CTRL_DATA_LEAVE = 0x345,
	TRACE_MP_LL_CTRL_FINAL_ENTER = 0x346,
	TRACE_MP_LL_CTRL_FINAL_LEAVE = 0x347,
	TRACE_MP_LL_CTRL_MSGHANDLER_ENTER = 0x348,
	TRACE_MP_LL_CTRL_MSGHANDLER_LEAVE = 0x349,

	// Protocol
	TRACE_PROTO_GETEP_REQ_SEND = 0x380,
	TRACE_PROTO_GETEP_REQ_RECV = 0x381,
	TRACE_PROTO_GETEP_RESP_SEND = 0x382,
	TRACE_PROTO_GETEP_RESP_RECV = 0x383,
	TRACE_PROTO_ALLOC_REQ_SEND = 0x384,
	TRACE_PROTO_ALLOC_REQ_RECV = 0x385,
	TRACE_PROTO_ALLOC_RESP_SEND = 0x386,
	TRACE_PROTO_ALLOC_RESP_RECV = 0x387,
	TRACE_PROTO_DATA_SEND   = 0x388,
	TRACE_PROTO_DATA_RECV   = 0x389,
	TRACE_PROTO_FINAL_SEND  = 0x38a,
	TRACE_PROTO_FINAL_RECV  = 0x38b,
};

///////////////////////////////////////////////////////////////////////
//
// API Function Calls

static inline void trace_mp_endpoint_create_enter(uint32_t domain,
		uint32_t node, uint32_t port, uint32_t buffer_size,
		size_t msg_size, uint32_t attr) {
	if (_mp_trace_config & TRACE_MP_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_EP_CREATE_ENTER, domain);
		OPTIMSOC_TRACE(TRACE_MP_EP_CREATE_ENTER, node);
		OPTIMSOC_TRACE(TRACE_MP_EP_CREATE_ENTER, port);
		OPTIMSOC_TRACE(TRACE_MP_EP_CREATE_ENTER, buffer_size);
		OPTIMSOC_TRACE(TRACE_MP_EP_CREATE_ENTER, msg_size);
		OPTIMSOC_TRACE(TRACE_MP_EP_CREATE_ENTER, attr);
	}
}

static inline void trace_mp_endpoint_create_leave(void* addr) {
	if (_mp_trace_config & TRACE_MP_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_EP_CREATE_LEAVE, addr);
	}
}

static inline void trace_mp_endpoint_get_enter(uint32_t domain,
		uint32_t node, uint32_t port) {
	if (_mp_trace_config & TRACE_MP_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_EP_GET_ENTER, domain);
		OPTIMSOC_TRACE(TRACE_MP_EP_GET_ENTER, node);
		OPTIMSOC_TRACE(TRACE_MP_EP_GET_ENTER, port);
	}
}

static inline void trace_mp_endpoint_get_leave(void* addr) {
	if (_mp_trace_config & TRACE_MP_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_EP_GET_LEAVE, addr);
	}
}

static inline void trace_mp_msg_send_enter(void* from, void* to,
		uint32_t *data, uint32_t size) {
	if (_mp_trace_config & TRACE_MP_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_MSG_SEND_ENTER, from);
		OPTIMSOC_TRACE(TRACE_MP_MSG_SEND_ENTER, to);
		OPTIMSOC_TRACE(TRACE_MP_MSG_SEND_ENTER, data);
		OPTIMSOC_TRACE(TRACE_MP_MSG_SEND_ENTER, size);
	}
}

static inline void trace_mp_msg_send_leave() {
	if (_mp_trace_config & TRACE_MP_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_MSG_SEND_LEAVE, 0);
	}
}


///////////////////////////////////////////////////////////////////////
//
// Low Level (internal) Function Calls

static inline void trace_mp_ll_ep_create_enter(uint32_t domain, uint32_t node, uint32_t port,
		uint32_t buffer_size, size_t msg_size, uint32_t attr) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_CREATE_ENTER, domain);
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_CREATE_ENTER, node);
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_CREATE_ENTER, port);
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_CREATE_ENTER, buffer_size);
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_CREATE_ENTER, msg_size);
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_CREATE_ENTER, attr);
	}
}

static inline void trace_mp_ll_ep_create_leave(void* addr) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_CREATE_LEAVE, addr);
	}
}

static inline void trace_mp_ll_ep_destroy_enter(void* addr) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_DESTROY_ENTER, addr);
	}
}

static inline void trace_mp_ll_ep_destroy_leave() {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_EP_DESTROY_LEAVE, 0);
	}
}

static inline void trace_mp_ll_mgmt_register_enter(void* ep) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_MGMT_REGISTER_ENTER, ep);
	}
}

static inline void trace_mp_ll_mgmt_register_leave() {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_MGMT_REGISTER_LEAVE, 0);
	}
}

static inline void trace_mp_ll_mgmt_get_enter(uint32_t domain,
		uint32_t node, uint32_t port, int local) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_MGMT_GET_ENTER, domain);
		OPTIMSOC_TRACE(TRACE_MP_LL_MGMT_GET_ENTER, node);
		OPTIMSOC_TRACE(TRACE_MP_LL_MGMT_GET_ENTER, port);
		OPTIMSOC_TRACE(TRACE_MP_LL_MGMT_GET_ENTER, local);
	}
}

static inline void trace_mp_ll_mgmt_get_leave(void* ep) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_MGMT_GET_LEAVE, ep);
	}
}

static inline void trace_mp_ll_ctrl_getep_enter(uint32_t tile, uint32_t domain,
		uint32_t node, uint32_t port) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_GETEP_ENTER, tile);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_GETEP_ENTER, domain);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_GETEP_ENTER, node);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_GETEP_ENTER, port);
	}
}

static inline void trace_mp_ll_ctrl_getep_leave(void* ep, uint32_t addr) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_GETEP_LEAVE, ep);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_GETEP_LEAVE, addr);
	}
}

static inline void trace_mp_ll_ctrl_alloc_enter(void *ep) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_ALLOC_ENTER, ep);
	}
}

static inline void trace_mp_ll_ctrl_alloc_leave(uint32_t addr, uint32_t index) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_ALLOC_LEAVE, addr);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_ALLOC_LEAVE, index);
	}
}

static inline void trace_mp_ll_ctrl_data_enter(void *ep, uint32_t address,
		uint32_t* buffer, uint32_t size) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_DATA_ENTER, ep);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_DATA_ENTER, address);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_DATA_ENTER, buffer);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_DATA_ENTER, size);
	}
}

static inline void trace_mp_ll_ctrl_data_leave() {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_DATA_LEAVE, 0);
	}
}

static inline void trace_mp_ll_ctrl_final_enter(void *ep, uint32_t index,
		uint32_t size) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_FINAL_ENTER, ep);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_FINAL_ENTER, index);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_FINAL_ENTER, size);
	}
}

static inline void trace_mp_ll_ctrl_final_leave() {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_FINAL_LEAVE, 0);
	}
}

static inline void trace_mp_ll_ctrl_msghandler_enter(uint32_t src, uint32_t req, uint32_t reqid) {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_MSGHANDLER_ENTER, src);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_MSGHANDLER_ENTER, req);
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_MSGHANDLER_ENTER, reqid);
	}
}

static inline void trace_mp_ll_ctrl_msghandler_leave() {
	if (_mp_trace_config & TRACE_MP_LOWLVL_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MP_LL_CTRL_MSGHANDLER_LEAVE, 0);
	}
}

///////////////////////////////////////////////////////////////////////
//
// Protocol Messages

static inline void trace_proto_getep_req_send(uint32_t tile, uint32_t domain,
		uint32_t node, uint32_t port) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
	    OPTIMSOC_TRACE(TRACE_PROTO_GETEP_REQ_SEND, tile);
	    OPTIMSOC_TRACE(TRACE_PROTO_GETEP_REQ_SEND, domain);
		OPTIMSOC_TRACE(TRACE_PROTO_GETEP_REQ_SEND, node);
		OPTIMSOC_TRACE(TRACE_PROTO_GETEP_REQ_SEND, port);
	}
}

static inline void trace_proto_getep_req_recv(uint32_t source, uint32_t domain,
		uint32_t node, uint32_t port) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
	    OPTIMSOC_TRACE(TRACE_PROTO_GETEP_REQ_RECV, source);
	    OPTIMSOC_TRACE(TRACE_PROTO_GETEP_REQ_RECV, domain);
		OPTIMSOC_TRACE(TRACE_PROTO_GETEP_REQ_RECV, node);
		OPTIMSOC_TRACE(TRACE_PROTO_GETEP_REQ_RECV, port);
	}
}

static inline void trace_proto_getep_resp_send(uint32_t src, uint32_t addr) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
		OPTIMSOC_TRACE(TRACE_PROTO_GETEP_RESP_SEND, src);
		OPTIMSOC_TRACE(TRACE_PROTO_GETEP_RESP_SEND, addr);
	}
}

static inline void trace_proto_getep_resp_recv(uint32_t addr) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
		OPTIMSOC_TRACE(TRACE_PROTO_GETEP_RESP_RECV, addr);
	}
}

static inline void trace_proto_alloc_req_send(uint32_t tile, uint32_t addr) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
	    OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_REQ_SEND, tile);
	    OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_REQ_SEND, addr);
	}
}

static inline void trace_proto_alloc_req_recv(uint32_t source, void* ep) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
	    OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_REQ_RECV, source);
	    OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_REQ_RECV, ep);
	}
}

static inline void trace_proto_alloc_resp_send(uint32_t ack, uint32_t addr,
		uint32_t index) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
		OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_RESP_SEND, ack);
		OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_RESP_SEND, addr);
		OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_RESP_SEND, index);
	}
}

static inline void trace_proto_alloc_resp_recv(uint32_t ack, uint32_t addr,
		uint32_t index) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
		OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_RESP_RECV, ack);
		OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_RESP_RECV, addr);
		OPTIMSOC_TRACE(TRACE_PROTO_ALLOC_RESP_RECV, index);
	}
}

static inline void trace_proto_data_send(void *ep, uint32_t addr,
		uint32_t sz) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
		OPTIMSOC_TRACE(TRACE_PROTO_DATA_SEND, ep);
		OPTIMSOC_TRACE(TRACE_PROTO_DATA_SEND, addr);
		OPTIMSOC_TRACE(TRACE_PROTO_DATA_SEND, sz);
	}
}

static inline void trace_proto_data_recv(uint32_t src, uint32_t* addr,
		uint32_t sz) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
		OPTIMSOC_TRACE(TRACE_PROTO_DATA_RECV, src);
		OPTIMSOC_TRACE(TRACE_PROTO_DATA_RECV, addr);
		OPTIMSOC_TRACE(TRACE_PROTO_DATA_RECV, sz);
	}
}

static inline void trace_proto_final_send(void *ep, uint32_t index,
		uint32_t sz) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
		OPTIMSOC_TRACE(TRACE_PROTO_FINAL_SEND, ep);
		OPTIMSOC_TRACE(TRACE_PROTO_FINAL_SEND, index);
		OPTIMSOC_TRACE(TRACE_PROTO_FINAL_SEND, sz);
	}
}

static inline void trace_proto_final_recv(uint32_t src, void* ep,
		uint32_t index, uint32_t sz) {
	if (_mp_trace_config & TRACE_MP_PROTO) {
		OPTIMSOC_TRACE(TRACE_PROTO_FINAL_RECV, src);
		OPTIMSOC_TRACE(TRACE_PROTO_FINAL_RECV, ep);
		OPTIMSOC_TRACE(TRACE_PROTO_FINAL_RECV, index);
		OPTIMSOC_TRACE(TRACE_PROTO_FINAL_RECV, sz);
	}
}
