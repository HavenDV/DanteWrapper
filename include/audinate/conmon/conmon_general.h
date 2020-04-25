/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : ConMon common types and functions.
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file conmon_general.h
 * ConMon generic types and definitions
 */
#ifndef _CONMON_GENERAL_H
#define _CONMON_GENERAL_H

#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#include "dante/dante_common.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The major version of this conmon release
 */
#define CONMON_VERSION_MAJOR  4

/**
 * The minor version of this conmon release
 */
#define CONMON_VERSION_MINOR  0

/**
 * The bugfix version of this conmon release
 */
#define CONMON_VERSION_BUGFIX 3

/**
 * The complete version of this conmon release as a packed 32-bit integer
 */
#define CONMON_VERSION_PACKED \
	((CONMON_VERSION_MAJOR << 24) | (CONMON_VERSION_MINOR << 16) | CONMON_VERSION_BUGFIX)

//----------------------------------------------------------
// ConMon standard types and constants
//----------------------------------------------------------

/**
 * An upper limit on the number of networks over which conmon will operate
 */
#define CONMON_MAX_NETWORKS 2

/**
 * The length (in bytes) of a conmon device ID
 */
#define CONMON_DEVICE_ID_LENGTH 8

/**
 * Each conmon device has a unique 64-bit identifier
 */
//typedef struct
//{
//	/** the device id data as an 8-byte array */
//	uint8_t data[CONMON_DEVICE_ID_LENGTH];
//} conmon_device_id_t;
typedef dante_device_id_t conmon_device_id_t;


/**
 * A helper function for device ID comparison
 *
 * @param X first device identifier
 * @param Y second device identifier
 */
#define conmon_device_id_equals(X,Y) (!(memcmp((X),(Y),CONMON_DEVICE_ID_LENGTH)))


enum { CONMON_DEVICE_ID_STRLEN = CONMON_DEVICE_ID_LENGTH * 2 };

/**
 * A buffer long enough to contain a device id string as formatted using
 * conmon_device_id_to_str.
 */
typedef char conmon_device_id_str_t [CONMON_DEVICE_ID_STRLEN + 1];

/**
 * Convert device ID to hex string
 *
 * @param dev_id source device identifier
 * @param str_buf destination string buffer.
 *  Must be at least CONMON_DEVICE_ID_STRLEN + 1 in size
 */
char *
conmon_device_id_to_str
(
	const conmon_device_id_t * dev_id,
	char * str_buf
		// Must be at least CONMON_DEVICE_ID_STRLEN + 1 in size
);

/**
 * Parse device ID from hex string
 *
 * @param dev_id destination device identifier
 * @param str source buffer containing device ID as hex string.
 *   Leading 0x is optional.
 */
conmon_device_id_t *
conmon_device_id_from_str
(
	conmon_device_id_t * dev_id,
	const char * str
		// Leading 0x is optional
);

/**
 * Parse device ID from hex string
 * Input buffer must be formatted exactly as output from conmon_device_id_to_str.
 *
 * @param dev_id destination device identifier
 * @param buf source buffer containing hexadecimal characters
 * @param len number of hex characters in buffer
 */
conmon_device_id_t *
conmon_device_id_from_buf
(
	conmon_device_id_t * dev_id,
	const char * buf,
	size_t len
);


/**
 * If there are multiple instances of conmon on a single device,
 * each one will have a unique process ID
 */
typedef dante_process_id_t conmon_process_id_t;

/**
 * Each conmon instance is uniquely identifiable by its instance id,
 * a combination of it's device id and process id
 */
//typedef struct
//{
//		/** the conmon instance's device id */
//	conmon_device_id_t device_id;
//		/** the conmon instance's process id */
//	conmon_process_id_t process_id;
//} conmon_instance_id_t;
typedef dante_instance_id_t conmon_instance_id_t;

/**
 * A helper function for instance ID comparison
 *
 * @param id1 first instance ID
 * @param id2 second instance ID
 */
AUD_INLINE aud_bool_t
conmon_instance_id_equals
(
	const conmon_instance_id_t * id1,
	const conmon_instance_id_t * id2
) {
	return (memcmp(id1,id2,sizeof(conmon_instance_id_t)) ? AUD_FALSE : AUD_TRUE);
}

/**
 * The length (in bytes) of a conmon vendor ID
 */
#define CONMON_VENDOR_ID_LENGTH 8

/**
 * ConMon vendors are provided with a unique vendor ID. All ConMon messages include a
 * vendor ID, allowing clients to determine whether a given message is of interest.
 */
typedef struct
{
		/** the vendor id data as an 8-byte array */
	uint8_t data[CONMON_VENDOR_ID_LENGTH];
} conmon_vendor_id_t;

/**
 * A helper function for vendor ID comparison
 *
 * @param id1 first vendor ID
 * @param id2 second vendor ID
 */
//#define conmon_vendor_id_equals(X,Y) (!(memcmp((X),(Y),CONMON_VENDOR_ID_LENGTH)))
AUD_INLINE aud_bool_t
conmon_vendor_id_equals
(
	const conmon_vendor_id_t * id1,
	const conmon_vendor_id_t * id2
) {
	return (memcmp(id1,id2,sizeof(conmon_vendor_id_t)) ? AUD_FALSE : AUD_TRUE);
}

/**
 * The length (in bytes) of a conmon name, including null.
 */
#define CONMON_NAME_LENGTH 32

/**
 * The length (in bytes) of a conmon interface name, including null.
 */
#define CONMON_INTERFACE_NAME_LENGTH 64

/**
 * Each conmon device is identified using a unique alpha-numeric name. A conmon name
 * is used when sending control messages and when subscribing to remote channels.
 */
typedef char conmon_name_t[CONMON_NAME_LENGTH];

/**
 * The name of an interface. This type is deprecated and should not be used
 *
 * @deprecated Interface names are generally only of interest to local devices.
 *    Should this information be needed, additional messages and types will
 *    required in order to handle unicode names etc..
 */
//typedef char conmon_interface_name_t[CONMON_INTERFACE_NAME_LENGTH];

/**
 * Copy a string to a conmon name, clipping if necessary
 * Returns length of dst after copy, or -1 if dst is NULL.
 *
 * @param dst destination string buffer (at least CONMON_NAME_LENGTH)
 * @param src source string
 */
int
conmon_name_copy (char * dst, const char * src);

enum
{
	CONMON_MEDIA_CHANNEL_TYPE_AUDIO_TX,
	CONMON_MEDIA_CHANNEL_TYPE_AUDIO_RX
};

/**
 * ConMon devices may monitor media channels, providing metering or
 * monitoring information for particular media channels. Media
 * channels are identified by a 16-bit media_channel_type and
 * a 16-bit index.
 */
//typedef uint16_t conmon_media_channel_type_t;

//---------------------------------------------------------
// ConMon channel types
//----------------------------------------------------------

enum conmon_channel_type
{
	CONMON_CHANNEL_TYPE_CONTROL,          // control messages          [always unicast]
	CONMON_CHANNEL_TYPE_METERING,         // media metering messages   [always unicast (for now)]
	CONMON_CHANNEL_TYPE_STATUS,           // status messages           [unicast+multicast TX, unicast or multicast RX]
	CONMON_CHANNEL_TYPE_BROADCAST,        // topology messages         [always multicast]
	CONMON_CHANNEL_TYPE_LOCAL,            // local (inter-device only) messages
	CONMON_CHANNEL_TYPE_SERIAL,           // serial bridging messages  [multicast, direct to client]
	CONMON_CHANNEL_TYPE_KEEPALIVE,        // device keepalive          [subscription only, no messages]
	CONMON_CHANNEL_TYPE_VENDOR_BROADCAST, // vendor-specific broadcast [always multicast, per-vendor multicast addresses]
	CONMON_CHANNEL_TYPE_MONITORING,       // monitoring messages       [unicast+multicast TX, unicast or multicast RX]
	CONMON_NUM_CHANNELS,

	CONMON_CHANNEL_TYPE_NONE = 0xFFFF // marker for illegal channel values
};

/**
 * A Conmon device has several channels, identified by channel_type.
 */
typedef uint16_t conmon_channel_type_t;

enum conmon_channel_direction
{
	CONMON_CHANNEL_DIRECTION_TX, // conmon messages transmitted from this device
	CONMON_CHANNEL_DIRECTION_RX  // conmon messages transmitted to this device
};

/**
 * Messages on a conmon channel have a direction, specified relative to
 * the server. RX messages are received from the network, TX messages are transmitted
 * onto the network.
 */
typedef uint16_t conmon_channel_direction_t;

/**
 * The maximum number of TX or RX subscriptions available to an individual channel
 */
#define CONMON_MAX_CHANNEL_SUBSCRIPTIONS 16 // max TX / RX subscriptions on any given channel

//----------------------------------------------------------
// Conmon Interfaces & addresses
//----------------------------------------------------------

/**
 * The length of an ethernet mac address
 */
#define CONMON_MAC_ADDRESS_LENGTH 6

/**
 * ConMon operates on one or more networks, each of which has a unique ID
 */
typedef uint16_t conmon_network_id_t;

/**
 * A type for representing active networks as a bitmask
 */
typedef uint16_t conmon_network_mask_t;

/**
 * A type for representing the OS-specified index for a given interface
 */
typedef uint32_t conmon_interface_index_t;

enum
{
	CONMON_NETWORK_FLAGINDEX_RESERVED,
	CONMON_NETWORK_FLAGINDEX_STATIC
};

enum
{
	CONMON_NETWORK_FLAG_RESERVED = (1 << CONMON_NETWORK_FLAGINDEX_RESERVED),
	CONMON_NETWORK_FLAG_STATIC = (1 << CONMON_NETWORK_FLAGINDEX_STATIC)
};

/**
 * Information regarding a single conmon network
 */
typedef struct
{
		/** the name of the interface on which this network is operating */
	//conmon_interface_name_t interface_name;
	char deprecated[CONMON_INTERFACE_NAME_LENGTH];

		/** the network id for this network */
	//conmon_network_id_t network_id;

		/** the index of the interface on which this network is operating */
	conmon_interface_index_t interface_index;

		/** flags for this interface */
	uint32_t flags;

		/** is the link up? */
	aud_bool_t is_up;

		/** the mac address of the interface on which this network is operating */
	uint8_t mac_address[CONMON_MAC_ADDRESS_LENGTH];

		/** The speed of this link in MB/s **/
	uint32_t link_speed;

		/** the ip address of the interface on which this network is operating (in <I>network</I> order) */
	uint32_t ip_address;

		/** the netmask of the interface (in <I>network</I> order) */
	uint32_t netmask;

		/** the gateway of the interface (in <I>network</I> order) */
	uint32_t gateway;

		/** the dns server of the interface (in <I>network</I> order) */
	uint32_t dns_server;

} conmon_network_t;

/**
 * A set of conmon networks.
 */
typedef struct
{
		/** the number of networks in the array */
	uint16_t num_networks;
		/** the network array */
	conmon_network_t networks[CONMON_MAX_NETWORKS];
} conmon_networks_t;

/**
 * Helper function to get a bitmask of the active network ids
 * in a conmon_networks structure
 *
 * @param networks conmon networks
 */
AUD_INLINE conmon_network_mask_t
conmon_networks_get_mask
(
	const conmon_networks_t * networks
) {
	conmon_network_mask_t mask;
	unsigned int n = networks->num_networks;

	mask = (1 << n) - 1;

	return mask;
}

/**
 * A conmon address is currently an IPv4 host/port pair.
 */
typedef struct
{
		/** ip address of the address (in <I>network</I> order) */
	uint32_t host;
		/** the local port number (in <I>host</I> order) */
	uint16_t port;
		/** a 2-byte field to ensure the structure is 4-byte aligned */
	uint16_t padding;
} conmon_address_t;

/**
 * A helper function for comparing conmon addresses
 *
 * @param p1 first address
 * @param p2 second address
 *
 * @return 0 iff p1 == p2, < 0 if p1 < p2, > 0 if p1 > p2
 *
 * Conceptually, performs p1 - p2 and returns a scalar with the same sign.
 */
int conmon_address_compare(const void * p1, const void * p2);

/**
 * A set of the conmon addresses acrosss one or more networks,
 * providing a single network 'endpoint' across multiple networks
 */
typedef struct
{
		/** the number of networks for which this endpoint has an address */
	uint16_t num_networks;
		/** the bitmask of network ids for which this endpoint has an address */
	conmon_network_mask_t network_mask;
		/** the endpoint address array */
	conmon_address_t addresses[CONMON_MAX_NETWORKS];
} conmon_endpoint_addresses_t;

/**
 * Helper function that compares multiple conmon endpoints
 *
 * @param a1 first address
 * @param a2 second address
 *
 * @return 0 iff a1 == a2, < 0 if a1 < a2, > 0 if a1 > a2
 *
 * Conceptually, performs a1 - a2 and returns a scalar with the same sign.
 */
int
conmon_endpoint_addresses_compare
(
	const conmon_endpoint_addresses_t * a1,
	const conmon_endpoint_addresses_t * a2
);


//----------------------------------------------------------
// ConMon subscriptions
//----------------------------------------------------------

/**
 * Subscriptions can change over time, both during the setup phase and
 * also as network conditions changed. Each conmon subscription has a status
 * that describes the current connection.
 */
typedef uint16_t conmon_rxstatus_t;

enum conmon_rxstatus
{
	CONMON_RXSTATUS_NONE           = 0x0000, // no subscription

	// transient setup states
	CONMON_RXSTATUS_PREPARING      = 0x0001, // preparing data structures etc..
	CONMON_RXSTATUS_RESOLVED       = 0x0002, // a subscription request is in progress

	// persistent states
	CONMON_RXSTATUS_UNRESOLVED     = 0x0010, // the device name is not yet resolved
	CONMON_RXSTATUS_UNICAST        = 0x0011, // the subscription is active and using unicast IP for transmission
	CONMON_RXSTATUS_MULTICAST      = 0x0012, // the subscription is active and using multicast IP for transmission
	CONMON_RXSTATUS_DOMAIN         = 0x0013, // the subscription is active using the domain connection

	// persistent failure states
	CONMON_RXSTATUS_COMMS_ERROR    = 0x0101, // couldn't send a subscription request
	CONMON_RXSTATUS_NO_CONNECTION  = 0x0102, // no response to subscription request
	CONMON_RXSTATUS_INVALID_REPLY  = 0x0103, // invalid data in subscription response
	                                         //   eg. wrong channel type, missing or wrong addresses
	CONMON_RXSTATUS_POLICY         = 0x0104, // Subscription failed for policy reasons 
	CONMON_RXSTATUS_TX_NO_CHANNEL  = 0x0110  // TX does not have the desired channel

};

/**
 * A subscription is identified by the instance id of the transmitter
 * and the destination addresses to which messages will be sent.
 */
typedef struct
{
		/** for informational use */
	conmon_name_t name;

		/** the instance id of the subscription source */
	conmon_instance_id_t instance_id;
		/** the destination addresses for the subscription */
	conmon_endpoint_addresses_t addresses;
} conmon_subscription_info_t;

//----------------------------------------------------------
// ConMon channel messages
//----------------------------------------------------------

/**
 * The maximum size of a conmon message, include the header
 */
#ifndef CONMON_MESSAGE_MAX_SIZE
	#define CONMON_MESSAGE_MAX_SIZE (1500-28) // need space for UDP/IP headers without fragmenting
#endif

/**
 * The size of a conmon header
 */
#define CONMON_MESSAGE_HEAD_SIZE 24

/**
 * The maximum sizeof that a conmon message body (payload) can ever be. This value is usefule for buffer allocation.
 * Note that the currently usable maximum may be smaller tan this value. The conmon client provides accessor functions 
 * indicating the current maximum body size for a given channel at any given time.
 */
#define CONMON_MESSAGE_MAX_BODY_SIZE (CONMON_MESSAGE_MAX_SIZE - CONMON_MESSAGE_HEAD_SIZE)

/**
 * A conmon message head is represented using an opaque handle
 */
typedef struct conmon_message_head conmon_message_head_t;

/**
 * A conmon message body
 */
typedef union
{
		/** The message body as an array of raw bytes */
	uint8_t data[CONMON_MESSAGE_MAX_BODY_SIZE];
		/** A placeholder to enforce 32-bit alignment for message body structures. Do not use. */
	uint16_t data16[CONMON_MESSAGE_MAX_BODY_SIZE/2];
	uint32_t data32[CONMON_MESSAGE_MAX_BODY_SIZE/4];
	uint32_t __alignment; 
} conmon_message_body_t;


/**
 * Each Conmon message has a message class. Currently the only defined
 * message class is CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC.
 */
typedef uint16_t conmon_message_class_t;

enum conmon_message_class
{
	CONMON_MESSAGE_CLASS_RESERVED        = 0xFFFE, // For Audinate internal use
	CONMON_MESSAGE_CLASS_VENDOR_SPECIFIC = 0xFFFF
};

/**
 * Get the sequence number for a given message.
 *
 * @param head the message head
 *
 * @note Messages sequence numbers are handle independently for each device's channel.
 */
uint16_t
conmon_message_head_get_seqnum
(
	const conmon_message_head_t * head
);

/**
 * Get the payload size for this message
 *
 * @param head the message head
 */
uint16_t
conmon_message_head_get_body_size
(
	const conmon_message_head_t * head
);

/**
 * Get the message class for this message
 *
 * @param head the message head
 */
conmon_message_class_t
conmon_message_head_get_message_class
(
	const conmon_message_head_t * head
);

/**
 * Get the vendor ID for this message
 *
 * @param head the message head
 */
const conmon_vendor_id_t *
conmon_message_head_get_vendor_id
(
	const conmon_message_head_t * head
);

/**
 * Get the source device ID for this message
 *
 * @param head the message head
 */
const conmon_device_id_t *
conmon_message_head_get_source_id
(
	const conmon_message_head_t * head
);

/**
 * Get the source process ID for this message
 *
 * @param head the message head
 */
conmon_process_id_t
conmon_message_head_get_process_id
(
	const conmon_message_head_t * head
);

/**
 * Get the source instance ID for this message
 * And store it in the instance_id object provided.
 *
 * @param head the message head
 * @param instance_id a pointer the location where the message's instance id will be stored
 */
void
conmon_message_head_get_instance_id
(
	const conmon_message_head_t * head,
	conmon_instance_id_t * instance_id
);

//----------------------------------------------------------
// Helper functions
//----------------------------------------------------------

// A helper function to get channel type as a string
const char *
conmon_channel_type_to_string
(
	conmon_channel_type_t channel_type
);

// A helper function to get conmon rxstatus as a string
const char *
conmon_rxstatus_to_string
(
	conmon_rxstatus_t rxstatus
);

// A helper function to print a device id to a string buffer
char *
conmon_device_id_to_string
(
	const conmon_device_id_t * id,
	char * buf,
	size_t len
);

// A helper function to print an instance id to a string buffer
char *
conmon_instance_id_to_string
(
	const conmon_instance_id_t * id,
	char * buf,
	size_t len
);

// A helper function to print a vendor id to a string buffer
char *
conmon_vendor_id_to_string
(
	const conmon_vendor_id_t * id,
	char * buf,
	size_t len
);

#ifdef __cplusplus
}
#endif

#endif

