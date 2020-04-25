/*
 * Created  : December 2014
 * Author   : Andrew White
 * Synopsis : AES67 routing support
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file routing_aes67.h
 *
 * Manual routing support for AES67 transport
 */
#ifndef _ROUTING_AES67_H
#define _ROUTING_AES67_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/routing.h"
#include "dante/routing_flows.h"
#include "dante/dante_aes67.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// AES67/SMPTE support
//----------------------------------------------------------


// Queries

/**
 * Get the AES67 multicast prefix used by this device
 * @param device the device object
 * @param pointer to IPv4 multicast prefix (low 16 bits are always zero)
 *
 * @return AUD_SUCCESS if the mcast prefix is valid
 *   or an error otherwise
*/

aud_error_t
dr_device_get_aes67_mcast_prefix
(
	const dr_device_t * device,
	uint32_t *ipv4_prefix
);

/**
* Is RTP supported on the device?
* @param device the device object
*
* @return AUD_TRUE if RTP is supported, AUD_FALSE otherwise
*/
aud_bool_t
dr_device_is_rtp_supported
(
	const dr_device_t * device
);

/**
* Is RTP enabled on the device?
* @param device the device object
*
* @return AUD_TRUE if RTP is enabled, AUD_FALSE otherwise
* Ony useful to call if dr_device_is_rtp_supported returns AUD_TRUE.
*/
aud_bool_t
dr_device_is_rtp_enabled
(
	const dr_device_t * device
);

/**
 * Get the flow class of a TX flow
 * Is the flow DANTE or AES67?
 * @param flow
 *
 * @return flow class (or 0 if invalid)
 */
aud_error_t
dr_txflow_get_flow_class
(
	const dr_txflow_t * flow,
	dante_flow_class_t * fclass
);

/**
* Get the AES67 flow origin address associated with a TX flow
* @param config flow configuration
* @param pointer to address storage
*/
aud_error_t
dr_txflow_get_aes67_sdp_origin_addr
(
	const dr_txflow_t * flow,
	uint32_t * origin_addr
);

/**
 * Get the media clock offset associated with a TX flow
 *
 * @param flow - AES67 TX flow
 *
 * @return media clock offset of the flow (or 0 if unknown or invalid)
 */
aud_error_t
dr_txflow_get_aes67_media_clock_offset
(
	const dr_txflow_t * flow,
	dante_sdp_media_clock_offset_t * offset
);

/**
 * Get the AES67 flow SDP session id associated with a TX flow
 * @param config flow configuration
 *
 * @return AES67 SDP session id (0 - not available)
 */
aud_error_t
dr_txflow_get_aes67_sdp_session_id
(
	const dr_txflow_t * flow,
	dante_sdp_session_id_t * session_id
);

/**
 * Get the flow class of a RX flow
 *
 * @param flow
 *
 * @return flow class (or 0 if invalid)
 */
aud_error_t
dr_rxflow_get_flow_class
(
	const dr_rxflow_t * flow,
	dante_flow_class_t * fclass
);

/**
 * Get the media clock offset associated with a RX flow
 *
 * @param flow - AES67 RX flow
 *
 * @return media clock offset of the flow (or 0 if unknown or invalid)
 */
aud_error_t
dr_rxflow_get_aes67_media_clock_offset
(
	const dr_rxflow_t * flow,
	dante_sdp_media_clock_offset_t * offset
);

/**
* Get the origin ipv4 address associated with a RX flow
*
* @param flow - AES67 RX flow
*
* @return ipv4 source addr of the flow (or 0 if unknown or invalid)
*/
aud_error_t
dr_rxflow_get_aes67_sdp_origin_addr
(
	const dr_rxflow_t * flow,
	uint32_t * origin_addr
);

/**
 * Get the AES67 flow SDP session id associated with a RX flow
 * @param config flow configuration
 *
 * @return AES67 SDP session id (0 - not available)
 */
aud_error_t
dr_rxflow_get_aes67_sdp_session_id
(
	const dr_rxflow_t * flow,
	dante_sdp_session_id_t * session_id
);


// Configuration

/**
 * Set the AES67 multicast prefix used by this device
 * @param device the device object
 * @param IPv4 multicast prefix (low 16 bits are always zero and must start with 239.x.x.x)
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 * @return AUD_SUCCESS if the mcast prefix was successfully
 *   created or an error otherwise
*/
aud_error_t
dr_device_set_aes67_mcast_prefix
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	uint32_t mcast_prefix
);

/**
 * Create a new aes67 multicast transmit flow configuration object
 *
 * @param device the device to be configured
 * @param id the id for this flow. Must be non-zero.
 * @param num_slots the number of slots the flow will have
 * @param config_ptr a pointer to the flow configation handle
 *
 * @return AUD_SUCCESS if the flow configuration object was successfully
 *   created or an error otherwise
 */
aud_error_t
dr_txflow_config_new_aes67_multicast
(
        dr_device_t * device,
        uint16_t id,
        uint16_t num_slots,
        dr_txflow_config_t ** config_ptr
);

/**
 * Create a new aes67 multicast receive flow configuration object
 *
 * @param device the device on which the flow is being configured
 * @param id the id for this flow. Must be non-zero.
 * @param num_slots the number of slots the flow will have
 * @param config_ptr a pointer to the flow configuration handle
 *
 * @return AUD_SUCCESS if the flow configuration object was successfully
 *   created or an error otherwise
 */
aud_error_t
dr_rxflow_config_new_aes67_multicast
(
	dr_device_t * device,
	uint16_t id,
	uint16_t num_slots,
	dr_rxflow_config_t ** config_ptr
);

/**
* Set all AES67 parameters for the Rx flow from a browsed SAP/SDP object
* @param config flow configuration
* @param SDP descriptor object
*/
aud_error_t
dr_rxflow_config_set_aes67_params_from_sap
(
	dr_rxflow_config_t * config,
	const dante_sdp_descriptor_t * sdp_descriptor
);

/**
 * Add a channel to a slot in an AES67 flow.
 *
 * The channel will receive audio from the given slot.  It can later be removed
 * by unsubscribing it.
 *
 * @param config the configuration object
 * @param channel The channel to be added. must be non-null.
 * @param slot the slot from which this channel will receive audio.
 */
aud_error_t
dr_rxflow_config_add_aes67_channel
(
	dr_rxflow_config_t * config,
	dr_rxchannel_t * channel,
	uint16_t slot
);

/**
* Get the asscoaited flow slot id for a given RX channel
* @param rx channel
*
* @return uint16_t slot id (0 - not available)
*/
uint16_t
dr_rxchannel_get_flow_slot_id
(
	const dr_rxchannel_t * rx
);


// Extracting SDP information

/**
 * Synthesise an SDP descriptor from an existing AES67 RX flow
 *
 * @param rxflow configured rxflow
 * @parem sdp_descriptor sdp descriptor ref that will be overwritten with result
 */
aud_error_t
dr_rxflow_synthesise_sdp_descriptor
(
	const dr_rxflow_t * rxflow,
	dante_sdp_descriptor_ref_t * sdp_descriptor
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
