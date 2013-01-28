/*
 * Copyright (c) 2011, The Multicore Association All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * mtapi_impl_spec.h
 *
 * Version 0.015,  October 2012
 *
 * Note: THE TYPE DEFINTIONS AND TYPES IN THIS FILE ARE REQUIRED.
 * THE SPECIFICE TYPES AND VALUES ARE IMPLEMENTATION DEFINED AS DESCRIBED
 * BELOW.
 * THE TYPES AND VALUES BELOW ARE SPECIFIC TO Poly-Messenger/MCAPI AND
 * INCLUDED ONLY TO EXEMPLIFY
 *
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MTAPI_IMPL_SPEC_H
#define MTAPI_IMPL_SPEC_H

#include <mca.h>
#include <mcapi.h>

/******************************************************************
           datatypes
******************************************************************/
typedef struct mtapi_task_hndl {
	void* task_descriptor;
} *mtapi_task_hndl_t;

typedef struct mtapi_action_hndl{
	void* action_descriptor;
} *mtapi_action_hndl_t;

typedef struct mtapi_queue_hndl{
	void* queue_descriptor;
} *mtapi_queue_hndl_t;

typedef struct mtapi_group_hndl{
	void* group_descriptor;
} *mtapi_group_hndl_t;

typedef struct {
  mca_boolean_t valid;
  unsigned int attribute_num;
  unsigned int bytes;
  void* attribute_d;
} attribute_entry_t;

typedef struct {
  attribute_entry_t entries[MAX_NUM_ATTRIBUTES];
} attributes_t;

typedef attributes_t mtapi_node_attributes_t;
typedef attributes_t mtapi_action_attributes_t;
typedef attributes_t mtapi_queue_attributes_t;
typedef attributes_t mtapi_task_attributes_t;
typedef attributes_t mtapi_group_attributes_t;

#endif /* MTAPI_IMPL_SPEC_H */

#ifdef __cplusplus
}
#endif /* __cplusplus */
