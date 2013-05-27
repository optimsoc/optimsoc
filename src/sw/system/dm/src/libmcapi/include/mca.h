/*
 * mca.h
 * Version 1.131, May 2010
*/

#ifndef MCA_H
#define MCA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mca_impl_spec.h"

/*
 * MCA type definitions
 */
typedef int    			mca_int_t;
typedef char 			mca_int8_t;
typedef short 			mca_int16_t;
typedef int 			mca_int32_t;
typedef long long 		mca_int64_t;
typedef unsigned int            mca_uint_t;
typedef unsigned char 	        mca_uint8_t;
typedef unsigned short 	        mca_uint16_t;
typedef unsigned int 		mca_uint32_t;
typedef unsigned long long 	mca_uint64_t;
typedef unsigned char		mca_boolean_t;
typedef unsigned int            mca_node_t;
typedef unsigned int		mca_status_t;
typedef unsigned int		mca_timeout_t;
typedef unsigned int		mca_domain_t;

/* Constants */
#define MCA_TRUE			  1
#define MCA_FALSE			  0
#define MCA_NULL			  0	/* MCA Zero value */
#define	MCA_INFINITE		        (~0)	/* Wait forever, no timeout */
#define MCA_RETURN_VALUE_INVALID        (~0) 
#define MCA_NODE_INVALID                (~0) 
#define MCA_DOMAIN_INVALID              (~0) 

/* In/out parameter indication macros */
#ifndef MCA_IN
#define MCA_IN const
#endif /* MCA_IN */

#ifndef MCA_OUT
#define MCA_OUT
#endif /* MCA_OUT */

/* Alignment macros */
#ifdef __GNUC__
#define MCA_DECL_ALIGNED __attribute__ ((aligned (32)))
#else
#define MCA_DECL_ALIGNED /* MCA_DECL_ALIGNED alignment macro currently only
							supports GNU compiler */
#endif /* __GNUC__ */

/*
 * MCA organization id's (for assignment of organization specific attribute numbers)
 */
#define MCA_ORG_ID_PSI 0 	/* PolyCore Software, Inc. */
#define MCA_ORG_ID_FSL 1 	/* Freescale, Inc. */
#define MCA_ORG_ID_MGC 2 	/* Mentor Graphics, Corp. */
#define MCA_ORG_ID_TBA 3 	/* To be assigned */
/* And so forth */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MCA_H */
