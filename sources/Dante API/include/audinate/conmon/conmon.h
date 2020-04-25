/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp <james.westendorp@audinate.com>
 * Synopsis : ConMon Client API
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

/**
 * @file conmon.h 
 * ConMon client API definition
 */
#ifndef _CONMON_H
#define _CONMON_H

#include "conmon_general.h"
#ifndef DAPI_FLAT_INCLUDE
#include "dante/dante_domains.h"
#include "dante/dapi_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Client config handles
//----------------------------------------------------------

/**
 * A structure representing configuration options for a single conmon 
 * client. This type is the primary means for controlling a conmon client.
 */
typedef struct conmon_client_config conmon_client_config_t;

/**
 * Create a new config object using default settings. 
 *
 * @param client_name the name the client will take when created using this config
 */
conmon_client_config_t * 
conmon_client_config_new
(
	const char * client_name
);

/**
 * Delete a config object
 *
 * @param config the config object
 */
void
conmon_client_config_delete
(
	conmon_client_config_t * config
);

/**
 * Set the client's name
 *
 * @param client_name the name the client will take when created using this config
 */
void
conmon_client_config_set_name
(
	conmon_client_config_t * config,
	const char * client_name
);

/**
 * Enable or disabled metering channel support (default is disabled)
 *
 * @param config the config object
 * @param enabled a flag indicating whether or not a metering channel endpoint should be created
 *
 * @note legacy clients always enabled the metering channel, regardless of whether the
 *   channel was to be used or not. With the new 'config' object it is now necessary to 
 *   explicitly enable metering support 
 *
 */
void 
conmon_client_config_set_metering_channel_enabled
(
	conmon_client_config_t * config,
	aud_bool_t enabled
);

/**
 * Set the port number to use for the client's metering channel endpoint.
 * The default is to bind to 0, ie. use an ephemeral port.
 *
 * @param config the config object
 * @param port the port number to be used for the metering channel endpoint
 */
void
conmon_client_config_set_metering_channel_port
(
	conmon_client_config_t * config,
	uint16_t port
);

/**
 * Allow metering channel bind failure  (default is disabled)
 *
 * @param config the config object
 * @param enabled a flag indicating whether or not a metering channel bind failure should be allowed
 *
 *
 */
void 
conmon_client_config_allow_metering_channel_init_failure
(
	conmon_client_config_t * config,
	aud_bool_t allow
);

/**
 * Enable or disable serial channel support (default is disabled)
 *
 * @param config the config object
 * @param enabled a flag indicating whether or not a serial channel endpoint should be created
 */
void 
conmon_client_config_set_serial_channel_enabled
(
	conmon_client_config_t * config,
	aud_bool_t enabled
);

/**
 * Enable or disabled vendor-specific multicast channel support (default is disabled)
 *
 * @param config the config object
 * @param enabled a flag indicating whether or not a vendor-specific multicast channel endpoint should be created
 */
void 
conmon_client_config_set_vendor_broadcast_channel_enabled
(
	conmon_client_config_t * config,
	aud_bool_t enabled
);

/**
 * Set address to use for the client's metering channel endpoint.
 * The value must be a valid multicast address
 *
 * @param config the config object
 * @param address the multicast address to be used for the vendor-specific broadcast channel endpoint
 */ 
void 
conmon_client_config_set_vendor_broadcast_channel_address
(
	conmon_client_config_t * config,
	uint32_t address
);

/**
 * Set the server port to which the client will connect. 
 * For debugging purposes only.
 *
 * @param config the config object
 * @param server_port the server port to which the client will connect. 0 means use default
 */
void
conmon_client_config_set_server_port
(
	conmon_client_config_t * config,
	uint16_t server_port
);

/**
* Set the server address to which the client will connect.
* For debugging purposes only.
*
* @param config the config object
* @param host_ipv4 the server address to which the client will connect.
*/
void
conmon_client_config_set_server_host_ipv4
(
conmon_client_config_t * config,
uint32_t host_ipv4
);

/**
 * Set the UNIX path to which the client will connect.
 * For use on platforms where the default in /tmp isn't appropriate.
 *
 * @param config the config object
 * @param server_path
 */
aud_error_t
conmon_client_config_set_server_unix_path
(
	conmon_client_config_t * config,
	const char * server_path
);

void
conmon_client_config_set_max_metering_packets_per_process
(
	conmon_client_config_t * config,
	unsigned int max_packets_per_process
);

void
conmon_client_config_set_max_serial_packets_per_process
(
	conmon_client_config_t * config,
	unsigned int max_packets_per_process
);

void
conmon_client_config_set_max_vendor_broadcast_packets_per_process
(
	conmon_client_config_t * config,
	unsigned int max_packets_per_process
);


//----------------------------------------------------------
// Client handles
//----------------------------------------------------------

/**
 * A structure representing a single conmon client. This type
 * is the primary means for controlling a conmon client.
 */
typedef struct conmon_client conmon_client_t;

/**
 * Get whether the metering channel is active for this client,
 * @param client conmon client handle
 * @return AUD_TRUE or AUD_FALSE
 */
aud_bool_t 
conmon_client_is_metering_channel_active
(
	 conmon_client_t * client
);

/**
 * Create a new client handle using the given dapi environment
 *
 * This function only creates the handle and memory. Call conmon_client_auto_connect
 * to initiate connection to the local conmon server.
 *
 * @param env Audinate environment.
 * @param config a client config object
 * @param client pointer to store new conmon client handle.
 */
aud_error_t
conmon_client_new_dapi
(
	dapi_t * dapi,
	const conmon_client_config_t * config,
	conmon_client_t ** client
);

/**
 * Create a new client handle using the given configuration object
 *
 * This function only creates the handle and memory. Call conmon_client_auto_connect
 * to initiate connection to the local conmon server.
 *
 * @param env Audinate environment.
 * @param config a client config object
 * @param client pointer to store new conmon client handle.
 */
aud_error_t
conmon_client_new_config
(
	aud_env_t * env,
	const conmon_client_config_t * config,
	conmon_client_t ** client
);

/**
 * Create a new client handle.
 *
 * This function only creates the handle and memory. Call conmon_client_auto_connect
 * to initiate connection to the local conmon server.
 *
 * @param env Audinate environment.
 * @param client pointer to store new conmon client handle.
 * @param client_name a textual name for this client. This value
 *   is primarily used for debugging purposes.
 *
 * @note For legacy reasons, this call always enables the metering channel
 */
aud_error_t
conmon_client_new
(
	aud_env_t * env,
	conmon_client_t ** client,
	const char * client_name
);

/**
 * Delete a client handle and free and resources held by it.
 *
 * If the client is connected it is automatically disconnected.
 *
 * @param client conmon client handle
 */
aud_error_t
conmon_client_delete
(
	conmon_client_t * client
);

/**
 * Get this client's env
 */
aud_env_t *
conmon_client_get_env
(
	conmon_client_t * client
);

/**
 * Sets the context for a client handle.
 *
 * The context is an arbitrary data pointer used to get from a ConMon client
 * object passed in a callback to the rest of the world.  Default context is
 * NULL.
 *
 * @param client conmon client handle
 * @param context data pointer.
 */
void
conmon_client_set_context
(
	conmon_client_t * client,
	void * context
);


/**
 * Gets the context for a client handle.
 *
 * @param client conmon client handle
 */
void *
conmon_client_context
(
	conmon_client_t * client
);

//----------------------------------------------------------
// Conmon client domain information and new event handler mechanism
//----------------------------------------------------------

aud_bool_t
conmon_client_is_domain_changing
(
	const conmon_client_t * client
);

dante_domain_uuid_t
conmon_client_get_domain_uuid
(
	const conmon_client_t * client
);

aud_bool_t
conmon_client_get_ddm_is_connected(const conmon_client_t * client);

typedef enum conmon_client_event_type
{
	CONMON_CLIENT_EVENT_TYPE__DOMAIN_CHANGING,
	CONMON_CLIENT_EVENT_TYPE__DOMAIN_CHANGED,
	CONMON_CLIENT_EVENT_TYPE__DDM_CONNECTION_CHANGED,
	CONMON_CLIENT_EVENT_TYPE__LOCAL_CONNECTION_CHANGED,
	CONMON_CLIENT_EVENT_TYPE__LOCAL_READY_CHANGED,
	CONMON_CLIENT_EVENT_TYPE__REMOTE_READY_CHANGED,
	CONMON_CLIENT_EVENT_TYPE__MAX_BODY_SIZE_CHANGED,
	CONMON_CLIENT_EVENT_TYPE__COUNT
} conmon_client_event_type_t;

enum
{
	CONMON_CLIENT_EVENT_FLAG__DOMAIN_CHANGING = (1 << CONMON_CLIENT_EVENT_TYPE__DOMAIN_CHANGING),
	CONMON_CLIENT_EVENT_FLAG__DOMAIN_CHANGED = (1 << CONMON_CLIENT_EVENT_TYPE__DOMAIN_CHANGED),
	CONMON_CLIENT_EVENT_FLAG__DDM_CONNECTION_CHANGED = (1 << CONMON_CLIENT_EVENT_TYPE__DDM_CONNECTION_CHANGED),
	CONMON_CLIENT_EVENT_FLAG__LOCAL_CONNECTION_CHANGED = (1 << CONMON_CLIENT_EVENT_TYPE__LOCAL_CONNECTION_CHANGED),
	CONMON_CLIENT_EVENT_FLAG__LOCAL_READY_CHANGED = (1 << CONMON_CLIENT_EVENT_TYPE__LOCAL_READY_CHANGED),
	CONMON_CLIENT_EVENT_FLAG__REMOTE_READY_CHANGED = (1 << CONMON_CLIENT_EVENT_TYPE__REMOTE_READY_CHANGED),
	CONMON_CLIENT_EVENT_FLAG__MAX_BODY_SIZE_CHANGED = (1 << CONMON_CLIENT_EVENT_TYPE__MAX_BODY_SIZE_CHANGED),
};

typedef unsigned int conmon_client_event_flags_t;

typedef struct conmon_client_event conmon_client_event_t;

conmon_client_t * conmon_client_event_get_client(const conmon_client_event_t * ev);
conmon_client_event_flags_t conmon_client_event_get_flags(const conmon_client_event_t * ev);

typedef void conmon_client_event_fn(const conmon_client_event_t * ev);

void conmon_client_set_event_callback(conmon_client_t * client, conmon_client_event_fn * fn);

//----------------------------------------------------------
// Client state
//----------------------------------------------------------

/**
 * Current local connection state of a conmon client.
 */
typedef enum conmon_client_state
{
	CONMON_CLIENT_NO_CONNECTION = 0,
		// The client is unconnected
	CONMON_CLIENT_CONNECTING,
		// The client is attempting to connect to the server
	CONMON_CLIENT_CONNECTED,
		// The client has connected to the server
	CONMON_CLIENT_RECONNECT_PENDING,
		// The client is disconnected but will attempt to reconnect.
		// This state is only used if auto_reconnection is enabled
	CONMON_CLIENT_NUM_STATES
} conmon_client_state_t;


/**
 * Return the connection state of a ConMon client.
 *
 * @param client conmon client handle
 *
 * @return The client's state
 */
conmon_client_state_t
conmon_client_state
(
	conmon_client_t * client
);


/**
 * Get the name given to this client.
 *
 * @param client conmon client handle
 *
 * @return The name of this client
 */
const char *
conmon_client_get_client_name
(
	const conmon_client_t * client
);

/**
 * Get the local conmon server's version
 *
 * @note This function is only valid if the client is currently connected.
 *
 * @param client conmon client handle
 *
 * @return The server version, or NULL if the
 *   client is not connected.
 */
const dante_version_t *
conmon_client_get_server_version
(
	const conmon_client_t * client
);

/**
 * Get the local conmon server version build number
 *
 * @note This function is only valid if the client is currently connected.
 *
 * @param client conmon client handle
 *
 * @return The server version build number, or NULL if the
 *   client is not connected.
 */
const dante_version_build_t *
conmon_client_get_server_version_build
(
	const conmon_client_t * client
);


/**
 * Get a description of the network interfaces used by the local conmon server.
 *
 * @note This function is only valid if the client is currently connected.
 *
 * @param client conmon client handle
 *
 * @return The networks on which the server is operating, or NULL if the
 *   client is not connected.
 */
const conmon_networks_t *
conmon_client_get_networks
(
	const conmon_client_t * client
);

/**
 * Get host name used by the local conmon server.
 *
 * @note This function is only valid if the client is currently connected.
 *
 * @param client conmon client handle
 *
 * @return The name used by the conmon server, or NULL if the
 *   client is not connected.
 */
const char *
conmon_client_get_dante_device_name
(
	const conmon_client_t * client
);

/**
 * Get device's DNS domain name. This data is informational only;
 * it is provided by the local conmon server but not otherwise used in any way.
 *
 * @note This function is only valid if the client is currently connected.
 *
 * @param client conmon client handle
 *
 * @return The device's dns domain name, or NULL if the client is not connected.
 */
const char *
conmon_client_get_dns_domain_name
(
	const conmon_client_t * client
);


/**
 * Get local conmon server's instance id.
 *
 * @param client the client handle
 *
 * @note This function is only valid if the client is currently connected.
 *
 * @param client conmon client
 *
 * @return The server's instance id, or NULL if the client is not
 *   connected.
 */
const conmon_instance_id_t *
conmon_client_get_instance_id
(
	conmon_client_t * client
);



//----------------------------------------------------------
// Request tracking
//----------------------------------------------------------

/**
	Identifier for a pending ConMon configuration request.
	
	ConMon operations are generally asynchronous.  The ConMon client sends
	a configuration request to the ConMon server, and some time later the
	server reports whether the request succeeded.  The result is returned
	to the client application via a callback.
	
	A configuration request is "pending" if it's callback has not yet fired.
	ConMon has a limit on the number of callbacks that can be pending
	simultaneously.
	
	There is a limit on the number of requests that can be pending.  Usually,
	a buffer is allocated on 'connect'.  The size of this buffer can be changed
	by calling conmon_client_set_request_limit before calling connect.
	
	Some requests do not require communication with the server and thus will
	not call a callback.  In this case, the request_id will be set to
	CONMON_CLIENT_NULL_REQ_ID.
 */
typedef const void * conmon_client_request_id_t;

/**
 * A constant value indicating a NULL request id. Valid request ids
 * will always have a value other this this.
 */
#define CONMON_CLIENT_NULL_REQ_ID NULL


/**
	Get the number of requests that can be pending.

 	@param client ConMon client handle
 	
 	@return Maximum number of pending requests.
 */
uint32_t
conmon_client_request_limit
(
	const conmon_client_t * client
);


/**
 	Set the number of requests that can be pending.
 	
 	Some platforms may not support setting this value.  Calling this function
 	on a connected or connecting client will have no effect.
 	
 	@param client Hanlde for unconnected ConMon client
 	@param new_limit New value for limit
 	
 	@return The new limit.  This may not be the same as the limit requested.
 */
uint32_t
conmon_client_set_request_limit
(
	conmon_client_t * client,
	uint32_t new_limit
);


/**
	Get the number of requests that are currently pending.

 	@param client ConMon client handle
 	
 	@return Number of pending requests.
 */
uint32_t
conmon_client_requests_pending
(
	const conmon_client_t * client
);


/**
	Generic response callback for a request.
	
	Receiving this indicates that the request is complete and the request
	identifier is no longer valid.
	
	Some requests use custom responses.
	
 	@param client ConMon client handle
 	@param request_id request identifier from request
	@param result The result of the request.  If not AUD_SUCCESS, then
		the request could not be completed.
 */
typedef void
conmon_client_response_fn
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	aud_error_t result
);

/**
 * Cancel a request that is in progress and release the resources.
 * If the request was a 'connect' request then the client will be 
 * automatically disconnected.
 *
 * @param client the conmon client
 * @param request_id the id of the request to be cancelled
 */
void
conmon_client_request_cancel
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id
);

/**
 * Get a request's user context
 * @param client the conmon client
 * @param request_id the id of the request to get context from
 */
void *
conmon_client_request_get_context
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id
);

/**
 *	Set a request's user context
 *	@param client the conmon client
 * @param request_id the id of the request to set context for
 */
void
conmon_client_request_set_context
(
	conmon_client_t * client,
	conmon_client_request_id_t request_id,
	void *context
);

	
//----------------------------------------------------------
// Connection & system change events
//----------------------------------------------------------

/**
 * Callback to be notified when the set of sockets being used by the client has changed.
 *
 * @param client the conmon client
 *
 * @deprecated applications should use DAPI's dante_runtime object for event loop management
 */
typedef void
conmon_client_sockets_changed_fn
(
	conmon_client_t * client
);

/**
 * Set the callback that will fire when the set of sockets being used by the client has changed.
 *
 * @param client the conmon client
 * @param fn the new callback function to use
 *
 * @deprecated applications should use DAPI's dante_runtime object for event loop management
 */
void
conmon_client_set_sockets_changed_callback
(
	conmon_client_t * client,
	conmon_client_sockets_changed_fn * fn
);

/**
 * Callback to be notified when the set of sockets being used by the client has changed.
 *
 * This information is also available via the more general-purpose conmon_client_event_fn and conmon_client_set_event_callback
 *
 * @param client the conmon client
 */
typedef void
conmon_client_connection_state_changed_fn
(
	conmon_client_t * client
);

/**
 * Set the callback that will fire when the client's connect state has changed.
 * tyhis callback is only fired when the client is in auto-connection mode, and only
 * when connection state changes are occur due to asynchronous events (internal timeouts
 * or message from the server), ie. during processing.
 *
 * This information is also available via the more general-purpose conmon_client_event_fn and conmon_client_set_event_callback
 *
 * @param client the conmon client
 * @param fn the new callback function to use
 */
void
conmon_client_set_connection_state_changed_callback
(
	conmon_client_t * client,
	conmon_client_connection_state_changed_fn * fn
);

/**
 * Set the client's server port
 *
 * Only valid when client is not connected.
 */
aud_error_t
conmon_client_set_server_port
(
	conmon_client_t * client,
	uint16_t server_port
);


/**
 * Enable auto-connection for this conmon client. When auto-connection is enabled,
 * the conmon client will assume responsibility for maintaining the connection
 * between the client and server by:
 * - Attempting to connect to the server
 * - Timing out the connection attempt if it takes too long
 * - Maintaining the connection using keepalives 
 * - Automatically disconnecting from the server when the connection has failed
 * - Attempting to re-establish a broken connection at regular intervals
 *
 * In auto-connect mode, the connect request is handled internally by the client. The
 * host application can use the connection_state_changed() callback to track connection state.
 *
 * Auto-connection can only be enabled when in the NO_CONNECTION state. 
 * Auto-connection is disabled by calling disconnect()
 *
 * @note: This function may change both the connection state and the set of client sockets
 */ 
aud_error_t
conmon_client_auto_connect
(
	conmon_client_t * client
);

/**
 * Connect the client to the local conmon server.
 *
 * A successful connect creates a file descriptor.  When data is ready for
 * reading on this file descriptor, conmon_client_process () should be called
 * to allow the client library to handle incoming ConMon messages.
 *
 * This function will return an error if the client is auto-connecting.
 * 
 * @param client unconnected client handle
 * @param result_fn callback for when request completes
 * @param request_id identifier of this request
 *
 * @return AUD_SUCCESS if the client successfully connected or 
 *   one of the audinate platform layer error codes if the
 *   connection failed.
 *
 * @deprecated applications should use auto_connect to allow the client to handle all disconnection and re-connection
 */
aud_error_t
conmon_client_connect
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id
);

/**
 * Get the client's socket. This function returns 
 * the client's primary socket, used to communicate with the server.
 * To access the complete set of sockets use by the client, call conmon_client_get_sockets.
 *
 * @param client conmon client handle
 *
 * @return the client's primary socket
 *
 * @deprecated applications should use DAPI's dante_runtime object for event loop management
 */
aud_socket_t
conmon_client_get_socket
(
	conmon_client_t * client
);

/**
 * Get the complete set of socket (or sockets) used by this client,
 * and add them to the 'sockets' structure provided.
 *
 * @param client conmon client handle
 * @param sockets the set of sockets to which the client's sockets will be added
 *
 * @return AUD_SUCCESS on success, another error code otherwise
 *
 * @deprecated applications should use DAPI's dante_runtime object for event loop management
 */
aud_error_t
conmon_client_get_sockets
(
	conmon_client_t * client,
	dante_sockets_t * sockets
);


/**
 * Disconnect a client from the server.
 *
 * This function tears down the connection to the server but does NOT
 * delete the client handle.
 *
 * Calling this function will disable auto-connection.
 *
 * @param client conmon client handle
 *
 * @note: This function can change both the connection state and the set of client sockets
 */
aud_error_t
conmon_client_disconnect
(
	conmon_client_t * client
);

/**
 * Process any pending ConMon client messages.
 * 
 * This function processes any asynchronous messages that have arrived
 * since the last time this function was called.
 *
 * @param client conmon client handle
 * 
 * @note This function is only valid if the client is currently connected
 *	or connecting.
 *
  * @deprecated applications should use DAPI's dante_runtime object for event loop management
 */
aud_error_t
conmon_client_process
(
	conmon_client_t * client
);

/**
 * Process any pending ConMon client messages.
 * 
 * This function processes any asynchronous messages that have arrived
 * since the last time this function was called.
 *
 * @param client conmon client handle
 * @param sockets a sockets collection whose 'read_fds' set indicates
 *   which of the conmon client's sockets are ready to be read
 * @param next_action_time the absolute timestamp denoting
 *   the time at which the client is scheduled to perform an internal.
 *   If the time is set to 0.0 then no actions are scheduled.
 *
 * @deprecated applications should use DAPI's dante_runtime object for event loop management
 */
aud_error_t
conmon_client_process_sockets
(
	conmon_client_t * client,
	dante_sockets_t * sockets,
	aud_utime_t * next_action_time
);

/**
 * Get the next action time for the client. This is the next time
 * at which the client needs to perform an action even if there are no
 * messages on a socket. The host application should call the process
 * function as soon as possible after this time even if no sockets are
 * ready to recv.
 *
 * @deprecated applications should use DAPI's dante_runtime object for event loop management
 */
aud_error_t
conmon_client_get_next_action_time
(
	conmon_client_t * client,
	aud_utime_t * next_action_time
);

/**
 * A callback function to notify a client that the server's network
 * information has changed in some way
 *
 * @param client conmon client handle
 */
typedef void
conmon_client_handle_networks_changed_fn
(
	conmon_client_t * client
);

/**
 * Set the callback function that fires when the server's
 * network information changes
 *
 * @param client conmon client handle
 * @param fn callback function
 */
void
conmon_client_set_networks_changed_callback
(
	conmon_client_t * client,
	conmon_client_handle_networks_changed_fn * fn
);

/**
 * A callback function to notify a client that the server's host name has changed
 *
 * @param client conmon client handle
 */
typedef void
conmon_client_handle_dante_device_name_changed_fn
(
	conmon_client_t * client
);

/**
 * Set the callback function that fires when the conmon server's 
 * device name is changed
 *
 * @param client conmon client handle
 * @param fn callback function
 */
void
conmon_client_set_dante_device_name_changed_callback
(
	conmon_client_t * client,
	conmon_client_handle_dante_device_name_changed_fn * fn
);

/**
 * A callback function to notify a client that the server's dns domain name has changed
 *
 * @param client conmon client handle
 */
typedef void
conmon_client_handle_dns_domain_name_changed_fn
(
	conmon_client_t * client
);

/**
 * Set the callback function that fires when the conmon server's 
 * dns domain name is changed
 *
 * @param client conmon client handle
 * @param fn callback function
 */
void
conmon_client_set_dns_domain_name_changed_callback
(
	conmon_client_t * client,
	conmon_client_handle_dns_domain_name_changed_fn * fn
);

//----------------------------------------------------------
// Local / Remote setup information
//----------------------------------------------------------

/**
 * Is the client currently in an appropriate state for setting up local configuration (TX registrations) or sending local messages
 */
aud_bool_t
conmon_client_is_local_ready
(
	conmon_client_t * client
);

/**
 * Is the client currently in an appropriate state for setting up remote configuration (RX registrations and subscriptions) or sending remote messages
 * If so, the client can setup remote configuration. If not, the client should wait until such a time 
 */
aud_bool_t
conmon_client_is_remote_ready
(
	conmon_client_t * client
);

//----------------------------------------------------------
// Subscription tracking and subscription change handling
//----------------------------------------------------------

struct conmon_client_subscription;

/**
 * An opaque definition for a conmon client's subscriptions. Functions
 * are provided for obtaining and interrogating a client's subscriptions.
 */
typedef struct conmon_client_subscription conmon_client_subscription_t;

/**
 * Get a subscription handle for the given subscription (channel type / device name)
 *
 * @param client conmon client handle
 * @param channel_type conmon channel type
 * @param device_name name of target device
 */
const conmon_client_subscription_t *
conmon_client_get_subscription
(
	conmon_client_t * client,
	conmon_channel_type_t channel_type,
	const char * device_name
);

/**
 * Get the channel type for the given subscription
 *
 * @param subscription conmon subscription handle
 */
conmon_channel_type_t
conmon_client_subscription_get_channel_type
(
	const conmon_client_subscription_t * subscription
);

/**
 * Get the device name for the given subscription 
 *
 * @param subscription conmon subscription handle
 */
const char *
conmon_client_subscription_get_device_name
(
	const conmon_client_subscription_t * subscription
);

/**
 * Get the device instance id (if known) for the given subscription
 *
 * @param subscription conmon subscription handle
 */
const conmon_instance_id_t *
conmon_client_subscription_get_instance_id
(
	const conmon_client_subscription_t * subscription
);

/**
 * Get the addresses (if known) for the given subscription
 *
 * @param subscription conmon subscription handle
 */
const conmon_endpoint_addresses_t *
conmon_client_subscription_get_addresses
(
	const conmon_client_subscription_t * subscription
);

/**
 * Get the receive status for the given subscription
 *
 * @param subscription conmon subscription handle
 */
conmon_rxstatus_t
conmon_client_subscription_get_rxstatus
(
	const conmon_client_subscription_t * subscription
);

/**
 * Get the available connections for the given subscription.
 * This mask denotes the interfaces on which conmon thinks it
 * can communicate to maintain the network connection
 *
 * @param subscription conmon subscription handle
 */
conmon_network_mask_t
conmon_client_subscription_get_connections_available
(
	const conmon_client_subscription_t * subscription
);

/**
 * Get the active connections for the given subscription.
 * This mask denotes the interfaces on which conmon is
 * actively maintaining the network connection
 *
 * @param subscription conmon subscription handle
 */
conmon_network_mask_t
conmon_client_subscription_get_connections_active
(
	const conmon_client_subscription_t * subscription
);

/**
 * The callback function contains a set of pointers to subscriptions
 * whose state has changed
 *
 * @param client conmon client handle
 * @param num_changes number of changed subscriptions
 * @param changes array of changed subscriptions
 */
typedef void
conmon_client_handle_subscriptions_changed_fn
(
	conmon_client_t * client,
	unsigned int num_changes,
	const conmon_client_subscription_t * const * changes
);

/**
 * Set the callback function to fire when a client's subscription
 * statuses have changed
 *
 * @param client conmon client handle
 * @param fn subscription changed callback
 */
void
conmon_client_set_subscriptions_changed_callback
(
	conmon_client_t * client,
	conmon_client_handle_subscriptions_changed_fn * fn
);

/**
 * Tell the server to subscribe to given channel on the given remote device.
 * This function initiates the subscription process. The subscriptions_changed
 * callback provides asynchronous identification of the current subscription status.
 *
 * @param client the client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param channel_type the channel to which the server should subscribe. The
 *   channel type may be either METERING or STATUS
 * @param device_name the name of the remote device to which the subscription
 *   should be made.
 *
 * @note This function is only valid if the client is currently connected.
 * @note the client does not allocate an internal 'client subscription' object until
 *   a response is received from the server
 */
aud_error_t
conmon_client_subscribe
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	conmon_channel_type_t channel_type,
	const char * device_name
);

/**
 * Tell the server to unsubscribe from the given channel on the given remote device.
 *
 * @param client the client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param channel_type the channel which is to be unsubscribed. The
 *   channel type may be any one of the monitoring channels, eg. metering or status
 * @param device_name the name of the remote device to which the subscription
 *   should be made.
 *
 * @note This function is only valid if the client is currently connected.
 */
aud_error_t
conmon_client_unsubscribe
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	conmon_channel_type_t channel_type,
	const char * device_name
);

/**
 * What is the total number of subscriptions supported by this client?
 * Note that not all subscriptions are in use.
 *
 * @param client the client handle
 */
uint16_t 
conmon_client_max_subscriptions
(
	const conmon_client_t * client
);

/**
 * Get the subscription at the given index.
 * If the subscription at the given index is unused, the
 * function returns NULL.
 *
 * @param client the client handle
 * @param index the index of the subscription to be obtained
 */
const conmon_client_subscription_t *
conmon_client_subscription_at_index
(
	const conmon_client_t * client,
	uint16_t index
);

/**
 * Request that the server accept messages from ALL transmitters. The 
 * server does not actually "subscribe" to each known device in the same way as for
 * per-device subscriptions. Instead, the server switches to
 * to multicast reception for the given channel (if supported) in order to receive
 * messages from all transmitters.
 *
 * If the client has also registered for RX status messages then it will receive
 * ALL messages that arrive on the status channel.
 *
 * Existing or new per-device subscriptions are unaffected by a global
 * subscription.  The server will provide appropriate filtering to deliver a
 * single copy of each message to the client.
 *
 * @param client the client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param channel_type the channel to which the server should subscribe.
 *   Currently, only channel type STATUS is supported.
 *
 * @note This function is only valid if the client is currently connected.
 * @note At present, the STATUS channel is the only channel for which
 *   it is legal to do an 'all' subscription, and only on certain platforms
 */
aud_error_t
conmon_client_subscribe_global
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	conmon_channel_type_t channel_type
);

/**
 * Cancel a previous 'subscribe global' request.
 *
 * This call has no effect on per-device subscriptions.
 *
 * @param client the client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param channel_type the channel to which the server should unsubscribe.
 *   Currently, only channel type STATUS is supported.
 */
aud_error_t
conmon_client_unsubscribe_global
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	conmon_channel_type_t channel_type
);

/**
 * Does this client have a global subscription on the given channel?
 *
 * @param client the client handle
 * @param channel_type the channel to which the server should unsubscribe.
 *   Currently, only channel type STATUS is supported.
 */
aud_bool_t
conmon_client_is_subscribed_global
(
	conmon_client_t * client,
	conmon_channel_type_t channel_type
);

/**
 * The conmon client can provide information about device name/instance
 * id mappings to the host application, if the client has this information available.
 *
 * For clients of lightweight devices, only a client's active subscriptions are searched
 * to find the given information.
 *
 * For clients on heavyweight devices, the server maintains a list of all known
 * devices on the network and provides mapping information to the clients for
 * remote devices for which messages have been received. In order to reduce
 * caching costs when receiving messages from the entire network, the mapping
 * for a particular instance id may only be retained for the scope of the
 * applications callback for that message. That is,
 * conmon_client_device_name_for_instance_id will return a meaningful value (if known)
 * while within the message callback but may not do so once the callback is complete.
 *
 * @param client the client handle
 * @param instance_id instance identifier to query
 *
 * @return device name string for instance identifier (or null on failure)
 *
 * @note the pointer returned remains valid only within the current callback.
 *   Application code that needs to hold onto the returned value should copy it
 *   into memory under the application code's control.
 */
const char *
conmon_client_device_name_for_instance_id
(
	const conmon_client_t * client,
	const conmon_instance_id_t * instance_id
);

/**
 * Return the instance identifier associated with the device name.
 *
 * @param client the client handle
 * @param device_name device name to query
 *
 * @return pointer to instance identifier (or null on failure)
 *
 * @note the pointer returned remains valid only within the current callback.
 *   Application code that needs to hold onto the returned value should copy it
 *   into memory under the application code's control.
 */
const conmon_instance_id_t *
conmon_client_instance_id_for_device_name
(
	const conmon_client_t * client,
	const char * device_name
);


//----------------------------------------------------------
// Control message structures and functions.
//----------------------------------------------------------

/**
 * A callback function to allow processing of incoming conmon control
 * messages.
 *
 * A client must have previously registered a control message callback
 * handler in order to receive control messages
 * 
 * @param client the client handle
 * @param head the head of the incoming message. The head is
 *   an opaque structure with fields available by accessor methods
 * @param body the body of the incoming message. The bdy contains the
 *   vendor-specific payload
 */
typedef void
conmon_client_handle_control_message_fn
(
	conmon_client_t * client,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
);

/**
 * Register to receive (or stop receiving) control messages that are
 * sent to this server.
 *
 * When the server receives a control message, it is passed to each
 * client that has registered an interest in control messages.
 *
 * @param client the client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param fn the function to be called when a control message arrives.
 *   If fn is NULL, the client will no longer receive control messages
 */
aud_error_t
conmon_client_register_control_messages
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	conmon_client_handle_control_message_fn * fn
);

/**
 * Get the control message handler function. 
 *
 * @param client the client handle
 * @param fn_ptr a pointer to a function pointer that will be set to the currently rtegistered callback.
 */
aud_error_t
conmon_client_get_control_messages_cb
(
	conmon_client_t * client,
	conmon_client_handle_control_message_fn ** fn_ptr
);


/**
 * Send a control message to a given device.
 *
 * @param client conmon client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param device_name the conmon device to which the message will be
 *   sent. a NULL device_name indicates that the messages should be sent
 *   to the local device.
 * @param message_class the class of message to be sent.
 * @param vendor_id the vendor_id value for the transmitted message.
 *   Clients on the destination machine can use this value to determine
 *   the type of message and whether or not it is relevant
 * @param body the payload for the message.
 * @param body_size the length (in bytes) of the message payload
 * @param server_timeout if server cannot send message within timeout, will
 *   respond with a failure.  If zero or NULL, the request will timeout if the
 *   control message cannot be sent immediately by the server.
 *
 * @note This function is only valid if the client is currently connected.
 */
aud_error_t
conmon_client_send_control_message
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	const char * device_name,
	conmon_message_class_t message_class,
	const conmon_vendor_id_t * vendor_id,
	const conmon_message_body_t * body,
	uint16_t body_size,
	const aud_utime_t * server_timeout
);


/**
 * Enable or disable reception of control messages from remote devices.
 * Control messages from the local device are still handled.
 * The server disallows reception of messages from any remote device
 * whenever any of its clients have told the server to disable remote reception.
 *
 * @param client conmon client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param remote_control_allowed new state
 */
aud_error_t
conmon_client_set_remote_control_allowed
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	aud_bool_t remote_control_allowed
);


/**
 * Get the maximum message body size supported for sending control messages.
 *
 * This value can change at runtime depending on how the conmon server is
 * connected to the rest of the system.  The MAX_BODY_SIZE_CHANGED event is
 * triggered if the value is changed after the client has connected to the server.
 *
 * @param client conmon client handle
 *
 * @return maximum message body size (in bytes)
 * @return 0 if this information is not available
 */
size_t
conmon_client_control_message_max_body_size
(
	const conmon_client_t * client
);


//----------------------------------------------------------
// Metering messages.
//----------------------------------------------------------

// Metering messages differ from other monitoring messages in that 
// messages are sent directly from client to client, rather than via 
// a server or servers.
//
// Only one client on each device is able to transmit metering messages.
// It must be accepted as transmitter by the server before it is allowed
// to send messages.
// 
// Multiple clients on each device can receive metering messages. Clients
// receive metering addresses from remote devices by registering for Rx metering
// and subscribing to the remote devices of interest.
// A client may also receive metering information from the local transmitter by
// registering for Tx metering messages, provided they are not themselves the transmitting
// client (a client may not send metering messages to itself).
//
// Metering message reception is managed using the standard monitoring message
// functions below.


/**
 * Notify the server that this client wants to be the metering 
 * transmitter client on this device.
 *
 * @param client the conmon client
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param enabled the new value specifying whether or now this client wishes to do tx metering
 *
 * @note: rx metering is automatically enabled by 
 * registering for metering channel messages in either direction
 */
aud_error_t
conmon_client_set_tx_metering_enabled
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	aud_bool_t enabled
);

/**
 * Is tx metering enabled for this client?
 *
 * @param client the conmon client
 * 
 * @return AUD_TRUE if tx metering has been enabled for this client, AUD_FALSE otherwise
 */
aud_bool_t
conmon_client_is_tx_metering_enabled
(
	conmon_client_t * client
);

//----------------------------------------------------------
// Monitoring messages
//----------------------------------------------------------


/**
 * A callback function to allow processing of incoming conmon monitoring
 * messages. All monitoring channel message including metering are handled
 * via this callback
 *
 * A client must have previously registered a monitoring message callback
 * handler in order to receive control messages
 * 
 * @param client the client handle
 * @param channel_type the channel on which the message arrived
 * @param channel_direction the direction the message is going:
 *   TX messages are sent by other clients on this conmon device; RX messages
 *   are received from remote devices
 * @param head the head of the incoming message. The head is
 *   an opaque structure with fields available by accessor methods
 * @param body the body of the incoming message. The body contains the
 *   vendor-specific payload
 */
typedef void 
conmon_client_handle_monitoring_message_fn
(
	conmon_client_t * client,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	const conmon_message_head_t * head,
	const conmon_message_body_t * body
);


/**
 * Register to receive (or stop receiving) monitoring messages on 
 * a given channel and in a given direction.
 * Registration for all monitoring channels including the metering channel
 * are made using this function. A client may register for both inbound
 * (RX) and outbound (TX) messages on a given monitoring channel. Outbound
 * messages are those sent from other clients attached to the same server.
 * Incoming messages are transmitted from a remote device to this device.
 * Incoming messages are only received when this device has subscribed to
 * the given channel on the remote device.
 *
 * @param client the client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param channel_type the monitoring channel in which the client is
 *   interested
 * @param channel_direction the message direction for which the client is
 *   to be registered.
 * @param fn the function to be called when a monitoring message arrives.
 *   If fn is NULL, the client will no longer receive monitoring messages
 *   for the given channel type/direction.
 *
 * @note This function is only valid if the client is currently connected.
 */
aud_error_t
conmon_client_register_monitoring_messages
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	conmon_client_handle_monitoring_message_fn * fn
);

/**
 * Get the monitoring message handler function for the given channel type and direction.
 *
 * @param client the client handle
 * @param channel_type the monitoring channel in which the client is
 *   interested
 * @param channel_direction the message direction for which the client is
 *   to be registered.
 * @param fn_ptr a pointer to a function pointer that will be set to the currently registered callback.
 */
aud_error_t
conmon_client_get_monitoring_messages_cb
(
	conmon_client_t * client,
	conmon_channel_type_t channel_type,
	conmon_channel_direction_t channel_direction,
	conmon_client_handle_monitoring_message_fn ** fn_ptr
);

/**
 * Send a monitoring message on the given channel. All
 * monitoring messages including metering are sent using this function.
 *
 * @param client the client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 * @param channel_type the monitoring channel on which to send the message.
 * @param message_class the class of message to be sent.
 * @param vendor_id the vendor_id value for the transmitted message.
 *   Clients on the destination machine can use this value to determine
 *   the type of message and whether or not it is relevant
 * @param body the payload for the message.
 * @param body_size the length (in bytes) of the message payload
 *
 * @note This function is only valid if the client is currently connected.
 *
 * @note at present, the only valid non-metering monitoring channel is 
 *   CONMON_CHANNEL_TYPE_STATUS. The channel_type parameter is provide for
 *   forwards compatibility
 */
aud_error_t
conmon_client_send_monitoring_message
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id,
	conmon_channel_type_t channel_type,
	conmon_message_class_t message_class,
	const conmon_vendor_id_t * vendor_id,
	const conmon_message_body_t * body,
	uint16_t body_size
);

/**
 * 'ping' the server by sending a no-op request.
 *
 * This function is useful as a means of ensuring that the server is still present
 * @param client the client handle
 * @param result_fn called when result is known
 * @param request_id identifier of request
 */
aud_error_t
conmon_client_send_noop
(
	conmon_client_t * client,
	conmon_client_response_fn * result_fn,
	conmon_client_request_id_t * request_id
);


/**
 * Get the maximum message body size supported for sending monitoring messages.
 *
 * This value can change at runtime depending on how the conmon server is
 * connected to the rest of the system.  The MAX_BODY_SIZE_CHANGED event is
 * triggered if the value is changed after the client has connected to the server.
 *
 * @param client conmon client handle
 *
 * @return maximum message body size (in bytes)
 * @return 0 if this information is not available
 */
size_t
conmon_client_monitoring_message_max_body_size
(
	const conmon_client_t * client
);

//----------------------------------------------------------
// Helper functions
//----------------------------------------------------------

const char *
conmon_client_event_type_to_string
(
	conmon_client_event_type_t type
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
