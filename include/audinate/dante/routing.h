/*
 * File     : $RCSfile$
 * Created  : January 2007
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Dante Routing Configuration API
 *
 * This software is copyright (c) 2008-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */
/**
 * @file routing.h
 * Routing API generic types and definitions
 */
#ifndef _DANTE_ROUTING_H
#define _DANTE_ROUTING_H

//----------------------------------------------------------
// Basic Types
//----------------------------------------------------------

#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"

#include "dante/dante_common.h"
#include "dante/dante_domains.h"
#include "dante/dapi_types.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** The major version number for the Routing API */
#define DR_VERSION_MAJOR  4
/** The minor version number for the Routing API */
#define DR_VERSION_MINOR  0
/** The bugfix version number for the Routing API */
#define DR_VERSION_BUGFIX 0

/** The complete version of the Routing API as a packed 32-bit value */
#define DR_VERSION_PACKED \
	((DR_VERSION_MAJOR << 24) | (DR_VERSION_MINOR << 16) | DR_VERSION_BUGFIX)


/**
 * Routing-specific result codes above and beyond those in the platform layer
 */

  /** The passed handle never existed or has been released. */
#define DR_ERR_INVALID_HANDLE       0x0100

  /** The limit of available handles has been reached. */
#define DR_ERR_NO_MORE_HANDLES      0x0101

  /** The device's capabilities have changed; the device handle must be closed and re-opened */
#define DR_ERR_CAPABILITIES_CHANGED 0x0140

  /** Tried to create a label matching the channel's own canonical name */
#define DR_ERR_OWN_CANONICAL_NAME   0x0150
	
  /** Tried to create a label matching another channel's own canonical name */
#define DR_ERR_OTHER_CANONICAL_NAME 0x0151
	
  /** The label exists somewhere on the device */
#define DR_ERR_LABEL_EXISTS         0x0152
	
  /** The label doesn't exist on the device */
#define DR_ERR_LABEL_DOESNT_EXIST   0x0153
	
/**
 * Dante devices consist of several components that can generally
 * be treated independently for actions such as updating state.
 */
typedef enum
{
		/** The device's transmit channels */
	DR_DEVICE_COMPONENT_TXCHANNELS,
		/** The device's receive channels */
	DR_DEVICE_COMPONENT_RXCHANNELS,
		/** The device's labels */
	DR_DEVICE_COMPONENT_TXLABELS,
		/** The device's transmit flows  */
	DR_DEVICE_COMPONENT_TXFLOWS,
		/** The device's receive flows  */
	DR_DEVICE_COMPONENT_RXFLOWS,
		/** 
		 * The device's properties. This includes:
		 * - current rx latency
		 * - maximum rx latency
		 * - legal unicast receive port range for manually-configured receive flows
		 */
	DR_DEVICE_COMPONENT_PROPERTIES,
	
		/** The total number of device components */
	DR_DEVICE_COMPONENT_COUNT
} dr_device_component_t;

/**
 * Device changes consist of component changes and non-component changes.
 * Non-component change indexes follow on from component change indexes.
 * These indexes are used to derive the change flag bitmasks.
 *
 * These indexes may change value in future if additional components are added.
 */
enum
{
	  /** An index for changes to a device's name */
	DR_DEVICE_CHANGE_INDEX_NAME = DR_DEVICE_COMPONENT_COUNT,
	  /** An index for changes to a device's state */
	DR_DEVICE_CHANGE_INDEX_STATE,
	  /** An index for changes to a device's stale settings */
	DR_DEVICE_CHANGE_INDEX_STALE,
	  /** An index for changes to a device's error states */
	DR_DEVICE_CHANGE_INDEX_STATUS,
	  /** An index for changes to a device's addresses */
	DR_DEVICE_CHANGE_INDEX_ADDRESSES,

	  /** An index for changes to a device's rxflow error flags */
	DR_DEVICE_CHANGE_INDEX_RXFLOW_ERROR_FLAGS,

	  /** An index for changes to a device's rxflow error fields */
	DR_DEVICE_CHANGE_INDEX_RXFLOW_EARLY_PACKETS,

	  /** An index for changes to a device's rxflow error fields */
	DR_DEVICE_CHANGE_INDEX_RXFLOW_LATE_PACKETS,

	  /** An index for changes to a device's rxflow error fields */
	DR_DEVICE_CHANGE_INDEX_RXFLOW_DROPPED_PACKETS,

	  /** An index for changes to a device's rxflow error fields */
	DR_DEVICE_CHANGE_INDEX_RXFLOW_OUT_OF_ORDER_PACKETS,

	  /** An index for changes to a device's rxflow error fields */
	DR_DEVICE_CHANGE_INDEX_RXFLOW_MAX_LATENCY,

      /** The total number of change indexes */
	DR_DEVICE_CHANGE_INDEX_COUNT
};

/** The change index type */
typedef uint32_t dr_device_change_index_t;

/**
 * Device change notifications use bits in a bitfield
 * to indicate which parts of a device have changed
 */
enum
{
		/** The device's transmit channels */
	DR_DEVICE_CHANGE_FLAG_TXCHANNELS = (1 << DR_DEVICE_COMPONENT_TXCHANNELS),
		/** The device's receive channels */
	DR_DEVICE_CHANGE_FLAG_RXCHANNELS = (1 << DR_DEVICE_COMPONENT_RXCHANNELS),
		/** The device's labels */
	DR_DEVICE_CHANGE_FLAG_TXLABELS = (1 << DR_DEVICE_COMPONENT_TXLABELS),
		/** The device's transmit flows  */
	DR_DEVICE_CHANGE_FLAG_TXFLOWS = (1 << DR_DEVICE_COMPONENT_TXFLOWS),
		/** The device's receive flows  */
	DR_DEVICE_CHANGE_FLAG_RXFLOWS = (1 << DR_DEVICE_COMPONENT_RXFLOWS),
		/** The device's properties  */
	DR_DEVICE_CHANGE_FLAG_PROPERTIES = (1 << DR_DEVICE_COMPONENT_PROPERTIES),

		/** The device's name */
	DR_DEVICE_CHANGE_FLAG_NAME = (1 << DR_DEVICE_CHANGE_INDEX_NAME),

		/** The device's state */
	DR_DEVICE_CHANGE_FLAG_STATE = (1 << (DR_DEVICE_CHANGE_INDEX_STATE)),

		/** One or more of the device's stale flags */
	DR_DEVICE_CHANGE_FLAG_STALE = (1 << (DR_DEVICE_CHANGE_INDEX_STALE)),

		/** One or more of the device's status flags */
	DR_DEVICE_CHANGE_FLAG_STATUS = (1 << (DR_DEVICE_CHANGE_INDEX_STATUS)),

		/** The device's addresses have changed */
	DR_DEVICE_CHANGE_FLAG_ADDRESSES = (1 << (DR_DEVICE_CHANGE_INDEX_ADDRESSES)),

		/** The device's rxflow error flags have changed  */
	DR_DEVICE_CHANGE_FLAG_RXFLOW_ERROR_FLAGS = (1 << (DR_DEVICE_CHANGE_INDEX_RXFLOW_ERROR_FLAGS)),

		/** The device's rxflow early packet counters have changed  */
	DR_DEVICE_CHANGE_FLAG_RXFLOW_EARLY_PACKETS = (1 << (DR_DEVICE_CHANGE_INDEX_RXFLOW_EARLY_PACKETS)),

		/** The device's rxflow late packet counters have changed  */
	DR_DEVICE_CHANGE_FLAG_RXFLOW_LATE_PACKETS = (1 << (DR_DEVICE_CHANGE_INDEX_RXFLOW_LATE_PACKETS)),

		/** The device's rxflow early packet counters have changed  */
	DR_DEVICE_CHANGE_FLAG_RXFLOW_DROPPED_PACKETS = (1 << (DR_DEVICE_CHANGE_INDEX_RXFLOW_DROPPED_PACKETS)),

		/** The device's rxflow early packet counters have changed  */
	DR_DEVICE_CHANGE_FLAG_RXFLOW_OUT_OF_ORDER_PACKETS = (1 << (DR_DEVICE_CHANGE_INDEX_RXFLOW_OUT_OF_ORDER_PACKETS)),

		/** The device's rxflow max latency has changed  */
	DR_DEVICE_CHANGE_FLAG_RXFLOW_MAX_LATENCY = (1 << (DR_DEVICE_CHANGE_INDEX_RXFLOW_MAX_LATENCY)),
};

/** The type used for device change flags */
typedef uint32_t dr_device_change_flags_t;

/**
 * Dante devices have several states. Remote devices are identified by
 * a name that must be resolved in order to obtain a network address.
 * Connections to local devices do not require resolution and may be
 * queried immediately.
 *
 * Most of the Routing API functionality requires that the device is
 * in the 'ACTIVE' state, ie. the device's capabilities are known.
 */
typedef enum
{
		/** The device is about to be deleted, it must not be used at all.
		   Deleted devices do not fire any events. */
	DR_DEVICE_STATE_DELETING,
		/** The device has entered a state from which it can not recover.
		    It must be closed. */
	DR_DEVICE_STATE_ERROR,
		/** The device is attempting to resolve an IP address from mDNS **/
	DR_DEVICE_STATE_RESOLVING,
		/** The device has an address but its capabilities are unknown **/
	DR_DEVICE_STATE_RESOLVED,
		/** The device is querying its capabilities but no response has
		    yet been received */
	DR_DEVICE_STATE_QUERYING,
		/** The device's capabilities are known and it is now fully
		    functional */
	DR_DEVICE_STATE_ACTIVE
} dr_device_state_t;

enum
{
		/** An index marking a device as having a name conflict */
	DR_DEVICE_STATUS_INDEX_NAME_CONFLICT,
		/** An index marking a device as being unlicensed */
	DR_DEVICE_STATUS_INDEX_UNLICENSED,
		/** An index marking a device as being in lockdown mode */
	DR_DEVICE_STATUS_INDEX_LOCKDOWN,
	DR_DEVICE_STATUS_COUNT
};

enum
{
		/** A flag marking a device as having a name conflict.
			A device with a name conflict has its routing
			functionality disabled until it is renamed */
	DR_DEVICE_STATUS_FLAG_NAME_CONFLICT = (1 << DR_DEVICE_STATUS_INDEX_NAME_CONFLICT),
		/** A flag marking a device as being unlicensed.
			Unlicensed devices have no routing functionality. */
	DR_DEVICE_STATUS_FLAG_UNLICENSED    = (1 << DR_DEVICE_STATUS_INDEX_UNLICENSED),
		/** A flag marking a device as being in lockdown mode.
			Devices in lockdown are only accessible via localhost. 
			This flag is only meaningful for local connections */
	DR_DEVICE_STATUS_FLAG_LOCKDOWN    = (1 << DR_DEVICE_STATUS_INDEX_LOCKDOWN),

};

/**
 * A set of flags indicating the overall functionality of the device.
 * A device's error state flags are determined when querying its capabilities. 
 */
typedef uint32_t dr_device_status_flags_t;

/**
 * A flag indicating that a label can be moved
 */
#define DR_MOVEFLAG_MOVE_EXISTING  0x0001

/**
 * A type used for controlling how labels should be managed during
 * creation and movement.
 */
typedef uint16_t dr_moveflag_t;

/**
 * A type for audio latencies (in microseconds)
 */
typedef uint32_t dr_latency_us_t;

struct dr_handle;
/**
 * Dante Routing uses handles in some places to provide a safer interface
 * to the dynamically changing data structures stored internally
 */
typedef struct dr_handle dr_handle_t;

struct dr_device;
/**
 * Each "Device" represents a handle to a Dante device.
 * This is the main handle into the library functionality - all channels
 * are linked to a specific device handle. When a device handle is freed,
 * all its associated channels are also freed.
 */
typedef struct dr_device dr_device_t;

struct dr_txchannel;
/**
 * A specific tx channel for a device. Each device has 
 * 0 or more tx channels; these structures are fixed for the 
 * life of the device handle. That is, closing a device
 * invalidates all the tx channel pointers that were associated
 * the device.
 */
typedef struct dr_txchannel dr_txchannel_t;

struct dr_rxchannel;
/**
 * A specific rx channel for a device. Each device has 
 * 0 or more rx channels; these structures are fixed for the 
 * life of the device handle.  That is, closing a device
 * invalidates all the tx channel pointers that were associated
 * the device.
 */
typedef struct dr_rxchannel dr_rxchannel_t;

/**
 * A 'label' is a custom name associated with a TX channel.
 * A label consists of a texutal name and the tx channel with which it is associated.
 */
typedef struct
{
	  /** The tx channel with which this channel is associated */
	dr_txchannel_t * tx;
	  /** The resource id for this label */
	dante_id_t id;
	  /** The textual name for this label */
	dante_name_t name;	
} dr_txlabel_t;


/**
 * A handle for accessing a device's TX flow information.
 * Each tx flow is accessed via a handle, as flow information
 * can change dynamically.
 */
typedef dr_handle_t dr_txflow_t;

/**
 * A handle for accessing a device's RX flow information.
 * Each rx flow is accessed via a handle, as flow information
 * can change dynamically.
 */
typedef dr_handle_t dr_rxflow_t;

/**
 * The routing API allows concurrent connections to multiple Dante devices.
 * Some of the resources required for device communications are shared
 * between devices. The dr_devices_t type manages the shared resources.
 */
typedef struct dr_devices dr_devices_t;

/**
 * A callback that the host application may use to be notified when
 * aspects of the device have changed.
 */
typedef void
dr_device_changed_fn
(
	dr_device_t * device,
	dr_device_change_flags_t change_flags
);

//----------------------------------------------------------
// Socket support
//----------------------------------------------------------


/**
 * A callback function indicating that the set of open sockets has changed
 * for the devices associated with the given 'devices' structure. If the
 * host application has previously cached the current set of sockets, the
 * cache must be updated.
 */
typedef void
dr_devices_sockets_changed_fn
(
	const dr_devices_t * routing
);

//----------------------------------------------------------
// Devices
//----------------------------------------------------------

/**
 * Create a new devices structure using the given dapi environment.
 *
 * @param dapi the dapi environment for this devices structure
 * @param devices_ptr a pointer to a location to store a new devices object pointer
 *
 * @return AUD_SUCCESS if a devices structure was successfully created
 *   or an error code othrwise
 */
aud_error_t
dr_devices_new_dapi
(
	dapi_t * dapi,
	dr_devices_t ** devices_ptr
);

/**
 * Create a new devices structure using the given environment.
 *
 * @param env the environment for this devices structure
 * @param devices_ptr a pointer to a location to store a new devices object pointer
 *
 * @return AUD_SUCCESS if a devices structure was successfully created
 *   or an error code othrwise
 */
aud_error_t
dr_devices_new
(
	aud_env_t * env,
	dr_devices_t ** devices_ptr
);

/**
 * Delete a "dr_devices_t" structure. A devices structure cannot be
 * deleted unless each device associated with the devices structure
 * has been closed.
 *
 * @param devices the devices object
 */
aud_error_t
dr_devices_delete
(
	dr_devices_t * devices
);

/**
 * Set the number of handles available for this devices structure.
 * Handles are required for certain actions in the routing API such
 * as flow configuration, to avoid dangling pointers for objects
 * that may be dynamically removed. The number of handles determines the maximum
 * number of handles available at one time.
 * 
 * @param devices the devices object
 * @param handles the new maximum number of handles for the devices object
 *
 * @note Handles are required for TX flows and RX flows. Device references are NOT
 *   handles in the sense used here.
 *
 * @note This function can only be called when there are no handles currently allocated. 
 */
aud_error_t
dr_devices_set_num_handles
(
	dr_devices_t * devices,
	unsigned int handles
);

/**
 * Get the total number of handles available for this devices structure. 
 * 
 * @param devices the devices object
 */
unsigned int 
dr_devices_get_num_handles
(
	dr_devices_t * devices
);

/**
 * Get the context value associated with the devices structure
 *
 * @param devices the devices object
 */
void *
dr_devices_get_context
(
	const dr_devices_t * devices
);

/**
 * Set the context associated with the devices structure
 *
 * @param devices the devices object
 * @param context the new context for this device
 */
void
dr_devices_set_context
(
	dr_devices_t * devices,
	void * context
);

/** 
 * Get the number of requests that may be active at one time. Each
 * asynchronous action requires a 'request'. These reqeuests are shared
 * between each devices associated with this devices structure.
 *
 * @param devices the devices object
 */
uint32_t
dr_devices_get_request_limit
(
	const dr_devices_t * devices
);


/** 
 * Set the number of requests that may be active at one time. Each
 * asynchronous action requires a 'request'. These reqeuests are
 * shared between each devices associated with this devices structure. 
 *
 * @param devices the devices object
 * @param new_limit the num maximum number of requests for this devices object
 */
uint32_t
dr_devices_set_request_limit
(
	dr_devices_t * devices,
	uint32_t new_limit
);

/**
 * How many requests are current pending for this devices structure?
 *
 * @param devices the devices object
 */
uint32_t
dr_devices_num_requests_pending
(
	const dr_devices_t * devices
);

/**
 * Set context on a request
 * @param devices the devices object
 * @param req the request
 * @param context arbitrary pointer, which will never be dereferenced
 */
void
dr_devices_set_request_context
(
	dr_devices_t * devices,
	dante_request_id_t req,
	void *context
);

/**
 * Get context from a request
 * @param devices the devices object
 * @param req the request
 * @return pointer, set by dr_devices_set_request_context
 */
void *
dr_devices_get_request_context
(
	dr_devices_t * devices,
	dante_request_id_t req
);

	
/**
 * Set the socket change callback for this function. This callback is
 * fired each time the set of active sockets changes for this devices
 * structure and each associated device.
 *
 * @param devices the devices object
 * @param sockets_changed the new callback for socket change events
 */
void
dr_devices_set_sockets_changed_callback
(
	dr_devices_t * devices,
	dr_devices_sockets_changed_fn * sockets_changed
);

/**
 * Get the socket change callback for this function.
 *
 * @param devices the devices object
 */
dr_devices_sockets_changed_fn *
dr_devices_get_sockets_changed_callback
(
	const dr_devices_t * devices
);

/**
 * Get the currently active sockets used by this devices structure and
 * each associated device. This set of sockets can be assumed constant
 * until the 'devices_sockets_changed' callback fired. Each time the
 * callback fires, the new set of sockets should be obtained.
 *
 * @param devices the devices object
 * @param sockets a pointer to a sockets structure that will be 
 *   updated to include all active sockets for this devices structure.
 */
aud_error_t
dr_devices_get_sockets
(
	const dr_devices_t * devices,
	dante_sockets_t * sockets
);

/**
 * Process any pending messages for this devices structure and each
 * associated device.
 *
 * @param devices the devices object
 * @param sockets the set of sockets that have been marked as ready
 *   for reading as a result of calling 'select' with the set of
 *   currently active sockets. The library clears sockets from the set once processed
 */
aud_error_t
dr_devices_process
(
	dr_devices_t * devices,
	dante_sockets_t * sockets
);

//----------------------------------------------------------
// Domains
//----------------------------------------------------------

dante_domain_uuid_t
dr_devices_get_domain_uuid
(
	const dr_devices_t * devices
);


typedef enum dr_devices_event_type
{
	DR_DEVICES_EVENT_TYPE_DOMAIN_CHANGING,
	DR_DEVICES_EVENT_TYPE_DOMAIN_CHANGED
} dr_devices_event_type_t;

typedef struct dr_devices_event dr_devices_event_t;

dr_devices_t * dr_devices_event_get_devices(const dr_devices_event_t * ev);
dr_devices_event_type_t dr_devices_event_get_type(const dr_devices_event_t * ev);

typedef void dr_devices_event_fn(dr_devices_event_t * ev);

void dr_devices_set_event_callback(dr_devices_t * devices, dr_devices_event_fn * fn);

//----------------------------------------------------------
// Dante Device Handles
//----------------------------------------------------------

/**
 * Create a device object that communicates with the local dante device.
 * No resolution phase is necessary when connecting to the local dante
 * device so the device begins in the RESOLVED state.
 *
 * This mechanism is significantly faster than dr_device_open_remote when
 * communicating with a local dante device.
 *
 * @param devices the devices object
 * @param device_ptr a pointer to a location to hold the new device object pointer.
 */
aud_error_t 
dr_device_open_local
(
	dr_devices_t * devices,
	dr_device_t ** device_ptr
);

/**
 * Currently used for debugging purposes only. Do not rely on the ongoing existence of this function
 */
aud_error_t 
dr_device_open_local_on_port
(
	dr_devices_t * devices,
	uint16_t port,
	dr_device_t ** device_ptr
);


// Open remote Dante device with config

//! configuration structure for dr_device_open_with_config
typedef struct dr_device_open dr_device_open_t;

/**
	Create a device object that communicates with a remote dante device.

	Under normal operation, the device object automatically begins a resolution
	phase to find the remote device's address and port, with the device object
	in the RESOLVING state.  Communication with the remote device is possible
	once the resolution phase is complete (i.e. the device is in the RESOLVED
	state).

	By default, discovery occurs on all interfaces and a single endpoint is used.
	To restrict discovery or for redundant operation, config interfaces using
	dr_device_open_config_enable_interface functions.

	For debugging, config may alternatively specify an address and port for
	each dante network.

	@param devices the devices object.
	@param config device config object.  This may be modified to normalise it
		during processing.
	@param device_ptr a pointer to a location to hold the new device object pointer.
 */
aud_error_t 
dr_device_open_with_config
(
	dr_devices_t * devices,
	dr_device_open_t * config,
	dr_device_t ** device_ptr
);


/**
	Obtain a new initialised config object.

	@param name advertised name of dante device.
		This memory must remain valid for the lifetime of the config object.
		If connecting by address, name may be null.
 */
dr_device_open_t *
dr_device_open_config_new
(
	const char * name
);

/**
	Discard an existing config object.
 */
void
dr_device_open_config_free
(
	dr_device_open_t * config
);

/**
	Enable device discovery and communication on specified network interface.

	@param config the config object
	@param dante_network_index index of dante network connected to this interface.
		For non-redundant operation, pass 0.
	@param os_intf_index index of OS network inteface.  Pass 0 to disable.
 */
void
dr_device_open_config_enable_interface_by_index
(
	dr_device_open_t * config,
	unsigned dante_network_index,
	unsigned os_intf_index
);

/**
	Enable device discovery and communication on specified network interface.

	@param config the config object
	@param dante_network_index index of dante network connected to this interface.
		For non-redundant operation, pass 0.
	@param os_intf_name name of OS network interface.  Pass NULL to disable.
		NOTE: On Windows, this is a wide character string.
 */
void
dr_device_open_config_enable_interface_by_name
(
	dr_device_open_t * config,
	unsigned dante_network_index,
	const aud_intf_char_t * os_intf_name
);

/**
	Enable communication to specified IPv4 address.

	@param config the config object
	@param dante_network_index index of dante network connected to this interface.
		For non-redundant operation, pass 0.
	@param address IPv4 address to connect to.
	@param port port to connect to.  Pass 0 for default.

	This mode is not compatible with enable_interface.  If both are given it is
	the same as if only enable_interface was called.
 */
void
dr_device_open_config_enable_address
(
	dr_device_open_t * config,
	unsigned dante_network_index,
	uint32_t address,
	uint16_t port
);


// Legacy open_remote modes

/**
 * Create a device object that communicates with a remote dante device.
 * The device object automatically begins a resolution phase to find the
 * remote device's address and port, with the device object in the
 * RESOLVING state. Communication with the remote device is possible once
 * the resolution phase is complete (i.e. the device is in the RESOLVED
 * state).
 *
 * @param devices the devices object
 * @param name the name of the device to open
 * @param device_ptr a pointer to a location to hold the new device object pointer.
 */
aud_error_t 
dr_device_open_remote
(
	dr_devices_t * devices,
	const char * name,
	dr_device_t ** device_ptr
);

/**
 * Create a device object that communicates with a remote dante device.
 * The device object automatically begins a resolution phase to find the
 * remote device's address and port, with the device object in the
 * RESOLVING state. Communication with the remote device is possible once
 * the resolution phase is complete (i.e. the device is in the RESOLVED
 * state). The API will only use resolve information from the given
 * interface. 
 *
 * @param devices the devices object
 * @param name the name of the device to open
 * @param interface_index the interface index to use for resolution information
 * @param device_ptr a pointer to a location to hold the new device object pointer.
 */
aud_error_t 
dr_device_open_remote_on_interface
(
	dr_devices_t * devices,
	const char * name,
	unsigned int interface_index,
	dr_device_t ** device_ptr
);

/**
 * Create a device object that communicates with a remote dante device.
 * The device object automatically begins a resolution phase to find the
 * remote device's address and port, with the device object in the
 * RESOLVING state. Communication with the remote device is possible once
 * the resolution phase is complete (i.e. the device is in the RESOLVED
 * state). 
 *
 * Calling this method with two interfaces will enable redundant routing
 * configuration. In this case the API will use resolve information from 
 * the given interfaces. It will also attempt to communicate with the device
 * on both interfaces. That is, messages will be sent to ALL addresses discovered
 * at the time of sending the message.
 *
 * @param devices the devices object
 * @param name the name of the device to open
 * @param num_interface_indexes the number of interfaces on which to communicate with the device
 * @param interface_indexes the interface indexes to use for resolution information
 * @param device_ptr a pointer to a location to hold the new device object pointer.
 *
 * @note The device object will enter the RESOLVED state as once an address has been found 
 *   on ANY interface. Address discovery will still continue on other interfaces after this
 *   time. The host application will be informed of changes to the devices address information
 *   via the DR_DEVICE_CHANGE_FLAG_ADDRESSES flag in the change callback.
 */
aud_error_t 
dr_device_open_remote_on_interfaces
(
	dr_devices_t * devices,
	const char * name,
	unsigned int num_interface_indexes,
	const unsigned int * interface_indexes,
	dr_device_t ** device_ptr
);

/**
 * Create a device object that communicates with a remote dante device.
 * The device object automatically begins a resolution phase to find the
 * remote device's address and port, with the device object in the
 * RESOLVING state. Communication with the remote device is possible once
 * the resolution phase is complete (i.e. the device is in the RESOLVED
 * state). 
 *
 * Calling this method with two interfaces will enable redundant routing
 * configuration. In this case the API will use resolve information from 
 * the given interfaces. It will also attempt to communicate with the device
 * on both interfaces. That is, messages will be sent to ALL addresses discovered
 * at the time of sending the message.
 *
 * @param devices the devices object
 * @param name the name of the device to open
 * @param num_interface_indexes the number of interfaces on which to communicate with the device
 * @param interface_indexes the interface indexes to use for resolution information
 * @param interface_bitmask the interface bit mask to specify on which interfaces we want to open communication with device
 * @param device_ptr a pointer to a location to hold the new device object pointer.
 *
 * @note The device object will enter the RESOLVED state as once an address has been found 
 *   on ANY interface. Address discovery will still continue on other interfaces after this
 *   time. The host application will be informed of changes to the devices address information
 *   via the DR_DEVICE_CHANGE_FLAG_ADDRESSES flag in the change callback.
 */

aud_error_t 
dr_device_open_remote_on_interfaces_extended
(
	dr_devices_t * devices,
	const char * name,
	unsigned int num_interface_indexes,
	const unsigned int * interface_indexes,
	uint32_t interface_bitmask,
	dr_device_t ** device_ptr
);

/**
 * Open a device using explicit addresses, bypassing the resolution step
 * This function is primarily intended for debugging 
 *
 * @param devices the devices object
 * @param num_addresses the numbers of addresses on which to communicate with the device
 * @param addresses the ipv4 addresses on which to communicate with the device
 * @param device_ptr a pointer to a location to hold the new device object pointer
 */
aud_error_t 
dr_device_open_addresses
(
	dr_devices_t * devices,
	unsigned int num_addresses,
	const unsigned int * addresses,
	dr_device_t ** device_ptr
);

/**
 * Open a device using explicit addresses and ports, bypassing the resolution step
 * This function is primarily intended for debugging 
 *
 * @param devices the devices object
 * @param num_addresses the numbers of addresses on which to communicate with the device
 * @param addresses the ipv4 addresses on which to communicate with the device
 * @param ports the udp ports on which to communicate with the device
 * @param device_ptr a pointer to a location to hold the new device object pointer
 */
aud_error_t 
dr_device_open_addresses_ports
(
	dr_devices_t * devices,
	unsigned int num_addresses,
	const unsigned int * addresses,
	const uint16_t * ports,
	dr_device_t ** device_ptr
);

/**
 * Open a device using the device's domain id.
 */
aud_error_t 
dr_device_open_domain_id
(
	dr_devices_t * devices, 
	uint32_t domain_id,
	dr_device_t ** device_ptr
);

/**
 * Is the device a local connection or a remote connection?
 *
 * @param device the device object
 * 
 * @return AUD_TRUE if the connection is local, AUD_FALSE otherwise
 */
aud_bool_t
dr_device_is_local
(
	const dr_device_t * device
);

/**
 * Close a connection to a device. Once closed, any channel and label
 * references will be freed and any handles will become invalid.
 *
 * @param device the device object
 */
void
dr_device_close
(
	dr_device_t * device
);

/**
 * Send a rename instruction to the device and closes the device object.
 * This function does not wait for a response from the device and closes the device handle
 * immediately after sending the request.
 * The user should check access policy when the device is in a domain before sending this request.
 *
 * @param device the device object
 * @param new_name the new name for this device
 */
aud_error_t
dr_device_close_rename
(
	dr_device_t * device,
	const char * new_name
);

/**
 * Set a context value for this device.
 *
 * @param device the device object
 * @param context the new context for this device
 */
void
dr_device_set_context
(
	dr_device_t * device,
	void * context
);

/**
 * Get the context value for this device
 *
 * @param device the device object
 */
void *
dr_device_get_context
(
	const dr_device_t * device
);

/**
 * Set the device change callback for this function.
 *
 * @param device the device object
 * @param device_changed the new callback for when the device object changes
 */
void
dr_device_set_changed_callback
(
	dr_device_t * device,
	dr_device_changed_fn * device_changed
);

/**
 * Get the device change callback for this function.
 *
 * @param device the device object
 */
dr_device_changed_fn *
dr_device_get_changed_callback
(
	const dr_device_t * device
);

//----------------------------------------------------------
// Device Error state interrogation
//----------------------------------------------------------

/**
 * What type of error caused the device to enter the error state?
 */
aud_error_t
dr_device_get_error_state_error
(
	const dr_device_t * device
);

/**
 * What was the device doing when it entered the error state?
 */
const char *
dr_device_get_error_state_action
(
	const dr_device_t * device
);

//----------------------------------------------------------
// Device state
//----------------------------------------------------------

/**
 * Get the devices object with which this device is associated
 *
 * @param device the devices object
 */
dr_devices_t *
dr_device_get_devices
(
	const dr_device_t * device
);

/**
 * Get the current state for this device
 *
 * All the dr_device query functions return an undefined value until the state
 * is DR_DEVICE_STATE_ACTIVE
 *
 * @param device the device object
 */
dr_device_state_t
dr_device_get_state
(
	const dr_device_t * device
);

/**
 * What is the name of this device?
 *
 * For local devices it is the actual name.
 * For remote devices this is the advertised name once known, and the name used
 *  in connection until then.
 *
 * @param device the device object
 *
 * @note for local devices, this name is not known unless explicitly
 *   obtained using the function dr_device_update_name.
 */
const char *
dr_device_get_name
(
	const dr_device_t * device
);

/**
 * What is the actual name for this device? This will be the same as
 * the device's advertised name unless there has been a name conflict.
 *
 * @param device the device object
 *
 * @note for local devices, this name is not known unless explicitly
 *   obtained using the function dr_device_update_name.
 */
const char *
dr_device_get_actual_name
(
	const dr_device_t * device
);

/**
 * What is the advertised name for this device? For remote devices, this is the
 * name provided when the device object was created. It is always the same as the
 * router's name unless there has been a name conflict.
 *
 * @param device the device object
 *
 * @note Only very recent devices support this function for local devices.
 */
const char *
dr_device_get_advertised_name
(
	const dr_device_t * device
);

/**
 * Returns the default name of the device.
 *
 * @param device the device object
 */
const char *
dr_device_get_default_name
(
	const dr_device_t * device
);

/**
 * Returns the name used in the "open" call, if any.
 *
 * @param device the device object
 */
const char *
dr_device_get_connect_name
(
	const dr_device_t * device
);

/**
 * Is this device using an explicit local connection?
 * This function returns true if the device was opened using "dr_device_open_local" or similar.
 * It will return false if device was discovered and happens to be co-resident
 *
 * @param device the device object.
 */
aud_bool_t
dr_device_is_local_connection
(
	const dr_device_t * device
);


/**
 * Get the ip address for this device.
 * 
 * @note This function implements dr_device_get_addresses() for only 1 address.
 *
 * @param device the device object
 * @param address a pointer to an address struct than will hold the device's resolved address
 *
 * @return AUD_SUCCESS if ok, or an error code otherwise
 */
aud_error_t
dr_device_get_address
(
	const dr_device_t * device,
	dante_ipv4_address_t * address
);

/**
 * Get the ip addresses for this device.
 * Device state must be DR_DEVICE_STATE_RESOLVED or greater, and have been resolved in the ADHOC domain.
 *
 * The function populates the array of addresses with one entry per local interface.
 * If the array is larger than needed, the output value of num_addresses will be the number of entries filled.
 * If the array is too small to hold all the addresses, the function populates as many entries as can fit in
 * the array, and sets num_addresses to the size required.
 * 
 * @param device the device object
 * @param num_addresses on input, the length of the addresses array. On output, the
 *   number of interfaces used to communicate with the device.
 * @param addresses a pointer to an array of address struct than will hold the device's resolved addresses
 *
 * @return AUD_SUCCESS if ok, or an error code otherwise
 *
 * @note for redundant connections, the returned array always has one entry per interface.
 *   if no address has been discovered on the given interface the address then the
 *   zero interface 0.0.0.0:0 will be returned
 */
aud_error_t
dr_device_get_addresses
(
	const dr_device_t * device,
	unsigned int * num_addresses,
	dante_ipv4_address_t * addresses
);

/**
 * Get the domain routing id for this device.
 * Device state must be DR_DEVICE_STATE_RESOLVED or greater, and have been resolved in a MANAGED domain.
 *
 * @param device the device object
 * @param addresses a pointer to an array of address struct than will hold the device's resolved addresses
 *
 * @return AUD_SUCCESS if ok, or an error code otherwise
 */
aud_error_t
dr_device_get_domain_routing_id
(
	const dr_device_t * device,
	dante_domain_routing_id_t * domain_routing_id
);

/**
 * Which version of the routing configuration protocol is supported by the device?
 * The value is available once the device has been resolved
 *
 * @param device the device object
 * @param version_ptr a pointer to a version structure that 
 *
 * @return AUD_SUCCESS if the version was obtained 
 */
aud_error_t
dr_device_get_protocol_version
(
	const dr_device_t * device,
	dante_version_t * version_ptr
);

/**
 * Get the device's error state flags? 
 * This function is only valid if a device's capabilities have been queried.
 *
 * @param device the device object
 * @param status_flags_ptr a pointer to a value that will store the device's
 *   status flags
 */
aud_error_t
dr_device_get_status_flags
(
	const dr_device_t * device,
	dr_device_status_flags_t * status_flags_ptr
);

/**
* Gets whether the device allowed configuration of tx aliases
* This function is only valid if a device's capabilities have been queried.
* This function only shows device capability to configure tx aliases. This function does
* not show whether a device can be configured in a domain. The user should check the access
* policy when the device is in a domain.
*
* @param device the device object
* @param can_configure_tx_aliases_ptr a pointer to a boolean that will store whether or not
*   this device allows tx aliases to be configured
*/
aud_error_t
dr_device_can_configure_tx_aliases
(
	const dr_device_t * device,
	aud_bool_t* can_configure_tx_aliases_ptr
);


/**
 * Is this device allowed to be renamed? 
 * This function is only valid if a device's capabilities have been queried.
 * This function only shows device capability to rename. This function does not show
 * whether a device can be renamed in a domain. The user should check the access
 * policy when the device is in a domain.
 *
 * @param device the device object
 * @param can_rename_ptr a pointer to a boolean that will store whether or not
 *   this device can be renamed
 */
aud_error_t
dr_device_can_rename
(
	const dr_device_t * device,
	aud_bool_t * can_rename_ptr
);

/**
* Does this device allow changes to the static bundle configuration
* This function is only valid if a device's capabilities have been queried.
* This function only shows device capability to edit static bundles. This function does
* not show whether a device can be configured in a domain. The user should check the access
* policy when the device is in a domain.
*
* @param device the device object
* @param can_edit_static_bundles_ptr a pointer to a boolean that will store whether or not
*   this device allows changes to static bundles
*/
aud_error_t
dr_device_can_edit_static_bundles
(
	const dr_device_t * device,
	aud_bool_t * can_edit_static_bundles_ptr
);

/**
 * Get the number of interfaces that this device supports. Note that
 * this value refers to the number of interfaces on the device being
 * configured, not the number of interfaces on the device hosting
 * the API. Device state must be ACTIVE for num interfaces to be known.
 *
 * @param device the device object
 */
uint16_t
dr_device_num_interfaces
(
	const dr_device_t * device
);

/**
 * Get the number of txchannels that this device supports. 
 * Device state must be ACTIVE for num interfaces to be known.
 *
 * @param device the device object
 */
uint16_t
dr_device_num_txchannels
(
	const dr_device_t * device
);

/**
 * Get the number of rxchannels that this device supports. 
 * Device state must be ACTIVE for num interfaces to be known.
 *
 * @param device the device object
 */
uint16_t
dr_device_num_rxchannels
(
	const dr_device_t * device
);


/**
 * Get the maximum number of transmit flows that this device can support
 *
 * @param device the device object
 * @param max_txflows_ptr a pointer to a location that will store the
 *   maximum number of transmit flows supported on this device
 *
 * @note Some older Dante devices do not provide this value; for these devices the
 * function returns AUD_ERR_VERSION
 */
aud_error_t
dr_device_max_txflows
(
	const dr_device_t * device,
	uint16_t * max_txflows_ptr
);

/**
 * Get the maximum number of receive flows that this device can support
 *
 * @param device the device object
 * @param max_rxflows_ptr a pointer to a location that will store the
 *   maximum number of receive flows supported on this device
 *
 * @note Some older Dante devices do not provide this value; for these devices the
 * function returns AUD_ERR_VERSION
 */
aud_error_t
dr_device_max_rxflows
(
	const dr_device_t * device,
	uint16_t * max_rxflows_ptr
);

/**
 * Get the maximum number of labels that this device can support
 *
 * @param device the device object
 * @param max_txlabels_ptr a pointer to a location that will store the
 *   maximum number of labels supported on this device
 *
 * @note Some older Dante devices do not provide this value; for these devices the
 * function returns AUD_ERR_VERSION
 */
aud_error_t
dr_device_max_txlabels
(
	const dr_device_t * device,
	uint16_t * max_txlabels_ptr
);

/**
 * Get the maximum number of slots that this device can handle for TX
 * flows
 *
 * @param device the device object
 */
uint16_t
dr_device_max_txflow_slots
(
	const dr_device_t * device
);

/**
 * Get the maximum number of slots that this device can handle for RX
 * flows
 *
 * @param device the device object
 */
uint16_t
dr_device_max_rxflow_slots
(
	const dr_device_t * device
);

/**
 * Request transmit channel information.
 * Returns the number of channels and an array containing the channels
 * themselves. The channel array is assumed to be fixed for the life
 * of the device handle, so the array needs only to be obtained once.
 * Once an array is obtained a call to dr_device_update with
 * DR_DEVICE_COMPONENT_TXCHANNELS will update the channel
 * information already obtained. The channels are freed automatically
 * by the library when the device handle is closed.
 * 
 * To get the number of tx channels without getting the channels
 * themselves, simply set 'channels' to NULL
 *
 * @param device the device object
 * @param num_channels a pointer to a location that will hold the number of transmit channels
 * @param channels a pointer to a location that will point to an array of transmit channels
 *
 * @returns AUD_SUCCESS if the channels were successfully obtained,
 *   an error code otherwise.
 */
aud_error_t
dr_device_get_txchannels
(
	const dr_device_t * device,
	uint16_t * num_channels,
	dr_txchannel_t *** channels
);

/**
 * Request receive channel information.
 * Returns the number of channels and an array containing the channels
 * themselves. The channel array is assumed to be fixed for the life
 * of the device handle, so the array needs only to be obtained once.
 * Once an array is obtained a call to dr_device_update with
 * DR_DEVICE_COMPONENT_RXCHANNELS will update the channel
 * information already obtained. The channels are freed automatically
 * by the library when the device handle is closed.
 * 
 * To get the number of rx channels without getting the channels
 * themselves, simply set 'channels' to NULL
 *
 * @param device the device object
 * @param num_channels a pointer to a location that will hold the number of receive channels
 * @param channels a pointer to a location that will point to an array of receive channels
 *
 * @returns AUD_SUCCESS if the channels were successfully obtained,
 *     an error code otherwise.
 */
aud_error_t
dr_device_get_rxchannels
(
	const dr_device_t * device,
	uint16_t * num_channels,
	dr_rxchannel_t *** channels
);

/**
 * Is any of device's information for the given component 'stale', ie.
 * needing updating?
 *
 * @param device the device object
 * @param component the component to be checked
 */
aud_bool_t
dr_device_is_component_stale
(
	const dr_device_t * device,
	dr_device_component_t component
);

/**
 * Forcefully mark an entire component as being stale. Subsequent
 * component updates will refresh the entire component state.
 *
 * @param device the device object
 * @param component the component to be marked as stale
 */ 
void
dr_device_mark_component_stale
(
	dr_device_t * device,
	dr_device_component_t component
);


//----------------------------------------------------------
// Device request tracking
//----------------------------------------------------------

/**
 * A device response function fires when a response message is received
 * for a previously send request.
 *
 * @param device the device object
 * @param request_id the id for this operation
 * @param result the result of the operation
 */
typedef void
dr_device_response_fn
(
	dr_device_t * device,
	dante_request_id_t request_id,
	aud_error_t result
);

/**
 * Is the given request currently active?
 *
 * @param device the device object
 * @param request_id the id for this operation
 * 
 */
aud_bool_t
dr_device_is_request_active
(
	dr_device_t * device,
	dante_request_id_t request_id
);

/**
 * Cancel an active request and release the id to be re-used.
 *
 * @param device the device object
 * @param request_id the id for this operation
 * 
 * @note This function does not communicate with the device in any way
 *   or attempt to undo the specified action, it simply releases the library
 *   resources associated with managing the request.
 */
aud_error_t
dr_device_cancel_request
(
	dr_device_t * device,
	dante_request_id_t request_id
);

/**
 * Send a no-op message to the device.
 * This is useful for testing that a device is active and for debugging
 * purposes. The device state must be RESOLVED but does not need to be
 * ACTIVE. That is, it is not necessary to have queried the device's
 * capabilities before sending a ping.
 *
 * @param device the device object
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 *
 * @note This message does not send a true network 'ping' but rather
 *   a no-op dante  configuration request.
 */
aud_error_t
dr_device_ping
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Instruct the device to store its current configuration immediately.
 *
 * @param device the device object
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 */
aud_error_t
dr_device_store_config
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Clear the device's configuration and revert to the default device state
 *
 * @param device the device object
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 */
aud_error_t
dr_device_clear_config
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Request the device's capabilities. Capabilities are assumed to be 
 * constant for the life of a device handle. If query_capabilities is
 * called multiple times and the capabilities change, the device goes
 * into the ERROR state, becoming unusable. The handle should be closed.
 * A new handle can then be re-created for the new capabilities. If this
 * method is called when the device is RESOLVED, it will change to the
 * QUERYING state. If the device is already QUERYING or ACTIVE, the
 * state does not change.
 *
 * @param device the device object
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 */
aud_error_t
dr_device_query_capabilities
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Obtain the device name from the device and update the device handle.
 * Primarily useful for debugging or when dealing with a local device handle.
 *
 * @param device the device object
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 */
aud_error_t
dr_device_update_name
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Update the device object's local data for the given component, by
 * requesting the device's current state. This function will ensure
 * that any parts of the component marked as 'stale' will be updated.
 * If the entire component has been markd stale then the entire
 * component state will be refreshed.
 *
 * @param device the device object
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 * @param component the component to be updated
 *
 * @note it may take multiple request-response interactions for this
 *   operation to complete. The response function will not fire until
 *   the last response arrives.
 */
aud_error_t
dr_device_update_component
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	dr_device_component_t component
);

/**
* Sends a rename instruction to the device.
* This function works for all devices. However a remote device (not opened using "dr_device_open_local")
* will go into error state after the name change.
* Unlike dr_device_close_rename(), this function does not close the device handle.
* 
* @param device the device object
* @param response_fn the function to call upon completion of this operation
* @param request_id the id for this operation
* @param new_name the new name for this device
*/
aud_error_t
dr_device_rename
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	const char * new_name
);

//----------------------------------------------------------
// Device properties
//----------------------------------------------------------

/**
 * Get the default tx latency for this device, in microseconds. This value
 * is used by the Dante device for user-configured flows,
 * as a suggested minimum latency value for receivers
 *
 * @param device the device whose latency is to be obtained
 *
 * @note Tx latency is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 */
dante_latency_us_t
dr_device_get_tx_latency_us
(
	const dr_device_t * device
);

/**
 * Get the minimum allowed latency for this device, in microseconds.
 *
 * @param device the device whose latency is to be obtained
 *
 * @note Tx latency is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 */
dante_latency_us_t
dr_device_get_tx_latency_min_us
(
	const dr_device_t * device
);

/**
 * Get the transmit fpp value used by this device when configuring subscriptions
 * 
 * @param device the device whose transmit fpp is to be obtained
 *
 * @note Preferred transmit fpp is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 */
dante_fpp_t
dr_device_get_tx_fpp
(
	const dr_device_t * device
);

/**
* Get the multicast transmit fpp value used by this device when configuring subscriptions
*
* @param device the device whose transmit fpp is to be obtained
*
* @note Suggested transmit fpp is part of the device 'properties'. An application can
*       check the validity of the current value by checking whether the device's
*       'properties' component is stale
*/
dante_fpp_t
dr_device_get_tx_fpp_suggested
(
const dr_device_t * device
);

/**
 * Get the default receive (rx) latency for this device, in microseconds. This value
 * is used by the Dante device when determining latency for audio flows.
 *
 * @param device the device whose latency is to be obtained
 *
 * @note RX latency is partof the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 */
dante_latency_us_t
dr_device_get_rx_latency_us
(
	const dr_device_t * device
);

/**
 * Get the maximum receive (rx) latency for this device, in microseconds.
 * 
 * @param device the device whose latency is to be obtained
 *
 * @note Maximum RX latency is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 */
dante_latency_us_t
dr_device_get_rx_latency_max_us
(
	const dr_device_t * device
);

/**
 * Get the minimum receive (rx) latency for this device, in microseconds.
 * 
 * @param device the device whose latency is to be obtained. 
 *
 * @note Minimum RX latency is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 */
dante_latency_us_t
dr_device_get_rx_latency_min_us
(
	const dr_device_t * device
);

/**
 * Get the default network latency compensation for this device, in microseconds. This value
 * is used by the Dante device when determining latency for audio flows.
 *
 * @param device the device whose latency is to be obtained
 *
 * @note RX latency network is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 *
 * @deprecated Rx Network Latency is no longer part of Dante latency calculations. This 
 *       this function may not be supported for newer Dante devices.
 */
dante_latency_us_t
dr_device_get_rx_latency_network_us
(
	const dr_device_t * device
);

/**
 * Get the preferred fpp value used by this receiver when configuring subscriptions
 * 
 * @param device the device whose receive fpp is to be obtained
 *
 * @note Preferred receive fpp is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 */
dante_fpp_t
dr_device_get_rx_fpp
(
	const dr_device_t * device
);

/**
 * Get the minimum fpp value allowed for this receiver. The receiver will not accept flows
 * with fpp less than this value, in order to limit packet rates.
 * 
 * @param device the device whose minimum receive fpp is to be obtained
 *
 * @note Minimum receive fpp is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 */
dante_fpp_t
dr_device_get_rx_fpp_min
(
	const dr_device_t * device
);

/**
 * Set the lockdown state for this device
 *
 * @param device the device to be configured
 * @param lockdown the new lockdown state for this device
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 *
 * @note this function is only valid for local connections
 */
aud_error_t
dr_device_set_lockdown
(
	dr_device_t * device,
	aud_bool_t lockdown,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Set the tx multicast performance properties. This function sends new settings to the device for:
 * - default tx latency for multicast flows sent by this device
 * - default fpp for multicast flows sent by this device
 * The values are applied to all existing flows that do not explictly override the default.
 * This function is disruptive and will cause audio glitches if the settings for latency and fpp change.
 *
 * WARNING: Calling this function on older devices may be result in their configuration being re-loaded.
 * Manually configured tx unicast flows will be lost as a result of this function. Manually configured 
 * multicast flows and subscription-based routes will be restored.
 *
 * @param device the device to be configured
 * @param latency_us the new latency value for the device, in microseconds.
 *   Set to zero to return to the device's default value for this setting.
 * @param fpp the new default fpp value for the device.
 *   Set to zero to return to the device's default value for this setting.
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 *
 * @note reconfiguration of existing flows may not be supported on older Dante devices.
 *   If an attempt is made to configure such a device, the function returns AUD_ERR_VERSION
 */
aud_error_t
dr_device_set_tx_performance_us
(
	dr_device_t * device,
	dante_latency_us_t latency_us,
	dante_fpp_t fpp,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Set the rx performance properties. This function sends new settings to the device for:
 * - receive latency
 * - receive fpp to the device.
 * The values are applied to all existing flows.
 * This function is disruptive and will cause audio glitches if the settings for latency and fpp change.
 *
 * @param device the device to be configured
 * @param latency_us the new latency value for the device, in microseconds.
 *   Set to zero to return to the device's default value for this setting.
 * @param requested_fpp the new requested fpp value for the device.
 *   Set to zero to return to the device's default value for this setting.
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 *
 * @note reconfiguration of existing flows may not be supported on older Dante devices.
 *   If an attempt is made to configure such a device, the function returns AUD_ERR_VERSION
 */
aud_error_t
dr_device_set_rx_performance_us
(
	dr_device_t * device,
	dante_latency_us_t latency_us,
	dante_fpp_t requested_fpp,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Set the 'network' component of the receive (rx) latency for the device.
 * This value is used by the Dante device when doing dynamic latency
 * determination for audio flows.
 * 
 * The 'reconfigure_existing_flows' flag specifies whether or not existing
 * audio flows should be reconfigured. Re-configuring audio latencies will
 * cause audio glitches and must only be use if this is indeed the desired behaviour.
 *
 * @param device the device to be configured
 * @param latency_network_us the new latency value for the device, in microseconds
 * @param reconfigure_existing_flows a flag indicating whether or not existing flows
 *    should be affected when setting a new device latency
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 *
 * @note reconfiguration of existing flows may not be supported on older Dante devices.
 *   If an attempt is made to configure such a device and the 
 *   'reconfigure_existing_flows' field set, the function returns AUD_ERR_VERSION
 *
 * @deprecated Rx Network Latency is no longer part of Dante Latency calculations.
     This function may not be supported on newer Dante devices.
 */
aud_error_t
dr_device_set_rx_latency_network_us
(
	dr_device_t * device,
	dante_latency_us_t latency_network_us,
	aud_bool_t reconfigure_existing_flows,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Are unicast performance settings configurable for this device?
 *
 * This function only shows device capability whether unicast performance can be set. 
 * This function does not show whether a device can be configured in a domain. 
 * The user should check the access policy when the device is in a domain.
 *
 * @param device the device
 */
aud_bool_t
dr_device_can_set_unicast_performance
(
	const dr_device_t * device
);

/**
 * Set the device unicast performance properties. This function sends new default settings to the device for:
 * - minimum transmit latency for unicast flows transmitted from thie device
 * - suggested fpp for unicast flows transmitted from this device
 * - minimum receive latency for unicast flows received on this device
 * - preferred fpp for unicast flows received on this device
 *
 * The same latency / fpp settings are applied to both tx and rx.
 * The values are applied to all existing flows.
 * This function is disruptive and will cause audio glitches if the settings for latency and fpp change.
 *
 * @param device the device to be configured
 * @param latency_us the new latency value for the device, in microseconds.
 *   Set to zero to return to the device's default value for this setting.
 * @param fpp the new fpp value for the device.
 *   Set to zero to return to the device's default value for this setting.
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 *
 * @note reconfiguration of existing flows may not be supported on older Dante devices.
 *   If an attempt is made to configure such a device, the function returns AUD_ERR_VERSION
 */
aud_error_t
dr_device_set_unicast_performance_us
(
	dr_device_t * device,
	dante_latency_us_t latency_us,
	dante_fpp_t fpp,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/** 
 * Get the default size used when creating adhoc flows
 *
 * @param device the device
 *
 * @note Minimum default adhoc flow size is part of the device 'properties'. An application can
 *      check the validity of the current value by checking whether the device's
 *      'properties' component is stale
 */
uint16_t
dr_device_get_rx_flow_default_slots
(
	const dr_device_t * device
);

/** 
 * Get the device's current clock subdomain name
 *
 * @param device the device
 *
 * @return NULL if the routing state is not yet ACTIVE, or if the clock is using the default subdomain
 *
 * @note Clock subdomain name is part of the device 'properties'. An application can
 *      check the validity of the current value by checking whether the device's
 *      'properties' component is stale
 */
const char *
dr_device_get_clock_subdomain_name
(
	const dr_device_t * device
);

/**
 * Does this device support network loopback?
 *
 * Network loopback takes the audio arriving on network receive channels and patches it across
 * to network transmit channels
 *
 * @param device the device
 */
aud_bool_t
dr_device_has_network_loopback
(
	const dr_device_t * device
);

/**
 * Is network loopback enabled on this device?
 *
 * Network loopback takes the audio arriving on network receive channels and patches it across
 * to network transmit channels
 *
 * @param device the device
 */
aud_bool_t
dr_device_get_network_loopback
(
	const dr_device_t * device
);

/**
 * Enable or disable 'network loopback' forthis device device.
 *
 * @param device the device to be configured
 * @param loopback the new value for the network loopback
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 */
aud_error_t
dr_device_set_network_loopback
(
	dr_device_t * device,
	aud_bool_t loopback,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Does this device support signal reference levels?
 *
 * Signal reference levels are meta-data that can be attached to TX channels and
 * are propagated with the subscription to the RX channels.
 *
 * @param device the device
 */
aud_bool_t
dr_device_has_signal_reference_levels
(
	const dr_device_t * device
);


//----------------------------------------------------------
// TX Channel & label functions
//----------------------------------------------------------

/**
 * Get the i-th transmit channel. 
 * index must be >= 0 and < N, where N is the number of device tx channels
 *
 * @param device the device object
 * @param index the index of the channel to be obtained
 */
dr_txchannel_t *
dr_device_txchannel_at_index
(
	dr_device_t * device,
	unsigned int index
);


/**
 * Get the transmit channel with a given high-level identifier. Transmit channel
 * identifiers start from 1.
 *
 * @param device the device object
 * @param id the id of the channel to be obtained
 */
dr_txchannel_t *
dr_device_txchannel_with_id
(
	dr_device_t * device,
	dante_id_t id
);

/**
 * Get the device for this channel
 *
 * @param tx the transmit channel
 */
dr_device_t *
dr_txchannel_get_device
(
	const dr_txchannel_t * tx
);

/**
 * Get this transmit channels id
 *
 * @param tx the transmit channel
 */
dante_id_t
dr_txchannel_get_id
(
	const dr_txchannel_t * tx
);

/**
 * Does the given channel contain stale information?
 *
 * @param tx the transmit channel
 */
aud_bool_t
dr_txchannel_is_stale
(
	const dr_txchannel_t * tx
);

/**
 * Mark the TX channel as being stale
 *
 * @param tx the transmit channel
 */
void
dr_txchannel_mark_stale
(
	dr_txchannel_t * tx
);

/**
 * Get the canonical name of the given TX channel. A transmit channel's
 * canonical name is fixed and will not change.
 *
 * @param tx the transmit channel
 *
 * @returns the name of the given channel
 */
const char *
dr_txchannel_get_canonical_name
(
	const dr_txchannel_t * tx
);

/**
 * Get the audio formats information for this channel.
 * 
 * @param tx the transmit channel
 */
const dante_formats_t *
dr_txchannel_get_formats
(
	const dr_txchannel_t * tx
);

/**
 * Get the sample rate of the channel, in samples per second (Hz)
 *
 * @param tx the transmit channel
 *
 * @deprecated use dr_txchannel_get_formats to access channel format information
 */
dante_samplerate_t
dr_txchannel_get_sample_rate
(
	const dr_txchannel_t * tx
);

/**
 * Get the number of encodings supported by this channel
 *
 * @param tx the transmit channel
 *
 * @deprecated use dr_txchannel_get_formats to access channel format information
 */
uint16_t
dr_txchannel_num_encodings
(
	const dr_txchannel_t * tx
);

/**
 * Get the index-th encoding supported by this channel
 *
 * @param tx the transmit channel
 * @param index the index of the encoding to be obtained
 *
 * @deprecated use dr_txchannel_get_formats to access channel format information
 */
dante_encoding_t
dr_txchannel_encoding_at_index
(
	const dr_txchannel_t * tx,
	uint16_t index
);


/**
 * Get all the labels for a given txchannel and store as many as possible
 * in the array provided. If the array is longer than or equal to the
 * number of labels, 'len' contains the number of channels and the
 * labels are all stored in the array. If there are more channel labels
 * than can fit in the array, the function fills the array up to the
 * length specified and 'len' is set to the total number of labels.
 *
 * The canonical name is not returned by this call.
 *
 * @param tx the channel whose label's are to be obtained
 * @param len on input, the length of the 'labels' array; on output,
 *   the number of labels for this channel
 * @param labels An array of labels.
 */
aud_error_t
dr_txchannel_get_txlabels
(
	const dr_txchannel_t * tx, // [IN]
	uint16_t * len,            // [IN/OUT]
	dr_txlabel_t * labels      // [OUT]
);

/**
 * Does the transmit channel have the given name as a canonical name
 * and / or as a label?
 *
 * @param tx the transmit channel
 * @param name the name to be checked against
 * @param txname_types a flag specifying whether the function should
 *   check the channels canonical name, labels or both. Legal values
 *   are DR_TXNAME_TYPE_CANONICAL, DR_TXNAME_TYPE_LABEL and
 *   DR_TXNAME_TYPE_ALL.
 */
aud_bool_t
dr_txchannel_has_name
(
	const dr_txchannel_t * tx,
	const char * name,
	dante_txname_type_t txname_types
);

/**
 * Add a label 'name' to the given channel.
 *
 * @param tx the transmit channel
 * @param response_fn the function to be called upon completion of this operation
 * @param request_id A request identifier for this operation
 * @param name the label to be added to this channel
 * @param moveflags A flag indicating whether the label sould be moved
 *   if it already exists on a different channel. Legal values are 0
 *   (don't move) and DR_MOVEFLAG_MOVE_EXISTING.
 */
aud_error_t
dr_txchannel_add_txlabel
(
	dr_txchannel_t * tx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	const char * name,
	dr_moveflag_t moveflags
);

/**
 * Replace the label 'old_name' with the label new_name on the given channel.
 *
 * @param tx the transmit channel
 * @param response_fn the function to be called upon completion of this operation
 * @param request_id A request identifier for this operation
 * @param old_name the old label name
 * @param new_name the new label name
 * @param moveflags A flag indicating whether the label sould be moved
 *   if it already exists on a different channel. Legal values are 0
 *   (don't move) and DR_MOVEFLAG_MOVE_EXISTING.
 */
aud_error_t
dr_txchannel_replace_txlabel
(
	dr_txchannel_t * tx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	const char * old_name,
	const char * new_name,
	dr_moveflag_t moveflags
);

/**
 * Remove a label from the given channel
 *
 * @param tx the transmit channel
 * @param response_fn the function to be called upon completion of this operation
 * @param request_id A request identifier for this operation
 * @param name the name of the label to be removed from this channel
 */
aud_error_t
dr_txchannel_remove_txlabel
(
	dr_txchannel_t * tx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	const char * name
);

/**
 * Has this tx channel been muted?
 *
 * @param tx the transmit channel
 *
 * @returns AUD_TRUE if the channel has been muted, AUD_FALSE otherwise
 */
aud_bool_t
dr_txchannel_is_muted
(
	const dr_txchannel_t * tx
);

/**
 * Mute or unmute a tx channel
 *
 * @param tx the transmit channel
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 * @param muted the new mute value for the channel
 *
 * @return AUD_SUCCESS if the channel mute/unmute was successfully initiated,
 *     an error code otherwise.
 */
aud_error_t
dr_txchannel_set_muted
(
	dr_txchannel_t * tx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	aud_bool_t muted
);

/**
 * Get the channel signal reference level of a TX channel
 *
 * @param tx the transmit channel
 *
 * @returns current signal reference level value
 */
dante_dbu_t
dr_txchannel_get_signal_reflevel
(
	const dr_txchannel_t * tx
);

/**
 * Set the channel signal reference level of a TX channel
 *
 * @param tx the transmit channel
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 * @param dbu channel signal reference level
 *
 * @returns AUD_SUCCESS on success, error code otherwise
 */
aud_error_t
dr_txchannel_set_signal_reflevel
(
	dr_txchannel_t * tx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	dante_dbu_t dbu
);


/**
 * Set the channel signal reference level of all TX channels on a device
 *
 * @param device the device to be configured
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 * @param dbu channel signal reference level
 */
aud_error_t
dr_device_set_txchannel_signal_reflevel
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	dante_dbu_t dbu
);


//----------------------------------------------------------
// TX Channel enable / disable Functions
//----------------------------------------------------------

/**
 * This function queries the 'enabled' setting for the given TX channel.
 *
 * @param tx the transmit channel
 *
 * @returns AUD_SUCCESS if the value was successfully obtained,
 * an error code otherwise
 */
aud_bool_t
dr_txchannel_is_enabled
(
	const dr_txchannel_t * tx
);

/**
 * Each tx channel can be enabled or disabled.
 *
 * Disabling a channel:
 * - removes its advertisement from the network.
 * - replaces the audio for this channel with zeroes
 *
 * Flows may contain disabled channels. A Dante device will continue
 * transmitting the flow while any slot in the flow contains an enabled
 * channel. If all slots are empty or contain a disabled channel then the
 * flow will cease.
 *
 * Disabled channels may still be included in TX flows but no audio
 * will be transmitted for this channel.
 *
 * This call does an implicit 'update' for this TX Channel
 *
 * @param tx the transmit channel
 * @param response_fn the function to be called upon completion of this operation
 * @param request_id A request identifier for this operation
 * @param enabled The new enabled status for this channel.
 *
 * @returns AUD_SUCCESS if the channel was successfully configured,
 *   an error code otherwise.
 */
aud_error_t
dr_txchannel_set_enabled
(
	dr_txchannel_t * tx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	const aud_bool_t enabled
);


//----------------------------------------------------------
// TX Label functions
//----------------------------------------------------------

/**
 * Get a list of labels for an entire device.
 * On input, 'len' contains the length of the 'labels' array.
 * The function fills up to 'len' elements in the array 'labels'.
 * Afterwards, 'len' contains the number of active labels the device has.
 *
 * @param device the device object
 * @param len on input, the length of the 'labels' array, on output, the number of labels 
 *  the device has.
 * @param labels an array that will be populated with the device's labels. If there are more
 *  labels than space in the array, only the available space is used.
 */
aud_error_t
dr_device_get_txlabels
(
	const dr_device_t * device, // [IN]
	uint16_t * len,             // [IN/OUT]
	dr_txlabel_t * labels       // [OUT]
);


/**
 * Get the transmit channel with the given canonical name or label
 *
 * @param device the device object
 * @param name the name or label of the channel of interest
 * @param txname_types A flag specifying which name types to consider,
 * in general this is will be 'DR_TXNAME_TYPE_ALL'
 */
dr_txchannel_t *
dr_device_txchannel_with_name
(
	const dr_device_t * device, 
	const char * name, 
	dante_txname_type_t txname_types
);

/**
 * Mark a label as stale
 *
 * @param device the device object
 * @param label_id identifier of label to mark as stale
 */
aud_error_t
dr_device_mark_txlabel_stale
(
	dr_device_t * device,
	dante_id_t label_id
);


/**
 * Get the TX label associated with an identifier
 *
 * @param device the device object
 * @param label_id identifier of label to get
 * @param label pointer to label data structure that will be popultated if call succeeds
 *
 * @returns AUD_ERR_NOTFOUND if there is no label currently associated with the given id
 * @returns AUD_ERR_RANGE if the label id is greater than the maximum given by dr_device_max_txlabels
 */
aud_error_t
dr_device_txlabel_with_id
(
	const dr_device_t * device, // [IN]
	dante_id_t label_id,	// [IN]
	dr_txlabel_t * label	// [OUT]
);


/**
 * Remove the TX label associated with an identifier
 *
 * Equivalent to doing dr_device_get_txlabel_by_id and then calling
 * dr_txchannel_remove_txlabel on the result.
 *
 * @param device the device object
 * @param response_fn the function to be called upon completion of this operation
 * @param request_id A request identifier for this operation
 * @param label_id identifier of label to remove
 */
aud_error_t
dr_device_remove_txlabel_with_id
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	dante_id_t label_id
);


//----------------------------------------------------------
// RX Channel Functions
//----------------------------------------------------------

/**
 * Get the i-th receive channel. 
 * index must be >= 0 and < N, where N is the number of device rx channels
 *
 * @param device the device object
 * @param index the index of the channel to be obtained
 */
dr_rxchannel_t *
dr_device_rxchannel_at_index
(
	dr_device_t * device,
	unsigned int index
);

/**
 * Get the device's receive channel with the given high-level identifier. Receive channel
 * identifiers start from 1.
 *
 * @param device the device object
 * @param id the identifier of the recieve channel to be obtained.
 */
dr_rxchannel_t *
dr_device_rxchannel_with_id
(
	dr_device_t * device,
	dante_id_t id
);

/**
 * Get the device for a given receive channel
 *
 * @param rx the receive channel
 */
dr_device_t *
dr_rxchannel_get_device
(
	const dr_rxchannel_t * rx
);

/**
 * Does the given channel contain stale information?
 *
 * @param rx the receive channel
 */
aud_bool_t
dr_rxchannel_is_stale
(
	const dr_rxchannel_t * rx
);

/**
 * Mark the RX channel as being stale
 *
 * @param rx the receive channel
 */
void
dr_rxchannel_mark_stale
(
	dr_rxchannel_t * rx
);


/**
 * Get the name of this RX channel
 *
 * @param rx the receive channel
 */
const char *
dr_rxchannel_get_name
(
	const dr_rxchannel_t * rx
);

/**
 * Get the channel's high-level id.
 *
 * @param rx the receive channel
 */
dante_id_t
dr_rxchannel_get_id
(
	const dr_rxchannel_t * rx
);

/**
 * Get the audio formats information for this channel.
 * 
 * @param rx the receive channel
 */
const dante_formats_t *
dr_rxchannel_get_formats
(
	const dr_rxchannel_t * rx
);

/**
 * Sample rate of channel, in samples per second (Hz)
 *
 * @param rx the receive channel
 *
 * @deprecated use dr_rxchannel_get_formats to access channel format information
 */
dante_samplerate_t
dr_rxchannel_get_sample_rate
(
	const dr_rxchannel_t * rx
);

/**
 * Get the number of encodings supported by a receive channel
 * 
 * @param rx the receive channel
 *
 * @deprecated use dr_rxchannel_get_formats to access channel format information
 */
uint16_t
dr_rxchannel_num_encodings
(
	const dr_rxchannel_t * rx
);

/**
 * Get the index'th encoding for the given receive channel.
 *
 * @param rx the receive channel
 * @param index the (zero-based) index of the encoding to be obtained.
 *
 * @deprecated use dr_rxchannel_get_formats to access channel format information
 */
dante_encoding_t
dr_rxchannel_encoding_at_index
(
	const dr_rxchannel_t * rx,
	uint16_t index
);

/**
 * Get the complete name of the network channel to which this
 * rx channel is subscribed in the form "channel\@device"
 * 
 * @param rx the receive channel
 *
 * @return NULL if not subscribed
 */
const char *
dr_rxchannel_get_subscription
(
	const dr_rxchannel_t * rx
);

/**
 * get just the channel name of the network channel to
 * which this rx channel is subscribed
 *
 * @param rx the receive channel
 *
 * @return NULL if not subscribed
 */
const char *
dr_rxchannel_get_subscription_channel
(
	const dr_rxchannel_t * rx
);

/**
 * get just the device name of the network channel to which
 * this rx channel is subscribed
 *
 * @param rx the receive channel
 *
 * @return NULL if not subscribed
 */
const char *
dr_rxchannel_get_subscription_device
(
	const dr_rxchannel_t * rx
);

/**
 * Get the subscription status for this RX channel.
 *
 *
 * @param rx the receive channel
 *
 * @return DR_RXSTATUS_NONE if the channel does not have a subscription,
 *   otherwise one of the DR_RXSTATUS values
 */
dante_rxstatus_t
dr_rxchannel_get_status
(
	const dr_rxchannel_t * rx
);

/**
 * Get the latency (in microseconds) for this channel's subscription,
 * ie. the latency for audio that is arriving on this channel. This
 * value is only meaningful if the channel's status is
 * DR_RXSTATUS_DYNAMIC or DR_RXSTATUS_STATIC.
 *
 * @param rx the receive channel
 * 
 * @note For some Dante devices, the API derives this information from 
 *   rx flow information rather than directly from channel information.
 *   The host application should always ensure that both the rx channel
 *   and rx flow components are up to date if this value is to be used.
 * 
 * @return The latency for this channel's subscription, or 0 if the channel
 *   does not have an active subscription
 */
dante_latency_us_t
dr_rxchannel_get_subscription_latency_us
(
	const dr_rxchannel_t * rx
);

/**
 * Subscribe the given rx channel to the network channel with the given
 * name if network or channel is NULL then unsubscribe the rx channel
 * from any network channel to which it may have been subscribed.
 * This call does an implicit 'update' for this RX Channel
 *
 * @param rx the receive channel
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 * @param device the name of the transmit device to which this receive channel will subscribe
 * @param channel the name of the transmit channel to which this receive channel will subscribe
 *
 * @return AUD_SUCCESS if the subscription was successfully initiated,
 *     an error code otherwise.
 */
aud_error_t
dr_rxchannel_subscribe
(
	dr_rxchannel_t * rx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	const char * device,
	const char * channel
);

/**
 * Get a bitmask describing available connections for this channel's
 * subscription. For a single interface device, this will be 1 if
 * subscribed or 0 if not. For a multiple interface device, it is a
 * bitfield describing which interface pairs are configured for receiving
 * data.
 *
 * @param rx the receive channel
 *
 * @return 0 if not subscribed or if subscription is not yet successful.
 */
uint8_t
dr_rxchannel_get_connections_available
(
	const dr_rxchannel_t * rx
);

/**
 * Get a bitmask describing the active connections for this channel's
 * subscription. Similar to connections_avail, except reports which
 * interfaces are actually receiving data.  In a correctly functioning
 * network, this value will be identical to that from
 * connections_available.
 *
 * @param rx the receive channel
 */
uint8_t
dr_rxchannel_get_connections_active
(
	const dr_rxchannel_t * rx
);

/**
 * Set the name of an rx channel
 *
 * @param rx the receive channel
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 * @param new_name the new_name for the receive channel
 *
 * @return AUD_SUCCESS if the channel rename was successfully initiated,
 *     an error code otherwise.
 */
aud_error_t
dr_rxchannel_set_name
(
	dr_rxchannel_t * rx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	const char * new_name
);

/**
 * Has this rx channel been muted?
 *
 * @param rx the receive channel
 *
 * @returns AUD_TRUE if the channel has been muted, AUD_FALSE otherwise
 */
aud_bool_t
dr_rxchannel_is_muted
(
	const dr_rxchannel_t * rx
);

/**
 * Mute or unmute an rx channel
 *
 * @param rx the receive channel
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 * @param muted the new mute value for the channel
 *
 * @return AUD_SUCCESS if the channel mute/unmute was successfully initiated,
 *     an error code otherwise.
 */
aud_error_t
dr_rxchannel_set_muted
(
	dr_rxchannel_t * rx,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	aud_bool_t muted
);


/**
	Does channel support RX subscribe to own device ("self")?
	
	Some Dante devices can subscribe to their own TX channels.  This is done by
	setting the subscribed device name to either the device's own name or null
	(empty string).
	
	@param rx the receive channel
	
	@return true if channel can subscribe to TX channels on own device,
		false otherwise.  Returns false if argument is invalid.
 */
aud_bool_t
dr_rxchannel_can_subscribe_to_self
(
	const dr_rxchannel_t * rx
);

/**
 * Does the device allow renaming of this channel?
 *
 * @param rx the receive channel
 *
 * @return true if channel can be renamed,	false otherwise.
 * Returns false if argument is invalid.
 *
 */
aud_bool_t
dr_rxchannel_can_rename
(
	const dr_rxchannel_t * rx
);

/**
* This function queries the 'enabled' setting for the given RX channel.
*
* @param rx the receive channel
*
* @returns AUD_SUCCESS if the value was successfully obtained,
* an error code otherwise
*/
aud_bool_t
dr_rxchannel_is_enabled
(
	const dr_rxchannel_t * rx
);


/**
 * Get the channel signal reference level of a rx channel
 *
 * @param rx the receive channel
 *
 * @returns current signal reference level value
 */
dante_dbu_t
dr_rxchannel_get_signal_reflevel
(
	const dr_rxchannel_t * rx
);


//----------------------------------------------------------
// TX flow operations
//----------------------------------------------------------

/**
 * Get the number of flows currently being transmitted by this
 * device
 *
 * @param device the device object
 */
uint16_t
dr_device_num_txflows
(
	dr_device_t * device
);

/**
 * Get a handle to a particular flow by index
 *
 * @param device the device object
 * @param index The 0-based flow index in the device's data
 * @param flow_ptr a pointer to a txflow handle
 */
aud_error_t
dr_device_txflow_at_index
(
	dr_device_t * device,
	uint16_t index,
	dr_txflow_t ** flow_ptr
);

/**
 * Get a handle to the transmit flow with the given high-level
 * bundle identifier.
 *
 * @param device the device object
 * @param id the flow id to be obtained
 * @param flow_ptr a pointer to a txflow handle
 */
aud_error_t
dr_device_txflow_with_id
(
	dr_device_t * device,
	dante_id_t id,
	dr_txflow_t ** flow_ptr
);

/**
 * Get the high-level identifier for the given transmit flow
 *
 * @param flow the flow handle
 * @param id_ptr a pointer to a location to store the flow's id
 */
aud_error_t
dr_txflow_get_id
(
	dr_txflow_t * flow,
	dante_id_t * id_ptr
);

/**
 * Get the name of the given transmit flow
 *
 * @param flow the flow handle
 * @param name_ptr a pointer to a location to store the flow's name
 */
aud_error_t
dr_txflow_get_name
(
	dr_txflow_t * flow,
	char ** name_ptr
);

/**
 * Get the sample rate and/or encoding for a given flow
 *
 * @param flow the flow handle
 * @param samplerate_ptr a pointer to a location to store the flow's sample rate
 * @param encoding_ptr a pointer to a location to store the flow's encoding
 */
aud_error_t
dr_txflow_get_format
(
	dr_txflow_t * flow,
	dante_samplerate_t * samplerate_ptr,
	dante_encoding_t * encoding_ptr
);

/**
 * Get the advertised latency value for a given flow
 *
 * @param flow the flow handle
 * @param latency_us_ptr a pointer to a location to store the flow's advertised (tx) latency
 */
aud_error_t
dr_txflow_get_latency_us
(
	dr_txflow_t * flow,
	dante_latency_us_t * latency_us_ptr
);

/**
 * Get the frames-per-packet value for a given flow
 *
 * @param flow the flow handle
 * @param fpp_ptr a pointer to a location to store the flow's frames-per-packet
 */
aud_error_t
dr_txflow_get_fpp
(
	dr_txflow_t * flow,
	dante_fpp_t * fpp_ptr
);

/**
 * How many slots are in the given flow?
 *
 * @param flow the flow handle
 * @param num_slots_ptr a pointer to a location to store the flow's slot count
 */
aud_error_t
dr_txflow_num_slots
(
	dr_txflow_t * flow,
	uint16_t * num_slots_ptr
);

/**
 * Which tx channel is being place in the slot?
 * If no channel is currently configured for this slot,
 * return NULL
 * @param flow the flow handle
 * @param slot the zero-based slot index
 * @param slot_ptr a pointer to a location to store the flow's slot value
 */
aud_error_t
dr_txflow_channel_at_slot
(
	dr_txflow_t * flow,
	uint16_t slot,
	dr_txchannel_t ** slot_ptr
);

/**
 * On how many interfaces is the flow operating?
 *
 * @param flow the flow handle
 * @param num_addresses_ptr a pointer to a location to store the flow's interface count
 */
aud_error_t
dr_txflow_num_interfaces
(
	dr_txflow_t * flow,
	uint16_t * num_addresses_ptr
);

/**
 * Get the interface-specific destination address and port for the
 * transmit flow. Interfaces are indentified as a zero-based index,
 * with 0 being ther primary network. For unconfigured interfaces the
 * address port will be 0
 *
 * @param flow the flow handle
 * @param intf the zero-based interface index
 * @param addr a pointer to a location to store the flow's address on the given interface
 */
aud_error_t
dr_txflow_address_at_index
(
	dr_txflow_t * flow,
	unsigned int intf,
	dante_ipv4_address_t * addr
);

/**
 * Get the destination router and flow name of the given transmit flow.
 * This only applies for dynamically configured TX flows.
 *
 * @param flow the flow handle
 * @param device_name_ptr on success, will point to the destination device's name
 *    May be NULL.
 * @param flow_name_ptr on success, will point to the destination flow's name
 *    May be NULL.
 */
aud_error_t
dr_txflow_get_destination
(
	dr_txflow_t * flow,
	char ** device_name_ptr,
	char ** flow_name_ptr
);

/**
 * Release a flow handle (does not affect the underlying flow)
 *
 * @param flow_ptr a pointer to a flow handle
 */
aud_error_t
dr_txflow_release
(
	dr_txflow_t ** flow_ptr
);

//----------------------------------------------------------
// RX flow operations
//----------------------------------------------------------

/**
 * Get the number of flows currently being received by this
 * device
 * 
 * @param device the device object
 */
uint16_t
dr_device_num_rxflows
(
	dr_device_t * device
);

/**
 * Get a handle to a particular flow by index
 *
 * @param device the device object
 * @param index The 0-based flow index in the device's data
 * @param flow_ptr a pointer to a flow handle
 */
aud_error_t
dr_device_rxflow_at_index
(
	dr_device_t * device,
	uint16_t index,
	dr_rxflow_t ** flow_ptr
);

/**
 * Get a handle to the receive flow with the given high-level
 * bundle identifier.
 *
 * @param device the device object
 * @param id The 1-based flow id
 * @param flow_ptr a pointer to a flow handle
 */
aud_error_t
dr_device_rxflow_with_id
(
	dr_device_t * device,
	dante_id_t id,
	dr_rxflow_t ** flow_ptr
);

/**
 * Return the rx flow, if any, that contains the given channel. If no such
 * flow exists then return NULL.
 * 
 * @param device the device object
 * @param rx the receive channel of interest
 * @param flow_ptr a pointer to a flow handle 
 *
 * @note In a stable system there is only one receive flow for any
 *   subscription. There may in rare cases be more than one flow per
 *   subscription; these states are transient and the system will
 *   stabilise on one flow per subscription.
 */
aud_error_t
dr_device_rxflow_with_channel
(
	dr_device_t * device,
	const dr_rxchannel_t * rx,
	dr_rxflow_t ** flow_ptr
);

/**
 * Get the high-level identifier for the given receive flow
 *
 * @param flow the flow handle
 * @param id_ptr a pointer to a location to store the flow's id
 */
aud_error_t
dr_rxflow_get_id
(
	dr_rxflow_t * flow,
	dante_id_t * id_ptr
);

/**
 * Get the user-friendly textual name for the given receive flow
 *
 * @param flow the flow handle
 * @param name_ptr a pointer to a location to store the flow's id
 */
aud_error_t
dr_rxflow_get_name
(
	dr_rxflow_t * flow,
	char ** name_ptr
);

/**
 * Is this flow using a multicast address?
 *
 * @param flow the flow handle
 * @param is_multicast_ptr a pointer to a location to store the flow's 'multicast' status
 */
aud_error_t
dr_rxflow_is_multicast
(
	dr_rxflow_t * flow,
	aud_bool_t * is_multicast_ptr
);

/**
 * Get the sample rate and/or encoding for a given flow
 *
 * @param flow the flow handle
 * @param samplerate_ptr a pointer to a location to store the flow's sample rate
 * @param encoding_ptr a pointer to a location to store the flow's encoding
 */
aud_error_t
dr_rxflow_get_format
(
	dr_rxflow_t * flow,
	dante_samplerate_t * samplerate_ptr,
	dante_encoding_t * encoding_ptr
);

/**
 * What is the name of the transmitting device from which this flow will
 * receive audio?
 *
 * If the flow has been configured with a template (either multicast or
 * unicast) then this value is the one specified by the template. Note
 * that dynamically configured flows might also set this value so the
 * presence of a value should not be taken as evidence for the existence
 * of a template.
 *
 * @param flow the receive flow
 * @param tx_device_name_ptr a pointer to a location to store a pointer
 *   to the flow's tx device name
 */
aud_error_t
dr_rxflow_get_tx_device_name
(
	const dr_rxflow_t * flow,
	char ** tx_device_name_ptr
);

/**
 * What is the name of the transmitting from from which this flow will
 * receive audio?
 *
 * If the flow has been configured with a multicast template then this
 * value is the one specified by the template. Dynamically configured
 * flows might also set this value so the presence of a value should not
 * be taken as evidence for the existence of a template.
 *
 * @param flow the receive flow
 * @param tx_flow_name_ptr a pointer to a location to store a pointer
 *   to the flow's tx flow name
 */
aud_error_t
dr_rxflow_get_tx_flow_name
(
	const dr_rxflow_t * flow,
	char ** tx_flow_name_ptr
);

/**
 * Get a bitset indicating the interfaces for which this flow has an active connection
 * (ie. it is receiving audio). Each bit represents one device interface; a bit is set only
 * if the flow has an address on that interface AND it is receiving audio on that interface.
 *
 * @param flow the flow handle
 * @param connections_active_ptr a pointer to a location to store a bitmask of the flows active connections
 *
 * @note This information is not available on certain older Dante devices; if this is the
 *   case the function returns AUD_ERR_VERSION.
 */
aud_error_t
dr_rxflow_get_connections_active
(
	dr_rxflow_t * flow,
	uint16_t * connections_active_ptr
);

/**
 * How many slots are in the given flow?
 *
 * @param flow the flow handle
 * @param num_slots_ptr a pointer to a location to store the flow's slot count
 */
aud_error_t
dr_rxflow_num_slots
(
	dr_rxflow_t * flow,
	uint16_t * num_slots_ptr
);

/**
 * How many receive channels are receiving the audio in the given flow slot
 *
 * @param flow the flow handle
 * @param slot the zero-based slot index
 * @param num_slot_channels_ptr a pointer to a location to store the number of 
 *   channels receiving from this flow slot.
 */
aud_error_t
dr_rxflow_num_slot_channels
(
	dr_rxflow_t * flow,
	uint16_t slot,
	uint16_t * num_slot_channels_ptr
);

/**
 * Get the n'th rxchannel receiving audio from the given slot.
 * n is a zero-based index.
 *
 * @param flow the flow handle
 * @param slot the zero-based slot index
 * @param n the zero-based slot channel index
 * @param channel_ptr a pointer to a location to store the n'th channel
 *   receiving from this flow slot.
 */
aud_error_t
dr_rxflow_slot_channel_at_index
(
	dr_rxflow_t * flow,
	uint16_t slot,
	uint16_t n,
	dr_rxchannel_t ** channel_ptr
);

/**
 * On how many interfaces is the flow operating?
 *
 * @param flow the flow handle
 * @param num_addresses_ptr a pointer to a location to store the flow's interface count
 */
aud_error_t
dr_rxflow_num_interfaces
(
	dr_rxflow_t * flow,
	uint16_t * num_addresses_ptr
);

/**
 * Get the interface-specific destination address and port for the
 * flow. Interfaces are indentified as a zero-based index,
 * with 0 being ther primary network. For unconfigured interfaces the
 * address port will be 0
 *
 * @param flow the flow handle
 * @param intf the zero-based interface index
 * @param addr a pointer to a location to store the flow's address on the given interface
 */
aud_error_t
dr_rxflow_address_at_index
(
	dr_rxflow_t * flow,
	unsigned int intf,
	dante_ipv4_address_t * addr
);

/**
 * Release a flow handle (does not affect the underlying flow)
 *
 * @param flow_ptr A pointer to the flow handle
 */
aud_error_t
dr_rxflow_release
(
	dr_rxflow_t ** flow_ptr
);

//----------------------------------------------------------
// String/Error handling
//----------------------------------------------------------

/**
 * Get a string for the given result value. Handles routing API errors
 * as well as platform errors
 *
 * @param result the error code
 * @param errbuf the buffer to hold the formatted message
 */
const char *
dr_error_message
(
	aud_error_t result,
	aud_errbuf_t errbuf
);

/**
 * Get a string for the given device state
 * 
 * @param state the device state
 */
const char *
dr_device_state_to_string
(
	dr_device_state_t state
);

/**
 * Get a string for the given component
 * 
 * @param component the device component
 */
const char *
dr_device_component_to_string
(
	dr_device_component_t component
);

/**
 * Get a string for the given change index (includes component indexes)
 * 
 * @param change_index the change index or component index
 */
const char *
dr_device_change_index_to_string
(
	dr_device_change_index_t change_index
);

#ifdef DR_LEGACY
// include template code to get access to tx flow configuration
#include "routing_flows.h"
// include legacy types and functions from main file
#include "routing_legacy.h"
#endif

#ifdef __cplusplus
}
#endif

#endif







