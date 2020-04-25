/*
 * File     : dante_sdp.h
 * Created  : February 2019
 * Author   : Varuni Witana, Mahanama Wickramasinghe, Andrew White
 * Synopsis : Dante SAP/SDP types and structures
 *
 * This software is copyright (c) 2004-2019 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */


/**
 * @file dante_sdp.h
 * Dante sdp types and functions
 */

#ifndef _DANTE_SDP_H
#define _DANTE_SDP_H

#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#include "dante/dante_common.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------
// SAP constants and types
//----------------------------------------------------------

/** SAP message type */
typedef enum
{
	DANTE_SAP_SESSION_ANNOUNCE,
	DANTE_SAP_SESSION_DELETE
} dante_sap_message_type_t;

/** SAP message identifier hash */
typedef uint16_t dante_sap_msg_id_hash_t;


//----------------------------------------------------------
// SDP constants and types
//----------------------------------------------------------

typedef struct dante_sdp_descriptor dante_sdp_descriptor_t;

/** SDP session stream direction */
typedef enum
{
	DANTE_SDP_DIR__UNDEF = 0,
	/** receive only */
	DANTE_SDP_DIR__RECV_ONLY,
	/** for interactive conferences */
	DANTE_SDP_DIR__SEND_RECV,
	/** send only */
	DANTE_SDP_DIR__SEND_ONLY
} dante_sdp_stream_dir_t;

/** SDP originator session id */
typedef uint64_t dante_sdp_session_id_t;
typedef uint64_t dante_sdp_session_version_t;

/** SDP session media clock offset */
typedef uint32_t dante_sdp_media_clock_offset_t;

/** SDP session stream payload type */
typedef uint8_t dante_sdp_payload_type_t;

typedef union dante_flow_transport_info dante_flow_transport_info_t;

//----------------------------------------------------------
// SDP accessor functions
//----------------------------------------------------------

/**
 * Get the SDP session direction
 * "sendonly|recvonly|sendrecv" SDP attribute
 * @param SDP descriptor object
 * @return session direction enum
 */
dante_sdp_stream_dir_t dante_sdp_get_stream_dir
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP session originator username
 * "<username>" from SDP "o=" field
 * @param SDP descriptor object
   @return SDP session originator username
 */
const char * dante_sdp_get_origin_username
(
	const dante_sdp_descriptor_t *sdp_desc
);

/**
 * Get the SDP session originator session id
 * "<sess-id>" from SDP "o=" field
 * @param SDP descriptor object
 * @return SDP session originator session id
 */
dante_sdp_session_id_t dante_sdp_get_session_id
(
	const dante_sdp_descriptor_t *sdp_desc
);

/**
 * Get the SDP session originator IPv4 addr
 * "<unicast-address>" from SDP "o=" field
 * @param SDP descriptor object
 * @return SDP session originator IP addr
 */
uint32_t dante_sdp_get_origin_addr
(
	const dante_sdp_descriptor_t *sdp_desc
);

/**
 * Get the SDP session media clock offset
 * "mediaclk:direct" SDP attribute
 * @param SDP descriptor object
 * @return RTP media clock offset
 */
dante_sdp_media_clock_offset_t dante_sdp_get_media_clock_offset
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP session name
 * <session name> from SDP "s=" field
 * @param SDP descriptor object
 * @return SDP session name
 */
const char * dante_sdp_get_session_name
(
	const dante_sdp_descriptor_t *sdp_desc
);

/**
 * Get the SDP session information
 * <session information> from SDP "i=" field
 * @param SDP descriptor object
 * @return SDP session information
 */
const char *
dante_sdp_get_session_information
(
	const dante_sdp_descriptor_t *sdp_desc
);

/**
 * Get the SDP session IPv4 connection address
 * <connection-address> from SDP "c=" field
 * @param SDP descriptor object
 * @return SDP session IPv4 address
 */
uint32_t dante_sdp_get_session_conn_addr
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP stream title
 * <media title> from SDP media "i=" field
 * @param SDP descriptor object
 * @return SDP media title
 */
const char *
dante_sdp_get_media_title
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get SDP session network clock reference
 * "gmid" from SDP attribute "ts-refclk:ptp=gmid:subdomain"
 * @param SDP descriptor object
 * @return SDP session network clock reference
 */
const dante_clock_grandmaster_uuid_t *dante_sdp_get_network_clock_ref
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP session network clock reference domain
 * "subdomain" from SDP attribute "ts-refclk:ptp=gmid:subdomain"
 * @param SDP descriptor object
 * @return SDP session network clock reference domain
 */
const dante_clock_subdomain_name_t *dante_sdp_get_network_clock_ref_domain
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP session stream number of channels
 * [number of ch] from SDP attribute "rtpmap:[rtp payload type] [encoding]/[sample rate]/[number of ch]"
 * @param SDP descriptor object
 * @return number of channels
 */
uint16_t dante_sdp_get_stream_num_chans
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP session stream encoding type
 * [encoding] from SDP attribute "rtpmap:[rtp payload type] [encoding]/[sample rate]/[number of ch]"
 * @param SDP descriptor object
 * @return encoding type
 */
uint16_t dante_sdp_get_stream_encoding
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP session stream sample rate
 * [sample rate] from SDP attribute "rtpmap:[rtp payload type] [encoding]/[sample rate]/[number of ch]"
 * @param SDP descriptor object
 * @return sample rate
 */
uint32_t dante_sdp_get_stream_sample_rate
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP session stream udp port number
 * [port] from the SDP "m=audio [port] RTP/AVP [rtp payload type]" field
 * @param SDP descriptor object
 * @return udp port
 */
uint16_t dante_sdp_stream_get_port
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the SDP session stream payload type
 * [payload type] from the "m=audio [port] RTP/AVP [payload type]" field
 * @param SDP descriptor object
 * @return payload type
 */
uint8_t dante_sdp_get_stream_payload_type
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Does the stream originate from a Dante device?
 * [keyword = Dante] from SDP attribute "keywds"
 * SDP is not supported between Dante devices
 * @param SDP descriptor object
 * @return (boolean) Dante keyword present
 */
aud_bool_t
dante_sdp_source_is_dante
(
	const dante_sdp_descriptor_t *sdp_desc
);


//----------------------------------------------------------
// Accessors for redundant SDP sessions
//----------------------------------------------------------

/**
 * Get the number of media descriptors in the group
 * @return 0 if no groups, 'dup' count if multiples
 */
uint8_t
dante_sdp_get_group_mdesc_count
(
	const dante_sdp_descriptor_t *sdp_desc
);


/**
 * Get the media ID of a media descriptor
 * @param SDP descriptor object
 * @param media index
 * @return RTP media clock offset
 */
const char *
dante_sdp_get_mdesc_id
(
	const dante_sdp_descriptor_t * sdp_desc,
	uint8_t media_index
);


/**
 * Get the clock offset for a media flow
 * "mediaclk:direct" SDP attribute
 * @param SDP descriptor object
 * @param media index
 * @return RTP media clock offset
 */
dante_sdp_media_clock_offset_t
dante_sdp_get_mdesc_clock_offset
(
	const dante_sdp_descriptor_t *sdp_desc,
	uint8_t media_index
);


/**
 * Get the media descriptor title
 * <media title> from SDP media "i=" field
 * @param SDP descriptor object
 * @param media index
 * @return SDP media title
 */
const char *
dante_sdp_get_mdesc_title
(
	const dante_sdp_descriptor_t *sdp_desc,
	uint8_t media_index
);



/**
 * Get the SDP media IPv4 connection address
 * <connection-address> from SDP "c=" field
 * @param SDP descriptor object
 * @param media index
 * @return SDP media IPv4 address
 */
uint32_t
dante_sdp_get_mdesc_conn_addr
(
	const dante_sdp_descriptor_t *sdp_desc,
	uint8_t media_index
);


/**
 * Get the SDP session stream udp port number
 * [port] from the SDP "m=audio [port] RTP/AVP [rtp payload type]" field
 * @param SDP descriptor object
 * @param media index
 * @return udp port
 */
uint16_t
dante_sdp_get_mdesc_stream_port
(
	const dante_sdp_descriptor_t *sdp_desc,
	uint8_t media_index
);


//----------------------------------------------------------
// SDP descriptor allocation
//----------------------------------------------------------

/**
 * Writeable version of a dante_sdp_descriptor_t.
 */
typedef struct dante_sdp_descriptor_ref dante_sdp_descriptor_ref_t;

/**
 * Get a reference to the (read-only) dante_sdp_descriptor from this ref
 *
 * @return NULL if the input is NULL or uninitialised
 */
const dante_sdp_descriptor_t *
dante_sdp_descriptor_from_ref
(
	const dante_sdp_descriptor_ref_t * ref
);


/**
 * Allocate a new sdp descriptor
 *
 * @param src
 *	if NULL, create a new uninitialised descriptor ref
 *	if non-NULL, copy the contents of the existing descriptor into the new descriptor
 */
dante_sdp_descriptor_ref_t *
dante_sdp_descriptor_alloc
(
	const dante_sdp_descriptor_t * src
);

/**
 * Free an sdp_descriptor allocated by dante_sdp_descriptor_alloc
 */
aud_error_t
dante_sdp_descriptor_free
(
	dante_sdp_descriptor_ref_t * ref
);

/*
 * Assign the contents of a dante_sdp_descriptor to another descriptor
 *
 * If the source descriptor is null, then the destination will be uninitialised
 *
 * @param dst destination descriptor ref (does nothing if NULL)
 * @param src source descriptor
 */
void
dante_sdp_descriptor_assign
(
	dante_sdp_descriptor_ref_t * dst,
	const dante_sdp_descriptor_t * src
);


//----------------------------------------------------------
// SDP descriptor serialisation
//----------------------------------------------------------

/**
 * Serialise a Dante SDP descriptor to a buffer
 *
 * Converts a Dante SDP descriptor into a format suitable for saving or
 * loading into persistent data.  It does not serialise the entire descriptor
 * but only those fields relevant to rx flow creation.
 *
 * @param sdp_descriptor input descriptor
 * @param data_buf memory to store descriptor
 * @param buflen buffer length.
 *	On call, length of data_buf.
 *  On successful return, length of stored data.
 */
aud_error_t
dante_sdp_descriptor_serialise
(
	const dante_sdp_descriptor_t * sdp_descriptor,
	void * data_buf,
	size_t * buflen
);


/**
 * Deserialise a Dante AES67 SDP descriptor from a buffer
 *
 * Reads back a serialised descriptor, populating only the fields relevant
 * for RX flow creation.
 *
 * @param sdp_descriptor allocated descriptor ref (overwritten by call)
 * @param data_buf serialised descriptor
 * @param buflen length of serialised descriptor
 */
aud_error_t
dante_sdp_descriptor_deserialise
(
	dante_sdp_descriptor_ref_t * d,
	const void * data_buf,
	size_t buflen
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* _DANTE_SDP_H */

