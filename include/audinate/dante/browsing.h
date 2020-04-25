/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Browsing API types and functions.
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

/**
 * @file browsing.h
 * Browsing API core object and method definitions
 */

#ifndef _DANTE_BROWSING_H
#define _DANTE_BROWSING_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/browsing_types.h"
#include "dante/browsing_legacy.h"
#include "dante/dante_common.h"
#include "dante/dante_domains.h"
#include "dante/dapi_types.h"
#include "conmon/conmon.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The major version number of the browsing API component
 */
#define DB_VERSION_MAJOR  4

/**
 * The minor version number of the browsing API component
 */
#define DB_VERSION_MINOR  0

/**
 * The bugfix version number of the browsing API component
 */
#define DB_VERSION_BUGFIX 0

/**
 * The browsing API version number as a packed 32-bit value
 */
#define DB_VERSION_PACKED \
	((DB_VERSION_MAJOR << 24) | (DB_VERSION_MINOR << 16) | DB_VERSION_BUGFIX)


//----------------------------------------------------------
// Browse Callbacks
//----------------------------------------------------------

/**
 * A callback to indicate that a browse's stored tree has changed
 * in one or more ways.
 *
 * @param browse core browsing object
 */
typedef void
db_browse_network_changed_fn
(
	const db_browse_t * browse
);

/**
 * A callback to indicate that the active socket set has changed for
 * a given browse.
 *
 * @param browse core browsing object
 */
typedef void
db_browse_sockets_changed_fn
(
	const db_browse_t * browse
);

//----------------------------------------------------------
// Main browse functionality
//----------------------------------------------------------

/**
 * initialise a browse config to the default settings
 *
 * @param config the browsing configuration to be initialised
 */
void
db_browse_config_init_defaults
(
	db_browse_config_t * config
);

/**
 * Create a new browse object that will browse the for information types
 * specified by the 'types' parameter. This function prepares the object
 * but does not start browsing.
 *
 * @param dapi A previously created dapi environment object
 * @param types The data types of interest for this browse type
 * @param browse_ptr A pointer that will hold the return browse handle.
 *
 * @return AUD_SUCCESS on success or another error code otherwise.
 */
aud_error_t
db_browse_new_dapi
(
	dapi_t * dapi,
	db_browse_types_t types,
	db_browse_t ** browse_ptr
);


/**
 * Create a new browse object that will browse the for information types
 * specified by the 'types' parameter. This function prepares the object
 * but does not start browsing.
 *
 * @param env A previously created audinate environment object
 * @param types The data types of interest for this browse type
 * @param browse_ptr A pointer that will hold the return browse handle.
 *
 * @return AUD_SUCCESS on success or another error code otherwise.
 */
aud_error_t
db_browse_new
(
	aud_env_t * env,
	db_browse_types_t types,
	db_browse_t ** browse_ptr
);

/**
 * Delete a browse object. Stops the browse actions and frees all
 * the browse's resources.
 *
 * @param browse core browsing object
 */
void
	db_browse_delete
(
db_browse_t * browse
);

/**
 * Start browsing using default settings. This functions begins the browse actions
 * required to discover the information of interest for this browse.
 *
 * @param browse core browsing object
 */
aud_error_t
db_browse_start
(
	db_browse_t * browse
);

/**
 * Start browsing on a specific interface, identified by index.
 * This functions begins the browse actions required to discover 
 * the information of interest for this browse.
 *
 * @param browse core browsing object
 * @param config additional browsing configuration
 */
aud_error_t
db_browse_start_config
(
	db_browse_t * browse,
	const db_browse_config_t * config

);

/**
 * Stop a browse. This function terminates the browse actions.
 *
 * All discovered nodes are invalidated and should be cleared.
 * This function does not trigger the network_changed or node_changed callbacks.
 *
 * @param browse core browsing object
 */
void
db_browse_stop
(
	db_browse_t * browse
);

/**
 * Restart browsing using last used settings.
 * If this is the first call to 'start', use the default settings.
 * If called on running browse, performs an implicit stop before restarting.
 *
 * All discovered nodes are invalidated and should be cleared.
 * This function does not trigger the network_changed or node_changed callbacks.
 *
 * @param browse core browsing object
 */
aud_error_t
db_browse_restart
(
	db_browse_t * browse
);

/**
 * Convert Dante ID64 to subtype, and filter by it.
 *
 * @param browse initialised but un-started browse object
 * @param filter_id Dante ID (manufacturer ID) to filter by
 *
 * Note: This feature requires support from devices being browsed.  Only newer
 * devices support this feature.
 * Currently, this feature is only supported for CONMON_DEVICE browses and
 * filters by manufacturer ID.
 *
 * Note: Call this function once only.  Configuring different filters for
 * different browse types is unsupported.
 */
aud_error_t
db_browse_set_id64_filter
(
	db_browse_t * browse,
	const dante_id64_t * filter_id
);


/**
 * Get the environment that this browse is using
 *
 * @param browse core browsing object
 */
aud_env_t *
db_browse_get_env
(
	const db_browse_t * browse
);

/**
 * Get the minimum number of sockets required for this browse. This value
 * is determined by the number of data types being browsed.
 *
 * @param browse core browsing object
 */
unsigned int
db_browse_get_min_sockets
(
	const db_browse_t * browse
);

/**
 * Get the current maximum number of sockets in use at any time for this
 * browse
 *
 * @param browse core browsing object
 */
unsigned int
db_browse_get_max_sockets
(
	const db_browse_t * browse
);
/**
 * Set the maximum number of sockets that the browse may have active at once.
 *
 * @param browse core browsing object
 * @param max_sockets new max sockets value.  Must be greater than or equal to
 *  the value provided by db_browse_get_min_sockets.
 */
aud_error_t
db_browse_set_max_sockets
(
	db_browse_t * browse,
	unsigned int max_sockets
);

/**
 * Gets the set of currently active sockets for the given browse and appends
 * them to the dr_sockets_t object. The sockets value can be cached until
 * the socket changed callback fires, when the sockets must again be
 * obtained from the browse.
 *
 * @param browse core browsing object
 * @param sockets Pointer to sockets structure to append to.
 */
aud_error_t
db_browse_get_sockets
(
	const db_browse_t * browse, // [IN]
	dante_sockets_t * sockets      // [IN/OUT]
);

/**
 * Get the context currently associated with this browse object
 *
 * @param browse core browsing object
 */
void *
db_browse_get_context
(
	const db_browse_t * browse
);

/**
 * Set the context currently associated with this browse object
 *
 * @param browse core browsing object
 * @param context the new context value for thies browse object
 */
void
db_browse_set_context
(
	db_browse_t * browse,
	void * context
);

/**
 * Get the network tree that this browse has created to store the
 * discovered information.
 *
 * @param browse core browsing object
 */
const db_browse_network_t *
db_browse_get_network
(
	const db_browse_t * browse
);

/**
 * Set the network changed callback function for this browse object.
 * This callback is fired when
 * there is one or more changes to the browse's network tree.
 *
 * @param browse core browsing object
 * @param network_changed callback for network changed event
 */
void
db_browse_set_network_changed_callback
(
	db_browse_t * browse,
	db_browse_network_changed_fn * network_changed
);

/**
 * Get the network changed callback function for this browse object
 *
 * @param browse core browsing object
 */
db_browse_network_changed_fn *
db_browse_get_network_changed_callback
(
	const db_browse_t * browse
);

/**
 * Set the node changed callback function for this browse object.
 * This callback is fired once for each change to the tree
 *
 * @param browse core browsing object
 * @param node_changed callback for node changed event
 */
void
db_browse_set_node_changed_callback
(
	db_browse_t * browse,
	db_browse_node_changed_fn * node_changed
);

/**
 * Get the node changed callback function for this browse object
 *
 * @param browse core browsing object
 */
db_browse_node_changed_fn *
db_browse_get_node_changed_callback
(
	const db_browse_t * browse
);

/**
 * Set the socket change callback for this browse object. This callback
 * is fired when the browse's set of active sockets has changed,
 * indicating that any previously cached sets of sockets are now
 * invalid.
 *
 * @param browse core browsing object
 * @param sockets_changed callback for the sockets changed event
 */
void
db_browse_set_sockets_changed_callback
(
	db_browse_t * browse,
	db_browse_sockets_changed_fn * sockets_changed
);

/**
 * Get the socket change callback for this browse object.
 *
 * @param browse core browsing object
 */
db_browse_sockets_changed_fn *
db_browse_get_sockets_changed_callback
(
	const db_browse_t * browse
);

/**
* The events types that can be signalled by the browsing object.
*/
typedef enum db_browse_event_type
{
	/** The event indicates a domain change has begun */
	DB_BROWSE_EVENT_TYPE_DOMAIN_CHANGING,
	/** The event indicates a domain change has completed */
	DB_BROWSE_EVENT_TYPE_DOMAIN_CHANGED
} db_browse_event_type_t;

/**
 * The browsing api event type
 */
typedef struct db_browse_event db_browse_event_t;

/** Gets the source browsing api object from the event */
db_browse_t * db_browse_event_get_browse(const db_browse_event_t * ev);

/** Gets the event type from the event */
db_browse_event_type_t db_browse_event_get_type(const db_browse_event_t * ev);

/**
 * The browsing api event callback function prototype
 */
typedef void db_browse_event_fn(db_browse_event_t * ev);

/**
* Sets the callback function to handle events frrom the browsing api
*/
void db_browse_set_event_callback(db_browse_t * browse, db_browse_event_fn * fn);

/**
 * The main browsing processing function. This function should
 * be called after select() has been called on the browse's active
 * sockets and one or more of the sockets may have been marked as
 * ready to read. This function will process all the sockets that
 * are ready and update its tree accordingly. Both the network change
 * and socket change callbacks may fire as a result of calling this
 * function.
 *
 * @param browse the browsing object to be processed
 * @param sockets a sockets collection whose 'read_fds' set indicates
 *   which browsing sockets are ready to be read
 * @param next_action_time the absolute timestamp denoting
 *   the time at which the next attempted resolve will timeout. This
 *   value indicates the next point in time at which the process
 *   function will have actions to perform regardless of whether any
 *   of its sockets are ready to read.
 *
 *
 */
aud_error_t
db_browse_process
(
	db_browse_t * browse,
	dante_sockets_t * sockets,
	aud_utime_t * next_action_time
);

/**
 * Gets the uuid for the domain which is being browsed by the given browse object
 * @param browse the browsing object
 */
dante_domain_uuid_t
db_browse_get_domain_uuid
(
	const db_browse_t * browse
);

/**
 * How many networks (ie non-localhost interfaces) are being browsed by the given browse object?
 *
 * @param browse the browsing object
 */
unsigned int
db_browse_num_interface_indexes
(
	const db_browse_t * browse
);

/**
 * What is the interface index for the given browse object?
 *
 * @param browse the browsing object
 * @param index the index of the network (ie non-localhost interface) for which an interface index is desired
 */
unsigned int
db_browse_interface_index_at_index
(
	const db_browse_t * browse,
	unsigned int index
);

/**
 * Is localhost browsing in use for the given browse object?
 *
 * @param browse the browsing object
 */
aud_bool_t
db_browse_using_localhost
(
	const db_browse_t * browse
);

/**
 * Prod the browsing API to attempt to rediscover missing devices
 *
 * @param browse the browsing object
 * @param browse_types the browse types for which rediscovery is required.
 *
 * Note: rediscover is the opposite of reconfirm.  Reconfirm checks whether existing
 * discovered devices should be removed.  Rediscover attempts to find devices that
 * were missed (due to system load or other issues).
 *
 * This function will force all devices to re-announce themselves.  It is useful
 * if devices announcements may have been missed / dropped due to heavy network
 * load.  This function itself will cause network load for all devices so should
 * be called sparingly.
 *
 * @note This is only applicable when browsing in the adhoc domain.  AUD_ERR_NOTSUPPORTED is returned if the browse is not browsing in the adhoc domain.
 */
aud_error_t
db_browse_rediscover
(
	db_browse_t * browse,
	db_browse_types_t browse_types
);

/**
 * get the device name from node
 *
 * @param browse the browsing object
 * @param node
 */
const char *
db_browse_get_node_device_name
(
    const db_node_t * node
);


//----------------------------------------------------------
// Miscellany
//----------------------------------------------------------

/**
 * Set delay before initiating browse when switching from managed domain to ad-hoc
 *
 * @param browse the browsing object
 * @param delay_s startup delay in seconds
 *
 * On some embedded platforms, the browsing API can stall while entering ad-hoc
 * browsing due to contention between browsing and Dante system components for
 * the mDNSResponder interface.  This call adds an artificial delay to ad-hoc
 * browsing startup (when switching from a managed domain) to provide time for
 * the system to stabilise.
 *
 * This function may be called at any time after the browsing object is created.
 * It applies to any subsequent managed -> adhoc transitions.
 *
 * The default value is 0 seconds.
 */
void
db_browse_set_adhoc_startup_delay
(
	db_browse_t * browse,
	uint32_t delay_s
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif


#endif
