/*
 * File     : $RCSfile$
 * Created  : January 2007
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Dante Routing API: legacy compatibility types and functions
 *
 * This software is copyright (c) 2008-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */
/**
 * @file routing_legacy.h
 * 
 * Definitions for backwards compatibility with older versions of the Dante API.
 */
#ifndef _DANTE_ROUTING_LEGACY_H
#define _DANTE_ROUTING_LEGACY_H

#ifndef _DANTE_ROUTING_H
#error dante/routing_legacy should never be included directly.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * routing API version structure
 *
 * @deprecated use dante_version_t instead
 */
typedef dante_version_t dr_version_t;

/**
 * Status codes for the RX Channels
 *
 * @deprecated use dante_rxstatus_t instead
 */
typedef dante_rxstatus_t dr_rxstatus_t;

  /** 
   * Channel is not subscribed or otherwise doing anything interesting.
   *
   * @deprecated: use DANTE_RXSTATUS_NONE instead
   */
#define DR_RXSTATUS_NONE DANTE_RXSTATUS_NONE
	
  /** 
   * Name not yet found
   *
   * @deprecated: use DANTE_RXSTATUS_UNRESOLVED instead
   */
#define DR_RXSTATUS_UNRESOLVED DANTE_RXSTATUS_UNRESOLVED
	
  /**
   * Active subsctipion to an automatically configured source flow 
   *
   * @deprecated: use DANTE_RXSTATUS_DYNAMIC instead
   */
#define DR_RXSTATUS_DYNAMIC DANTE_RXSTATUS_DYNAMIC
	
  /**
   * Active subscription to a manually configured source flow
   *
   * @deprecated: use DANTE_RXSTATUS_STATIC instead
   */
#define DR_RXSTATUS_STATIC DANTE_RXSTATUS_STATIC
	
  /** 
   * Name has been found, but not yet processed. This is an transient state 
   *
   * @deprecated: use DANTE_RXSTATUS_RESOLVED instead
   */
#define DR_RXSTATUS_RESOLVED DANTE_RXSTATUS_RESOLVED
	
  /** 
   * Name has been found and processed; setting up flow. This is an transient state 
   *
   * @deprecated: use DANTE_RXSTATUS_IN_PROGRESS instead
   */
#define DR_RXSTATUS_IN_PROGRESS DANTE_RXSTATUS_IN_PROGRESS

  /** 
   * Error: an error occurred while trying to resolve name
   *
   * @deprecated: use DANTE_RXSTATUS_RESOLVE_FAIL instead
   */
#define DR_RXSTATUS_FAIL DANTE_RXSTATUS_RESOLVE_FAIL
	
  /** 
   * Error: The name was found but the connection process failed
   * (the receiver could not communicate with the transmitter)
   *
   * @deprecated: use DANTE_RXSTATUS_NO_CONNECTION instead
   */
#define DR_RXSTATUS_NO_CONNECTION DANTE_RXSTATUS_NO_CONNECTION

  /** 
   * Error: Channel formats do not match
   *
   * @deprecated: use DANTE_RXSTATUS_CHANNEL_FORMAT instead
   */
#define DR_RXSTATUS_CHANNEL_FORMAT DANTE_RXSTATUS_CHANNEL_FORMAT
  
  /** 
   * Error: Flow formats do not match, 
   * e.g. Multicast flow with more slots than receiving device can handle
   *
   * @deprecated: use DANTE_RXSTATUS_FLOW_FORMAT instead
   */
#define DR_RXSTATUS_FLOW_FORMAT DANTE_RXSTATUS_FLOW_FORMAT
	
  /**
   * Receiver is out of resources (e.g. flows)
   *
   * @deprecated: use DANTE_RXSTATUS_NO_RX instead
   */
#define DR_RXSTATUS_NO_RX DANTE_RXSTATUS_NO_RX
	
  /**
   * Receiver couldn't set up the flow
   *
   * @deprecated: use DANTE_RXSTATUS_RX_FAIL instead
   */
#define DR_RXSTATUS_RX_FAIL DANTE_RXSTATUS_RX_FAIL
	
  /**
   * Transmitter is out of resources (e.g. flows)
   *
   * @deprecated: use DANTE_RXSTATUS_NO_TX instead
   */
#define DR_RXSTATUS_NO_TX DANTE_RXSTATUS_NO_TX
	
  /**
   * Transmitter couldn't set up the flow
   *
   * @deprecated: use DANTE_RXSTATUS_TX_FAIL instead
   */
#define DR_RXSTATUS_TX_FAIL DANTE_RXSTATUS_TX_FAIL
	
  /** 
   * Receiver got a QoS failure (too much data) when setting up the flow.
   *
   * @deprecated: use DANTE_RXSTATUS_QOS_FAIL_RX instead
   */
#define DR_RXSTATUS_QOS_FAIL_RX DANTE_RXSTATUS_QOS_FAIL_RX
	
  /**
   * Transmitter got a QoS failure (too much data) when setting up the flow.
   *
   * @deprecated: use DANTE_RXSTATUS_QOS_FAIL_TX instead
   */
#define DR_RXSTATUS_QOS_FAIL_TX DANTE_RXSTATUS_QOS_FAIL_TX
	
  /** 
   * Unexpected system failure.
   *
   * @deprecated: use DANTE_RXSTATUS_SYSTEM_FAIL instead
   */
#define DR_RXSTATUS_SYSTEM_FAIL DANTE_RXSTATUS_SYSTEM_FAIL


/**
 * Dante device, channel and label names are 32-byte characters.
 * A trailing NULL terminator must be included within the 32 characters
 *
 * @deprecated use DANTE_NAME_LENGTH instead
 */
#define DR_NAME_LENGTH DANTE_NAME_LENGTH

/**
 * A buffer long enough to hold the name of a Dante device, label or channel
 *
 * @deprecated use dante_name_t instead
 */
typedef dante_name_t dr_name_t;

  /**
   * A flag indicating a transmit channel's canonical name 
   *
   * @deprecated use DANTE_TXNAME_TYPE_CANONICAL instead
   */
#define DR_TXNAME_TYPE_CANONICAL DANTE_TXNAME_TYPE_CANONICAL
  /** 
   * A flag indicating a label associated with a transmit channel 
   *
   * @deprecated use DANTE_TXNAME_TYPE_LABEL instead
   */
#define DR_TXNAME_TYPE_LABEL DANTE_TXNAME_TYPE_LABEL
  /**
   * A mask for all possible txname types
   *
   * @deprecated use DANTE_TXNAME_TYPE_ALL instead
   */
#define DR_TXNAME_TYPE_ALL DANTE_TXNAME_TYPE_ALL

 /**
  * Bitfield for tx name types
  *
  * @deprecated use dante_txname_type_t instead
  */
typedef dante_txname_type_t dr_txname_type_t;


/**
 * Dante device identifiers are unsigned 16-bit values. Channel and Flow identifiers
 * are 1-based.
 *
 * @deprecated use drm_id_t instead
 */
typedef dante_id_t dr_id_t;


/**
 * A type for audio latencies (in microseconds)
 *
 * @deprecated use dante_latency_us_t instead
 */
typedef dante_latency_us_t dr_latency_t;

/**
 * A type specifier for sample rate
 *
 * @deprecated use dante_samplerate_t instead
 */
typedef dante_samplerate_t dr_samplerate_t;

/**
 * A type specifier for sample encoding
 *
 * @deprecated use dante_encoding_t instead
 */
typedef dante_encoding_t dr_encoding_t;

/** 
 * A constant specifiying the NULL encoding; used as a return value in error cases 
 *
 * @deprecated use DANTE_ENCODING_NONE instead
 */
#define DR_ENCODING_NONE DANTE_ENCODING_NONE

/**
 * A wrapper type to simplify passing sockets between the host application
 * and the routing API.
 *
 * @note The interpretation of 'n' is platform dependent. On a posix
 *   platform the value of 'n' is the highest fd in 'read_fds' plus one.
 *   On a windows platform the 'n' is the number of sockets in 'read_fds'.
 *
 * @deprecated use dante_sockets_t instead
 */
typedef dante_sockets_t dr_sockets_t;

/**
 * Helper method to a socket to the socket set, by adding s to read_fds
 * and updating 'n'. The behaviour this function is platform dependent.
 * On a posix platform, the new value for 'n' is the maximum of its old
 * value and (s+1). On a windows platform, n is incremented by 1.
 *
 * @deprecated use dante_sockets_add instead
 */
#define dr_sockets_add(SOCKETS,FD) dante_sockets_add(SOCKETS,FD)

/**
 * Clear a dr_sockets_t structure. Sets 'n' to 0 and calls FD_ZERO on
 * read_fds.
 *
 * @deprecated use dante_sockets_clear instead
 */
#define dr_sockets_clear(SOCKETS) dante_sockets_clear(SOCKETS)

/**
 * Asynchronous request-response communications are tracked using request
 * IDs. When a request message is sent, the host application provides a
 * callback function and the API provides a request id. When a response
 * arrives, the callback function fires with the request id as a
 * parameter.
 *
 * @deprecated use dante_request_id_t instead
 */
typedef dante_request_id_t dr_request_id_t;

/** 
 * A marker to indicate a non-existent request id or an invalid request
 * id value.
 *
 * @deprecated use DANTE_NULL_REQUEST_ID instead
 */
#define DR_NULL_REQ_ID DANTE_NULL_REQUEST_ID

/**
 * A flow address is an address-port combo
 *
 * @deprecated use dante_ipv4_address_t instead
 */
typedef dante_ipv4_address_t dr_address_t;

//----------------------------------------------------------
// Device operations
//----------------------------------------------------------

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
 * @deprecated This function is not supported on newer Dante devices.
 */
dante_latency_us_t
dr_device_get_rx_latency_network_us
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
 * 
 * @deprecated used dr_device_get_rx_latency_us instead
 */
AUD_INLINE dante_latency_us_t
dr_device_get_rx_latency
(
	const dr_device_t * device
) {
	return dr_device_get_rx_latency_us(device);
}

/**
 * Set the default receive (rx) latency for the device. This value
 * is used by the Dante device when determining latency for audio flows.
 * 
 * The 'reconfigure_existing_flows' flag specifies whether or not existing
 * audio flows should be reconfigured. Re-configuring audio latencies will
 * cause audio glitches and must only be use if this is indeed the desired behaviour.
 *
 * @param device the device to be configured
 * @param latency the new latency value for the device, in microseconds
 * @param reconfigure_existing_flows a flag indicating whether or not existing flows
 *    should be affected when setting a new device latency
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 *
 * @note reconfiguration of existing flows may not be supported on older Dante devices.
 *   If an attempt is made to configure such a device and the 
 *   'reconfigure_existing_flows' field set, the function returns AUD_ERR_VERSION
 *
 * @deprecated use dr_device_set_rx_performance instead
 *
 */
aud_error_t
dr_device_set_rx_latency
(
	dr_device_t * device,
	dante_latency_us_t latency,
	aud_bool_t reconfigure_existing_flows,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Set the preferred fpp value that this receiver will request when
 * configuring subscriptions.
 *
 * @param device the device to be configured
 * @param fpp the new recieve fpp value for the device
 * @param reconfigure_existing_flows a flag indicating whether or not existing dynamic flows
 *    should be affected when setting a new device fpp. Manually configured flows are not changed.
 * @param response_fn the function to call when this operation has completed.
 * @param request_id a pointer to a value that will the request id for this operation.
 *
 * @note Preferred receive fpp is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 *
 * @deprecated use dr_device_set_rx_performance instead
 */
aud_error_t
dr_device_set_rx_fpp
(
	dr_device_t * device,
	dante_fpp_t fpp,
	aud_bool_t reconfigure_existing_flows,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Get the maximum receive (rx) latency for this device, in microseconds.
 * 
 * @param device the device whose latency is to be obtained
 *
 * @note Maximum RX latency is part of the device 'properties'. An application can
 *       check the validity of the current value by checking whether the device's
 *       'properties' component is stale
 *
 * @deprecated used dr_device_get_rx_latency_max_us instead
 */
AUD_INLINE dante_latency_us_t
dr_device_get_rx_latency_max
(
	const dr_device_t * device
) {
	return dr_device_get_rx_latency_max_us(device);
}

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
 * @deprecated This function is not be supported on newer Dante devices.
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
 * @deprecated used dr_device_set_rx_latency_network_us instead
 */
AUD_INLINE aud_error_t
dr_device_set_rx_latency_network
(
	dr_device_t * device,
	dante_latency_us_t latency_network_us,
	aud_bool_t reconfigure_existing_flows,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
) {
	return dr_device_set_rx_latency_network_us(device,
		latency_network_us, reconfigure_existing_flows,
		response_fn, request_id);
}

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
 * @deprecated used dr_device_get_rx_latency_network_us instead
 */
AUD_INLINE dante_latency_us_t
dr_device_get_rx_latency_network
(
	const dr_device_t * device
) {
	return dr_device_get_rx_latency_network_us(device);
}

//----------------------------------------------------------
// Tx Operations
//----------------------------------------------------------

/**
 * Get a TX flow configuration object that allows modifications to
 * an existing manually-configured flow. A modify operation
 * allows changes to channels in any slot but the number of slots is
 * fixed. A modify operation guarantees glitch-free operation for all
 * slots that are not altered.
 *
 * @param flow the flow to be modified
 * @param config_ptr a pointer to a location to hold the new config object pointer
 *
 * @note 'flow' must be an existing manually-configured flow. 
 *   It is illegal to modify automatically-configured flows.
 *
 * @deprecated use dr_txflow_replace_channels
 */
AUD_INLINE aud_error_t
dr_txflow_modify
(
	dr_txflow_t * flow,
	dr_txflow_config_t ** config_ptr
) {
	return dr_txflow_replace_channels(flow, config_ptr);
}

/**
 * Set the channel at the given slot for this flow configuration object
 *
 * @param config the config object
 * @param slot the zero-based index of the slot to be configured
 * @param channel the transmit channel to be placed in the slot
 * 
 * @deprecated This function is deprecated as it does not respect rules
 *   regarding having the same channel appear multiple times in the same
 *   flow. While such a configuration is legal, it is not sensible as
 *   there is no value gained by having a single flow carrying multiple
 *   copies of the same channel
 */
aud_error_t
dr_txflow_config_set_channel_at_slot
(
	dr_txflow_config_t * config,
	uint16_t slot,
	dr_txchannel_t * channel
);

//----------------------------------------------------------
// Rx Channel operations
//----------------------------------------------------------

/**
 * Get the complete name of the network channel to which this
 * rx channel is subscribed in the form "channel\@device"
 * 
 * @param rx the receive channel
 *
 * @return NULL if not subscribed
 *
 * @deprecated Use dr_rxchannel_get_subscription_channel and 
 *   dr_rxchannel_get_subscription_device instead
 */
const char *
dr_rxchannel_get_subscription
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
 *
 * @deprecated Use dr_rxchannel_get_subscription_latency_us instead
 */
AUD_INLINE dante_latency_us_t
dr_rxchannel_get_subscription_latency
(
	const dr_rxchannel_t * rx
) {
	return dr_rxchannel_get_subscription_latency_us(rx);
}

/**
 * Get a string representation of the given receive status.
 *
 * @param status the status code
 *
 * @return a string representing the status code
 *
 * @deprecated use dr_rxstatus_to_string instead
 */
AUD_INLINE const char *
dr_rxstatus_to_string
(
	dante_rxstatus_t status
) {
	return dante_rxstatus_to_string(status);
}

#ifdef __cplusplus
}
#endif

#endif

