/*
Copyright (c) 2010, The Multicore Association
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

(1) Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
(2) Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution. 

(3) Neither the name of the Multicore Association nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef RESOURCE_TREE_H
#define RESOURCE_TREE_H

#include <mrapi.h>

#ifdef MRAPI_HAVE_INTTYPES_H
#include <stdint.h>
#endif

typedef enum {
  RSRC_UINT16_T,
  RSRC_UINT32_T,
} rsrc_type_t;

/******************************************************************
  An example resource tree, consisting of two cores, a cross bar switch,
  an L3 cache, and DDR memory.  Each leaf node has a set of simple
  attributes.
 ******************************************************************/
/* DDR attributes */
uint16_t ddr_size = 1024;  /* MB */
uint32_t baseaddr = 0xfffff000;
void *ddr_attr[2] = {&ddr_size, &baseaddr};
rsrc_type_t ddr_type_1 = RSRC_UINT16_T;
rsrc_type_t ddr_type_2 = RSRC_UINT32_T;
void *ddr_types[2] = {&ddr_type_1, &ddr_type_2};
mrapi_attribute_static ddr_static_1 = MRAPI_ATTR_STATIC;
mrapi_attribute_static ddr_static_2 = MRAPI_ATTR_STATIC;
mrapi_attribute_static *ddr_static[2] = {&ddr_static_1, &ddr_static_2};
mrapi_boolean_t ddr_start_1 = MRAPI_FALSE;
mrapi_boolean_t ddr_start_2 = MRAPI_FALSE;
mrapi_boolean_t *ddr_start[2] = {&ddr_start_1, &ddr_start_2};

/* L3 cache attributes */
uint16_t l3cache_size = 1024; /* KB */
uint32_t l3cache_hits = 0;
void *l3cache_attr[2] = {&l3cache_size, &l3cache_hits};
rsrc_type_t l3cache_type_1 = RSRC_UINT16_T;
rsrc_type_t l3cache_type_2 = RSRC_UINT32_T;
void *l3cache_types[2] = {&l3cache_type_1, &l3cache_type_2};
mrapi_attribute_static l3cache_static_1 = MRAPI_ATTR_STATIC;
mrapi_attribute_static l3cache_static_2 = MRAPI_ATTR_DYNAMIC;
mrapi_attribute_static *l3cache_static[2] = {&l3cache_static_1, &l3cache_static_2};
mrapi_boolean_t l3cache_start_1 = MRAPI_FALSE;
mrapi_boolean_t l3cache_start_2 = MRAPI_FALSE;
mrapi_boolean_t *l3cache_start[2] = {&l3cache_start_1, &l3cache_start_2};

/* Cores 0 and 1 attributes */
uint32_t core0_id = 200000001;
uint16_t core0_L1_size = 32;  /* KB */
uint16_t core0_L2_size = 128; /* KB */
void *core0_attr[3] = {&core0_id, &core0_L1_size, &core0_L2_size};
rsrc_type_t core0_type_1 = RSRC_UINT32_T;
rsrc_type_t core0_type_2 = RSRC_UINT16_T;
rsrc_type_t core0_type_3 = RSRC_UINT16_T;
void *core0_types[3] = {&core0_type_1, &core0_type_2, &core0_type_3};
mrapi_attribute_static core0_static_1 = MRAPI_ATTR_STATIC;
mrapi_attribute_static core0_static_2 = MRAPI_ATTR_STATIC;
mrapi_attribute_static core0_static_3 = MRAPI_ATTR_STATIC;
mrapi_attribute_static *core0_static[3] = {&core0_static_1, &core0_static_2, &core0_static_3};
mrapi_boolean_t core0_start_1 = MRAPI_FALSE;
mrapi_boolean_t core0_start_2 = MRAPI_FALSE;
mrapi_boolean_t core0_start_3 = MRAPI_FALSE;
mrapi_boolean_t *core0_start[3] = {&core0_start_1, &core0_start_2, &core0_start_3};

uint32_t core1_id = 400000002; //FIXME: compile warning
uint16_t core1_L1_size = 64;  /* KB */
uint16_t core1_L2_size = 256;  /* KB */
void *core1_attr[3] = {&core1_id, &core1_L1_size, &core1_L2_size};
rsrc_type_t core1_type_1 = RSRC_UINT32_T;
rsrc_type_t core1_type_2 = RSRC_UINT16_T;
rsrc_type_t core1_type_3 = RSRC_UINT16_T;
void *core1_types[3] = {&core1_type_1, &core1_type_2, &core1_type_3};
mrapi_attribute_static core1_static_1 = MRAPI_ATTR_STATIC;
mrapi_attribute_static core1_static_2 = MRAPI_ATTR_STATIC;
mrapi_attribute_static core1_static_3 = MRAPI_ATTR_STATIC;
mrapi_attribute_static *core1_static[3] = {&core1_static_1, &core1_static_2, &core1_static_3};
mrapi_boolean_t core1_start_1 = MRAPI_FALSE;
mrapi_boolean_t core1_start_2 = MRAPI_FALSE;
mrapi_boolean_t core1_start_3 = MRAPI_FALSE;
mrapi_boolean_t *core1_start[3] = {&core1_start_1, &core1_start_2, &core1_start_3};

/* Crossbar attributes */
uint16_t crossbar_number_ports = 8;
void *crossbar_attr[1] = {&crossbar_number_ports};
rsrc_type_t crossbar_type_1 = RSRC_UINT16_T;
void *crossbar_types[1] = {&crossbar_type_1};
mrapi_attribute_static crossbar_static_1 = MRAPI_ATTR_STATIC;
mrapi_attribute_static *crossbar_static[1] = {&crossbar_static_1};
mrapi_boolean_t crossbar_start_1 = MRAPI_FALSE;
mrapi_boolean_t *crossbar_start[1] = {&crossbar_start_1};

/* Nodes */
mrapi_resource_t core0 = {"Core 0", CPU, 0, NULL, 3, core0_types, core0_attr, core0_static, core0_start};
mrapi_resource_t core1 = {"Core 1", CPU, 0, NULL, 3, core0_types, core1_attr, core1_static, core1_start};
mrapi_resource_t *core_complex_children[2] = {&core0, &core1};
mrapi_resource_t core_complex = {"Core complex", CORE_COMPLEX, 2, core_complex_children, 0, NULL, NULL, NULL, NULL};
mrapi_resource_t crossbar = {"Crossbar", CROSSBAR, 0, NULL, 1, crossbar_types, crossbar_attr, crossbar_static, crossbar_start};
mrapi_resource_t l3cache = {"L3 Cache", CACHE, 0, NULL, 2, l3cache_types, l3cache_attr, l3cache_static, l3cache_start};
mrapi_resource_t ddr = {"DDR", MEM, 0, NULL, 2, ddr_types, ddr_attr, ddr_static, ddr_start};
mrapi_resource_t *chip_children[4] = {&core_complex, &crossbar, &l3cache, &ddr};
mrapi_resource_t chip = {"Chip", SYSTEM, 4, chip_children, 0, NULL, NULL, NULL, NULL};

#endif

#ifdef __cplusplus
extern } 
#endif /* __cplusplus */
