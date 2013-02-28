/*
 * fibonacci.h
 *
 *  Created on: Oct 24, 2012
 *      Author: ga49qez
 */

#ifndef FIBONACCI_H_
#define FIBONACCI_H_

#include "mtapi.h"
#include "mtapi_optimsoc_spec.h"

void fibonacciActionFunction (
	MTAPI_IN void* args,
	MTAPI_IN mtapi_size_t arg_size,
	MTAPI_OUT void* result_buffer,
	MTAPI_IN mtapi_size_t result_buffer_size,
	MTAPI_IN void* node_local_data,
	MTAPI_IN mtapi_size_t node_local_data_size,
	mtapi_task_context_t* task_context);

#endif /* FIBONACCI_H_ */
