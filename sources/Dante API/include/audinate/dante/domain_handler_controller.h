/*
 * File     : domain_handler_controller.h
 * Created  : August 2016, February 2017
 * Author   : James Westendorp, Andrew White
 * Synopsis : Domain handling for DAPI clients (controllers)
 *
 * This software is copyright (c) 2016-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */

/**
 * @file domain_handler_controller.h
 * Domain Handler for Standalone DAPI controllers
 */

#ifndef _DANTE_DOMAIN_HANDLER_CONTROLLER_H
#define _DANTE_DOMAIN_HANDLER_CONTROLLER_H

#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#include "dante/dante_domains.h"
#include "dante/dapi_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if DAPI_ENVIRONMENT != DAPI_ENVIRONMENT__STANDALONE
#error DAPI_ENVIRONMENT must be set to DAPI_ENVIRONMENT__STANDALONE to use domain_handler_controller.h
#endif

// Configuration structure for embedded domain handlers, currently just a placeholder
typedef struct dante_domain_handler_config dante_domain_handler_config_t;

/**
 * DAPI Controllers have different behaviours depending on whether they are embedded on a device or running as standalone controllers.
 * Standalone controllers on PC platform require explicit ddm discovery and support user-based authentication with access to potentially multiple domains.
 *   Access control within a domain is based on the role assigned to that user within the current domain
 * Embedded local controllers (controllers that only control the device onwhich they are running) have coarse-grained Read or Read/Write permissions for the local device.
 * Embedded remote controllers (controllers on a device that control other Dante devices in a network) are given access control rights based on the role assigned the the device
 *   and can only control other devices within the current domain.
 */ 

#define DANTE_ROLE_NAME_LENGTH 128

typedef struct dante_domain_info
{
	dante_domain_uuid_t uuid;		//identity of current domain
	dante_access_policy_t access_control_policy_summary;
		// access control policy indicating whether the user is able to control/config devices in the domain
	dante_domain_id_t id;
		// the 16bit short id for the domain
	char name[DANTE_DOMAIN_NAME_LENGTH];
		// the name of the domain
	dante_clock_subdomain_name_t clock_subdomain_name;
		// the name of clocking subdomain for the domain
	char role[DANTE_ROLE_NAME_LENGTH];
		// the user's role with the domain. This field is just a textual label. Applications should not attempt to 
		// determine access control capabilities based on the value of this field.

} dante_domain_info_t;

//----------------------------------------------------------
// Domain Handler Infrastructure
//----------------------------------------------------------

/**
 * Dante domain handler client
*/
typedef struct dante_domain_handler dante_domain_handler_t;

/**
* Get current Dante API handler from the domain handler.
*
* @param handler domain handler
*/
dapi_t *
dante_domain_handler_get_dapi
(
	dante_domain_handler_t * handler
);


//----------------------------------------------------------
// Discovery and Connection
//----------------------------------------------------------

/** Enumeration for the states of the Dante Domain Handler */
typedef enum ddh_state
{
	DDH_STATE_DISABLED,       //!< The domain handler is disabled
	DDH_STATE_DISCOVERING,    //!< The domain handler is attempting to discover a Dante Domain Manager
	DDH_STATE_DISCONNECTED,   //!< The domain handler has discovererd a Dante Domain Manager but is disconnected

	DDH_STATE_IDENTIFYING,    //!< The domain handler is retrieving identity information about the Dante Domain Manager
	DDH_STATE_IDENTIFIED,     //!< The domain handler has identified the Dante Domain Manager

	DDH_STATE_CONNECTING,     //!< The domain handler is connecting to the Dante Domain Manager (without identification)

	// DDH_STATE_THIRD_PARTY_AUTH, // For if/when we support azure, etc
	DDH_STATE_CONNECTED,	  //!< The domain handler is connected to a Dante Domain Manager
	DDH_STATE_ERROR,

	DDH_NUM_STATES
} ddh_state_t;

/** 
 * Get current state of the domain handler.
 *
 * @param handler domain handler
 */
ddh_state_t
dante_domain_handler_get_state
(
	const dante_domain_handler_t * handler
);


/**
 * Initiate the Domain Manager discovery process.
 *
 * @pre handler is in the DISABLED or DISCONNECTED state
 * @post On success, handler is in the DISCOVERING state and any manually configured addresses are removed
 * @param interface_index pointer to the interface index or NULL to use the first active interface.
 * @note some platforms do not support interface selection for discovery,
 *  and the interface parameter will be ignored
 */
aud_error_t
dante_domain_handler_start_discovery
(
	dante_domain_handler_t * handler,
	const uint32_t* interface_index
);


/**
* Stop the Domain Manager discovery process
*
* @pre handler is in the DISCOVERING state
* @post On success, handler is in the DISABLED state
*
*/
aud_error_t
dante_domain_handler_stop_discovery
(
	dante_domain_handler_t * handler
);

/** 
 * Manually configure a domain manager by hostname and port, bypassing domain manager discovery
 *
 * @pre handler is in the DISABLED, DISCOVERING or DISCONNECTED state
 * @post On success, handler is in the DISCONNECTED state
 */
aud_error_t
dante_domain_handler_set_manual_ddm
(
	dante_domain_handler_t * handler,
	const char* hostname,
	uint16_t port
);
	
/**
 * Disable the domain handler
 *
 * @pre handler is in the DISABLED, DISCOVERING or DISCONNECTED state
 * @post On success, handler is in the DISABLED state
 */
aud_error_t
dante_domain_handler_disable
(
	dante_domain_handler_t * handler
);

/**
 * Attempt to identify the Domain Manager
 * @pre handler is in the DISCONNECTED state
 * @post On success, handler is in the IDENTIFYING state
 */
aud_error_t
dante_domain_handler_identify
(
	dante_domain_handler_t * handler
);

/**
 * Authenticate credentials and connect with the identified Domain Manager
 * If invoked from the DISCONNECTED state, the IDENTIFYING / IDENTIFIED states are skipped.
 * @pre handler is in the DISCONNECTED or IDENTIFIED state
 * @post On success, handler is in the CONNECTING state
 */
aud_error_t
dante_domain_handler_connect
(
	dante_domain_handler_t * handler,
	const char* username_utf8,
	const char* password_utf8
);

/**
 * Terminate a connection to the domain manager
 * @pre handler is in the IDENTIFYING, IDENTIFIED, CONNECTING or CONNECTED state.
 * @post handler is in the DISCONNECTED state
 */
aud_error_t
dante_domain_handler_disconnect
(
	dante_domain_handler_t * handler
);

/**
 * Get a fingerprint for the identified DDM.
 *
 * This is a printable null terminated cstring which can be saved and compared
 * to previous occasions when we have connected to the server.
 *
 * Note: Versions of the Domain Manager prior to 1.1 share a common fingerprint.
 * @pre handler is in the IDENTIFIED, CONNECTING or CONNECTED state.
 */
aud_error_t
dante_domain_handler_get_identity
(
	dante_domain_handler_t * handler,
	const char ** identity
);

/**
 * Get DDM host and port when in CONNECTED state
 * @pre handler is in the DISCONNECTED, IDENTIFYING, IDENTIFIED, CONNECTING or CONNECTED state.
 * 
 */
aud_error_t
dante_domain_handler_get_address
(
	dante_domain_handler_t * handler,
	const char ** host,
	uint16_t * port
);


//----------------------------------------------------------
// Current domain
//----------------------------------------------------------

/**
 * Get current domain information from the domain handler
 */
dante_domain_info_t
dante_domain_handler_get_current_domain
(
	const dante_domain_handler_t * handler
);

/**
* Get current domain identity from the domain handler
*/
dante_domain_uuid_t
dante_domain_handler_get_current_domain_uuid
(
	const dante_domain_handler_t * handler
);

/**
 * Set the current domain by name
 * Must be in the CONNECTED state
 */
aud_error_t
dante_domain_handler_set_current_domain_by_name
(
	dante_domain_handler_t * handler,
	const char * name
);

/**
 * Set the current domain by domain id
 * Must be in the CONNECTED state
 */
aud_error_t
dante_domain_handler_set_current_domain_by_id
(
	dante_domain_handler_t * handler,
	dante_domain_id_t id
);


/**
 * Set the current domain by domain UUID
 * Must be in the CONNECTED state
 */
aud_error_t
dante_domain_handler_set_current_domain_by_uuid
(
	dante_domain_handler_t * handler,
	dante_domain_uuid_t uuid
);

//----------------------------------------------------------
// Available domains
//----------------------------------------------------------

/**
 * Gets the number of domains available 
 * @pre handler is in the CONNECTED state
 *
 */
unsigned int
dante_domain_handler_num_available_domains
(
	const dante_domain_handler_t * handler
);

/**
 * Gets the information about the available domain at an index.
 If the domain at a given index is not found, it returns the structure 'dante_domain_info_t' 
 with member values set to zero: Check member 'id' (dante_domain_id_t) for non-zero value.
 */
dante_domain_info_t
dante_domain_handler_available_domain_at_index
(
	const dante_domain_handler_t * handler,
	unsigned int index
);

/**
 * Gets the information about the available domain with the given multi-byte encoded domain name.
 If the domain with a given name is not found, it returns the structure 'dante_domain_info_t'
 with member values set to zero: Check member 'id' (dante_domain_id_t) for non-zero value.
 */
dante_domain_info_t
dante_domain_handler_available_domain_with_name
(
	const dante_domain_handler_t * handler,
	const char * name
);

/**
 * Gets the information about the available domain with the given id.
 If the domain with a given id is not found, it returns the structure 'dante_domain_info_t'
 with member values set to zero: Check member 'id' (dante_domain_id_t) for non-zero value.
 */
dante_domain_info_t
dante_domain_handler_available_domain_with_id
(
	const dante_domain_handler_t * handler,
	dante_domain_id_t id
);

/**
 * Gets the information about the available domain with the given domain id.
 If the domain with a given uuid is not found, it returns the structure 'dante_domain_info_t'
 with member values set to zero: Check member 'id' (dante_domain_id_t) for non-zero value.
 */
dante_domain_info_t
dante_domain_handler_available_domain_with_uuid
(
	const dante_domain_handler_t * handler,
	dante_domain_uuid_t uuid
);

//----------------------------------------------------------
// Access control
//----------------------------------------------------------

/**
 * Gets set of valid capabilities known to the domain handler
 */
/*const dante_capability_mask_t *
dante_domain_handler_get_valid_capability_mask
(
	const dante_domain_handler_t * handler
);*/

/**
 * Check whether the current user has write permissions for the given capability on the given device.
 *
 * If target_name if NULL, the function returns true if the current user has write permissions
 * on any device in the current domain.
 *
 * This function always returns AUD_TRUE when the current domain is ADHOC,
 * and AUD_FALSE when the current domain is NONE.
 */
aud_bool_t
dante_domain_handler_has_write_permission
(
	dante_domain_handler_t * handler,
	dante_capability_t capability,
	const char * target_name
);

//----------------------------------------------------------
// Domain Manager Changes
//----------------------------------------------------------

/** Enumeration for the changes that can be triggered in the dante domain handler */
typedef enum ddh_change_type
{ 
	DDH_CHANGE_TYPE_ERROR,				//!< An error event occurred (asynchronous only)
	DDH_CHANGE_TYPE_STATE,				//!< domain handler state has changed
	DDH_CHANGE_TYPE_CURRENT_DOMAIN,		//!< domain handler current domain has changed
	DDH_CHANGE_TYPE_AVAILABLE_DOMAINS,	//!< the list of available domains (or their parameters) has changed
	DDH_CHANGE_TYPE_COUNT
} ddh_change_type_t;

/**
 * These flags indicate changes in domain handler data.
 * They may be queried synchronously after any dante_domain_handler call.
 * The same flags appear in asynchronous event notifications.
 */
enum
{
	DDH_CHANGE_FLAG_ERROR = (1 << DDH_CHANGE_TYPE_ERROR),
		//!< flag to indicate an error event occurred (asynchronous only)
	DDH_CHANGE_FLAG_STATE = (1 << DDH_CHANGE_TYPE_STATE),
		//!< flag to indicate the domain handler state has changed
	DDH_CHANGE_FLAG_CURRENT_DOMAIN = (1 << DDH_CHANGE_TYPE_CURRENT_DOMAIN),
		//!< flag to indicate the domain handler current domain has changed
	DDH_CHANGE_FLAG_AVAILABLE_DOMAINS = (1 << DDH_CHANGE_TYPE_AVAILABLE_DOMAINS)
	    //!< flag to indicate the list of available domains (or their parameters) has changed

};

/** 
 * A bitmask indicating the set of change types that occurred
 * @see ddh_change_flag_t 
 */
typedef uint32_t ddh_change_flags_t;

/**
 * Check whether the most recent dante_domain_handler function call caused any
 * changes.  Change and event query functions do not modify the flags.
 */
ddh_change_flags_t
dante_domain_handler_get_last_change_flags
(
	const dante_domain_handler_t * handler
);


// Changes structure

/**
 * Structure describing changes that have occurred in the domain handler.ju
 */
typedef struct ddh_changes ddh_changes_t;

/**
 * Get the domain handler associated with this changes structure
 */
dante_domain_handler_t *
ddh_changes_get_domain_handler
(
	const ddh_changes_t * changes
);

/**
 * Get the change flags that describe this change.
 */
ddh_change_flags_t
ddh_changes_get_change_flags
(
	const ddh_changes_t * changes
);

/**
 * Get the error code for this change event.
 * @pre DDH_CHANGE_FLAG_ERROR is set
 */
aud_error_t
ddh_changes_get_error_code
(
	const ddh_changes_t * changes
);


/**
 * Get a description of the changes caused by the last dante_domain_handler
 * operation.
 *
 * @note Currently, this structure only contains a copy of the changes flags
 * accessible via dante_domain_handler_get_last_change_flags().
 */
const ddh_changes_t *
dante_domain_handler_get_last_changes
(
	const dante_domain_handler_t * handler
);


//----------------------------------------------------------
// Domain Manager Change Events
//----------------------------------------------------------

/**
 * Event callback to indicate asynchronous domain manager changes
 */
typedef void
ddh_change_event_fn(const ddh_changes_t * ev);

/**
 * Set a callback for change events in the dante domain handler.
 *
 * This callback is triggered by asynchonous events.  Changes that occur during
 * the executation of dante_domain_handler functions are queried via
 * dante_domain_handler_get_last_change_flags() or dante_domain_handler_get_last_changes().
 */
void
dante_domain_handler_set_event_fn
(
	dante_domain_handler_t * handler,
	ddh_change_event_fn * fn
);

/**
 * Set a context pointer for the domain handler.
 */
void
dante_domain_handler_set_context
(
	dante_domain_handler_t * handler,
	void * context
);

/**
 * Get a context pointer for the domain handler.
 */
void *
dante_domain_handler_get_context
(
	dante_domain_handler_t * handler
);


//----------------------------------------------------------
// Utilities
//----------------------------------------------------------

/**
 * Utilty method to get a string representation of the ddh_state_t enumeration
 * @param state the state
 */
const char * ddh_state_to_string(ddh_state_t state);

/**
 * Utilty method to get a string representation of the ddh_change_type_t enumeration
 * @param type the change type
 */
const char * ddh_change_type_to_string(ddh_change_type_t type);

/**
 * Utilty method to get a string representation of the ddh_change_flags_t bitmask
 * @param flags the change flags
 * @param buf an output buffer to hold the string represenation
 * @param len the size of the output buffer
 */
const char * ddh_change_flags_to_string(ddh_change_flags_t flags, char * buf, size_t len);

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------

#endif //_DANTE_DOMAIN_HANDLER_H
