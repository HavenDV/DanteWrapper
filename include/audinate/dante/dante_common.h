/*
 * File     : $RCSfile$
 * Created  : October 2009
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Dante API common header
 *
 * This software is copyright (c) 2009-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */

/**
 * @file dante_common.h
 * Generic Dante types and structures that are used across the entire Dante system
 */

#ifndef _DANTE_COMMON_H
#define _DANTE_COMMON_H

#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Version information for libraries, protocols and devices
 */
typedef struct dante_version
{
		/** Major version number */
	uint8_t major;
		/** Minor version number */
	uint8_t minor;
		/** Bugfix version number */
	uint16_t bugfix;
} dante_version_t;

/**
 * Version build information for dante products. This structure
 * provides more detailed build information about a particular
 * version of a product. This structure may be extended in future.
 */
typedef struct dante_version_build
{
		/** 
		 * The build number for a particular version. Valid build numbers are always non-zero.
		 * a zero build number indicate absence of build information for a particular version.
		 */
	uint32_t build_number;
} dante_version_build_t;

/** 
 * Helper function to convert from a dante version to a uint32 with 8/8/16 bit fields sizes
 *
 * @param version the version to be converted
 * 
 * @return the verion as a uint32
 */
AUD_INLINE uint32_t 
dante_version_to_uint32_8_8_16(const dante_version_t * version)
{
	return (uint32_t) ((version->major << 24) | (version->minor << 16) | version->bugfix);
}


/** 
 * Helper function to convert to a dante version from a uint32 with 8/8/16 bit field sizes
 *
 * @param value the value to be converted
 * @param version_ptr a pointer to a version to store the converted value
 */
AUD_INLINE void 
dante_version_from_uint32_8_8_16(uint32_t value, dante_version_t * version_ptr)
{
	if (version_ptr)
	{
		version_ptr->major = value >> 24;
		version_ptr->minor = (value >> 16) & 0xFF;
		version_ptr->bugfix = value & 0xFFFF;
	}
}

//----------------------------------------------------------
// Basic constants and types
//----------------------------------------------------------

/**
 * Dante device, channel and label names are 32-byte characters.
 * Device names and channel/label names have additional restrictions on allowed characters within the name.
 * Validaity functions are provided below for each type of name
 * A trailing NULL terminator must be included within the 32 characters
 *
 * @see dr_name_is_valid_device_name
 * @see dr_name_is_valid_channel_or_label_name
 */
#define DANTE_NAME_LENGTH 32

/**
 * A buffer long enough to hold the name of a Dante device, label or channel
 */
typedef char dante_name_t[DANTE_NAME_LENGTH];

/**
 * A friendly name is a utf-8 encoded string that describes 
 * manufacturers, products etc. Friendly names are UTF-8 encoded
 *
 * A trailing NULL terminator must be included within the string
 */
#define DANTE_FRIENDLY_NAME_LENGTH 128

/**
 * A buffer long enough to hold the name of a Dante friendly name
 */
typedef char dante_friendly_name_t[DANTE_FRIENDLY_NAME_LENGTH];



/**
 * In the case of name conflict, extra characters can be added to the advertised
 * Dante name, resulting in a "name" that is longer than DANTE_NAME_LENGTH.
 *
 * Therefore, use a slightly longer than normal buffer for advertised names.
 */
enum { DANTE_ADVERTISED_NAME_LENGTH = DANTE_NAME_LENGTH + 8 };

/**
 * A buffer large enough to hold an advertised name, including extra character in case of a name conflict
 */
typedef char dante_advertised_name_t[DANTE_ADVERTISED_NAME_LENGTH];

/**
 * A dns dotted domain name is limited to 254 characters long when including a trailing '\\0'
 */
#define DANTE_DNS_DOMAIN_NAME_LENGTH 256

/**
 * A buffer long enough to hold a DNS domain name
 */
typedef char dante_dns_domain_name_t[DANTE_DNS_DOMAIN_NAME_LENGTH];

/**
 * A clock subdomain name identifies a group of devices sharing the same clock subdomain.
 */
#define DANTE_CLOCK_SUBDOMAIN_NAME_LENGTH 16

/**
 * A buffer long enough to hold a clock subdomain name
 */
typedef struct
{
	  /** The clock subdomain name */
	char data[DANTE_CLOCK_SUBDOMAIN_NAME_LENGTH];
} dante_clock_subdomain_name_t;

/** 
 * A variable containing the default clock subdomain name
 */
extern const dante_clock_subdomain_name_t DANTE_DEFAULT_CLOCK_SUBDOMAIN_NAME;

/**
 * A clock grandmaster uuid identifies a mac address of grandmaster
 */
#define DANTE_CLOCK_GRANDMASTER_UUID_LENGTH	6

typedef struct
{
	  /** The grandmaster UUID */
	uint8_t data[DANTE_CLOCK_GRANDMASTER_UUID_LENGTH];
	uint16_t padding;	// padding
} dante_clock_grandmaster_uuid_t;

/**
 * Dante device names must be valid DNS labels of up to 31 characters.
 * Valid characters are A-Z, a-z, 0-9 and hyphen (-)
 * Device names may not start or end with a hyphen.
 */
aud_bool_t
dante_name_is_valid_device_name
(
	const char * name
);

/**
 * Channel and Label names may not contain the characters dot (.), at (@) and equals (=)
 */
aud_bool_t
dante_name_is_valid_channel_or_label_name
(
	const char * name
);

/**
 * Test whether two dante names are the same.
 *
 * This test takes into account Dante case sensitity rules: two names that are not
 * equal to strcmp may nonetheless be equal in Dante.
 *
 * @param name1 the first name
 * @param name2 the second name
 *
 * @return non-zero if the two strings are equal dante names, AUD_FALSE otherwise
 */
aud_bool_t
dante_name_equals(const char * name1, const char * name2);


/**
 * Copy a string as a dante name, checking for legality.
 *
 * If the source string is too long (but otherwise legal), DANTE_NAME_LENGTH-1
 * characters will be copied and AUD_ERR_TRUNCATED will be returned.
 *
 * If the source string contains illegal characters, AUD_ERR_INVALIDDATA will
 * be returned and the destination will be set to the empty string.
 *
 * @param dst the destination buffer (usually a dante_name_t)
 * @param src the source buffer
 *
 * @return AUD_SUCCESS if the name was successfully copied, an error code otherwise
 */
aud_error_t
dante_name_copy
(
	char * dst,
	const char * src
);


/**
 * Copy a string as an advertised name.
 *
 * If the source string is too long (but otherwise legal), DANTE_ADVERTISED_NAME_LENGTH-1
 * characters will be copied and AUD_ERR_TRUNCATED will be returned.
 *
 * Unlike dante_name_copy, this function does not check legality.
 *
 * @param dst the destination buffer (usually a dante_advertised_name_t)
 * @param src the source buffer
 *
 * @return AUD_SUCCESS if the name was successfully copied, an error code otherwise
 */
aud_error_t
dante_advertised_name_copy
(
	char * dst,
	const char * src
);

/**
* Return a valid dante name based on the host name. If the host name is not valid
* a default name will be substituted.
*
* @param dante_name the dante name buffer
* @param len length of the dante name buffer
*
* @return AUD_SUCCESS if the dante name was successfully created
*/
aud_error_t
dante_name_from_host_name
(
	char * dante_name,
	size_t * len
);

#ifndef DANTE_IPV4_ADDR8_LENGTH
#define DANTE_IPV4_ADDR8_LENGTH 4
#endif

/**
 * A union representing an IPv4 address
 */
typedef union dante_ipv4_addr
{
	  /** An IPv4 address represented as a single 32-bit value */
	uint32_t addr32;
	  /** An IPv4 address represented as an array of 4 bytes */
	uint8_t addr8[DANTE_IPV4_ADDR8_LENGTH];
} dante_ipv4_addr_t;

/**
 * A standard representation of an ipv4 address
 */
typedef struct dante_ipv4_address
{
	  /** A 4-byte ipv4 address (in NETWORK order) */
	uint32_t host;
	  /** A port number (in HOST order) */
	uint16_t port;
} dante_ipv4_address_t;


/** The length of an ethernet address */
#define DANTE_ETHER_LENGTH 6

/** A structure for an ethernet address */
typedef struct dante_ethernet_address
{
	/** Payload */
	uint8_t addr[DANTE_ETHER_LENGTH];
} dante_ethernet_address_t;

typedef enum
{
	DANTE_NETWORK_ENDPOINT_FAMILY_NONE,
	DANTE_NETWORK_ENDPOINT_FAMILY_IPV4,
	DANTE_NETWORK_ENDPOINT_FAMILY_ETHERNET
} dante_network_endpoint_family_t;

typedef struct dante_network_endpoint
{
	/** The type (family) of endpoint */
	dante_network_endpoint_family_t family; 

	union
	{
		/** IPV4 endpoint payload */
		dante_ipv4_address_t ipv4;

		/** Ethernet endpoint payload */
		dante_ethernet_address_t ethernet;

	} endpoint; //!< Union of endpoint payloads

} dante_network_endpoint_t;


//----------------------------------------------------------
// Access control types
//----------------------------------------------------------

/**
	Describes a simple access control policy value
 */
typedef enum dante_access_policy
{
	DANTE_ACCESS_POLICY__UNDEF = 0,
	DANTE_ACCESS_POLICY__NO_ACCESS,
	DANTE_ACCESS_POLICY__READ_ONLY,
	DANTE_ACCESS_POLICY__READ_WRITE
} dante_access_policy_t;

/**
 * Returns a static string representation of an access control policy value.
 * @param policy
 * 	access control policy value to convert
 * @return 
 * 	string representation of access control policy value
 */
const char *
dante_access_policy_to_string(dante_access_policy_t policy);

/**
 * This enumeration describes fine grained access control capabilities.
 */
enum dante_capability
{
	DANTE_CAPABILITY_NAME,
	DANTE_CAPABILITY_INTERFACE_REDUNDANCY,
	DANTE_CAPABILITY_SWITCH_VLAN,
	DANTE_CAPABILITY_PREFERRED_MASTER_CLOCK,
	DANTE_CAPABILITY_EXTERNAL_WORD_CLOCK,
	DANTE_CAPABILITY_SAMPLE_RATE_PULLUP,
	DANTE_CAPABILITY_SAMPLE_RATE,
	DANTE_CAPABILITY_ENCODING,
	DANTE_CAPABILITY_DEVICE_LATENCY,
	DANTE_CAPABILITY_HAREMOTE_BRIDGE_MODE,
	DANTE_CAPABILITY_INTERFACE_IPV4_ADDRESSES,
	DANTE_CAPABILITY_TX_CHANNEL_LABELS,
	DANTE_CAPABILITY_TX_FLOW,
	DANTE_CAPABILITY_RX_CHANNEL_NAMES,
	DANTE_CAPABILITY_RX_CHANNEL_SUBSCRIPTIONS,
	DANTE_CAPABILITY_AES67_INTEROPERABILITY,
	DANTE_CAPABILITY_DEVICE_UPGRADE,
#if 1
	DANTE_CAPABILITY_CLOCK_UNICAST_DELAY_REQUESTS,
	DANTE_CAPABILITY_CLEAR_CONIFG,
	DANTE_CAPABILITY_INTERACFE_CLEAR_ERROR_COUNTER,
	DANTE_CAPABILITY_PDK_BASEBOARD_CONFIG,
	DANTE_CAPABILITY_INTERFACE_LINK_LOCAL,
	DANTE_CAPABILITY_IGMP_VERSION,
	DANTE_CAPABILITY_SERIAL_PORT_CONFIG,
	DANTE_CAPABILITY_AUDIO_RX_ERROR_THRESHOLD,
	DANTE_CAPABILITY_GPIO_CONFIG,
#endif
	DANTE_CAPABILITY_REBOOT,
	DANTE_CAPABILITY_DEVICE_CODEC,
	DANTE_CAPABILITY_COUNT
};

/**
 * A type for individual capabilities.
 */
typedef uint16_t dante_capability_t;

/** 
 * Calculates caps length of capabilty mask given required capacity
 */
#define DANTE_CAPABILITY_MASK_CAPACITY_TO_CAPS_LENGTH(capacity) ((((capacity) - 1) >> 3) + 1)

/**
 * A type defining a fixed size capability mask.
 * For internal use only
 */
typedef struct dante_capability_mask
{
	// Actual (used) length in bytes
	uint16_t caps_length; 

	// A mask definiing of the 'caps' values are valid
	uint8_t valid_caps[DANTE_CAPABILITY_MASK_CAPACITY_TO_CAPS_LENGTH(DANTE_CAPABILITY_COUNT)];

	// The capabilities mask
	uint8_t caps[DANTE_CAPABILITY_MASK_CAPACITY_TO_CAPS_LENGTH(DANTE_CAPABILITY_COUNT)];
} dante_capability_mask_t;

/** 
 * Calculates string length of capabilty mask given required capacity
 */
#define DANTE_CAPABILITY_MASK_CAPACITY_TO_STRING_LENGTH(capacity) ((DANTE_CAPABILITY_MASK_CAPACITY_TO_CAPS_LENGTH(capacity) * 2) + 1)


/**
 * A type defining a string representation of either the caps or the valid_caps of a capability mask.
 */
typedef char dante_capability_mask_str_t[DANTE_CAPABILITY_MASK_CAPACITY_TO_STRING_LENGTH(DANTE_CAPABILITY_COUNT)];

/**
 * Converts capability mask validity and capability information into strings in hexadecimal format
 * @param mask
 *	pointer to initialised capability mask to convert
 * @param valid_caps
 * 	pointer to fixed string buffer for valid capabilities
 * @param caps
 * 	pointer to fixed string buffer for capabilities
 * @result
 * 	AUD_SUCCESS on success. AUD_ERROR_TRUNCATED on truncation.
 */
aud_error_t
dante_capability_mask_to_string(const dante_capability_mask_t *mask, dante_capability_mask_str_t valid_caps, dante_capability_mask_str_t caps);


//----------------------------------------------------------
// Routing constants and types. These are the parts of the 
// internal routing model definition that are publicly visible
//----------------------------------------------------------


/**
 * flag codes used when accessing labels
 */

  /** A flag indicating a transmit channel's canonical name */
#define DANTE_TXNAME_TYPE_CANONICAL  0x0001
  /** A flag indicating a label associated with a transmit channel */
#define DANTE_TXNAME_TYPE_LABEL      0x0002
  /** A mask for all possible txname types */
#define DANTE_TXNAME_TYPE_ALL \
	(DANTE_TXNAME_TYPE_CANONICAL | DANTE_TXNAME_TYPE_LABEL)

  /** Bitfield for tx name types */
typedef unsigned int dante_txname_type_t;

enum dante_rxstatus
{
	  /** Channel is not subscribed or otherwise doing anything interesting */
	DANTE_RXSTATUS_NONE = 0,
	
	  /** Name not yet found */
	DANTE_RXSTATUS_UNRESOLVED = 1,
	
	  /** Name has been found, but not yet processed. This is an transient state */
	DANTE_RXSTATUS_RESOLVED = 2,

	  /** Error: an error occurred while trying to resolve name */
	DANTE_RXSTATUS_RESOLVE_FAIL = 3,

	  /** Channel is successfully subscribed to own TX channels (local loopback mode) */
	DANTE_RXSTATUS_SUBSCRIBE_SELF = 4,

	  /** Name explicitly does not exist */
	DANTE_RXSTATUS_RESOLVED_NONE = 5,

	  /** 
	   * A flow has been configured but does not have sufficient information to
	   * establish an audio connection.
	   * 
	   * For example, configuring a template with no associations.
	   */
	DANTE_RXSTATUS_IDLE = 0x7, 

	  /** Name has been found and processed; setting up flow. This is an transient state */
	DANTE_RXSTATUS_IN_PROGRESS = 0x8,

	  /** Active subscritpion to an automatically configured source flow */
	DANTE_RXSTATUS_DYNAMIC = 9,

	  /** Active subscription to a manually configured source flow */
	DANTE_RXSTATUS_STATIC = 10,

	  /** Manual flow configuration bypassing the standard subscription process */
	DANTE_RXSTATUS_MANUAL = 0xE,

	  /** Error: The name was found but the connection process failed
          (the receiver could not communicate with the transmitter) */
	DANTE_RXSTATUS_NO_CONNECTION = 0xF,
		
	  /** Error: Channel formats do not match */
	DANTE_RXSTATUS_CHANNEL_FORMAT,

	  /** Error: Flow formats do not match, 
          e.g. Multicast flow with more slots than receiving device can handle */
	DANTE_RXSTATUS_BUNDLE_FORMAT,

	  /** Error: Receiver is out of resources (e.g. flows) */
	DANTE_RXSTATUS_NO_RX,

	  /** Error: Receiver couldn't set up the flow */
	DANTE_RXSTATUS_RX_FAIL,

	  /** Error: Transmitter is out of resources (e.g. flows) */
	DANTE_RXSTATUS_NO_TX,

	  /** Error: Transmitter couldn't set up the flow */
	DANTE_RXSTATUS_TX_FAIL,

	  /** Error: Receiver got a QoS failure (too much data) when setting up the flow. */
	DANTE_RXSTATUS_QOS_FAIL_RX,

	  /** Error: Transmitter got a QoS failure (too much data) when setting up the flow. */
	DANTE_RXSTATUS_QOS_FAIL_TX,

	  /** Error: TX rejected the address given by rx (usually indicates an arp failure) */
	DANTE_RXSTATUS_TX_REJECTED_ADDR,

	  /** Error: Transmitter rejected the bundle request as invalid */
	DANTE_RXSTATUS_INVALID_MSG,

	  /** Error: TX channel latency higher than maximum supported RX latency */
	DANTE_RXSTATUS_CHANNEL_LATENCY,

	  /** Error: TX and RX and in different clock subdomains */
	DANTE_RXSTATUS_CLOCK_DOMAIN,

	  /** Error: Attempt to use an unsupported feature */
	DANTE_RXSTATUS_UNSUPPORTED,

	  /** Error: All rx links are down */
	DANTE_RXSTATUS_RX_LINK_DOWN,

	  /** Error: All tx links are down */
	DANTE_RXSTATUS_TX_LINK_DOWN,

	  /** Error: can't find suitable protocol for dynamic connection */
	DANTE_RXSTATUS_DYNAMIC_PROTOCOL,

	  /** Channel does not exist (eg no such local channel) */
	DANTE_RXSTATUS_INVALID_CHANNEL,
		
	  /** Tx Scheduler failure */
	DANTE_RXSTATUS_TX_SCHEDULER_FAILURE,

	  /** The given subscription to self was disallowed by the device */
	DANTE_RXSTATUS_SUBSCRIBE_SELF_POLICY,


	  /** Template-based subscription failed: template and subscription device names don't match */
	DANTE_RXSTATUS_TEMPLATE_MISMATCH_DEVICE = 0x40,

	  /** Template-based subscription failed: flow and channel formats don't match */
	DANTE_RXSTATUS_TEMPLATE_MISMATCH_FORMAT,

	  /** Template-based subscription failed: the channel is not part of the given multicast flow */
	DANTE_RXSTATUS_TEMPLATE_MISSING_CHANNEL,

	  /** Template-based subscription failed: something else about the template configuration
	      made it impossible to complete the subscription using the given flow */
	DANTE_RXSTATUS_TEMPLATE_MISMATCH_CONFIG,

	  /** Template-based subscription failed: the unicast template is full */
	DANTE_RXSTATUS_TEMPLATE_FULL,

	/** Error: RX device does not have a supported subscription mode (unicast/multicast) available */
	DANTE_RXSTATUS_RX_UNSUPPORTED_SUB_MODE,

	/** Error: TX device does not have a supported subscription mode (unicast/multicast) available */
	DANTE_RXSTATUS_TX_UNSUPPORTED_SUB_MODE,

	  /** Error: TX access control denied the request */
	DANTE_RXSTATUS_TX_ACCESS_CONTROL_DENIED = 0x60,

	  /** TX access control request is in progress */
	DANTE_RXSTATUS_TX_ACCESS_CONTROL_PENDING,

	  /** Unexpected system failure. */
	DANTE_RXSTATUS_SYSTEM_FAIL = 0xFF
};

// data type for specifying per-interface status
//typedef uint8_t dante_interface_status_t;

	/** A type for receiver status code enumerations */
typedef uint8_t dante_rxstatus_t;

	/** A type for routing flow, channel and label ids */
typedef uint16_t dante_id_t;

/**
 * A value for frames-per-packet
 */
typedef uint16_t dante_fpp_t;

/**
 * A value for representing latencies in microseconds
 */
typedef uint32_t dante_latency_us_t;

/**
 * A type for audio sample rates
 */ 
typedef uint32_t dante_samplerate_t;


/** Size (in bytes) of 64 bit identifier */
#define DANTE_ID64_LEN 8

/** Unique 64 bit identifier */
typedef struct dante_id64
{
	uint8_t data[DANTE_ID64_LEN];
} dante_id64_t;

/**
 * Are 2 id64s equal?
 */
AUD_INLINE aud_bool_t
dante_id64_equals
(
	const dante_id64_t * id1,
	const dante_id64_t * id2
) {
	return !(memcmp(id1->data, id2->data, DANTE_ID64_LEN));
}


extern const dante_id64_t k_dante_id64_zero;


/** 
 * Does an id64 contain any non-zero bytes?
 */
AUD_INLINE aud_bool_t
dante_id64_is_non_zero
(
	const dante_id64_t * id
) {
	return memcmp(id->data, k_dante_id64_zero.data, DANTE_ID64_LEN) ? AUD_TRUE : AUD_FALSE;
}


/**
	Split a dante subscription string into components

	@param subscription
		Source subscription: channel@device[.domain.example.com.]
	@param channel_label
		Pointer to 32 character buffer (eg dante_name_t) to store output channel label
	@param device_label
		Pointer to 32 character buffer (eg dante_name_t) to store output device label

	@return
		Domain name, or pointer to "" if no domain suffix.
		NULL on bad input.

	@note
		channel_label must be non-NULL
		device_label may be NULL, in which case the return value is the device label
		plus any domain name.

		If device_label is NULL, the string matches the regexp ([^.@]{1,31})@(.+)
			and assigns \1 to channel_label and returns \2.
		If device_label is non-NULL, the string matches the regexp
			([^.@]+{1,31})@([^.@]{1,31})(\.(.+))?
			and assigns \1 to channel_label, \2 to device_label and returns \4.
		No additional validation is performed.
 */
const char *
drm_split_subscription
(
	const char * subscription,
	char * channel_label,
	char * device_label
);


//----------------------------------------------------------
// Dante audio encodings
//----------------------------------------------------------

/*
	Understanding Dante Encodings

	Some Dante channels support more than one format simultaneously. Data sent to
	or received from a flow in a supported format will be converted from the
	channel's "native" format (the format of the attached bus) to the flow's
	format (or vice-versa).

	The description of a multi-format channel includes the "native" format and
	a list of all non-PCM formats supported by the channel (including the native
	format if not PCM).

	Some Dante channels support automatic conversion of PCM formats. For example,
	PCM16 data can be automatically zero-padded to PCM24, or PCM32 data
	automatically truncated to PCM24. These are not enumerated as part of the
	format-list, but available as a separate bit-map indicating the supported
	word lengths.  Examples:
		0x0002 - supports 2-byte PCM data (eg PCM16)
		0x0004 - supports 3-byte PCM data (eg PCM20, PCM24)
		0x000E - supports 2, 3 or 4-byte PCM data (ie anything from PCM16 to PCM32)
	
	A channel that supports PCM will also provide the "native" PCM encoding. If
	the native encoding is PCM, the native PCM encoding will be the same as the
	channel's native encoding.  Otherwise, it is the number of bits of PCM data
	that will be processed during sample conversion.
 */

/**
 * A type representing a single audio encoding format
 */
typedef uint16_t dante_encoding_t;

/** 
 * A type representing a set of supported, byte-aligned PCM encodings.
 * Eg. 0x000E represents support for 16, 24 and 32 bit PCM encodings.
 */
typedef uint16_t dante_encoding_pcm_map_t;


// Common encodings
enum conmon_dante_encodings
{
	  /** A constant specifiying the NULL encoding; used as a return value in error cases */
	DANTE_ENCODING_NONE = 0,

	// PCM encodings
	// All values from 1 to 64 represent valid PCM encodings with the given bit depth
	// These are the most common.
	// Dante PCM values are assumed to occupy the smallest whole number of bytes
	// that will fit, with the least significant bits being zero-padded.
	DANTE_ENCODING_PCM8 = 8,
	DANTE_ENCODING_PCM16 = 16,
	DANTE_ENCODING_PCM24 = 24,
	DANTE_ENCODING_PCM32 = 32,
	DANTE_ENCODING_PCM_MAX = 64
};

/**
 * Test if the encoding is a PCM encoding, and return the bit depth if it is.
 *
 * @param encoding the encoding
 *
 * @note this functions as a boolean 'is_pcm' test as long as the result is
 *   not directly compared to 'AUD_TRUE'.
 */
AUD_INLINE uint16_t
dante_encoding_pcm(dante_encoding_t encoding)
{
	return (encoding <= DANTE_ENCODING_PCM_MAX ? encoding : 0);
}

/**
 * Is the given encoding a PCM encoding?
 * @param encoding the encoding
 */
AUD_INLINE aud_bool_t
dante_encoding_is_pcm(dante_encoding_t encoding)
{
	return dante_encoding_pcm(encoding) != 0;
}

/**
 * Get the width (in bytes) of the given PCM encoding
 *
 * @param pcm_encoding a pcm encoding
 */
AUD_INLINE unsigned int
dante_encoding_pcm_bytes(dante_encoding_t pcm_encoding)
{
	return ((dante_encoding_pcm(pcm_encoding) + 7) >> 3);
}

/**
 * A structure to hold information about an encoding
 */
typedef struct dante_encoding_info
{
	/** How many bytes does the encoding use for a single sample? */
	unsigned int bytes_per_sample;

	/** Is the encoding a custom (non-standard, usually vendor-specific) encoding */
	aud_bool_t custom;

	/** Is this encoding normally visible for end users? */
	aud_bool_t visible;
} dante_encoding_info_t;

/**
 * Get information about the given encoding and store it in the information structure provided.
 * @param encoding the encoding being queried
 * @param info the structure to hold the information about the encoding
 *
 * @return AUD_ERR_NOTFOUND if no information is available for the given encoding
 */
aud_error_t
dante_encoding_get_info
(
	dante_encoding_t encoding,
	dante_encoding_info_t * info
);

//----------------------------------------------------------
// Dante channel sample formats
//----------------------------------------------------------

/*
	A dante format describes the sample format of a channel.  Dante formats
	are used in two roles:
	- options: a sample rate and set of available encodings
	- selection: a sample rate and a single encoding
 */

/**
 * A structure representing a single audio format (one sample rate, one encoding)
 */
typedef struct dante_format dante_format_t;

/**
 * A structure representing a single audio format (one sample rate, one encoding)
 */
struct dante_format
{
	  /** The format's samplerate */
	dante_samplerate_t samplerate;
	  /** The format's encoding */
	dante_encoding_t encoding;
};

/**
 * Compare two single formats
 *
 * @param f1 the first format
 * @param f2 the second format
 *
 * @return AUD_TRUE if the formats are equal, AUD_FALSE otherwise
 */
AUD_INLINE aud_bool_t
dante_format_equals
(
	const dante_format_t * f1,
	const dante_format_t * f2
) {
	return f1 == f2 ||
		(f1->samplerate == f2->samplerate && f1->encoding == f2->encoding);
}

/**
 * Is the format valid? A valid format has a non-zero sample rate and a non-zero encoding
 *
 * @param f the format
 *
 * @return AUD_TRUE if the format is value, AUD_FALSE otherwise
 */
AUD_INLINE aud_bool_t
dante_format_is_valid
(
	const dante_format_t * f
) {
	return f->samplerate && f->encoding;
}


/**
 * A type representing a set of candidate formats (one samplerate, one or more candidate encodings)
 */
typedef struct dante_formats dante_formats_t;

/**
 * Check whether a 'formats' structure includes the given format
 *
 * @param formats the 'formats' structure to be checked
 * @param format the format structure to check for
 *
 * @return AUD_TRUE if the 'formats' structure contains the given format; AUD_FALSE otherwise.
 */
aud_bool_t
dante_formats_has_format
(
	const dante_formats_t * formats,
	const dante_format_t * format
);

/**
 * Get the samplerate for the given candidate formats 
 *
 * @param formats the formats structure
 */
dante_samplerate_t
dante_formats_get_samplerate(const dante_formats_t * formats);

/**
 * Get the native encoding for a given candidate format. This is the
 * encoding format used natively by the channel in the underlying audio engine
 *
 * @param formats the formats structure
 */
dante_encoding_t
dante_formats_get_native_encoding(const dante_formats_t * formats);


/**
 * Get the number of available non-pcm encodings in the given formats structure
 *
 * @param formats the formats structure
 */
uint16_t
dante_formats_num_non_pcm_encodings(const dante_formats_t * formats);


/**
 * Get the array containing available non-pcm encodings in the given formats structure
 *
 * @param formats the formats structure
 */
const dante_encoding_t *
dante_formats_get_non_pcm_encodings(const dante_formats_t * formats);

/**
 * Get the non-pcm encoding at the given index
 *
 * @param formats the formats structure
 * @param index the index of the desited encoding
 */
dante_encoding_t
dante_formats_non_pcm_encoding_at_index
(
	const dante_formats_t * formats,
	unsigned int index
);

/** 
 * Get the native PCM encoding, if any, for the given format
 *
 * @param formats the formats structure
 */
dante_encoding_t
dante_formats_get_native_pcm(const dante_formats_t * formats);

/** 
 * Get a bitmap describing the supported PCM byte widths for the given format
 *
 * @param formats the formats structure
 */
dante_encoding_pcm_map_t
dante_formats_get_pcm_map(const dante_formats_t * formats);

/**
 * What is the preferred encoding for a receiver (rx), given
 * a set of candidate (tx) encodings
 *
 * @param tx the transmitter's encodings
 * @param rx the receiver's encodings
 *
 * @return the preferred encoding, or 0 if no encoding is possible
 */
dante_encoding_t
dante_formats_get_preferred_encoding
(
	const dante_formats_t * rx,
	const dante_formats_t * tx
);

/**
 * What is the preferred format for a receiver (rx), given
 * a candidate (tx) format. This function requires an exact samplerate match and
 * uses the preferred encoding for the two formats.
 *
 * @param tx the transmitter's encodings
 * @param rx the receiver's encodings
 * @param format the preferred format
 *
 * @return AUD_TRUE if a preferred format was found, AUD_FALSE if no suitable format is possible
 */
aud_bool_t
dante_formats_get_preferred_format
(
	const dante_formats_t * rx,
	const dante_formats_t * tx,
	dante_format_t * format 
);

//----------------------------------------------------------
// types and structures use primarily in conmon but
// could be elsewhere too
//----------------------------------------------------------

/**
 * The length (in bytes) of a conmon device ID
 */
#define DANTE_DEVICE_ID_LENGTH 8

/**
 * Each dante device has a unique 64-bit identifier
 */
typedef struct dante_device_id
{
	/** the device id data as an 8-byte array */
	uint8_t data[DANTE_DEVICE_ID_LENGTH];
} dante_device_id_t;

/**
 * The instance of dante on this device (almost always zero)
 */
typedef uint16_t dante_process_id_t;

/**
 * Each conmon instance is uniquely identifiable by its instance id,
 * a combination of it's device id and process id
 */
typedef struct dante_instance_id
{
		/** the dante instance's device id */
	dante_device_id_t device_id;

		/** the dante instance's process id */
	dante_process_id_t process_id; 
} dante_instance_id_t;

extern const dante_device_id_t k_dante_device_id_zero;

extern const dante_instance_id_t k_dante_instance_id_zero;

/** 
 * Does dante device id contain any non-zero bytes?
 */
AUD_INLINE aud_bool_t
dante_device_id_is_non_zero
(
	const dante_device_id_t * id
) {
	return memcmp(id->data, k_dante_device_id_zero.data, DANTE_DEVICE_ID_LENGTH) ? AUD_TRUE : AUD_FALSE;
}

AUD_INLINE aud_bool_t
dante_instance_id_equals
(
	const dante_instance_id_t * id1,
	const dante_instance_id_t * id2
) {
	return !memcmp(id1, id2, sizeof(dante_instance_id_t));
}

AUD_INLINE aud_bool_t
dante_instance_id_is_non_zero
(
	const dante_instance_id_t * id
) {
	return !dante_instance_id_equals(id, &k_dante_instance_id_zero);
}

//----------------------------------------------------------
// Rx Flow Error Monitoring Types
//----------------------------------------------------------

// Types of rxflow errors
enum
{
	// A flag indicating packets that arrived too early
	DANTE_RXFLOW_ERROR_TYPE_EARLY_PACKETS,

	// A flag indicating packets that arrived too late
	DANTE_RXFLOW_ERROR_TYPE_LATE_PACKETS,

	// A flag indicating packets that arrived out-of-order relative to previous packet
	DANTE_RXFLOW_ERROR_TYPE_OUT_OF_ORDER_PACKETS,

	// A flag indicating packets that were dropped or lost
	DANTE_RXFLOW_ERROR_TYPE_DROPPED_PACKETS,

	// A flag indicating measurement of latest received packet
	DANTE_RXFLOW_ERROR_TYPE_MAX_LATENCY,

	// A flag indicating measurement of maximum interval between received packets
	DANTE_RXFLOW_ERROR_TYPE_MAX_INTERVAL,

	// The number of rxflow error types 
	DANTE_NUM_RXFLOW_ERROR_TYPES
};

// A typedef for tracking a single rxflow error type
typedef uint16_t dante_rxflow_error_type_t;

// rx flow error flags
enum
{
	DANTE_RXFLOW_ERROR_FLAG_EARLY_PACKETS        = (1 << DANTE_RXFLOW_ERROR_TYPE_EARLY_PACKETS),
	DANTE_RXFLOW_ERROR_FLAG_LATE_PACKETS         = (1 << DANTE_RXFLOW_ERROR_TYPE_LATE_PACKETS),
	DANTE_RXFLOW_ERROR_FLAG_OUT_OF_ORDER_PACKETS = (1 << DANTE_RXFLOW_ERROR_TYPE_OUT_OF_ORDER_PACKETS),
	DANTE_RXFLOW_ERROR_FLAG_DROPPED_PACKETS      = (1 << DANTE_RXFLOW_ERROR_TYPE_DROPPED_PACKETS),
	DANTE_RXFLOW_ERROR_FLAG_MAX_LATENCY          = (1 << DANTE_RXFLOW_ERROR_TYPE_MAX_LATENCY),
	DANTE_RXFLOW_ERROR_FLAG_MAX_INTERVAL         = (1 << DANTE_RXFLOW_ERROR_TYPE_MAX_INTERVAL)
};

// A bitmask of rxflow error flags.
typedef uint16_t dante_rxflow_error_flags_t;

// Error reports are timestamped. The units for 'subseconds' 
// are not defined and may vary between devices, but will
// be consistent within a single device, allowing for
// comparisons between timestamps
typedef struct dante_rxflow_error_timestamp
{
	uint32_t seconds;
	uint32_t subseconds;
} dante_rxflow_error_timestamp_t;

//----------------------------------------------------------
// AES67 TX/RX flows types
//----------------------------------------------------------
//
// Audio flow class types
enum dante_flow_class
{
	DANTE_FLOW_CLASS__UNDEF = 0,
	DANTE_FLOW_CLASS__DANTE_IP,
	DANTE_FLOW_CLASS__AES67_MCAST_IP,
	DANTE_FLOW_CLASS__AES67_UCAST_IP,
	DANTE_FLOW_CLASS__DANTE_RTP_VIDEO_IP,

	DANTE_FLOW_CLASS_COUNT
};


typedef uint16_t dante_flow_class_t;
typedef uint32_t dante_flow_class_mask_t;

AUD_INLINE dante_flow_class_mask_t
dante_flow_class_to_mask(dante_flow_class_t flow_class)
{
	return flow_class ? (1 << (flow_class - 1)) : 0;
}

AUD_INLINE aud_bool_t
dante_flow_class_in_mask
(
	dante_flow_class_mask_t mask,
	dante_flow_class_t flow_class
)
{
	return (dante_flow_class_to_mask(flow_class) & mask) != 0;
}


//----------------------------------------------------------
// Pull up Types
//----------------------------------------------------------
enum
{
	DANTE_PULLUP_NONE,
	DANTE_PULLUP_PLUSFOURPOINTONESIXSIXSEVEN,
	DANTE_PULLUP_PLUSPOINTONE,
	DANTE_PULLUP_MINUSPOINTONE,
	DANTE_PULLUP_MINUSFOUR,
	DANTE_PULLUP_MAX
};

typedef uint16_t dante_pullup_t;


//----------------------------------------------------------
// Channel signal reference level
//----------------------------------------------------------

/*
 * Dante channel information may optionally include a 'signal reference level' value.
 */

/**
 * A type for representing audio dBu values
 */
typedef int16_t dante_dbu_t;

// Two special values are defined:
enum dante_dbu
{
	DANTE_DBU_UNSET = 0x7FFF,
		// Special value indicating that no signal reference level value is set.
		// App should pick a suitable default.
	DANTE_DBU_INVALID = 0x7FFE
		// Special value indicating that the signal reference level value is
		// invalid / uninitialised.  "unset" is considered a value.
};

//----------------------------------------------------------
// Dante Network Index
//----------------------------------------------------------

/*
* A type for representing dante network index.
*/
typedef uint16_t dante_network_index_t;

enum dante_network_index
{
	DANTE_NETWORK_INDEX_PRIMARY = 0,
	DANTE_NETWORK_INDEX_SECONDARY = 1,
	DANTE_NETWORK_INDEX_UNKNOWN = 0xFFFF
};


//----------------------------------------------------------
// Utility types and structures used across the Dante API
//----------------------------------------------------------

/**
 * Asynchronous request-response communications are tracked using request
 * IDs. When a request message is sent, the host application provides a
 * callback function and the API provides a request id. When a response
 * arrives, the callback function fires with the request id as a
 * parameter.
 */
typedef const void * dante_request_id_t;

/** 
 * A marker to indicate a non-existent request id or an invalid request
 * id value.
 */
#define DANTE_NULL_REQUEST_ID NULL

#if AUD_ENV_HAS_SELECT == 1

/**
 * A wrapper type to simplify passing sockets between the host application
 * and the routing API.
 *
 * @note The interpretation of 'n' is platform dependent. On a posix
 *   platform the value of 'n' is the highest fd in all the fdsets, plus one.
 *   On a windows platform the 'n' is the total number of sockets in all the fdsets.
 */
typedef struct dante_sockets
{	
	  /** The 'n' value as used in a 'select' call. */
	int n;
	  /** The 'read' fd set for a 'select' call */
	fd_set read_fds;
	  /** The 'write' fd set for a 'select' call */
	fd_set write_fds;
} dante_sockets_t;

/**
 * Clear a dr_sockets_t structure. Sets 'n' to 0 and calls FD_ZERO on
 * read_fds.
 *
 * @param sockets the socket set that will be cleared.
 */
AUD_INLINE void
dante_sockets_clear
(
	dante_sockets_t * sockets
) {
	sockets->n = 0;
	FD_ZERO(&sockets->read_fds);
	FD_ZERO(&sockets->write_fds);
}

/**
 * @deprecated use dante_sockets_add_read
 */
#define dante_sockets_add dante_sockets_add_read

/**
 * @deprecated use dante_sockets_remove_read
 */
#define dante_sockets_remove dante_sockets_remove_read

/**
 * Helper method to add a read socket to the socket set, by adding s to read_fds
 * and updating 'n'. The behaviour this function is platform dependent.
 * On a posix platform, the new value for 'n' is the maximum of its old
 * value and (s+1). On a windows platform, n is incremented by 1 (though the value is ignored by select())
 *
 * @param sockets the socket set to which the socket will be added
 * @param s the socket to be added
 */
AUD_INLINE void
dante_sockets_add_read
(
	dante_sockets_t * sockets,
	aud_socket_t s
) {
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4127)
	if (!FD_ISSET(s, &sockets->read_fds))
	{
		FD_SET(s, &sockets->read_fds);
		sockets->n++;
	}
#pragma warning(pop)
#else
	FD_SET(s, &sockets->read_fds);
	sockets->n = MAX(sockets->n, s+1);
#endif
}

/**
 * Helper method to add a write socket to the socket set, by adding s to write_fds
 * and updating 'n'. The behaviour this function is platform dependent.
 * On a posix platform, the new value for 'n' is the maximum of its old
 * value and (s+1). On a windows platform, n is incremented by 1 (though the value is ignored by select())
 *
 * @param sockets the socket set to which the socket will be added
 * @param s the socket to be added
 */
AUD_INLINE void
dante_sockets_add_write
(
	dante_sockets_t * sockets,
	aud_socket_t s
) {
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4127)
	if (!FD_ISSET(s, &sockets->write_fds))
	{
		FD_SET(s, &sockets->write_fds);
		sockets->n++;
	}
#pragma warning(pop)
#else
	FD_SET(s, &sockets->write_fds);
	sockets->n = MAX(sockets->n, s+1);
#endif
}

/**
 * Helper method to remove a read socket from a socket set, by removing s from read_fds
 * and updating 'n'. The behaviour this function is platform dependent.
 * On a posix platform, the new value for 'n' is the value +1 of the new maximum fd.
 * On a windows platform, n is decremented by 1 (though the alue is ignored by select())
 *
 * @param sockets the socket set to which the socket will be added
 * @param s the socket to be added
 */
AUD_INLINE void
dante_sockets_remove_read
(
	dante_sockets_t * sockets,
	aud_socket_t s
) {
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4127)
	if (FD_ISSET(s, &sockets->read_fds))
	{
		FD_CLR(s, &sockets->read_fds);
		sockets->n--;
	}
#pragma warning(pop)
#else
	int i = 0;
	if (sockets->n == s+1)
	{
		sockets->n = 0;
		for (i = 0; i < s; i++)
		{
			if (FD_ISSET(i, &sockets->read_fds))
			{
				sockets->n = i+1;
			}
		}
	}
	FD_CLR(s, &sockets->read_fds);
#endif
}

/**
 * Helper method to remove a write socket from a socket set, by removing s from write_fds
 * and updating 'n'. The behaviour this function is platform dependent.
 * On a posix platform, the new value for 'n' is the value +1 of the new maximum fd.
 * On a windows platform, n is decremented by 1 (though the alue is ignored by select())
 *
 * @param sockets the socket set to which the socket will be added
 * @param s the socket to be added
 */
AUD_INLINE void
dante_sockets_remove_write
(
	dante_sockets_t * sockets,
	aud_socket_t s
) {
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4127)
	if (FD_ISSET(s, &sockets->write_fds))
	{
		FD_CLR(s, &sockets->write_fds);
		sockets->n--;
	}
#pragma warning(pop)
#else
	int i = 0;
	if (sockets->n == s+1)
	{
		sockets->n = 0;
		for (i = 0; i < s; i++)
		{
			if (FD_ISSET(i, &sockets->write_fds))
			{
				sockets->n = i+1;
			}
		}
	}
	FD_CLR(s, &sockets->write_fds);
#endif
}


/**
 * Helper method to copy a dante_sockets set.
 * Simple assignment is not cross-platform safe.
 *
 * @param src the source sockets set
 * @param dst the destination sockets set
 */
AUD_INLINE void
dante_sockets_copy
(
	const dante_sockets_t * src,
	dante_sockets_t * dst
) {
	dst->n = src->n;
	AUD_FD_COPY(&src->read_fds, &dst->read_fds);
	AUD_FD_COPY(&src->write_fds, &dst->write_fds);
}

#endif

/**
 * Dante includes several client components, some of which attempt to maintain a connection to a server.
 * This enumeration represents the possible states for these connections.
 */
typedef enum
{
		// The client is not attempting to acquire or maintain a connection to a server
	DANTE_CLIENT_STATE_NONE,

		// The client wants to be connected to a server but is not currently connected 
		// or attempting to connect (its idle, eg, while awating a timeout to retry connection)
	DANTE_CLIENT_STATE_DISCONNECTED,

		// The client is attemting to discover the server
		// Needed when a server is identified by name
	DANTE_CLIENT_STATE_RESOLVING,

		// The client is currently attempting to connect to a server
		// ie. it has sent a message and is awaiting a response
	DANTE_CLIENT_STATE_CONNECTING,

		// The client is actively connected to a server
	DANTE_CLIENT_STATE_CONNECTED,

		// The client is disconnecting from the server.
		// This state may not be seen if the client does not wait 
		// for a disconnection response
	DANTE_CLIENT_STATE_DISCONNECTING,

		// Something went badly wrong, the client couldn't recover and is 
		// now in an error state
	DANTE_CLIENT_STATE_ERROR,

		// The number of possible connection states
	DANTE_CLIENT_NUM_STATES
} dante_client_state_t;


//----------------------------------------------------------
// Debugging functions
//----------------------------------------------------------

/**
 * Get a string for the given status value
 * 
 * @param status the status
 */
const char *
dante_rxstatus_to_string
(
	dante_rxstatus_t status
);

/**
 * Get a string for the given connection state value
 *
 * @param state the state for which a string representation is desired
 */
const char *
dante_client_state_to_string
(
	dante_client_state_t state
);

/** The minimum length of a buffer with enough room for to print an id64 in hex, with a trailing NULL terminator */
#define DANTE_ID64_BUF_LENGTH (DANTE_ID64_LEN*2 + 1)

/** A buffer with enough room for to print an id64 in hex, with a trailing NULL terminator */
typedef char dante_id64_str_t[DANTE_ID64_BUF_LENGTH];

/*
	Render a dante_id64_t as a hex string.

	@param src_id Source identifier
	@param buf Output string buffer.
	@param len the 

	@return pointer to populated buffer
 */
char *
dante_id64_to_hex_string
(
	const dante_id64_t * id64,
	dante_id64_str_t id64_str
);

/*
	Parse a dante_id64_t from a hex string.

	@param dst_id Destination identifier. Contents undefined on failure.
	@param str Source string. May include leading 0x.  May contain embedded whitespace.
		Trailing characters beyond those needed for id64 are ignored.
 */
aud_error_t
dante_id64_from_hex_string
(
	dante_id64_t * id64,
	const char * buf
);

/*
	Render a dante_id64_t as an ASCII string.

	@note: a dante identifier is not inherently ASCII.  Characters that cannot be
		represented in ASCII will be rendered as '.'.

	@param src_id Source identifier
	@param buf Output string buffer. Must be at least 8 characters long.

	@return pointer to populated buffer
 */
const char *
dante_id64_to_ascii_string
(
	const dante_id64_t * src_id,
	char * buf,
	size_t len
);

/*
	Copy a dante_id64_t from an ASCII string.

	@param dst_id Destination identifier. Contents undefined on failure.
	@param str Source string.

	This function copies up to the first 8 characters of str to dst_id
	and zero-pads the remainder if the string is less than 8 characters long.
 */
aud_error_t
dante_id64_from_ascii_string
(
	dante_id64_t * dst_id,
	const char * str
);

/** The minimum length of a buffer with enough room for to print a device id in hex, with a trailing NULL terminator */
#define DANTE_DEVICE_ID_BUF_LENGTH (DANTE_DEVICE_ID_LENGTH*2 + 1)

/** A buffer with enough room for to print a device id in hex, with a trailing NULL terminator */
typedef char dante_device_id_str_t[DANTE_DEVICE_ID_BUF_LENGTH];

/**
 * Print the device id into the given buffer as hex with a trailing NULL terminator
 *
 * @param id the id to print
 * @param str the buffer
 *
 * @return the buffer pointer
 */
char *
dante_device_id_to_string
(
	const dante_device_id_t * id,
	dante_device_id_str_t str
);

/** The minimum length of a buffer with enough room for to print an instance id in hex, with a trailing NULL terminator */
#define DANTE_INSTANCE_ID_BUF_LENGTH (DANTE_DEVICE_ID_LENGTH*2 + 6 + 1)

/** A buffer with enough room for to print an instance id in hex, with a trailing NULL terminator */
typedef char dante_instance_id_str_t[DANTE_INSTANCE_ID_BUF_LENGTH];

/**
 * Print the instance id into the given buffer as hex with a trailing NULL terminator
 *
 * @param id the id to print
 * @param str the buffer
 *
 * @return the buffer pointer
 */
char *
dante_instance_id_to_string
(
	const dante_instance_id_t * id,
	dante_instance_id_str_t str
);

aud_bool_t
dante_instance_id_from_string
(
	dante_instance_id_t * id,
	const char * str
);

/**
 * Get a string representation of the given rxflow error flag
 */
const char *
dante_rxflow_error_type_to_string
(
	dante_rxflow_error_type_t type
);


/**
 * Covert a dante ID 64 (eg manufacturer ID) into a string suitable for use in DNS-SD.
 *
 * @param id64 dante ID 64
 * @param buf destination buffer
 *
 * @return buffer pointer
 *
 * Algorithm:
 *
 * First, trailling nulls are omitted
 *
 * The remainder of the id will be rendered as text if:
 * - isalnum(byte) || byte == '-' for the first byte
 * - isalnum(byte) || byte == '-' || byte == '_' for the all other bytes
 *
 * Otherwise, the remainder of the id will be rendered as a string _abcd... ,
 * where ab, cd, ... are the bytes in the string rendered as pairs of hex digits
 */
char *
dante_id64_to_dnssd_text
(
	const dante_id64_t * id64,
	char * buf
);

/*
 * As dante_id64_to_dnssd_text, but always use hex form.
 */
char *
dante_id64_to_dnssd_hex
(
	const dante_id64_t * id64,
	char * buf
);

enum
{
	DANTE_ID64_DNSSD_BUF_LENGTH = DANTE_ID64_LEN * 2 + 1 + 1
		//!< Minimum length of buffer that can hold a rendered ID64, including null
};


/*
 * Convert a string into a dante ID 64.
 *
 * @param id destination dante ID 64
 * @param str source string buffer
 *
 * @return AUD_TRUE on success, AUD_FALSE if the string cannot be converted
 */
aud_bool_t
dante_id64_from_dnssd_text
(
	dante_id64_t * id64,
	const char * buf
);


/*
 * Convert a string prefix into a dante ID 64.
 *
 * This can be used to convert only a prefix, or to convert a character buffer
 * that is not null terminated.
 *
 * @param id destination dante ID 64
 * @param str source string buffer
 * @param len maximum number of characters to process (0 to run until a null is encountered)
 * @param options modify strictness of matching
 *
 * Options allow the matching to be forgiving of extra data.  By default, a non-zero
 * len will require the identifier to exactly fill the buffer, neither reaching a
 * NULL or having extra trailing characters.
 *
 * @return AUD_TRUE on success, AUD_FALSE if the string cannot be converted
 */
aud_bool_t
dante_id64_from_dnssd_text_prefix
(
	dante_id64_t * id64,
	const char * buf,
	size_t len,
	unsigned options
);

enum
{
	DANTE_ID64_FROM_DNSSD_TEXT__ALLOW_EARLY_NULL = 1 << 0,
		//!< If len is non-zero, allow a null before reading 'len' characters
		//!< ie treat len as maximum rather exact ID length
	DANTE_ID64_FROM_DNSSD_TEXT__ALLOW_TRAILING   = 1 << 1,
		//!< allow trailing characters in buffer after reading a maximum length ID
};

#if AUD_ENV_HAS_SELECT == 1

char *
dante_sockets_to_string
(
	const dante_sockets_t * sockets,
	char * buf,
	size_t len
);

#endif


//----------------------------------------------------------
// Audio Interop Modes
//----------------------------------------------------------

enum dante_audio_interop_mode
{
	DANTE_AUDIO_INTEROP_MODE__AES67 = 0,

	DANTE_AUDIO_INTEROP_MODE_COUNT
};

typedef uint16_t dante_audio_interop_mode_t;

enum dante_audio_interop_mode_flag
{
	DANTE_AUDIO_INTEROP_MODE_FLAG__AES67        = (1 << DANTE_AUDIO_INTEROP_MODE__AES67)
};

typedef uint32_t dante_audio_interop_mode_flags_t;


#ifdef __cplusplus
}
#endif

#endif

