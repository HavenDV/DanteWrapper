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
 * @file browsing_types.h
 * Core types and definitions for Dante browsing implementation
 */

#ifndef _BROWSING_TYPES_H
#define _BROWSING_TYPES_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/dante_common.h"
#include "dante/dante_domains.h"
#include "dante/dante_aes67.h"
#include "conmon/conmon.h"
#endif

/**
 * The maximum number of interfaces on which the browsing API will run
 */
#define DB_BROWSE_MAX_INTERFACE_INDEXES 2

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Browse types and constants
//----------------------------------------------------------

struct db_browse;
/**
* The main type in the browsing API is the db_browse_t object.
* This object encapsulates one or more browses and accumulates
* the discovered information into a tree that can be interrogated by the
* host API.
*/
typedef struct db_browse db_browse_t;

struct db_browse_network;
/**
 * The root element in a browsing tree is the 'network'.
 * A network has a set of devices as children.
 */
typedef struct db_browse_network db_browse_network_t;

struct db_browse_device;
/**
 * The device node in a browsing tree. A device stores
 * information about dante media devices that a browse has discovered.
 * A device node has a network node as its parent and a set of channels
 * as its children
 */
typedef struct db_browse_device db_browse_device_t;

struct db_browse_channel;
/**
 * A transmit channel node in a browsing tree. A
 * channel node stores information about dante media channels
 * that a browse has discovered. A channel node has a device
 * node as its parent and a set of label nodes as its children
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
typedef struct db_browse_channel db_browse_channel_t;

struct db_browse_label;
/**
 * A label node in the browsing tree. A label node
 * stores information about dante transmit channel labels
 * that a browse has discovered. A label node has a channel
 * node as its parent and no children.
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
typedef struct db_browse_label db_browse_label_t;

/**
 * An AES67 flow node. An AES67 flow node stores
 * information about the AES67 SAP/SDP structure that a browse has discovered.
 * @deprecated Browsing for AES67 flows specifically is deprecated. Use DB_BROWSE_TYPE_SDP to discover AES67 and/or SMPTE SDP descriptors.
 */
typedef struct db_browse_sdp db_browse_aes67_t;

/**
* An Sdp flow node. An Sdp flow node stores
* information about the AES67/SMPTE SDP structure that a browse has discovered.
*/
typedef struct db_browse_sdp db_browse_sdp_t;

//----------
typedef enum
{

	/** The media (audio) routing interface of a Dante device */
	DB_BROWSE_TYPE_MEDIA_DEVICE     = 0x0001,

	/**
	 * A media (audio) channel available for subscription
	 * @note Media Channels will not be discovered when browsing a Dante Domain
	 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
	 */
	DB_BROWSE_TYPE_MEDIA_CHANNEL    = 0x0002,

	/** The control interface of a Dante device */
	DB_BROWSE_TYPE_CONMON_DEVICE    = 0x0004,

	/**
	 * Devices that have reverted to 'safe' mode
	 * @note Safe Mode devices will not be discovered when browsing a Dante Domain
	 */
	DB_BROWSE_TYPE_SAFE_MODE_DEVICE = 0x0008,

	/**
	 * Devices that have reverted to 'upgrade' mode
	 * @note Upgrade Mode devices will not be discovered when browsing a Dante Domain
	 */
	DB_BROWSE_TYPE_UPGRADE_MODE_DEVICE = 0x0010,

	/** Dante Via Devices */
	DB_BROWSE_TYPE_VIA_DEVICE = 0x0020,

	/** AES67 Devices
	* Discovers SAP/SDP adverts in the Ad-Hoc domain.
	* @deprecated This is only applicable in the Ad-Hoc domain.  Use DB_BROWSE_TYPE_SDP to discover all available SDP descriptors.
	*/
	DB_BROWSE_TYPE_AES67_FLOW = 0x0040,

	/** SDP Descriptors
	* Discovers available SDP descriptors in the current Dante domain (including AES67 SAP/SDP adverts on the local subnet).
	*/
	DB_BROWSE_TYPE_SDP = 0x0080

} browse_types;

/**
 * A helper definition denoting all media types
 *
 * @deprecated Browsing for channels/labels is deprecated. Use DB_BROWSE_TYPE_MEDIA_DEVICE to discover devices and then use routing api to query the channels/labels for a device.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
#define DB_BROWSE_TYPES_ALL_MEDIA \
	( DB_BROWSE_TYPE_MEDIA_DEVICE \
	| DB_BROWSE_TYPE_MEDIA_CHANNEL \
	)

/**
 * A helper definition denoting all browsable types
 *
 * @deprecated Browsing for channels/labels is deprecated and can lead to network instability. Applications should not use DB_BROWSE_TYPES_ALL as this will enable channel browsing.
 * @see DB_BROWSE_TYPE_MEDIA_CHANNEL
 */
#define DB_BROWSE_TYPES_ALL \
	( DB_BROWSE_TYPE_MEDIA_DEVICE \
	| DB_BROWSE_TYPE_MEDIA_CHANNEL \
	| DB_BROWSE_TYPE_CONMON_DEVICE \
	| DB_BROWSE_TYPE_SAFE_MODE_DEVICE \
	| DB_BROWSE_TYPE_UPGRADE_MODE_DEVICE \
	| DB_BROWSE_TYPE_VIA_DEVICE \
	| DB_BROWSE_TYPE_AES67_FLOW \
	| DB_BROWSE_TYPE_SDP \
	)

/**
 * A helper definition denoting all browsable types
 */
#define DB_BROWSE_TYPES_ALL_DEVICES \
	( DB_BROWSE_TYPE_MEDIA_DEVICE \
	| DB_BROWSE_TYPE_CONMON_DEVICE \
	| DB_BROWSE_TYPE_SAFE_MODE_DEVICE \
	| DB_BROWSE_TYPE_UPGRADE_MODE_DEVICE \
	| DB_BROWSE_TYPE_VIA_DEVICE \
	)

/**
 * The browsing API supports different types of Dante advertisements.
 * A single browse object can support one or more advertisement types.
 */
typedef uint32_t db_browse_types_t;

/**
 * The maximum number of interfaces on which the browsing API will run
 */
#define DB_BROWSE_MAX_INTERFACE_INDEXES 2

/**
 * Browsing configuration options.
 */
typedef struct
{
	/**
	* The number of networks on which to browse, not including localhost.
	* Must be a number between 1 and DB_BROWSE_MAX_INTERFACE_INDEXES.
	* If num_interface_indexes is set to 1 and the interface index is set to kDNSServiceInterfaceIndexAny,
	* the browse will lock to a single interface once the first non-localhost advert has been found.
	*
	* The default value is 1
	*/
	uint32_t num_interface_indexes;

	/**
	* The interface index(es) for the networks on wihich browsing will take place.
	* If num_interface_indexes is set to 1 and the interface index is set to kDNSServiceInterfaceIndexAny,
	* the browse will lock to a single interface once the first non-localhost advert has been found.
	*
	* The default value is kDNSServiceInterfaceIndexAny
	*/
	uint32_t interface_indexes[DB_BROWSE_MAX_INTERFACE_INDEXES];
	/**
	* The bit mask of interface_indexes.
	* If a bit is turned on for a particular interface index ,then it means we can browse on that interface.
	* If a bit is turned on for a particular interface index ,then it means we will not browse on that interface.
	*
	* The default value is (uint32_t)-1 ,all interfaces are enabled by default
	*/
	uint32_t interface_bitmask;

	/**
	* Specify whether or not to track adverts on the localhost interface.
	*
	* The default value is TRUE
	*/
	aud_bool_t localhost;

	/** how long to wait on a resolution before giving up */
	aud_utime_t resolve_timeout;
} db_browse_config_t;


//----------------------------------------------------------
// Per-change callbacks
//----------------------------------------------------------

/**
* Browsing node types.
*/
typedef enum
{
	DB_NODE_TYPE_DEVICE     = 0,
	DB_NODE_TYPE_CHANNEL    = 1,  // @deprecated
	DB_NODE_TYPE_LABEL      = 2,  // @deprecated
	DB_NODE_TYPE_AES67_SAP  = 3,  // @deprecated
	DB_NODE_TYPE_SDP        = 3,
} db_node_type_t;

/**
 * Aggregate type for all browsing node types.
 * Use the value in 'type' to determine which member of the '_' struct to use.
 */
typedef struct
{
	/**  A flag indicating the type of node that has changed */
	db_node_type_t type;

	union
	{
		// Node pointer for device nodes
		db_browse_device_t * device;

		// Node pointer for channel nodes
		db_browse_channel_t * channel;

		// Node pointer for label nodes
		db_browse_label_t * label;

		// Node pointer for AES67 nodes
		db_browse_aes67_t * aes67;

		// Node pointer for AES67 nodes
		db_browse_sdp_t * sdp;

	} _; //!< Union of node pointers
} db_node_t;

/**
 * Changes that can occur for a node.
 *
 * These are used for the db_browse_node_changed_fn callback.
 */
typedef enum
{
	DB_NODE_CHANGE_ADDED,
	// The node was added.
	DB_NODE_CHANGE_MODIFIED,
	// The node previously existed but has been changed.
	DB_NODE_CHANGE_REMOVED
	// The node is being removed.
} db_node_change_t;

/**
 * Callback for a node changed event.
 *
 * @param browse core browsing object
 * @param node node that was changed
 * @param change type of change
 */
typedef void
db_browse_node_changed_fn
(
	db_browse_t * browse,
	const db_node_t * node,
	db_node_change_t change
);

//----------------------------------------------------------
// Network browse functions
//----------------------------------------------------------

// Devices
//----------------------------------------------------------

/**
 * How many devices are in the current network tree?
 *
 * @param tree core network browsing object
 */
unsigned int
db_browse_network_get_num_devices
(
	const db_browse_network_t * tree
);

/**
 * Get the device at the given child index for the current network tree
 *
 * @param tree core network browsing object
 * @param index entry index (from 0)
 *
 * @return NULL if the index is invalid
 */
db_browse_device_t *
db_browse_network_device_at_index
(
	const db_browse_network_t * tree,
	unsigned int index
);


/**
 * Get the device node with the given name for the current network tree
 *
 * @param tree core network browsing object
 * @param device_name name of device
 *
 * @return NULL if no such device exists in the network tree
 */
db_browse_device_t *
db_browse_network_device_with_name
(
	const db_browse_network_t * tree,
	const char * device_name
);

/**
 * Get the device node with the given conmon instance ID (if one exists).
 *
 * @param tree core network browsing object
 * @param instance_id instance indentifier of device
 *
 * @return NULL if no such device has been found.
 */
db_browse_device_t  *
db_browse_network_device_with_instance_id
(
	const db_browse_network_t * tree,
	const conmon_instance_id_t * instance_id
);


/**
 * Get core network browsing object associated with device.
 *
 * @param device device node
 */
db_browse_network_t *
db_browse_device_get_network
(
	const db_browse_device_t * device
);


/**
 * Get the types of information available for this device across all
 * interfaces.
 * If the function returns '0' then the device was not explictly
 * discovered, but exists to encapsulate a channel or label that was
 * discovered for this device.
 *
 * @param device device node
 */
db_browse_types_t
db_browse_device_get_browse_types
(
	const db_browse_device_t * device
);

/**
 * Get the types of information seen for this device on a given network
 * If the function returns '0' then no device information has been seen
 * on the given network.
 *
 * @param device device node
 * @param network the network index. This is NOT an interface index
 */
db_browse_types_t
db_browse_device_get_browse_types_on_network
(
	const db_browse_device_t * device,
	unsigned int network
);

/**
 * Get the network interface bit mask for a specific browse type for this device
 * If the function returns '0' then no device information has been seen
 * for a particular browse type.
 *
 * @param device device node
 * @param browse_types the browse types for which network interface mask is required
 *
 */
unsigned int
db_browse_device_get_networks_for_browse_type
(
	const db_browse_device_t * device,
	db_browse_types_t types
);

/**
 * Get the types of information seen for this device on the localhost interface.
 * If the function returns '0' then no device information has been seen
 * on localhost
 *
 * @param device device node
 */
db_browse_types_t
db_browse_device_get_browse_types_on_localhost
(
	const db_browse_device_t * device
);

/**
 * What is the current ARCP protocol version for this device?
 * This value is only known if
 * a media advertisement for this device has been discovered
 * That is, the DB_BROWSE_TYPE_MEDIA_DEVICE bit must be set when calling
 * db_browse_device_get_browse_types is called for this device.
 *
 * @param device device node
 */
const dante_version_t *
db_browse_device_get_arcp_version
(
	const db_browse_device_t * device
);

/**
 * What is the router application version for this device?
 * This value is only known if
 * a media advertisement for this device has been discovered
 * That is, the DB_BROWSE_TYPE_MEDIA_DEVICE bit must be set when calling
 * db_browse_device_get_browse_types is called for this device.
 *
 * @param device device node
 */
const dante_version_t *
db_browse_device_get_router_version
(
	const db_browse_device_t * device
);


/**
 * What is the minimum ARCP protocol version supported for this device?
 * This value is only known if
 * a media advertisement for this device has been discovered
 * That is, the DB_BROWSE_TYPE_MEDIA_DEVICE bit must be set when calling
 * db_browse_device_get_browse_types is called for this device.
 *
 * @param device device node
 */
const dante_version_t *
db_browse_device_get_arcp_min_version
(
	const db_browse_device_t * device
);

/**
 * What is the router info string for this device?
 * This value is only known if
 * the media advertisement for this device has been discovered
 * That is, the DB_BROWSE_TYPE_MEDIA_DEVICE bit must be set when calling
 * db_browse_device_get_browse_types is called for this device.
 *
 * @param device device node
 */
const char *
db_browse_device_get_router_info
(
	const db_browse_device_t * device
);

/**
 * Get the name for this device.
 *
 * @param device device node
 */
const char *
db_browse_device_get_name
(
	const db_browse_device_t * device
);

/**
 * Get the default name for this device.
 *
 * @param device device node
 */
const char *
db_browse_device_get_default_name
(
	const db_browse_device_t * device
);

/**
 * Get the safe mode version for this device.
 *
 * @param device device node
 */
uint16_t
db_browse_device_get_safe_mode_version
(
	const db_browse_device_t * device
);

/**
 * Get the upgrade mode version for this device.
 *
 * @param device device node
 */
uint16_t
db_browse_device_get_upgrade_mode_version
(
	const db_browse_device_t * device
);

/**
 * Get the dante instance id for this device (if known).
 * This value is only known if
 * the safe mode advertisement for this device has been discovered
 * That is, the DB_BROWSE_TYPE_SAFE_MODE_DEVICE bit must be set when calling
 * db_browse_device_get_browse_types is called for this device.
 *
 * @param device device node
 */
const dante_instance_id_t *
db_browse_device_get_safe_mode_instance_id
(
	const db_browse_device_t * device
);

/**
 * Get the conmon instance id for this device (if known).
 * This value is only known if
 * the conmon advertisement for this device has been discovered
 * That is, the DB_BROWSE_TYPE_CONMON_DEVICE bit must be set when calling
 * db_browse_device_get_browse_types is called for this device.
 *
 * @param device device node
 */
const conmon_instance_id_t *
db_browse_device_get_instance_id
(
	const db_browse_device_t * device
);

/**
 * Get the conmon vendor id for this device (if known).
 * This value is only known if
 * the conmon advertisement for this device has been discovered
 * That is, the DB_BROWSE_TYPE_CONMON_DEVICE bit must be set when calling
 * db_browse_device_get_browse_types is called for this device.
 *
 * This value may not be set for all Dante devices.
 *
 * @param device device node
 */
const dante_id64_t *
db_browse_device_get_vendor_id
(
	const db_browse_device_t * device
);

/**
 * Get the conmon vendor-specific broadcast channel address
 * for this device (if known).
 *
 * This value is only known if
 * the conmon advertisement for this device has been discovered
 * That is, the DB_BROWSE_TYPE_CONMON_DEVICE bit must be set when calling
 * db_browse_device_get_browse_types is called for this device.
 *
 * This value may not be set for all Dante devices.
 *
 * @param device device node
 */
uint32_t
db_browse_device_get_vendor_broadcast_address
(
	const db_browse_device_t * device
);

/**
 * Get the manufacturer id for this device (if known).
 * This value is only known if the conmon or media advertisement for this device
 * has been discovered (and the device supports it)
 * That is, the DB_BROWSE_TYPE_CONMON_DEVICE or DB_BROWSE_TYPE_MEDIA_DEVICE bit
 * must be set when calling db_browse_device_get_browse_types for this device.
 *
 * This value may not be set for all Dante devices.
 *
 * @param device device node
 */
const dante_id64_t *
db_browse_device_get_manufacturer_id
(
	const db_browse_device_t * device
);

/**
 * Get the model id for this device (if known).
 * This value is only known if the conmon or media advertisement for this device
 * has been discovered (and the device supports it)
 * That is, the DB_BROWSE_TYPE_CONMON_DEVICE or DB_BROWSE_TYPE_MEDIA_DEVICE bit
 * must be set when calling db_browse_device_get_browse_types for this device.
 *
 * This value may not be set for all Dante devices.
 *
 * @param device device node
 */
const dante_id64_t *
db_browse_device_get_model_id
(
	const db_browse_device_t * device
);

uint16_t
db_browse_device_get_via_port
(
	const db_browse_device_t * device
);

const dante_version_t *
db_browse_device_get_via_min_version
(
	const db_browse_device_t * device
);

const dante_version_t *
db_browse_device_get_via_curr_version
(
	const db_browse_device_t * device
);

/**
 * Get user context for this device node
 *
 * The context pointer provides a reference back into the caller's data structures.
 *
 * @param device device node
 */
void *
db_browse_device_get_context
(
	const db_browse_device_t * device
);

/**
 * Set user context for this device node
 *
 * @param device device node
 * @param context user context
 */
void
db_browse_device_set_context
(
	db_browse_device_t * device,
	void * context
);

/**
* Re-confirm the validity of one or more aspects of this device. Optionally also reconfirm all children
*
* @param device device node
* @param browse_types the browse types for which re-confirmation is required. If this value is set to
*   zero then all discovered types will be reconfirmed.
* @param reconfirm_children should this node's children also be reconfirmed?
*
* @note This is only applicable when browsing in the adhoc domain.  AUD_ERR_NOTSUPPORTED is returned if the browse is not browsing in the adhoc domain.
*/
aud_error_t
db_browse_device_reconfirm
(
	db_browse_device_t * device,
	db_browse_types_t browse_types, // if 0 then reconfirm all known devices for thie
	aud_bool_t reconfirm_children
);

/**
* Re-resolve the validity of given aspect of a device.
*
* @param device device node
* @param browse_types are the browse types for which re-resolve is required. At the moment, this function can reresolve
* DB_BROWSE_TYPE_MEDIA_DEVICE or DB_BROWSE_TYPE_CONMON_DEVICE or DB_BROWSE_TYPE_SAFE_MODE_DEVICE or
* DB_BROWSE_TYPE_UPGRADE_MODE_DEVICE.
* If browse_types is set to zero, it will re-resolve all of the above aspects.
*
* @note This is only applicable when browsing in the adhoc domain.  AUD_ERR_NOTSUPPORTED is returned if the browse is not browsing in the adhoc domain.
*/
aud_error_t
db_browse_device_reresolve
(
	db_browse_device_t * device,
	db_browse_types_t browse_types // if 0 then reconfirm all known devices for thie
);

// AES67 Flows
//----------------------------------------------------------

/**
* Get a pointer to the SDP descriptor for a browsed AES67 node
* @param aes67 AES67 node
* @param sdp_desc_ptr pointer to assign to point to the SDP descriptor
* @note Do NOT deallocate the dante_sdp_descriptor_t structure obtained from this function.
* @deprecated Browsing for AES67 flows specifically is deprecated. Use DB_BROWSE_TYPE_SDP to discover AES67 and/or SMPTE SDP descriptors.
*/
void db_browse_aes67_get_sdp_descriptor
(
	const db_browse_aes67_t *aes67,
	const dante_sdp_descriptor_t **sdp_desc_ptr
);

/**
 * How many AES67 SDP descriptors have been discovered?
 *
 * @param browse browsing object
 */
unsigned int
db_browse_get_num_aes67_descriptors
(
	const db_browse_t * browse
);

/**
 * Get the AES67 SDP descriptor at the given child index
 *
 * @param browse browsing object
 * @param index entry index (from 0)
 *
 * @return NULL if the index is invalid
 *
 * @note Indexes (and total entries) may change after any call to db_browse_process
 * @note Returned descriptor may become invalid after any call to db_browse_process
 * @deprecated Browsing for AES67 flows specifically is deprecated. Use DB_BROWSE_TYPE_SDP to discover AES67 and/or SMPTE SDP descriptors.
 */
const dante_sdp_descriptor_t *
db_browse_aes67_descriptor_at_index
(
	const db_browse_t * browse,
	unsigned int index
);


// SDP Descriptors
//----------------------------------------------------------

/**
* Get a pointer to the Descriptor for a browsed SDP node
* @param sdp SDP node
* @param sdp_desc_ptr pointer to assign to point to the SDP descriptor
* @note Do NOT deallocate the dante_sdp_descriptor_t structure obtained from this function.
*/
void db_browse_sdp_get_descriptor
(
	const db_browse_sdp_t *sdp,
	const dante_sdp_descriptor_t ** sdp_desc_ptr
);


/**
* How many SDP descriptors have been discovered?
*
* @param browse browsing object
*/
unsigned int
db_browse_get_num_sdp_descriptors
(
	const db_browse_t * browse
);

/**
* Get the SDP descriptor at the given child index
*
* @param browse browsing object
* @param index entry index (from 0)
*
* @return NULL if the index is invalid
*
* @note Indexes (and total entries) may change after any call to db_browse_process
* @note Returned descriptor may become invalid after any call to db_browse_process
*/
const dante_sdp_descriptor_t *
db_browse_sdp_descriptor_at_index
(
	const db_browse_t * browse,
	unsigned int index
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // _BROWSING_TYPES_H
