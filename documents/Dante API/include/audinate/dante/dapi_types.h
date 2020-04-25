/*
 * File     : dapi_types.h
 * Created  : May 2017
 * Author   : James Westendorp
 * Synopsis : Forward definitions of some key dapi types
 *
 * This software is copyright (c) 2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */

/**
 * @file dapi_types.h
 * Forward definitions of some key dapi types
 */

#ifndef _DANTE_DAPI_TYPES_H
#define _DANTE_DAPI_TYPES_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/dante_common.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Forward definitions for DAPI
//----------------------------------------------------------

#define DAPI_ENVIRONMENT__UNKNOWN    0
#define DAPI_ENVIRONMENT__EMBEDDED   1
#define DAPI_ENVIRONMENT__STANDALONE 2
#define DAPI_ENVIRONMENT__INTERNAL   3

typedef unsigned int dapi_environment_t;

const char * dapi_environment_to_string(dapi_environment_t dapi_env);

/**
 * A dapi_t structure represents the overall environment for a dapi-enabled application
 */
typedef struct dapi dapi_t;

/**
 * Get audinate specific environment for a dapi environment
 */
aud_env_t * dapi_get_env(dapi_t * dapi);

/**
 * Get the dapi environment (if it exists) that owns the given audinate specific environment
 */
dapi_t * aud_env_get_dapi(aud_env_t * env);

#ifdef __cplusplus
}
#endif

#endif
