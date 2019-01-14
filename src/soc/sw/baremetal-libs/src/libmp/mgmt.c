#include "mgmt.h"
#include "endpoint.h"
#include "control.h"
#include "trace.h"

#include <stdio.h>

optimsoc_mp_mgmt_type_t _mgmt_type;

// The endpoint handles are stored in a linked list with this element
struct ep_list_elem {
    struct ep_list_elem *next;
    struct endpoint_handle *handle;
};

// Pointers to the actual list
struct ep_list_elem *_eplist_first;
struct ep_list_elem *_eplist_last;

optimsoc_mp_result_t mgmt_init(optimsoc_mp_mgmt_type_t type) {
	if (type != OPTIMSOC_MP_MGMT_DOMAIN_IS_TILE) {
		return OPTIMSOC_MP_ERR_INVALID_PARAMETER;
	}

	_mgmt_type = type;
	_eplist_first = 0;
	_eplist_last = 0;

	return OPTIMSOC_MP_SUCCESS;
}

optimsoc_mp_result_t mgmt_register(struct endpoint_handle* ep) {
	trace_mp_ll_mgmt_register_enter(ep);

	struct ep_list_elem *e = malloc(sizeof(struct ep_list_elem));
	if (!e) {
		return OPTIMSOC_MP_ERR_NO_MEMORY;
	}
	e->handle = ep;
	e->next = 0;

	if (_eplist_first==0) {
		_eplist_first = e;
		_eplist_last = e;
	} else {
		_eplist_last->next = e;
		_eplist_last = e;
	}

	trace_mp_ll_mgmt_register_leave();

	return OPTIMSOC_MP_SUCCESS;
}

optimsoc_mp_result_t mgmt_get(struct endpoint_handle ** eph,
		uint32_t domain, uint32_t node, uint32_t port, int local) {
	trace_mp_ll_mgmt_get_enter(domain, node, port, local);

	*eph = 0;

    // Try to find endpoint in local database
    struct ep_list_elem *lit;
    for (lit = _eplist_first; lit != 0; lit = lit->next) {
        if ((lit->handle->domain == domain) &&
                (lit->handle->node == node) &&
                (lit->handle->port == port)) {
            *eph = lit->handle;
            break;
        }
    }

    if (*eph != 0) {
        trace_mp_ll_mgmt_get_leave(*eph);

        return OPTIMSOC_MP_SUCCESS;
    }

    if (local) {
    	return OPTIMSOC_MP_ERR_CANNOT_RESOLVE;
    }

    struct endpoint_remote *ep;
    uint32_t tile;
    optimsoc_mp_result_t ret;

    if (_mgmt_type == OPTIMSOC_MP_MGMT_DOMAIN_IS_TILE) {
    	tile = domain;
    } else {
    	return OPTIMSOC_MP_ERR_CANNOT_RESOLVE;
    }

    ret = control_get_endpoint(&ep, tile, domain, node, port);
    if (ret != OPTIMSOC_MP_SUCCESS) {
    	return ret;
    }

    *eph = calloc(1, sizeof(struct endpoint_handle));
    (*eph)->domain = domain;
    (*eph)->node = node;
    (*eph)->port = port;
    (*eph)->type = REMOTE;
    (*eph)->ptr.remote = ep;

    // It is okay if it doesn't get registered, not cached then and
    // we need another lookup the next time.
    mgmt_register(*eph);

    trace_mp_ll_mgmt_get_leave(*eph);

    return OPTIMSOC_MP_SUCCESS;
}

void mgmt_print_db() {
	puts("List of known endpoints:\n");

	struct ep_list_elem *lit;
    for (lit = _eplist_first; lit != 0; lit = lit->next) {
    	endpoint_print(lit->handle, "  ");
    }
}
