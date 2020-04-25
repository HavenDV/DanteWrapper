/*
 * File     : dante_runtime.h
 * Created  : May 2015
 * Author   : Andrew White
 * Synopsis : Hooks to allow Dante libraries synchronous processing time
 *
 * This software is copyright (c) 2015-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */
/**
 * @file dante_runtime.h
 * Hooks to allow Dante libraries synchronous processing time
 */

#ifndef _DANTE_RUNTIME_H
#define _DANTE_RUNTIME_H

#include "dante_common.h"

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------
// Dante Process data type

typedef struct dante_runtime dante_runtime_t;

// Runtime

/**
	Indicates whether the list of sockets changed since the last call
	to dante_runtime_get_sockets_and_timeout.
 */
aud_bool_t
dante_runtime_sockets_have_changed
(
	const dante_runtime_t *
);

/**
	Get the current sockets and timeout.

	If sockets is non-null, adds the currently active sockets to the file
	descriptors in the dante_sockets_t structure.

	@param runtime dante runtime object
	@param sockets initialised dante_sockets_t data structure.  Active sockets
		to select on will be added to those listed here.
		If NULL, then only the timeout will be updated.
	@param max_timeout maximum time that should pass before 'process' is next called.
 */
aud_error_t
dante_runtime_get_sockets_and_timeout
(
	dante_runtime_t * runtime,
	dante_sockets_t * sockets,	// [IN/OUT]
	aud_utime_t * max_timeout	// [OUT]
);


/**
	Once the select loop has fired, process the given sockets
	and non-socket events
	
	@param runtime dante runtime object
	@param sockets input dante_sockets_t provided for processing
 */
aud_error_t
dante_runtime_process_with_sockets
(
	dante_runtime_t * runtime,
	dante_sockets_t * sockets	// [IN]
);

/**
	Once the select loop has fired, process non-socket events

	@param runtime dante runtime object
*/
aud_error_t
dante_runtime_process
(
	dante_runtime_t * runtime
);


//----------------------------------------------------------
// Async notification

typedef void
dante_runtime_async_change_fn
(
	const dante_runtime_t * runtime,
	void * context
);

/**
	Set a callback to catch asynchronous change events.

	Windows and OS X programs often perform an asynchronous socket wait, to avoid
	calling a blocking select in their main processing thread.  This opens the
	possibility that the set of sockets or the timeout can change while the code
	is blocked waiting on sockets.  Clients can set and catch this callback to
	allow them to wake a blocked select.

	@return AUD_ERR_NOTSUPPORTED if asynchronous change notifications are not supported on the given platform.
	
 */
aud_error_t
dante_runtime_set_async_change_event
(
	dante_runtime_t * runtime,
	dante_runtime_async_change_fn * cb,
	void * context
);

/**
 * Enable or disable change notifications for the dante runtime.
 * Notifications will only fire once enabled.
 */
aud_error_t
dante_runtime_enable_async_change_event
(
	dante_runtime_t * runtime,
	aud_bool_t enable
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

