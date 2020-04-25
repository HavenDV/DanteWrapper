/*
 * File     : dapi_routing_monitor.c
 * Created  : March 2016
 * Author   : Andrew White
 * Synopsis : Sample code for monitoring subscriptions on a local or remote device
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */


//----------
// Overview

/*
Purpose:
	This sample application detects changes in the RX channel state on the
	device and updates its routing model

	- Connect to local ConMon
	- Connect to routing device
	- Listen for channel update events (via ConMon) 
	- Update routing model

Key differences between local & remote monitoring:

	- local often starts at boot time, so needs to wait for the local system
		to be up (i.e. Dante Ready)
	- remote needs name of the remote device to monitor
	- local connects to ConMon TX status and local channels,
		remote connects to ConMon RX status channel and subscribes to remote device


Program behaviour and state model:

(0) Perform memory allocation and other initialisation

(1) Set-up connection to ConMon server.  Use auto-connect mode.
	Action: attempt ConMon connection
	Dante State: LOCAL_DANTE_NOT_READY
	Timer: None (handled by client auto-connect)
Trigger for next state is conmon_connection_state_changed -> CONMON_CLIENT_CONNECTED

(2) Set-up conmon channels.
	Action: register for ConMon status channels
	Dante State: LOCAL_DANTE_HAVE_CONMON
	Timer: CONNECT_TIMER_CONMON_CHANNELS
Trigger for next state is register_cm_status_response + register_cm_local_response
On timeout, retry this state

(3) Wait for Dante ready
	Action: send DANTE_READY query
	Dante State: LOCAL_DANTE_HAVE_CONMON
	Timer: CONNECT_TIMER_READY_QUERY
Trigger for next state is handle_conmon_status_message -> DANTE_READY (or ROUTING_READY)
On timeout, retry this state
[Skip this state on remote, but can go back to it if we get ROUTING_READY false]

(4.1) Connect to router [local]
	Action: open connection to local routing
Immediately go to query capabilities

(4.2) Connect to router [remote]
	Action: open connection to remote routing and wait for resolution
	Dante State: LOCAL_DANTE_READY
	Timer: not applicable (retries are handled internally)
Trigger for next state is routing device state change -> RESOLVED.

(5) Query capabilities
	Action: query reouter capabilities
	Dante State: LOCAL_DANTE_READY
	Timer: CONNECT_TIMER_QUERY_CAP
Trigger for next state is query_capabilities_response successful.
On timeout, retry this state


(6) Update channel state
	Action: update all routing components
	Dante State: LOCAL_DANTE_ACTIVE
	Timer: CONNECT_TIMER_UPDATE
Trigger for next state is routing_message_response for all pending requests
On timeout, cancel and retry any pending requests
On failure, retry connection to local router

(7) Waiting
	Action: none
	Dante State: LOCAL_DANTE_ACTIVE
	Timer: None
Trigger for next state is process_rx_channel_change_notification.  This returns to 'update channel state'.


Unusual events

The following events can cause unusual state transitions:

- conmon_connection_state_changed -> Not CONMON_CLIENT_CONNECTED
	Return to waiting for ConMon connection

- handle_conmon_status_message -> routing ready false
	Return to waiting for Dante ready (skip query since it's implicit in the message we just received)
 */


//----------
// Include

// Audinate includes
#include "audinate/dante_api.h"

// System includes
#if defined(WIN32)
#include <assert.h>
#include <errno.h>
#define bzero(a, b) memset(a, 0, b)
#else
#include <sys/errno.h>
#include <assert.h>
#endif // !defined(WIN32)
#include <stdio.h>

// Local includes
#include "dapi_simple_timer.h"

// Conditional includes
#include "dapi_utils.h"
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
#include "dapi_utils_domains.h"
#endif

//----------
// Global paramters

#define NUM_TIMER_EVENTS 4
	// This defines the maximum number of different timers the app supports.
	// The sample infrastructure uses one; adjust the maximum as required for
	// the application code.

//#define DAPI_SKIP_DANTE_READY
	// Turning this on means we skip querying Dante Ready and go straight to
	// waiting for routing ready.  Enable this if you're testing the code on
	// a PC (Mac or Win) with DVS or Via: Dante Ready only applies to embedded
	// devices.
	// OS X sets this flag in the examples Makefile
	//
	// Only applies to local mode; remote only ever waits for ROUTING_READY
	//	and only if it gets ROUTING_READY false

#ifndef _WIN32
#define ENABLE_STDIN_EVENTS
	// If enabled, input on stdin will cause the program to write the routing
	// state to stdout.  Useful for testing.
	// Windows doesn't use a STDIN console, so don't enable it on Windows
#endif


//----------
// Simple error logging

static void
log_dante_error
(
	aud_error_t error,
	const char * str
)
{
	const char * err_name = aud_error_get_name(error);
	if (! err_name)
		err_name = "??";
	fprintf(stderr, "%s: %d (%s)\n"
		, str
		, error
		, err_name
	);
}


/*
	Wrapper to check if Dante result code indicates that an error occurred,
	and log it if it did.

	Return non-zero if error occured.
 */
static int
check_for_dante_error
(
	aud_error_t error,
	const char * str
)
{
	if (error == AUD_SUCCESS)
	{
		return 0;
	}

	log_dante_error(error, str);
	return (int) error;
}


//----------
// Our global environment

/*
	This enum tracks where we are up to in getting Dante up and running.
 */
typedef enum local_dante_state
{
	LOCAL_DANTE_NOT_READY = 0,
		// Dante infrastructure is not ready
	LOCAL_DANTE_HAVE_CONMON,
		// Connected to ConMon
	LOCAL_DANTE_READY,
		// Dante infrastructure is ready, waiting for routing ready
	LOCAL_DANTE_ACTIVE
		// Dante ready for us to talk to it
} local_dante_state_t;

// We use the connect timer as a timeout for a few different things; this tracks what
// it's currently doing
typedef enum connect_timer_op
{
	CONNECT_TIMER_NONE = 0,
	CONNECT_TIMER_CONMON_CHANNELS,
	CONNECT_TIMER_READY_QUERY,
	CONNECT_TIMER_RETRY_DEVICE,
	CONNECT_TIMER_QUERY_CAP,
	CONNECT_TIMER_UPDATE,
} connect_timer_op_t;


typedef struct request_tracker_conmon
{
	conmon_client_request_id_t req_id;
	aud_bool_t ok;
} request_tracker_conmon_t;


typedef struct app_global
{
	dapi_t * dapi;
	dante_runtime_t * runtime;
	conmon_client_t * conmon;
	dr_devices_t * routing;
	uint32_t verbose;

	struct app_global_args
	{
		aud_bool_t is_local;
		const char * device_name;
		#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
		dapi_utils_ddm_config_t ddm_config;
		#endif
	} args;

	local_dante_state_t dante_state;
	dr_device_t * routing_device;
	dapi_simple_timer_event_t * connect_timer;
	connect_timer_op_t connect_timer_op;

	aud_bool_t running;

	// sockets
	aud_bool_t sockets_changed;
	dante_sockets_t sockets;

	// timers
	dapi_simple_timer_t * timers;

	// Message tracking
	struct app_request
	{
		// request ID for registering conmon channels
		request_tracker_conmon_t register_cm_status;
		request_tracker_conmon_t register_cm_local;
		request_tracker_conmon_t cm_subscribe;

		// request ID for query capabilities
		dante_request_id_t query_capabilities;

		// request IDs for updating components
		dante_request_id_t update_component[DR_DEVICE_COMPONENT_COUNT];
	} requests;

	// Routing info
	struct app_global_routing_info
	{
		dr_device_state_t state;
		uint16_t num_rxc;
		dr_rxchannel_t ** rxc;
	} routing_info;

	// Buffers
	struct app_global_buffers
	{
		// shared buffer for all outgoing conmon messages
		struct app_global_buffers_conmon
		{
			conmon_message_body_t body;
		} conmon;
	} buffers;
} app_global_t;


enum
{
	// Bitmask of all the routing components we care about
	ALL_ROUTING_COMPONENTS_MAP =
		(1 << DR_DEVICE_COMPONENT_RXCHANNELS) |
		(1 << DR_DEVICE_COMPONENT_PROPERTIES)
};


//----------
// Timeouts

static const aud_utime_t RETRY_DEVICE_TIMEOUT = { 3, 0 };
static const aud_utime_t QUERY_CAP_TIMEOUT = { 3, 0 };


//----------
// Verbose logging

/*
	Enable selecting which parts of the system get extra logging.

	For example, enabling KEY_CALLBACKS and not ALL_CALLBACKS will mean we
	log callbacks that interest us, but not callbacks that we ignore (useful
	for testing that our state changes are happening).
	Enabling ALL_CALLBACKS will log every callback (useful for making sure
	we're getting callbacks, and looking for unexpected calls).

	We can switch this all off normally - it's just for testing stuff out.
 */
enum
{
	VERBOSE_LOG__KEY_CALLBACKS = 1 << 0,
	VERBOSE_LOG__ALL_CALLBACKS = 1 << 1,
	VERBOSE_LOG__CM_SEND = 1 << 2,
	VERBOSE_LOG__ROUTING_UPDATE = 1 << 3,
	VERBOSE_LOG__STATE_CHANGE = 1 << 4,
	VERBOSE_LOG__TRACE = 1 << 5,
	VERBOSE_LOG__DEVICE_STATE_CHANGE = 1 << 6
};

#define VERBOSE_ENABLED 1
#if VERBOSE_ENABLED == 1

static aud_bool_t
check_verbose_log
(
	app_global_t * app,
	uint32_t log_class
)
{
	return (app->verbose & log_class);
}

#define VERBOSE_LOG(LOG_CLASS, MSG, ...) \
	{ if (check_verbose_log(app, VERBOSE_LOG__ ## LOG_CLASS )) { fprintf(stderr, MSG "\n", ## __VA_ARGS__ ); } }

#else

#define VERBOSE_LOG(LOG_CLASS, MSG, ...)

#endif


//----------
// Declarations for app callbacks

// These are the callbacks that your app will use to do useful stuff.
// Declare them here; stub implementatons can be found at the end of this file.

static conmon_client_response_fn register_cm_status_response;
static conmon_client_response_fn register_cm_local_response;
static conmon_client_response_fn cm_subscribe_response;

static conmon_client_handle_monitoring_message_fn handle_conmon_status_message;

static dr_device_changed_fn handle_device_change;

static dr_device_response_fn query_capabilities_response;
static dr_device_response_fn routing_message_response;

static dapi_simple_timer_event_fn connect_timer_fired;


//----------
// Utility functions

static aud_error_t
connect_routing_device
(
	app_global_t * app
);


/*
	Because we share the connect timer, write a wrapper to manage access to it so that
	we don't get lost about our state
 */
static aud_error_t
connect_timer_stop
(
	app_global_t * app,
	connect_timer_op_t timer_op
)
{
	if (! app->connect_timer_op)
		return AUD_ERR_NOTFOUND;
			// already inactive
	
	if (timer_op && app->connect_timer_op != timer_op)
	{
		fprintf(stderr, "Error: cancelling timer for %u with %u\n"
			, app->connect_timer_op, timer_op
		);
		return AUD_ERR_INVALIDSTATE;
		// We shouldn't cancel a timer with a different timer
		// If we really intended to do so, pass CONNECT_TIMER_NONE
	}

	dapi_simple_timer_event_stop(app->connect_timer);
	app->connect_timer_op = CONNECT_TIMER_NONE;
	return AUD_SUCCESS;
}


static aud_error_t
connect_timer_schedule
(
	app_global_t * app,
	connect_timer_op_t timer_op,
	const aud_utime_t * timeout
)
{
	if (! timeout)
	{
		// NULL timeout is equivalent to stop
		return connect_timer_stop(app, timer_op);
	}

	if (! timer_op)
	{
		fprintf(stderr, "Error: must provide timer op\n");
		return AUD_ERR_INVALIDPARAMETER;
	}

	// we generally shouldn't clobber someone else's timer
	if (app->connect_timer_op)
	{
		if (timer_op != app->connect_timer_op)
		{
			fprintf(stderr, "Warning: overwriting timer %u with %u\n"
				, app->connect_timer_op, timer_op
			);
		}
	}

	dapi_simple_timer_event_schedule_once(app->connect_timer, NULL, timeout);
	app->connect_timer_op = timer_op;
	return AUD_SUCCESS;
}


static void
send_ready_query
(
	app_global_t * app
)
{
	//aud_error_t result;
	conmon_message_body_t * body = &app->buffers.conmon.body;
	uint16_t body_size;

	conmon_audinate_message_type_t msg_type;
	const aud_utime_t * backoff;

#ifndef DAPI_SKIP_DANTE_READY
	if (app->dante_state < LOCAL_DANTE_READY)
	{
		static const aud_utime_t DANTE_READY_QUERY_DELAY = { 1, 0 };

		msg_type = CONMON_AUDINATE_MESSAGE_TYPE_DANTE_READY_QUERY;
		backoff = & DANTE_READY_QUERY_DELAY;

		conmon_audinate_init_query_message(
			body, msg_type, 1000000	// 1us congestion delay
		);
		body_size = conmon_audinate_query_message_get_size(body);

		conmon_client_send_monitoring_message(
			app->conmon,
			NULL, NULL,
				// for this case, we don't need to confirm successful send
				// we will either get a response back, or retry
			CONMON_CHANNEL_TYPE_LOCAL,
			CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC,
			CONMON_VENDOR_ID_AUDINATE,
			body, body_size
		);
			// Dante ready query is sent on the local monitoring channel
	}
	else
#endif
	{
		static const aud_utime_t ROUTING_READY_QUERY_DELAY = { 5, 0 };
			// use a longer timeout for this because we don't want to stress
			// the system if it's too heavily loaded

		msg_type = CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_READY_QUERY;
		backoff = & ROUTING_READY_QUERY_DELAY;

		conmon_audinate_init_query_message(
			body, msg_type, 1000000	// 1us congestion delay
		);
		body_size = conmon_audinate_query_message_get_size(body);
	
		conmon_client_send_control_message(
			app->conmon,
			NULL, NULL,
				// for this case, we don't need to confirm successful send
				// we will either get a response back, or retry
			NULL,
				// sending to ourself only
			CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC,
			CONMON_VENDOR_ID_AUDINATE,
			body, body_size,
			NULL
				// OK for this to fail immediately
		);
	}

	VERBOSE_LOG(CM_SEND, "Sent ready query 0x%04x", msg_type);

	// schedule a timer to retry if we don't get a result
	connect_timer_schedule(app, CONNECT_TIMER_READY_QUERY, backoff);
}


// Handling routing component updates

static void
discard_current_routing_update
(
	app_global_t * app,
	dr_device_component_t component
)
{
	dante_request_id_t curr_req_id = app->requests.update_component[component];
	if (curr_req_id == DANTE_NULL_REQUEST_ID)
		return;

	if (app->routing_device)
	{
		dr_device_cancel_request(app->routing_device, curr_req_id);
	}

	app->requests.update_component[component] = DANTE_NULL_REQUEST_ID;
}

#if 0
/*
	We don't need this function in this version of the program

	As written, we only discard all routing updates when we close the connection,
	and in that case DAPI automatically discards the back end and we blank our
	copy of the request IDs and other state.

	Enable this if there's a need to discard_all without resetting the device connection.
 */
static void
discard_current_routing_updates
(
	app_global_t * app,
	unsigned component_map
)
{
	dr_device_component_t i;
	if (! component_map)
	{
		component_map = (1 << DR_DEVICE_COMPONENT_COUNT) - 1;
			// 0 => everything
	}

	for (i = 0; i < DR_DEVICE_COMPONENT_COUNT; i++)
	{
		if (component_map & (1 << i))
		{
			discard_current_routing_update(app, i);
		}
	}
}
#endif


static void
update_routing_component
(
	app_global_t * app,
	dr_device_component_t component,
	aud_bool_t force_stale
)
{
	static const aud_utime_t UPDATE_TIMEOUT = { 3, 0 };

	if (! app->routing_device)
		return;

	discard_current_routing_update(app, component);

	if (force_stale)
	{
		dr_device_mark_component_stale(app->routing_device, component);
	}
	dr_device_update_component(
		app->routing_device,
		routing_message_response,
		&app->requests.update_component[component],
		component
	);
	// Set the timer in case we don't get a response
	connect_timer_schedule(app, CONNECT_TIMER_UPDATE, &UPDATE_TIMEOUT);
	VERBOSE_LOG(ROUTING_UPDATE, "Updating component #%u with request %p"
		, (unsigned) component
		, app->requests.update_component[component]
	);
}


static void
update_routing_components
(
	app_global_t * app,
	unsigned component_map,
	aud_bool_t force_stale
)
{
	dr_device_component_t i;
	if (! component_map)
	{
		component_map = (1 << DR_DEVICE_COMPONENT_COUNT) - 1;
			// 0 => everything
	}

	for (i = 0; i < DR_DEVICE_COMPONENT_COUNT; i++)
	{
		if (component_map & (1 << i))
		{
			update_routing_component(app, i, force_stale);
		}
	}
}


// ConMon updates

static void
cancel_conmon_request
(
	app_global_t * app,
	request_tracker_conmon_t * req
)
{
	if (req->req_id != CONMON_CLIENT_NULL_REQ_ID)
	{
		conmon_client_request_cancel(app->conmon, req->req_id);
		req->req_id = CONMON_CLIENT_NULL_REQ_ID;
	}
	req->ok = AUD_FALSE;
}

static void
cancel_conmon_requests
(
	app_global_t * app
)
{
	if (! app->conmon)
		return;

	cancel_conmon_request(app, &app->requests.register_cm_status);
	if (! app->args.is_local)
	{
		cancel_conmon_request(app, &app->requests.register_cm_local);
	}
	else
	{
		cancel_conmon_request(app, &app->requests.register_cm_status);
	}
}


static aud_error_t
register_cm_status_monitoring_channel
(
	app_global_t * app,
	request_tracker_conmon_t * req,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	conmon_client_response_fn * result_fn
)
{
	aud_error_t result;

	if (req->ok)
		return AUD_SUCCESS;

	cancel_conmon_request(app, req);

	result = conmon_client_register_monitoring_messages(
		app->conmon,
		result_fn,
		&req->req_id,
		channel_type,
		channel_direction,
		handle_conmon_status_message
			// all channel types use the same message callback
	);
	return result;
}


static aud_error_t
subscribe_cm_rx_status_monitoring_channel
(
	app_global_t * app,
	request_tracker_conmon_t * req,
	conmon_channel_type_t channel_type,
	conmon_client_response_fn * result_fn
)
{
	aud_error_t result;

	if (req->ok)
		return AUD_SUCCESS;

	cancel_conmon_request(app, req);

	result = conmon_client_subscribe(
		app->conmon,
		result_fn,
		&req->req_id,
		channel_type,
		app->args.device_name
	);
	return result;
}


static void
connect_conmon_channels
(
	app_global_t * app
)
{
	static const aud_utime_t CONNECT_CHANNELS_TIMEOUT = { 3, 0 };

	// We want status messages from the monitored device.

	if (app->args.is_local)
	{
		// These will all be TX messages (originating from the local device)
		// Some of these will be on the STATUS channel, and some will be on the
		// LOCAL channel (which is for status messages that don't leave the
		// device).
	
		// Register for status channel
		register_cm_status_monitoring_channel(
			app,
			&app->requests.register_cm_status,
			CONMON_CHANNEL_TYPE_STATUS,
			CONMON_CHANNEL_DIRECTION_TX,
			register_cm_status_response
		);
		// register for local channel
		register_cm_status_monitoring_channel(
			app,
			&app->requests.register_cm_local,
			CONMON_CHANNEL_TYPE_LOCAL,
			CONMON_CHANNEL_DIRECTION_TX,
			register_cm_local_response
		);
	}
	else
	{
		// These will all be RX messages (originating from the remote device)
		// on the STATUS channel

		// Register for status channel
		register_cm_status_monitoring_channel(
			app,
			&app->requests.register_cm_status,
			CONMON_CHANNEL_TYPE_STATUS,
			CONMON_CHANNEL_DIRECTION_RX,
			register_cm_status_response
		);
		subscribe_cm_rx_status_monitoring_channel(
			app,
			&app->requests.cm_subscribe,
			CONMON_CHANNEL_TYPE_STATUS,
			cm_subscribe_response
		);
	}

	connect_timer_schedule(app, CONNECT_TIMER_CONMON_CHANNELS, &CONNECT_CHANNELS_TIMEOUT);
}


/*
	Check if the conmon channels are done, and if so move on to next stage
 */
static void
check_conmon_channels
(
	app_global_t * app
)
{
	if (app->args.is_local)
	{
		if (app->requests.register_cm_status.ok && app->requests.register_cm_local.ok)
		{
			connect_timer_stop(app, CONNECT_TIMER_CONMON_CHANNELS);
			send_ready_query(app);
		}
	}
	else
	{
		if (app->requests.register_cm_status.ok && app->requests.cm_subscribe.ok)
		{
			connect_timer_stop(app, CONNECT_TIMER_CONMON_CHANNELS);
			connect_routing_device(app);
		}
	}
}


// Device updates

static aud_error_t
query_device_capabilities
(
	app_global_t * app
)
{
	aud_error_t result;

	VERBOSE_LOG(TRACE, "Sending query capabilities");

	result = dr_device_query_capabilities(
		app->routing_device, query_capabilities_response, &app->requests.query_capabilities
	);
	if (result != AUD_SUCCESS)
	{
		log_dante_error(result, "Query capabilities failed");
		connect_timer_schedule(app, CONNECT_TIMER_RETRY_DEVICE, &RETRY_DEVICE_TIMEOUT);
		return result;
	}

	connect_timer_schedule(app, CONNECT_TIMER_QUERY_CAP, &QUERY_CAP_TIMEOUT);
	return result;
}


static aud_error_t
open_routing_device
(
	app_global_t * app
)
{
	aud_error_t result;

	if (app->args.is_local)
	{
		VERBOSE_LOG(TRACE, "Opening local connection");

		result = dr_device_open_local(app->routing, &app->routing_device);
	}
	else
	{
		VERBOSE_LOG(TRACE, "Opening connection to %s", app->args.device_name);
		dr_device_open_t * device_open_config =
			dr_device_open_config_new(app->args.device_name);
		if (device_open_config)
		{
			result = dr_device_open_with_config(
				app->routing, device_open_config, &app->routing_device
			);
			dr_device_open_config_free(device_open_config);
		}
		else
		{
			result = AUD_ERR_NOMEMORY;
		}
	}
	if (check_for_dante_error(result, "Open local failed"))
	{
		return result;
	}

	dr_device_set_context(app->routing_device, app);
		// Since there's only one device, use 'app' as the context
	dr_device_set_changed_callback(app->routing_device, handle_device_change);

	app->routing_info.state = dr_device_get_state(app->routing_device);
	if (app->args.is_local)
	{
		app->dante_state = LOCAL_DANTE_ACTIVE;
		VERBOSE_LOG(STATE_CHANGE, "Device connected: State set to ACTIVE");

		query_device_capabilities(app);
	}
	// else wait until we resolve or fail

	return AUD_SUCCESS;
}


static void
clear_routing_device
(
	app_global_t * app
)
{
	dr_device_close(app->routing_device);
	app->routing_device = NULL;

	bzero(&app->routing_info, sizeof(app->routing_info));
}


static aud_error_t
connect_routing_device
(
	app_global_t * app
)
{
	aud_error_t result;

	if (app->routing_device)
	{
		return AUD_ERR_INVALIDSTATE;
	}

	result = open_routing_device(app);
	if (result != AUD_SUCCESS)
	{
		app->dante_state = LOCAL_DANTE_READY;
		VERBOSE_LOG(STATE_CHANGE, "Device connect failed: State set to READY");
		if (app->args.is_local)
		{
			send_ready_query(app);
				// fall back to start and retry
		}
		else
		{
			connect_timer_schedule(app, CONNECT_TIMER_RETRY_DEVICE, &RETRY_DEVICE_TIMEOUT);
		}
	}
	return result;
}


static aud_error_t
reconnect_routing_device
(
	app_global_t * app
)
{
	if (app->routing_device)
	{
		clear_routing_device(app);
	}
	return connect_routing_device(app);
}


static void
process_routing_ready_true
(
	app_global_t * app,
	const conmon_message_body_t * rr_msg
)
{
	if (app->routing_device)
	{
		// check that our details are up to date
		uint16_t rr_nrxc = conmon_audinate_routing_ready_status_num_rxchannels(rr_msg);
		if (rr_nrxc == app->routing_info.num_rxc)
		{
			// reload device info
			update_routing_components(app, ALL_ROUTING_COMPONENTS_MAP, AUD_TRUE);
			return;
		}
		else
		{
			VERBOSE_LOG(STATE_CHANGE, "RX channels changed from %u -> %u"
				, (unsigned) app->routing_info.num_rxc
				, (unsigned) rr_nrxc
			);
			// Device might have completely changed; close and reopen
			clear_routing_device(app);
		}
	}

	connect_routing_device(app);
}


static void
process_rx_channel_change_notification
(
	app_global_t * app,
	const conmon_message_body_t * rr_msg
)
{
	uint16_t i, j, n;
	aud_bool_t changes = AUD_FALSE;

	if (! app->routing_device)
		return;

	n = conmon_audinate_id_set_num_elements(rr_msg);
	VERBOSE_LOG(TRACE, "RX change: %u elements", (unsigned) n);
	for (i = 0; i < n; i++)
	{
		uint8_t bitmap = conmon_audinate_id_set_element_at_index(rr_msg, i);
		if (bitmap)
		{
			unsigned base = i * 8;
			for (j = 0; j < 8; j++)
			{
				if (bitmap & (1 << j))
				{
					unsigned channel_index = base + j;
					if (channel_index >= app->routing_info.num_rxc)
					{
						// shouldn't get this - we're obviously out-of-date
						reconnect_routing_device(app);
						return;
					}

					dr_rxchannel_t * channel = app->routing_info.rxc[channel_index];
					dr_rxchannel_mark_stale(channel);
					changes = AUD_TRUE;
					VERBOSE_LOG(TRACE, "Marking channel %u stale", (unsigned) channel_index + 1);
				}
			}
		}
	}

	if (changes)
	{
		update_routing_component(app, DR_DEVICE_COMPONENT_RXCHANNELS, AUD_FALSE);
	}
}


//----------
// Dante event handlers

static void
device_sockets_changed
(
	const dr_devices_t * routing
)
{
	app_global_t * app = dr_devices_get_context(routing);
	app->sockets_changed = AUD_TRUE;

	VERBOSE_LOG(ALL_CALLBACKS, "Device sockets changed");
}


static void
reset_all_connections
(
	app_global_t * app
)
{
 	if (app->dante_state > LOCAL_DANTE_NOT_READY)
 	{
		if (app->routing_device)
		{
			clear_routing_device(app);
		}
		cancel_conmon_requests(app);
		app->dante_state = LOCAL_DANTE_NOT_READY;
	}
}


static void
conmon_connection_state_changed
(
	conmon_client_t * conmon
)
{
	app_global_t * app = conmon_client_context(conmon);

	// Only care if we're connected or not
	conmon_client_state_t state = conmon_client_state(conmon);

	VERBOSE_LOG(KEY_CALLBACKS, "ConMon state changed to %u", (unsigned) state);

	// Clear out old state, if any
	reset_all_connections(app);

	if (state == CONMON_CLIENT_CONNECTED)
	{
		app->dante_state = LOCAL_DANTE_HAVE_CONMON;

		// Set up our channels
		connect_conmon_channels(app);
	}
}


static void
query_capabilities_response
(
	dr_device_t * device,
	dante_request_id_t request_id,
	aud_error_t result
)
{
	app_global_t * app = dr_device_get_context(device);

	VERBOSE_LOG(KEY_CALLBACKS, "Received response for query_capabilities request %p", request_id);

	if (request_id != app->requests.query_capabilities)
	{
		// Ignore
		return;
	}

	connect_timer_stop(app, CONNECT_TIMER_QUERY_CAP);
	app->requests.query_capabilities = NULL;

	result = dr_device_get_rxchannels(
		app->routing_device,
		&app->routing_info.num_rxc,
		&app->routing_info.rxc
	);
	if (result != AUD_SUCCESS)
	{
		log_dante_error(result, "Failed to get RX channel count");
	}

	// Update all our relevant information
	update_routing_components(app, ALL_ROUTING_COMPONENTS_MAP, AUD_TRUE);
}


static void
routing_message_response
(
	dr_device_t * device,
	dante_request_id_t request_id,
	aud_error_t result
)
{
	// This handler just confirms that our routing updates are completed, and
	// turns off the timer.

	AUD_UNUSED(result);

	app_global_t * app = dr_device_get_context(device);
	size_t i;
	unsigned incomplete = 0;

	VERBOSE_LOG(KEY_CALLBACKS, "Received response for request %p", request_id);

	for (i = 0; i < DR_DEVICE_COMPONENT_COUNT; i++)
	{
		if (request_id == app->requests.update_component[i])
		{
			app->requests.update_component[i] = DANTE_NULL_REQUEST_ID;
		}
		else if (app->requests.update_component[i] != DANTE_NULL_REQUEST_ID)
		{
			incomplete ++;
		}
	}

	if (! incomplete)
	{
		connect_timer_stop(app, CONNECT_TIMER_UPDATE);
	}
}


//----------
// App declarations

// **********
// Declare local functions called by the app callbacks here


// **********

//----------
// App callbacks

// These are the callbacks that your app will use to do useful stuff.

static void 
handle_conmon_status_message
(
	conmon_client_t * client,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
)
{
	app_global_t * app = conmon_client_context(client);

	conmon_message_class_t msg_class = conmon_message_head_get_message_class(head);
	const conmon_vendor_id_t * v = conmon_message_head_get_vendor_id(head);
	conmon_audinate_message_type_t aud_type;

	// Validate message source - We are only interested in TX Status or local messages
	conmon_channel_direction_t expected_direction =
		(app->args.is_local ? CONMON_CHANNEL_DIRECTION_TX : CONMON_CHANNEL_DIRECTION_RX);
	if (channel_direction != expected_direction)
	{
		return;
	}
	switch(channel_type)
	{
	case CONMON_CHANNEL_TYPE_STATUS:
	case CONMON_CHANNEL_TYPE_LOCAL:
		break;
	default:
		return;
	}

	// Validate message type
	if (msg_class != CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC)
	{
		return;
	}
	if (conmon_message_head_get_body_size(head) < conmon_audinate_message_get_head_size())
	{
		return;
	}
	if (!conmon_vendor_id_equals(v, CONMON_VENDOR_ID_AUDINATE))
	{
		return;
	}
	aud_type = conmon_audinate_message_get_type(body);

	VERBOSE_LOG(ALL_CALLBACKS, "Got ConMon message type 0x%04x on %s"
		, aud_type, ((channel_type == CONMON_CHANNEL_TYPE_LOCAL) ? "local" : "status")
	);

	if (app->args.is_local && app->dante_state < LOCAL_DANTE_READY)
	{
		// At this point, we only care about DANTE_READY
#ifndef DAPI_SKIP_DANTE_READY
		if (aud_type == CONMON_AUDINATE_MESSAGE_TYPE_DANTE_READY)
#else
		if (aud_type == CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_READY)
			// for testing on non-embedded devices, the present of routing ready can double as dante_ready
#endif
		{
			VERBOSE_LOG(KEY_CALLBACKS, "Got DANTE_READY");

			connect_timer_stop(app, CONNECT_TIMER_NONE);
				// don't care what we were doing; interrupt it
			app->dante_state = LOCAL_DANTE_READY;
			VERBOSE_LOG(STATE_CHANGE, "Dante ready: State set to READY");
			connect_routing_device(app);
		}
		return;
	}

	switch(aud_type)
	{
	case CONMON_AUDINATE_MESSAGE_TYPE_ROUTING_READY_STATUS:
		connect_timer_stop(app, CONNECT_TIMER_NONE);
			// don't care what we were doing; interrupt it
		if (conmon_audinate_routing_ready_status_is_ready(body))
		{
			VERBOSE_LOG(KEY_CALLBACKS, "Got ROUTING_READY == yes");
			process_routing_ready_true(app, body);
		}
		else
		{
			VERBOSE_LOG(KEY_CALLBACKS, "Got ROUTING_READY == no");
			clear_routing_device(app);
		}
		return;

	case CONMON_AUDINATE_MESSAGE_TYPE_RX_CHANNEL_CHANGE:
		VERBOSE_LOG(KEY_CALLBACKS, "Got RX channel change");
		process_rx_channel_change_notification(app, body);
		return;

	default:
		return;
	}
}


static void
handle_register_cm_response
(
	request_tracker_conmon_t * req,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	if (request_id == req->req_id)
	{
		req->req_id = CONMON_CLIENT_NULL_REQ_ID;
		req->ok = (result == AUD_SUCCESS);
			// handle failure when we evaluate retries
	}
}

static void
register_cm_status_response
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	app_global_t * app = conmon_client_context(client);

	VERBOSE_LOG(ALL_CALLBACKS, "conmon register status response");

	handle_register_cm_response(&app->requests.register_cm_status, request_id, result);
	check_conmon_channels(app);
}

static void
register_cm_local_response
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	app_global_t * app = conmon_client_context(client);

	VERBOSE_LOG(ALL_CALLBACKS, "conmon register local response");

	handle_register_cm_response(&app->requests.register_cm_local, request_id, result);
	check_conmon_channels(app);
}

static void
cm_subscribe_response
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
)
{
	app_global_t * app = conmon_client_context(client);

	VERBOSE_LOG(ALL_CALLBACKS, "conmon register subscribe response");

	handle_register_cm_response(&app->requests.cm_subscribe, request_id, result);
	check_conmon_channels(app);
}


static void
print_rxchannels
(
	app_global_t * app
)
{
	unsigned i;

	if (! app->routing_device || dr_device_get_state(app->routing_device) < DR_DEVICE_STATE_ACTIVE)
	{
		printf("Device not ready\n");
		return;
	}

	for (i = 0; i < app->routing_info.num_rxc; i++)
	{
		dr_rxchannel_t * channel = app->routing_info.rxc[i];
		dante_rxstatus_t chan_status;

		printf("%02u: %s", i+1, dr_rxchannel_get_name(channel));
		chan_status = dr_rxchannel_get_status(channel);
		if (chan_status != DANTE_RXSTATUS_NONE)
		{
			const char * sub_target = dr_rxchannel_get_subscription(channel);
			printf(" -> %s : %s (%u)"
				, sub_target
				, dante_rxstatus_to_string(chan_status)
				, (unsigned) chan_status
			);
		}
		fputc('\n', stdout);
	}
	fflush(stdout);
}


static void
process_device_state_change
(
	app_global_t * app
)
{
	dr_device_state_t old_state = app->routing_info.state;
	dr_device_state_t new_state = dr_device_get_state(app->routing_device);
	if (new_state == old_state)
		return;

	VERBOSE_LOG(DEVICE_STATE_CHANGE, "Device state changed %u -> %u"
		, app->routing_info.state, new_state
	);
	app->routing_info.state = new_state;

	switch (new_state)
	{
	case DR_DEVICE_STATE_ERROR:
		clear_routing_device(app);
		connect_timer_schedule(app, CONNECT_TIMER_RETRY_DEVICE, &RETRY_DEVICE_TIMEOUT);
		break;

	case DR_DEVICE_STATE_RESOLVING:
		// nothing to do
		break;

	case DR_DEVICE_STATE_RESOLVED:
		query_device_capabilities(app);
		break;

	case DR_DEVICE_STATE_QUERYING:
		// nothing to do
		break;

	case DR_DEVICE_STATE_ACTIVE:
		// nothing to do
		// handled by query_capabilities_response
		break;

	default:
		;
	}
}

static void
handle_device_change
(
	dr_device_t * device,
	dr_device_change_flags_t change_flags
)
{
	app_global_t * app = (app_global_t *) dr_device_get_context(device);

	VERBOSE_LOG(ALL_CALLBACKS, "Device changed: 0x%02lx", (unsigned long) change_flags);

	// First check for state change
	if (change_flags & DR_DEVICE_CHANGE_FLAG_STATE)
	{
		process_device_state_change(app);
	}


	// Then check for value changes

	if (change_flags & DR_DEVICE_CHANGE_FLAG_RXCHANNELS)
	{
		VERBOSE_LOG(KEY_CALLBACKS, "Device RX channels changed");

		//**********
		fputs("RX channels updated\n", stdout);

		print_rxchannels(app);

		//**********
	}
}


static void
connect_timer_fired
(
	dapi_simple_timer_event_t * timer,
	const aud_utime_t * time,
	void * context
)
{
	app_global_t * app = (app_global_t *) context;
	connect_timer_op_t op;

	AUD_UNUSED(timer);

	if (! time)
		return;

	op = app->connect_timer_op;
	fprintf(stderr, "Connect timer fired with op == %u\n", (unsigned) op);
	app->connect_timer_op = CONNECT_TIMER_NONE;

	switch(op)
	{
	case CONNECT_TIMER_CONMON_CHANNELS:
		connect_conmon_channels(app);
		break;

	case CONNECT_TIMER_READY_QUERY:
		send_ready_query(app);
		break;

	// We'll keep retrying these, and expect a ROUTING_READY == no if something goes wrong
	case CONNECT_TIMER_RETRY_DEVICE:
	case CONNECT_TIMER_QUERY_CAP:
		reconnect_routing_device(app);
		break;

	case CONNECT_TIMER_UPDATE:
		update_routing_components(app, 0, AUD_FALSE);
		break;

	default:
		break;
	}
}


//----------
// Example STDIN handler

#ifdef ENABLE_STDIN_EVENTS

/*
	As a simple example of how to combine Dante and non-Dante sockets, we
	set up a handler to print the current rxchannels information when we
	get input on STDIN.

	NOTE: doesn't work on Windows
 */

static void
process_stdin
(
	app_global_t * app
)
{
	// we're just using this socket as a trigger, so we don't care about the contents
	// so drain the contents
	for (;;)
	{
		int ch = fgetc(stdin);
		if (ch == 0 || ch == '\n')
			break;
	}

	print_rxchannels(app);
}

#endif


//----------
// Initialising

static aud_error_t
init_conmon
(
	app_global_t * app
)
{
	aud_error_t result;

	conmon_client_config_t * config = conmon_client_config_new("my_sample_app");
	if (! config)
	{
		log_dante_error(AUD_ERR_NOMEMORY, "Failed to create conmon client config");
		return AUD_ERR_NOMEMORY;
	}

	// default client config is fine

	result = conmon_client_new_dapi(app->dapi, config, &app->conmon);

	// regardless of result, we free the config struct
	conmon_client_config_delete(config);

	if (check_for_dante_error(result, "Create conmon failed"))
	{
		return result;
	}

	conmon_client_set_context(app->conmon, app);
	conmon_client_set_connection_state_changed_callback(app->conmon, conmon_connection_state_changed);

	conmon_client_auto_connect(app->conmon);
	return AUD_SUCCESS;
}


static aud_error_t
init_routing
(
	app_global_t * app
)
{
	aud_error_t result = dr_devices_new_dapi(app->dapi, &app->routing);
	if (check_for_dante_error(result, "Create devices failed"))
		return result;

	dr_devices_set_context(app->routing, app);
	dr_devices_set_sockets_changed_callback(app->routing, device_sockets_changed);

	app->routing_device = NULL;
		// make sure this is NULL

	return AUD_SUCCESS;
}


//----------
// Run loop

// Utilities

static void
reload_sockets(app_global_t * app)
{
	aud_error_t result;

	app->sockets_changed = AUD_FALSE;
	dante_sockets_clear(&app->sockets);

	result = dante_runtime_get_sockets_and_timeout(app->runtime, &app->sockets, NULL);
	if (result != AUD_SUCCESS)
	{
		log_dante_error(result, "Failed to get Dante sockets");
	}

	// **********
	// Add your client sockets here

	// dante_sockets_add_read(&app->sockets, my_socket);
	#ifdef ENABLE_STDIN_EVENTS
	dante_sockets_add_read(&app->sockets, STDIN_FILENO);
	#endif

	// **********

	#if VERBOSE_ENABLED > 0
	{
		fprintf(stderr, "Sockets changed: ");
		unsigned int i;
		for (i = 0; i < (unsigned int) app->sockets.n; i++)
		{
			if (FD_ISSET(i, &app->sockets.read_fds))
			{
				fprintf(stderr, " %u", i);
			}
		}
		fputc('\n', stderr);
	}
	#endif
}


static void
set_next_action(aud_utime_t * base, const aud_utime_t * curr)
{
	if (base->tv_sec || base->tv_usec)
	{
		if (aud_utime_compare(base, curr) <= 0)
		{
			return;
		}
	}
	* base = * curr;
}


/*
	Calculate the next timeout for 'select'

	NOTE: next_timeout is relative time, internal timing uses absolute time
 */
static void
calculate_select_timeout(app_global_t * app, struct timeval * next_timeout)
{
	aud_error_t result;
	aud_utime_t timeout;
	const aud_utime_t * timeout_p;
	aud_utime_t next_action;

	next_action = AUD_UTIME_ZERO;

	result = dante_runtime_get_sockets_and_timeout(app->runtime, NULL, &timeout);
	if (! check_for_dante_error(result, "Failed to get conmon action time"))
	{
		set_next_action(&next_action, &timeout);
	}

	timeout_p = dapi_simple_timer_get_next_firing(app->timers);
	if (timeout_p)
	{
		set_next_action(&next_action, timeout_p);
	}

	if (next_action.tv_sec || next_action.tv_usec)
	{
		aud_utime_t curr_time;

		result = aud_utime_get(&curr_time);
		if (! check_for_dante_error(result, "Failed to get time"))
		{
			if (aud_utime_compare(&next_action, &curr_time) > 0)
			{
				aud_utime_sub(&next_action, &curr_time);
				next_timeout->tv_sec = next_action.tv_sec;
				next_timeout->tv_usec = next_action.tv_usec;
			}
			else
			{
				// Fire as soon as possible
				next_timeout->tv_sec = 0;
				next_timeout->tv_usec = 0;
			}
			return;
		}
	}

	// use a default timeout
	next_timeout->tv_sec = 5;
	next_timeout->tv_usec = 0;
}


// The loop

static int
run_loop(app_global_t * app)
{
	aud_error_t aud_result;

	app->running = AUD_TRUE;
	app->sockets_changed = AUD_TRUE;

	while(app->running)
	{
		dante_sockets_t curr_sockets;
		dante_sockets_t * active_sockets;
		struct timeval select_timeout;
		int select_result;

		aud_bool_t sockets_changed =
			app->sockets_changed || dante_runtime_sockets_have_changed(app->runtime);
		if (sockets_changed)
		{
			reload_sockets(app);
		}

		dante_sockets_copy(&app->sockets, &curr_sockets);

		calculate_select_timeout(app, &select_timeout);
		

		// Select
		select_result = select(
			curr_sockets.n,
			&curr_sockets.read_fds,
			NULL,	// curr_sockets.write_fds
			NULL,
			&select_timeout
		);

		if (select_result > 0)
		{
			// We have sockets to process
			active_sockets = &curr_sockets;
		}
		else
		{
			// no sockets
			active_sockets = NULL;

			// did we get an error or just timeout
			if (select_result < 0)
			{
				// got an error - log it
				int error = errno;
				if (error != EINTR)
				{
					perror("Select failed");
					// Could abort here, but let's keep trying
					assert(! "Select failed");
				}
			}
		}

		// Call process functions.  If we have active sockets, include them.

		// Need to call process for dante_runtime in case there is time-based work.
		aud_result = dante_runtime_process_with_sockets(
			app->runtime,
			active_sockets
		);
		check_for_dante_error(aud_result, "Dante sockets failed");

		if (active_sockets)
		{
			// **********
			// Check your client socket/s here

			// example - read STDIN
			#ifdef ENABLE_STDIN_EVENTS
			if (FD_ISSET(STDIN_FILENO, &active_sockets->read_fds))
			{
				process_stdin(app);
			}
			#endif

			// **********
		}

		// Fire any expired timers
		aud_result = dapi_simple_timer_process(app->timers, NULL);
		check_for_dante_error(aud_result, "Process timers failed");
	}

	return 0;
}


//----------
// Main

// put argv[0] into global in case we need it
static const char * g_progname;

static void
usage(const char * err_msg)
{
	if (err_msg)
	{
		fputs(g_progname, stderr);
		fputs(": ", stderr);
		fputs(err_msg, stderr);
		fputc('\n', stderr);
	}
	fprintf(stderr,
		"Usage: %s .|device_name\n"
	#if (DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE)
		"\tDDM connection options:\n"
		"\t  --ddm[=<hostname>:<port>] --domain=<domain>\n"
		"\t  --user=<user> --pass=<password>\n"
	#endif
		, g_progname
	);
}


static int
check_option(const char * opt, app_global_t * app)
{
#if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE
	aud_error_t parse_result;
	if (dapi_utils_ddm_config_parse_one(&app->args.ddm_config, opt, &parse_result))
	{
		if (parse_result != AUD_SUCCESS)
		{
			fprintf(stderr,
				"%s: Invalid domain option '%s'\n"
				, g_progname
				, opt
			);
			return -1;
		}
		return 0;
	}
#endif

	fprintf(stderr, "%s: unknown option '%s'\n"
		, g_progname
		, opt
	);
	return -1;
}

static int
check_args(int argc, char ** argv, app_global_t * app)
{
	const char * device_name;
	aud_bool_t opts_done = AUD_FALSE;
	int presult;
	unsigned num_args = 0;

	int i;
	for (i = 0; i < argc; i++)
	{
		const char * arg = argv[i];
		if (! opts_done && arg[0] == '-')
		{
			if (arg[1] == '-' && !arg[2])
			{
				opts_done = AUD_TRUE;
				continue;
			}
			presult = check_option(arg, app);
			if (presult)
			{
				return presult;
			}
		}
		else if (num_args == 0)
		{
			num_args++;
			device_name = arg;
			if (strcmp(device_name, ".") == 0)
			{
				app->args.is_local = AUD_TRUE;
				app->args.device_name = NULL;
			}
			else if (! dante_name_is_valid_device_name(device_name))
			{
				fprintf(stderr, "%s: invalid device name '%s'\n"
					, g_progname
					, device_name
				);
				usage(NULL);
				return -1;
			}
			else
			{
				app->args.is_local = AUD_FALSE;
				app->args.device_name = device_name;
			}
		}
		else
		{
			usage("Too many devices");
		}
	}

	if (! num_args)
	{
		usage("No device name");
		return -1;
	}
	return 0;
}

int
main(int argc, char ** argv)
{
	int presult = 0;
	aud_error_t aud_result;
	app_global_t app_buf = { NULL };
	app_global_t * app = & app_buf;

	app->verbose = 0xff;

	// handle args
	g_progname = argv[0];
	presult = check_args(argc - 1, argv + 1, app);
	if (presult < 0)
	{
		return 1;
	}

#ifdef WIN32
	dapi_utils_check_quick_edit_mode(AUD_FALSE);
#endif

	// run program
	aud_result = dapi_new(&app->dapi);
	if (check_for_dante_error(aud_result, "Audinate environment failed"))
	{
		return 1;
	}

	app->runtime = dapi_get_runtime(app->dapi);

	#if (DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__STANDALONE)
	if (app->args.ddm_config.connect)
	{
		dante_domain_handler_t * domain_handler = dapi_get_domain_handler(app->dapi);

		aud_result = dapi_utils_ddm_connect_blocking(
			&app->args.ddm_config, domain_handler, app->runtime, NULL
		);
		if (check_for_dante_error(aud_result, "Audinate environment failed"))
		{
			presult = 1;
			goto l__cleanup;
		}
	}
	#endif

	aud_result = init_conmon(app);
	if (check_for_dante_error(aud_result, "Conmon not initialised"))
	{
		presult = 1;
		goto l__cleanup;
	}

	aud_result = init_routing(app);
	if (check_for_dante_error(aud_result, "Routing not initialised"))
	{
		presult = 1;
		goto l__cleanup;
	}

	app->timers = dapi_simple_timer_new(NUM_TIMER_EVENTS);
	if (! app->timers)
	{
		fprintf(stderr, "Failed to init timers\n");
		presult = 1;
		goto l__cleanup;
	}

	// grab a timer for use in the main code
	app->connect_timer = dapi_simple_timer_event_alloc(app->timers);
	if (check_for_dante_error(aud_result, "No timers available"))
	{
		presult = 1;
		goto l__cleanup;
	}
	app->connect_timer_op = CONNECT_TIMER_NONE;
	dapi_simple_timer_event_set_callback(app->connect_timer, connect_timer_fired, app);

	// **********
	// Initialise your code here


	// **********

	presult = run_loop(app);
		// if all goes well, this never returns

l__cleanup:
	// **********
	// Clean up your code here


	// **********

	if (app->timers)
	{
		dapi_simple_timer_delete(app->timers);
		app->timers = NULL;
	}

	if (app->routing)
	{
		dr_devices_delete(app->routing);
		app->routing = NULL;
		app->routing_device = NULL;
	}

	if (app->conmon)
	{
		conmon_client_delete(app->conmon);
		app->conmon = NULL;
	}

	return presult;
}


///----------
