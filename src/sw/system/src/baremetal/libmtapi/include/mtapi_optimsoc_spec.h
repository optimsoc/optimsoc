/*
 * mtapi_optimsoc_spec.h
 *
 *  Created on: Oct 30, 2012
 *      Author: ga49qez
 */

#ifndef MTAPI_OPTIMSOC_SPEC_H_
#define MTAPI_OPTIMSOC_SPEC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mtapi_runtime.h"
#include "mtapi.h"

/******************************************************************
	constants and declarations
 ******************************************************************/

/******************************************************************
	definitions
 ******************************************************************/
#define MTAPI_CHECK_STATUS(where, status, exit) \
    if (status != MTAPI_SUCCESS) { \
      mtapiRT_status_error(&mtapi_display_status, status, where, exit); }

#define MCAPI_CHECK_STATUS(where, status, exit) \
    if (status != MCAPI_SUCCESS) { \
      mtapiRT_status_error(&mcapi_display_status, status, where, exit); }

/******************************************************************
	operations
 ******************************************************************/

#ifdef __cplusplus
extern }
#endif /* __cplusplus */

#endif /* MTAPI_OPTIMSOC_SPEC_H_ */
