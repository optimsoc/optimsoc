#pragma once

#include <stdint.h>
#include "include/optimsoc-baremetal.h"

extern uint32_t _optimsoc_trace_config;

enum {
	TRACE_MPSIMPLE_IRQHANDLER_ENTER = 0x100,
	TRACE_MPSIMPLE_IRQHANDLER_LEAVE = 0x101,
	TRACE_MPSIMPLE_SEND_ENTER = 0x102,
	TRACE_MPSIMPLE_SEND_LEAVE = 0x103,
	TRACE_MPSIMPLE_CTREADY_ENTER = 0x104,
	TRACE_MPSIMPLE_CTREADY_LEAVE = 0x105,
};

static inline void trace_mpsimple_irqhandler_enter() {
	if (_optimsoc_trace_config & TRACE_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_IRQHANDLER_ENTER, 0);
	}
}

static inline void trace_mpsimple_irqhandler_leave() {
	if (_optimsoc_trace_config & TRACE_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_IRQHANDLER_LEAVE, 0);
	}
}

static inline void trace_mpsimple_send_enter(uint32_t tile, uint32_t size,
		void* addr) {
	if (_optimsoc_trace_config & TRACE_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_SEND_ENTER, tile);
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_SEND_ENTER, size);
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_SEND_ENTER, addr);
	}
}

static inline void trace_mpsimple_send_leave() {
	if (_optimsoc_trace_config & TRACE_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_SEND_LEAVE, 0);
	}
}

static inline void trace_mpsimple_ctready_enter(uint32_t rank, uint32_t endpoint) {
	if (_optimsoc_trace_config & TRACE_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_CTREADY_ENTER, rank);
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_CTREADY_ENTER, endpoint);
	}
}

static inline void trace_mpsimple_ctready_leave(uint32_t result) {
	if (_optimsoc_trace_config & TRACE_FUNC_CALLS) {
		OPTIMSOC_TRACE(TRACE_MPSIMPLE_CTREADY_LEAVE, result);
	}
}
