/*
 * File     : dapi_utils.h
 * Created  : March 2017
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : Helper functions for Dante API
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
#ifndef _DAPI_UTILS_H
#define _DAPI_UTILS_H

#include "audinate/dante_api.h"
#include <stdio.h>
#include "dante/domain_handler.h"
#include "dante/dante_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

// Some cross-platfrom niceties
#ifdef WIN32

#define SNPRINTF _snprintf
#define STRCASECMP _stricmp

#else

#define SNPRINTF snprintf
#define STRCASECMP strcasecmp

#endif

	
#define DAPI_UTILS_STRINGIFY_(X) #X
#define DAPI_UTILS_STRINGIFY(X) DAPI_UTILS_STRINGIFY_(X)

/**
 * Perform a single select() call, using the sockets and timeouts provided by the dante_runtime.
 * Optionally include a single user socket in the select() as well.
 * If a user socket is passed in and is select()ed, it will be set in the out_sockets structure
 */
aud_error_t 
dapi_utils_step(dante_runtime_t * runtime, aud_socket_t in_sock, dante_sockets_t * out_sockets);

#ifdef WIN32

/**
 * Windows console quick-edit mode can cause the entires windows app to stall. Dante API applications are time-sensitive and 
 * their even loops need to be regularly processed. This function will check if quick edit mode is enabled, then optionally attempt 
 * to disable it, and then print a warning if it is still enabled.
 */
void dapi_utils_check_quick_edit_mode(aud_bool_t disable);

#endif

#ifdef __cplusplus
}
#endif

#endif

