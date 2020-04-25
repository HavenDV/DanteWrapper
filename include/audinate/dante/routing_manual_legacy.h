/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Manual routing legacy types and functions.
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file routing_manual_legacy.h
 *
 * Legacy manual routing legacy types and definitions. All the functions in this file are
 * deprecated and may be removed in future releases.
 */
#ifndef _ROUTING_MANUAL_LEGACY_H
#define _ROUTING_MANUAL_LEGACY_H

#ifndef _ROUTING_MANUAL_H
#error routing_manual_legacy.h should never be included directly
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the latency for this rx flow, in microseconds. Rx flow latency is only
 * meaningful for static rx flows and should be ignored for dynamically
 * allocated rx flows.
 *
 * @param flow the flow handle
 * @param latency_us_ptr a pointer to a location to store the flow's latency in microseconds
 *
 * @deprecated use dr_rxflow_get_latency_us instead
 */
AUD_INLINE aud_error_t
dr_rxflow_get_latency
(
	dr_rxflow_t * flow,
	dante_latency_us_t * latency_us_ptr
) {
	return dr_rxflow_get_latency_us(flow, latency_us_ptr);
}

/**
 * Get a new manual RX flow configuration object for a flow
 * with the given id.
 *
 * @param device the device on which the flow is being configured
 * @param id the id for this flow. Must be non-zero.
 * @param num_slots the number of slots that this flow will have
 * @param config_ptr a pointer to the pointer that will hold the
 *   new configuration object.
 * 
 * @return AUD_SUCCESS if the flow configuration object was successfully
 *   created or an error otherwise
 *
 * @deprecated use dr_rxflow_config_new_manual instead
 */
AUD_INLINE aud_error_t
dr_rxflow_config_new
(
	dr_device_t * device,
	uint16_t id,
	uint16_t num_slots,
	dr_rxflow_config_t ** config_ptr
) {
	return dr_rxflow_config_new_manual(device, id, num_slots, config_ptr);
}


/**
 * Get the latency for this rx flow config object, in microseconds.
 *
 * @param config the configuration object
 *
 * @deprecated use dr_rxflow_config_get_latency_us instead
 */
AUD_INLINE dante_latency_us_t
dr_rxflow_config_get_latency
(
	dr_rxflow_config_t * config
) {
	return dr_rxflow_config_get_latency_us(config);
}

/**
 * Set the latency for this rx flow config object, in microseconds. This value must
 * be less than or equal to the device's maximum rx latency.
 *
 * @param config the configuration object
 * @param latency_us the latency for this flow, in microseconds
 *
 * @deprecated use dr_rxflow_config_set_latency_us instead
 */
AUD_INLINE aud_error_t
dr_rxflow_config_set_latency
(
	dr_rxflow_config_t * config,
	dante_latency_us_t latency_us
) {
	return dr_rxflow_config_set_latency_us(config, latency_us);
}

/**
 * Add a channel to the flow at the given slot. If the channel is already in the flow,
 * it will be moved

 * @param config the configuration object
 * @param channel The channel to be added. must be non-null.
 * @param slot the slot from which this channel will receive audio.
 *
 * @deprecated used dr_rxflow_config_add_manual_channel instead
 */
AUD_INLINE aud_error_t
dr_rxflow_config_add_channel
(
	dr_rxflow_config_t * config,
	dr_rxchannel_t * channel,
	uint16_t slot
) {
	return dr_rxflow_config_add_manual_channel(config, channel, slot);
}

/**
 * Remove the channel from the given flow
 *
 * @param config the configuration object
 * @param channel the channel to be removed, must be non-null.
 *
 * @deprecated used dr_rxflow_config_remove_manual_channel instead
 */
AUD_INLINE aud_error_t
dr_rxflow_config_remove_channel
(
	dr_rxflow_config_t * config,
	dr_rxchannel_t * channel
) {
	return dr_rxflow_config_remove_manual_channel(config, channel);
}

/**
 * Get an rx flow configuration object that allows modifications to
 * an existing manually-configured flow. A modify operation
 * allows changes to channels in any slot but the number of slots is
 * fixed. A modify operation guarantees glitch-free operation for all
 * slots that are not altered.
 *
 * @param flow the rx flow to be modified
 * @param config_ptr a pointer to the pointer that will hold the
 *   new configuration object.
 *
 * @note 'flow' must be an existing manually-configured flow. 
 *   It is illegal to modify automatically-configured flows.
 *
 * @deprecated use dr_rxflow_modify_manual_channels instead
 */
AUD_INLINE aud_error_t
dr_rxflow_modify
(
	const dr_rxflow_t * flow,
	dr_rxflow_config_t ** config_ptr
) {
	return dr_rxflow_modify_manual_channels(flow, config_ptr);
}


/**
 *	Renamed to dr_rxflow_modify_manual_channels, for clarity.
 *
 * @param flow the rx flow to be modified
 * @param config_ptr a pointer to the pointer that will hold the
 *   new configuration object.
 *
 * @note 'flow' must be an existing manually-configured flow. 
 *   It is illegal to modify automatically-configured flows.
 *
 * @deprecated use dr_rxflow_modify_manual_channels instead
 */
AUD_INLINE aud_error_t
dr_rxflow_modify_manual
(
	const dr_rxflow_t * flow,
	dr_rxflow_config_t ** config_ptr
) {
	return dr_rxflow_modify_manual_channels(flow, config_ptr);
}


#ifdef __cplusplus
}
#endif


#endif

