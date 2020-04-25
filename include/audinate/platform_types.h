/*
 * File     : $RCSfile$
 * Created  : March 2007
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Basic types in the API
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file platform_types.h
 *
 * Standard cross-platform type definitions used throughout the Dante API.
 *
 * Do not include this file directly, instead include "audinate/dante_api.h"
 */
#ifndef _AUD_PLATFORM_TYPES_H
#define _AUD_PLATFORM_TYPES_H

#ifndef AUD_PLATFORM
#error platform_types.h should never be included directly
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** A cross-platform boolean type definition */
typedef unsigned int aud_bool_t;

/** A cross-platform definition for FALSE */
#define AUD_FALSE 0

/** A cross-platform definition for TRUE */
#define AUD_TRUE 1

#ifndef NO
/** A synonym for AUD_FALSE */
#define NO AUD_FALSE
#endif

#ifndef YES
/** A synonym for AUD_TRUE */
#define YES AUD_TRUE
#endif

#ifndef INT32_MAX
/** The maximum possible value for a signed 32-bit value */
#define INT32_MAX 0xFFFFFFFF
#endif

#ifndef UINT32_MAX
/** The maximum possible value for an unsigned 32-bit value */
#define UINT32_MAX 0xFFFFFFFF
#endif

/**
 * Mark unused parameter in functions, useful for removing compiler warnings
 * when parameters are known to be not needed.
 */
#ifndef AUD_UNUSED
#define AUD_UNUSED(PARAM) (void) PARAM;
#endif


// Define common MAX and MIN macros if not already defined.

#ifndef MAX
/** Define MAX if we don't already have it */
#define MAX(x,y) (((x) >= (y)) ? (x) : (y))
#endif

#ifndef MIN
/** Define MIN if we don't already have it */
#define MIN(x,y) (((x) <= (y)) ? (x) : (y))
#endif

//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
