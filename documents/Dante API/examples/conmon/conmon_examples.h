/*
 * File     : conmon_examples.h
 * Created  : July 2008
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : Common information for the conmon example clients
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
#ifndef _CONMON_EXAMPLES_H
#define _CONMON_EXAMPLES_H

// include the main API header
#include "audinate/dante_api.h"
#include "conmon_aud_print_msg.h"
#include "conmon_aud_print_control_msg.h"

#include <stdio.h>

// Some cross-platfrom niceties
#ifdef WIN32

#define SNPRINTF _snprintf
#define STRCASECMP _stricmp

#else

#define SNPRINTF snprintf
#define STRCASECMP strcasecmp

#endif


#ifndef MAX
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))
#endif

#define ONE_SECOND_US 1000000

#if 1
//----------------------------------------------------------
// Event loop infrastructure
//----------------------------------------------------------

/*
	For the conmon examples, we set up a little helper to handle socket processing
	and timeout.

	This code handles:
		- maintaining the sockets
		- 'select' and 'process'
		- updating the sockets when they change

	We store the event loop handler in static memory so that we can get to it without
	pulling it out of the client context.  This works for small examples with per-
	process memory.  Real code should usually carry a reference to the loop info
	inside the application context.
 */

/*
	General structure:

	conmon_examples_event_loop_init(client);
		// call this once

	while(! loop_done_condition())
	{
		result = conmon_examples_event_loop_run_once(&k_timeout);
		// handle result
	}
 */


/*
	Initialise the event loop info

	This code sets up the sockets changed callback internally.
 */
aud_error_t
conmon_examples_event_loop_init
(
	conmon_client_t * client
);


/*
	Run the event loop once, waiting for sockets or timeout.

	Returns:
		AUD_SUCCESS if sockets were processed
		AUD_ERR_TIMEDOUT if the timeout expired
			NOTE: This means the select call timed out.  The ConMon client
			may set the select timeout to be less than the input timeout.
		AUD_ERR_INTERRUPTED if the select call was interrupted
			This is usually not an error.
		Other error code on error

	See conmon_examples_event_loop_failed to quickly filter between 'error'
	and 'non-error' return values
 */
aud_error_t
conmon_examples_event_loop_run_once
(
	const aud_utime_t * timeout
);

/*
	As above, but take timeout in seconds.
 */
AUD_INLINE aud_error_t
conmon_examples_event_loop_run_once_seconds
(
	unsigned long timeout_s
)
{
	aud_utime_t timeout;
	timeout.tv_sec = timeout_s;
	timeout.tv_usec = 0;
	return conmon_examples_event_loop_run_once(&timeout);
}

/*
	As above, but express timeout as absolute time
 */
aud_error_t
conmon_examples_event_loop_run_until
(
	const aud_utime_t * timeout
);


/*
	Helper function to filter 'failure' and 'normal' results from event_loop_run
 */
AUD_INLINE aud_bool_t
conmon_examples_event_loop_failed
(
	aud_error_t result
)
{
	switch(result)
	{
	case AUD_SUCCESS:
		// got some sockets
	case AUD_ERR_TIMEDOUT:
		// timed out before getting sockets
	case AUD_ERR_INTERRUPTED:
		// non-error event caused loop to exit early
		return AUD_FALSE;
	default:
		// something went wrong and trying again won't fix it
		return AUD_TRUE;
	}
}


// Adding user sockets to loop

/*
	Get a reference to the user sockets for the event loop info.
	Add any non-conmon sockets to this set to have them automatically added
	to the event loop as required.
	Remove them if they go away.
 */
dante_sockets_t *
conmon_examples_event_loop_get_user_sockets(void);

/*
	Notify that the sockets have changed.
	This is usually user sockets, but should also be called if an action outside of
	'process' could change socket state (eg connect or disconnect).
	NOTE: the event loop is initialised 'changed', so it is not necessary to call this
	before the first run_once call.
 */
void
conmon_examples_event_loop_set_sockets_changed(void);


/*
	Get reference to the sockets that were active after the last run.

	Returns
		Pointer to active sockets.
		NULL if no sockets were active.
 */
dante_sockets_t *
conmon_examples_event_loop_get_active_sockets(void);
#endif

//----------------------------------------------------------
// Support functions for the example clients
//----------------------------------------------------------

aud_error_t
conmon_example_sleep
(
	const aud_utime_t * at
);

aud_error_t
conmon_example_client_process
(
	conmon_client_t * client,
	dante_sockets_t * sockets,
	const aud_utime_t * timeout,
	aud_utime_t * next_action_timeout
);


//----------------------------------------------------------
// Printing functions for the example clients
//----------------------------------------------------------

const char *
conmon_example_client_state_to_string
(
	conmon_client_state_t state
);

const char *
conmon_example_channel_type_to_string
(
	conmon_channel_type_t channel_type
);

const char *
conmon_example_rxstatus_to_string
(
	conmon_rxstatus_t rxstatus
);

// A helper function to print a device id to a string buffer
char *
conmon_example_device_id_to_string
(
	const conmon_device_id_t * id,
	char * buf,
	size_t len
);

// A helper function to print an instance id to a string buffer
char *
conmon_example_instance_id_to_string
(
	const conmon_instance_id_t * id,
	char * buf,
	size_t len
);

// A helper function to print a vendor id to a string buffer
char *
conmon_example_vendor_id_to_string
(
	const conmon_vendor_id_t * id,
	char * buf,
	size_t len
);

// A helper function to print a vendor id to a string buffer
char *
conmon_example_model_id_to_string
(
	const conmon_audinate_model_id_t * id,
	char * buf,
	size_t len
);

// A helper function to print a set of endpoint addresses to a string buffer
char *
conmon_example_endpoint_addresses_to_string
(
	const conmon_endpoint_addresses_t * addresses,
	char * buf,
	size_t len
);

// A helper function to print a set of endpoint addresses to a string buffer
char *
conmon_example_networks_to_string
(
	const conmon_networks_t * networks,
	char * buf,
	size_t len
);

char *
conmon_example_subscription_infos_to_string
(
	uint16_t num_subscriptions,
	const conmon_subscription_info_t ** subscriptions,
	char * buf,
	size_t len
);

char *
conmon_example_client_subscriptions_to_string
(
	uint16_t num_subscriptions,
	const conmon_client_subscription_t * const * subscriptions,
	char * buf,
	size_t len
);

char *
conmon_example_metering_peaks_to_string
(
	const conmon_metering_message_peak_t * peaks,
	uint16_t num_peaks,
	char * buf,
	size_t len
);

char *
conmon_example_event_to_string
(
	const conmon_client_event_t * ev,
	char * buf,
	size_t len
);

#endif
