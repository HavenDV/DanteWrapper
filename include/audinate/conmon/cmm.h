/*
 * File     : $RCSfile$
 * Created  : August 2010
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : ConMon Manager API
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

/**
 * @file cmm.h 
 * ConMon Manager API types and definitions
 */
#ifndef _CMM_H
#define _CMM_H

#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#include "dante/dante_common.h"
#include "dante/dapi_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** An enumeration representing the different possible client connection states */
typedef enum
{
		/** The client is initialised but disconnected */
	CMM_CONNECTION_STATE_INITIALISED,

		/** the client is connecting to the server */
	CMM_CONNECTION_STATE_CONNECTING,

		/** The client is connected to the server */
	CMM_CONNECTION_STATE_CONNECTED,

		/** The number of different connection states */
	CMM_NUM_CONNECTION_STATES
} cmm_connection_state_t;


enum
{
	CMM_CLIENT_EVENT_INDEX_REQUEST_COMPLETED,
	CMM_CLIENT_EVENT_INDEX_CONNECTION_CHANGED,
	CMM_CLIENT_EVENT_INDEX_PROCESS_CHANGED,
	CMM_CLIENT_EVENT_INDEX_OPTIONS_CHANGED,
	CMM_CLIENT_EVENT_INDEX_STATE_CHANGED,
	CMM_NUM_CLIENT_EVENTS
};

enum
{
		/** An event flag indicating that the last request has been completed */
	CMM_CLIENT_EVENT_FLAG_REQUEST_COMPLETED  = (1 << CMM_CLIENT_EVENT_INDEX_REQUEST_COMPLETED),

		/** An event flag indicating that the client's connect state has changed */
	CMM_CLIENT_EVENT_FLAG_CONNECTION_CHANGED = (1 << CMM_CLIENT_EVENT_INDEX_CONNECTION_CHANGED),

		/** An event flag indicating that the client's process information has changed */
	CMM_CLIENT_EVENT_FLAG_PROCESS_CHANGED    = (1 << CMM_CLIENT_EVENT_INDEX_PROCESS_CHANGED),

		/** An event flag indicating that the set of available options has changed */
	CMM_CLIENT_EVENT_FLAG_OPTIONS_CHANGED    = (1 << CMM_CLIENT_EVENT_INDEX_OPTIONS_CHANGED),

		/** An event flag indicating that the manager's current state has changed */
	CMM_CLIENT_EVENT_FLAG_STATE_CHANGED      = (1 << CMM_CLIENT_EVENT_INDEX_STATE_CHANGED)
};

/** A set of flags describing an event */
typedef uint32_t cmm_client_event_flags_t;


/** A description of an event. */
typedef struct
{
		/** Flags representing the effects of this event */
	cmm_client_event_flags_t flags;

		/** The result of the last message, if the event includes a request completion */
	aud_error_t result;

} cmm_client_event_info_t;

/** The maximum number of characters in an interface name, including a trailing zero */
#define CMM_INTERFACE_NAME_LENGTH 64

/** The maximum number of characters in a processes friendly name, including a trailing zero */
#define CMM_PROCESS_NAME_LENGTH 64

enum
{
		/** The system state is unknown. Should not normally be seen */
	CMM_SYSTEM_STATUS_UNKNOWN        = 0x0000, 

		/** Normal operation, an interface is selected and the conmon core is running */
	CMM_SYSTEM_STATUS_READY          = 0x0001, 

		/** The manager is switching from one interface to another and the conmon core is not running */
	CMM_SYSTEM_STATUS_CHANGING       = 0x0002,

		/** The manager's current configuration is invalid and the conmon core is not running */
	CMM_SYSTEM_STATUS_INVALID_CONFIG = 0x0003,

	/** The manager needs to be restarted, as interface specified in config is added or deleted */
	CMM_SYSTEM_STATUS_OPTIONS_CHANGED = 0x0004

};

/** A type representing the overall system status */
typedef uint16_t cmm_system_status_t;

/** A type representing a process connected to the manager */
typedef struct cmm_process cmm_process_t;

/**
 * Get the friendly name of a client process
 *
 * @param process the process object
 */
const char *
cmm_process_get_name
(
	const cmm_process_t * process
);

/**
 * Get the id of a client process
 *
 * @param process the process object
 */
uint32_t
cmm_process_get_id
(
	const cmm_process_t * process
);

/**
 * Is the given client process locking the interface configuration?
 *
 * @param process the process object
 */
aud_bool_t
cmm_process_is_locking_configuration
(
	const cmm_process_t * process
);

/**
 * Are two processes equal?
 *
 * @param p1 the first process object
 * @param p2 the second process object
 */
aud_bool_t
cmm_process_equals
(
	const cmm_process_t * p1,
	const cmm_process_t * p2
);

//----------------------------------------------------------
// Information that is fixed for the life of the client connection
//----------------------------------------------------------

/** A structure representing fixed cmm info */
typedef struct cmm_info cmm_info_t;

/**
 * Get the Conmon Manager version
 *
 * @param info the info object
 * @param version_ptr a pointer to a version object that will be set to the current conmon manager version 
 */
void
cmm_info_get_version
(
	const cmm_info_t * info,
	dante_version_t * version_ptr
);

/**
 * Get the maximum number of Dante networks supported by the manager,
 * ie. 1 for a non-redundant Dante device, 2 for a redundant Dante device
 *
 * @param info the info object
 */
uint16_t
cmm_info_max_dante_networks
(
	const cmm_info_t * info
);

//----------------------------------------------------------
// the set of options currently available to clients
//----------------------------------------------------------

/** A structure representing an ethernet interface */ 
typedef struct cmm_interface cmm_interface_t;

/**
 * Get the name of the ethernet interface
 *
 * @param iface the interface object
 */
const wchar_t *
cmm_interface_get_name
(
	const cmm_interface_t * iface
);

/**
 * Get the mac address of the ethernet interface
 *
 * @param iface the interface object
 */
const uint8_t *
cmm_interface_get_mac_address
(
	const cmm_interface_t * iface
);

/**
 *  Is the interface configured (has a non zero mac address and name)?
 *
 * @param iface the interface object
 */

aud_bool_t 
cmm_interface_is_configured
(
	const cmm_interface_t * iface
);

/** A structure representing the set of available options supported by the conmon manager */
typedef struct cmm_options cmm_options_t;

/**
 * How many candidate interfaces are on the device?
 *
 * @param options the options object
 */
uint16_t
cmm_options_num_interfaces
(
	const cmm_options_t * options
);

/**
 * Get the candidate interface at the given index of the options structure
 *
 * @param options the options object
 * @param index the index of the interface to be returned
 */
const cmm_interface_t *
cmm_options_interface_at_index
(
	const cmm_options_t * options,
	uint16_t index
);

/**
 * Get the candidate interface with the given name
 *
 * @param options the options object
 * @param name the name of the interface to be returned
 */
const cmm_interface_t *
cmm_options_interface_with_name
(
	const cmm_options_t * options,
	const wchar_t * name
);

/**
 * Get the candidate interface with the given MAC address
 *
 * @param options the options object
 * @param mac_address the mac address of the interface to be returned
 */
const cmm_interface_t *
cmm_options_interface_with_mac_address
(
	const cmm_options_t * options,
	const uint8_t * mac_address
);

//----------------------------------------------------------
// CMM Config
//----------------------------------------------------------

/** A structure representing manager configuration */
typedef struct cmm_config cmm_config_t;

/**
 * Allocate a new configuration object. Once the object is no longer
 * required, it must be freed using cmm_config_delete
 */
cmm_config_t *
cmm_config_new(void);

/**
 * Delete a configuration object previously allocated using cmm_config_new.
 *
 * @param config the config object
 */
void
cmm_config_delete
(
	cmm_config_t * config
);

/**
 * Clear a configuration object
 *
 * @param config the config object
 */
void
cmm_config_reset
(
	cmm_config_t * config
);

/**
 * Are two configuration objects equal?
 *
 * @param c1 the first config object
 * @param c2 the second config object
 */
aud_bool_t
cmm_config_equals
(
	const cmm_config_t * c1,
	const cmm_config_t * c2
);

/**
 * Validate a configuration against a conmon manager info structure and options structure.
 *
 * @param config the config object
 * @param info the current info
 * @param options the current options
 */
aud_bool_t
cmm_config_validate
(
	const cmm_config_t * config,
	const cmm_info_t * info,
	const cmm_options_t * options
);

/**
 * how many interfaces have been configured for this configuration object?
 *
 * @param config the config object
 */
uint16_t
cmm_config_num_interfaces
(
	const cmm_config_t * config
);

/**
 * Add an interface to the end of the list of configured interfaces for the configuration object
 *
 * @param config the config object
 * @param options the set of currently available options
 * @param mac_address the mac address of the interface to be added
 */
aud_error_t
cmm_config_add_interface
(
	cmm_config_t * config,
	const cmm_options_t * options,
	const uint8_t * mac_address
);

/**
* Add an interface to the end of the list of configured interfaces for the configuration object
*
* @param config the config object
* @param options the set of currently available options
* @param name the name of the interface to be added
*/
aud_error_t
cmm_config_add_interface_with_name
(
	cmm_config_t * config,
	const cmm_options_t * options,
	const wchar_t * name
);

/**
 * Get the interface at the given index of the configuration object.
 * the index corresponds to the dante interface index. ie. Primary=0, Secondary=1
 *
 * @param config the config object
 * @param dante_index the index of the interface to be returned
 */
const cmm_interface_t *
cmm_config_interface_at_index
(
	const cmm_config_t * config,
	uint16_t dante_index
);
/**
 * Add an empty interface to the end of the list of configured interfaces for the configuration object?
 *
 * @param config the config object
 */

aud_error_t cmm_config_add_null_interface
(
	cmm_config_t * config
);

/**
 * Get the candidate interface with the given MAC address
 *
 * @param config the config object
 * @param mac_address the mac address of the interface to be returned
 */

const cmm_interface_t *
cmm_config_interface_with_mac_address
(
	const cmm_config_t * config,
	const uint8_t * mac_address
);

//----------------------------------------------------------
// CMM State
//----------------------------------------------------------

/** A structure representing the current manager state */
typedef struct cmm_state cmm_state_t;

/**
 * Get the current system status
 *
 * @param state the state object
 */
cmm_system_status_t
cmm_state_get_system_status
(
	const cmm_state_t * state
);

/**
 * Get the current Dante device name
 *
 * @param state the state object
 */
const char *
cmm_state_get_dante_device_name
(
	const cmm_state_t * state
);

/**
 * Get the current manager configuration
 *
 * @param state the state object
 */
const cmm_config_t *
cmm_state_get_current_config
(
	const cmm_state_t * state
);

/**
 * Get the pending manager configuration. If non-null then a change 
 * in configuration is imminent, and the conmon core will shortly be taken down 
 * in order to switch to a new interface.
 *
 * @param state the state object
 */
const cmm_config_t *
cmm_state_get_pending_config
(
	const cmm_state_t * state
);

/**
 * How many client processes are currently connected to the manager?
 *
 * @param state the state object
 */
uint16_t
cmm_state_num_processes
(
	const cmm_state_t * state
);

/**
 * Get the client process at the given index
 *
 * @param state the state object
 * @param index the index of the process to be returned
 */
const cmm_process_t *
cmm_state_process_at_index
(
	const cmm_state_t * state,
	uint16_t index
);

//----------------------------------------------------------
// CMM Client
//----------------------------------------------------------

/** A structure representing a client */
typedef struct cmm_client cmm_client_t;

/** 
 * A type definition for the event handler function 
 *
 * @param client the client generating the event
 * @param event_info a description of the event
 */
typedef void
cmm_client_event_fn
(
	cmm_client_t * client,
	const cmm_client_event_info_t * event_info
);

/**
 * Create a new client with the given env
 *
 * @param dapi a dapi environment object
 */
cmm_client_t * 
cmm_client_new_dapi
(
	dapi_t * dapi
);

/**
 * Create a new client with the given env
 *
 * @param env an env object
 */
cmm_client_t * 
cmm_client_new
(
	aud_env_t * env
);

/**
 * override the default cmm server port.
 * for debugging purposes only.
 */
aud_error_t
cmm_client_set_server_port
(
	cmm_client_t * client,
	uint16_t server_port
);

/**
 * Initialise the client. This should be called before connecting to the manager
 *
 * @param client the cmm client
 * @param process_name the friendly name for this client process
 */
aud_error_t
cmm_client_init
(
	cmm_client_t * client,
	const char * process_name
);

/**
 * Attempt to connect to the manager. Use the event callback to track the
 * connection state and whether the connection request has completed.
 *
 * @param client the cmm client
 */
aud_error_t
cmm_client_connect
(
	cmm_client_t * client
);

/**
 * Disconnect from the manager.
 *
 * @param client the cmm client
 */
aud_error_t
cmm_client_disconnect
(
	cmm_client_t * client
);

/**
 * Terminate the client. This is the reverse of the 'initialise' call
 *
 * @param client the cmm client
 */
void
cmm_client_terminate
(
	cmm_client_t * client
);

/**
 * Delete the client.
 *
 * @param client the cmm client
 */
void
cmm_client_delete
(
	cmm_client_t * client
);

/**
 * Get the user-specified context for this client
 *
 * @param client the cmm client
 */
void *
cmm_client_get_context
(
	cmm_client_t * client
);

/**
 * Set a user context for this client
 *
 * @param client the cmm client
 * @param context the new user context for this object
 */
void
cmm_client_set_context
(
	cmm_client_t * client,
	void * context
);

/**
 * Get the event handler for this client
 *
 * @param client the cmm client
 */
cmm_client_event_fn *
cmm_client_get_event_cb
(
	cmm_client_t * client
);

/**
 * Set an event handler for this client
 *
 * @param client the cmm client
 * @param event_cb the new event callback
 */
void
cmm_client_set_event_cb
(
	cmm_client_t * client,
	cmm_client_event_fn * event_cb
);

/**
 * Get the clients current connection state
 *
 * @param client the cmm client
 */
cmm_connection_state_t
cmm_client_get_connection_state
(
	const cmm_client_t * client
);

/**
 * Does the client currently have a reuqest in progress?
 *
 * @param client the cmm client
 */
aud_bool_t
cmm_client_has_active_request
(
	const cmm_client_t * client
);

/**
 * If the client has a request in progress then cancel it and release the resource.
 *
 * @param client the cmm client
 */
aud_error_t
cmm_client_cancel_active_request
(
	cmm_client_t * client
);

/**
 * Get the client's socket
 *
 * @param client the cmm client
 */
aud_socket_t
cmm_client_get_socket
(
	cmm_client_t * client
);

/**
 * The main processing loop for this client. Should be called whenever
 * the client's socket is ready to read or when the current time is later than
 * last returned value of 'next_action_time'.
 *
 * @param client the cmm client
 * @param next_action_time a pointer to a variable that will be set to the next action time for this client
 */
aud_error_t
cmm_client_process
(
	cmm_client_t * client,
	aud_utime_t * next_action_time
);

/**
 * Get the next action time for the client. The client's process function should be
 * called as soon as possible after the time given, even if there are no messages to read
 * on the client's socket.
 *
 * @param client the cmm client
 * @param next_action_time a pointer to a variable that will be set to the next action time for this client
 */
aud_error_t
cmm_client_get_next_action_time
(
	cmm_client_t * client,
	aud_utime_t * next_action_time
);

/**
 * Get the client's own 'process' information
 *
 * @param client the cmm client
 */
const cmm_process_t *
cmm_client_get_process
(
	const cmm_client_t * client
);

/**
 * Get the current system information. This field is only valid once the client
 * has completed a connection to the server.
 *
 * @param client the cmm client
 */
const cmm_info_t *
cmm_client_get_system_info
(
	const cmm_client_t * client
);

/**
 * Get the current system information. This field is only valid once the client
 * has completed a connection to the server and received the first 'options'
 * event from the server
 *
 * @param client the cmm client
 */
const cmm_options_t *
cmm_client_get_system_options
(
	const cmm_client_t * client
);

/**
 * Get the current system information. This field is only valid once the client
 * has completed a connection to the server and received the first 'state'
 * event from the server
 *
 * @param client the cmm client
 */
const cmm_state_t *
cmm_client_get_system_state
(
	const cmm_client_t * client
);

/**
 * Each client maintains a single config buffer that can be used if needed 
 * for temporary storage, to minimise dynamic alloactions. This is useful
 * when configuring a new interface.
 *
 * @param client the cmm client
 */
cmm_config_t *
cmm_client_get_temp_config
(
	cmm_client_t * client
);

enum
{
	/** A flag indicating that a system configuration object has the 'config' field configured */
	CMM_SYSTEM_CONFIG_FLAG_CONFIG = 0x0001,

	/** A flag indicating that a system configuration object has the 'lock' field configured */
	CMM_SYSTEM_CONFIG_FLAG_LOCK   = 0x0002
};

/** A type representing system configuration flags */
typedef uint16_t cmm_system_config_flags_t;

/** A structure representing a new system configuration */
typedef struct
{
		/** Flags indicating which part(s) of the structure have been configured */
	cmm_system_config_flags_t flags;
		/** A configuration object. Only valid if the CMM_SYSTEM_CONFIG_FLAG_CONFIG flag is set */
	cmm_config_t * config;
		/** A configuration object. Only valid if the CMM_SYSTEM_CONFIG_FLAG_LOCK flag is set */
	aud_bool_t lock;
} cmm_system_config_t;

/**
 * Apply a new system configuration to the manager. The system_config
 * structure should have been configured with a new config and/or a new lock value.
 * This is the primary system configuration function in the CMM API. 
 *
 * @param client the cmm client
 * @param system_config a new system configuration to be sent to the server
 */
aud_error_t
cmm_client_set_system_config
(
	cmm_client_t * client,
	const cmm_system_config_t * system_config
);

void
cmm_client_set_sleeping
(
	cmm_client_t * client,
	aud_bool_t sleeping
);

//----------------------------------------------------------
// Utilities
//----------------------------------------------------------

/**
 * Get a string representing the given system status
 *
 * @param system_status the system status
 */
const char *
cmm_system_status_to_string
(
	cmm_system_status_t system_status
);


#ifdef __cplusplus
}
#endif

#endif
