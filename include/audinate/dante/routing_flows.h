/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Routing template routing types and functions.
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file routing_flows.h
 *
 * Routing flow types and definitions. These functions allow a host
 * application to control the way audio routes are established in order
 * to provide predictable flow creation and
 * thus predictable network utilitization.
 *
 * There are several elements to template-based routing:
 * - Configuring the properties of a receive flow to control its source 
 *   device and overall link utilization 
 * - For multicast routing, configuring a multicast transmit flow 
 * - Associating recieve channels with receive flows. Channels that have
 *   an associated flow will only attempt to complete their
 *   subscriptions using the given flow and will not initiate ad-hoc
 *   subscription completion, ensuring  predictable resource usage.
 */
#ifndef _DANTE_ROUTING_FLOWS_H
#define _DANTE_ROUTING_FLOWS_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/routing.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct dr_txflow_config;
/**
 * A handle for a TX flow configuration structure. A TX
 * flow configuration is created and manipulated as appropriate. The
 * configuration is then either applied or discarded.
 */
typedef struct dr_txflow_config dr_txflow_config_t;

struct dr_rxflow_config;
/**
 * A handle for a receive flow configuration structure. A receive
 * flow configuration is created and manipulated as appropriate. The
 * configuration is then either applied or discarded.
 */
typedef struct dr_rxflow_config dr_rxflow_config_t;

//----------------------------------------------------------
// Rx Channel information / configuration
//----------------------------------------------------------

/**
 * Does the device support recieve templates? (older devices do not)
 *
 * @param device the device being queried
 *
 * @return AUD_TRUE if the device supports receive templates, AUD_FALSE otherwise
 */
aud_bool_t
dr_device_supports_rx_templates
(
	const dr_device_t * device
);

//----------------------------------------------------------
// Rx Channel information / configuration
//----------------------------------------------------------

/** 
 * Get the id of the flow with which this flow has been associated.
 * If no flow has been associated with this channel then return 0
 * 
 * @param channel the receive channel
 *
 * @return The channel's associated flow, or NULL if no associatation
 *   has been made.
 */
dante_id_t
dr_rxchannel_get_associated_flow_id
(
	const dr_rxchannel_t * channel
);

//----------------------------------------------------------
// Tx Flow user-configuration information
//----------------------------------------------------------

/**
 * Was this flow manually or automatically configured?
 *
 * @param flow the flow handle
 * @param manual_ptr a pointer to a location to store the flow's 'manual' status
 */
aud_error_t
dr_txflow_is_manual
(
	dr_txflow_t * flow,
	aud_bool_t * manual_ptr
);

//----------------------------------------------------------
// Rx flow user-configuration information
//----------------------------------------------------------

/**
 * Does the flow have a multicast template?
 *
 * @param flow the flow handle
 * @param is_multicast_template_ptr a pointer to a location to store the flow's 'multicast template' status
 */
aud_error_t
dr_rxflow_is_multicast_template
(
	dr_rxflow_t * flow,
	aud_bool_t * is_multicast_template_ptr
);

/**
 * Does the flow have a unicast template?
 *
 * @param flow the flow handle
 * @param is_unicast_template_ptr a pointer to a location to store the flow's 'unicast template' status
 */
aud_error_t
dr_rxflow_is_unicast_template
(
	dr_rxflow_t * flow,
	aud_bool_t * is_unicast_template_ptr
);

//----------------------------------------------------------
// Tx Flow configuration
//----------------------------------------------------------

/**
 * Create a new transmit flow configuration object
 *
 * @param device the device to be configured
 * @param id the id of the flow to be configured
 * @param num_slots the number of slots the flow will have
 * @param config_ptr a pointer to the flow configation handle
 *
 * @return AUD_SUCCESS if the flow configuration object was successfully
 *   created or an error otherwise
 */
aud_error_t
dr_txflow_config_new
(
	dr_device_t * device,
	uint16_t id,
	uint16_t num_slots,
	dr_txflow_config_t ** config_ptr
);

/**
 * Get a transmit flow configuration object that allows modifications to
 * an existing user-configured flow. A modify operation only allows changes
 * to the channels in the flow's slot contents.
 *
 * A modify operation guarantees glitch-free operation for all
 *   slots that are not altered.
 *
 * @param flow the flow to be modified
 * @param config_ptr a pointer to a location to hold the new config object
 *   pointer
 *
 * @note 'flow' must be an existing user-configured flow. 
 *   It is illegal to modify automatically-configured flows.
 */
aud_error_t
dr_txflow_replace_channels
(
	dr_txflow_t * flow,
	dr_txflow_config_t ** config_ptr
);

/**
 * How many slots are in the flow configuration object?
 *
 * @param config the config object
 *
 * @return the numbner of flow slots specified by the configuration object
 */
uint16_t
dr_txflow_config_num_slots
(
	dr_txflow_config_t * config
);

/**
 * Get the channel at the given slot for this flow configuration object
 *
 * @param config the config object
 * @param slot the zero-based slot index 
 *
 * @return the channel at the given slot for this flow configuration object
 */
dr_txchannel_t * 
dr_txflow_config_channel_at_slot
(
	dr_txflow_config_t * config,
	uint16_t slot
);

/**
 * What is the name of the flow being configured? May be NULL if a new flow
 * is being configured and no name has been set.
 *
 * @param config the config object
 *
 * @return the configured name for this flow
 */
const char *
dr_txflow_config_get_name
(
	const dr_txflow_config_t * config
);

/**
 * Add a channel to a flow at the given slot. If the channel is already in
 * the flow, it will be moved to the given slot and the old slot marked as
 * empty.
 *
 * @param config the config object
 * @param channel the transmit channel to be added to this config or moved
 * @param slot the zero-based index of the slot where the channel will be
 * added / moved
 *
 * @return AUD_SUCCESS if the channel was added / moved to the slot or an
 *   error otherwise.
 */
aud_error_t
dr_txflow_config_add_channel
(
	dr_txflow_config_t * config,
	dr_txchannel_t * channel,
	uint16_t slot
);

/**
 * Remove a channel from the flow. The slot that contained the channel is
 * now empty.
 *
 * @param config the config object
 * @param channel the transmit channel to be remove
 *
 * @return AUD_SUCCESS if the channel was removed from the configuration
 *   or an error otherwise.
 */
aud_error_t
dr_txflow_config_remove_channel
(
	dr_txflow_config_t * config,
	dr_txchannel_t * channel
);

/**
 * Set the name for the flow being configured.
 *
 * @param config the config object
 * @param name the new name for the flow. If NULL then the flow will
 *   revert to its default name.
 * 
 * @return AUD_SUCCESS if the flow config was updated or an error
 *   otherwise
 */
aud_error_t
dr_txflow_config_set_name
(
	dr_txflow_config_t * config,
	const char * name
);

/**
 * Set the latency for the flow being configured.
 *
 * @param config the config object
 * @param latency_us the new latency for the config object, in microseconds. If latency is zero then
 *   the flow will use the device's default tx latency value
 * 
 * @return AUD_SUCCESS if the flow config was updated or an error
 *   otherwise
 *
 * @note Tx flow configuration is not supported on older Dante devices.
 */
aud_error_t
dr_txflow_config_set_latency_us
(
	dr_txflow_config_t * config,
	dante_latency_us_t latency_us
);

/**
 * Set the frames-per-packet for the flow being configured.
 *
 * @param config the config object
 * @param fpp the new frames-per-packet for the config object. If fpp is zero then
 *   the flow will use the device's default tx fpp value
 * 
 * @return AUD_SUCCESS if the flow config was updated or an error
 *   otherwise
 */
aud_error_t
dr_txflow_config_set_fpp
(
	dr_txflow_config_t * config,
	dante_fpp_t fpp
);

/**
 * Set the encoding for the given flow config.
 *
 * @param config the te flow config
 * @param encoding the encoding for the flow
 * 
 * @return AUD_SUCCESS if the flow config was updated or an error
 *   otherwise
 */
aud_error_t
dr_txflow_config_set_encoding
(
	dr_txflow_config_t * config,
	dante_encoding_t encoding
);

/**
 * Commit the changes specified by the 'config' object.
 * This operation also releases the config object handle.
 *
 * @note Successful completion of this function does not mean that the
 *   configuration change was successfully appliedon on the device, just
 *   that the message was successfully sent to the device.
 *
 * @param config the config object
 * @param response_fn the function to call upon completion of this
 *   operation
 * @param request_id the id for this operation
 *
 * @return AUD_SUCCESS if the configuration was sent to the device, or an
 *   error otherwise
 */
aud_error_t
dr_txflow_config_commit
(
	dr_txflow_config_t * config,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Discard the changes specified by the 'config' object
 * This operation also releases the config object handle.
 *
 * @param config the config object
 *
 * @return AUD_SUCCESS
 */
aud_error_t
dr_txflow_config_discard
(
	dr_txflow_config_t * config
);

/**
 * Delete the flow and release the handle. This operation is 
 * only allowed on user-configured flows.
 *
 * @note Successful completion of this function does not mean that the
 *   configuration change was successfully appliedon on the device, just
 *   that the message was successfully sent to the device.
 *
 * @param flow_ptr a pointer to a flow handle
 * @param response_fn the function to call upon completion of this
 *   operation
 * @param request_id the id for this operation
 *
 * @return AUD_SUCCESS if a delete message was successfully sent to the
 *   device or an error otherwise
 */
aud_error_t
dr_txflow_delete
(
	dr_txflow_t ** flow_ptr,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);


/**
 * Delete all user-configured transmit flows.
 * 
 * @note Successful completion of this function does not mean that the
 *   configuration change was successfully applied on on the device, just
 *   that the message was successfully sent to the device.
 *
 * @param device the device to be configured
 * @param response_fn the function to call upon completion of this
 *   operation
 * @param request_id the id for this operation
 *
 * @return AUD_SUCCESS if a delete message was successfully sent to the
 *   device or an error otherwise
 */
aud_error_t
dr_device_delete_all_txflows
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

//----------------------------------------------------------
// Rx Flow template configuration
//----------------------------------------------------------

/**
 * Create a new unicast template configuration object for a flow
 * with the given id.
 *
 * This object is used to create a receive flow.  To modify an existing
 * active user-configured flow, use dr_rxflow_modify.
 *
 * @note If the id value is non-zero, the configuration will be applied to
 * the flow with the given id, overwriting any previous configuration for
 * that flow. A zero id is legal and means that the device should choose
 * an appropriate flow. If a existing flow already matches the given
 * configuration, no changes are made. Otherwise, the device searches for
 * an inactive slot to use. In no inactive slots are available, the call
 * fails. In any case, no existing flows will be changed. In general,
 * it is recommended that the host application choose an id to ensure that
 * the flow is created.
 *
 * @param device the device on which the flow is being configured
 * @param flow_id the id for this flow.
 * @param tx_device the name of the device from which the flow will
 *   receive audio.
 * @param num_slots the number of slots that this flow will have
 * @param config_ptr a pointer to the pointer that will hold the
 *   new configuration object.
 */
aud_error_t
dr_rxflow_config_new_unicast
(
	dr_device_t * device,
	dante_id_t flow_id,
	const char * tx_device,
	uint16_t num_slots,
	dr_rxflow_config_t ** config_ptr
);

/**
 * Create a new multicast template configuration object for a flow
 * with the given id.
 *
 * This object is used to create a receive flow.  To modify an existing
 * active user-configured flow, use dr_rxflow_modify.
 *
 * @note If the id value is non-zero, the configuration will be applied to
 * the flow with the given id, overwriting any previous configuration for
 * that flow. A zero id is legal and means that the device should choose
 * an appropriate flow. If a existing flow already matches the given
 * configuration, no changes are made. Otherwise, the device searches for
 * an inactive slot to use. In no inactive slots are available, the call
 * fails. In any case, no existing flows will be changed. In general,
 * it is recommended that the host application choose an id to ensure that
 * the flow is created.
 *
 * @param device the device on which the flow is being configured
 * @param flow_id the id for this flow.
 * @param tx_flow the name of the transmit flow from which the flow
 *   will receive audio,
 * @param tx_device the name of the device from which the flow will
 *   receive audio.
 * @param config_ptr a pointer to the pointer that will hold the
 *   new configuration object.
 */
aud_error_t
dr_rxflow_config_new_multicast
(
	dr_device_t * device,
	dante_id_t flow_id,
	const char * tx_device,
	const char * tx_flow,
	dr_rxflow_config_t ** config_ptr
);

/**
 * Get a receive flow configuration object that allows modifications to
 * associations for an existing flow. The config object contains an initially
 * empty set of associations to which the host application can add new associations.
 * A new set of associations overrides all old associations. Channels that were previously
 * associated but are not in the new set have their subscriptions and associations cleared.
 * Channels in both the old and new assocations sets will have their subscription updated
 * if necessary. If the subscription is unchanged then audio will not be interrupted.
 * 
 * @note 'flow' must be a flow with an existing user-configured template.
 *   Dynamically-configured flows may not be modified.
 *
 * @param flow the flow to be modified
 * @param config_ptr a pointer to a location to hold the new config object
 *   pointer
 *
 * @return AUD_SUCCESS if a modification object was successfully obtained or
 *   an error otherwise.
 */
aud_error_t
dr_rxflow_replace_associations
(
	const dr_rxflow_t * flow,
	dr_rxflow_config_t ** config_ptr
);

/**
 * Does the config object desribe a multicast template?
 *
 * @param config the configuration object
 *
 * @return AUD_TRUE if the config object describes a multicast template, AUD_FALSE otherwise
 */
aud_bool_t
dr_rxflow_config_is_multicast_template
(
	const dr_rxflow_config_t * config
);

/**
 * Does the config object desribe a unicast template?
 *
 * @param config the configuration object
 *
 * @return AUD_TRUE if the config object describes a unicast template, AUD_FALSE otherwise
 */
aud_bool_t
dr_rxflow_config_is_unicast_template
(
	const dr_rxflow_config_t * config
);


/**
 * How many slots are in the flow configuration object?
 *
 * @param config the configuration object
 *
 * @return the number of slots in the configuration object
 */
uint16_t
dr_rxflow_config_num_slots
(
	const dr_rxflow_config_t * config
);

/**
 * What is the current samplerate for the flow configuration object?
 *
 * @param config the configuration object
 *
 * @return the samplerate of the configuration object
 */
dante_samplerate_t
dr_rxflow_config_get_samplerate
(
	const dr_rxflow_config_t * config
);

/**
 * What is the current audio encoding for the flow configuration object?
 *
 * @param config the configuration object
 *
 * @return the audio encoding of the configuration object
 */
dante_encoding_t
dr_rxflow_config_get_encoding
(
	const dr_rxflow_config_t * config
);

/**
 * What is the transmit device name for this flow configuration object?
 *
 * @param config the configuration object
 *
 * @return the transmit device name of the configuration object
 */
const char *
dr_rxflow_config_get_tx_device_name
(
	const dr_rxflow_config_t * config
);

/**
 * What is the transmit flow name for this flow configuration object?
 * Note that this value is only set for multicast template flow
 * configuration objects.
 *
 * @param config the configuration object
 *
 * @return the transmit flow name of the configuration object
 */
const char *
dr_rxflow_config_get_tx_flow_name
(
	const dr_rxflow_config_t * config
);

/**
 * Set the audio encoding for this flow.
 *
 * @param config configuration object
 * @param encoding encoding for flow
 *
 * Encoding might not be checked for consistency until commit.
 */
aud_error_t
dr_rxflow_config_set_encoding
(
	dr_rxflow_config_t * config,
	dante_encoding_t encoding
);


/**
 * Add a channel to the list of channels that will be associated with the
 * given flow as part of the flow's initial configuration. Unicast flow
 * configurations allow a maximum of num_slots initial recieve channel
 * associations as part of the initial configuration. Multicast flow
 * configurations allow an arbitrary number of channels to be added
 * (including multiple channels with the same subscription) but no more
 * than max_rx_flow_slots distinct subscriptions.
 *
 * @param config the configuration object
 * @param channel the channel to add to the configuration
 * @param tx_channel_name the transmit channel part of the receive
 *   channel's subscription
 *
 * @return AUD_SUCCESS if the channel was successfully added to the set
 *   of initial associations or an error otherwise.
 */
aud_error_t
dr_rxflow_config_add_associated_channel
(
	dr_rxflow_config_t * config,
	const dr_rxchannel_t * channel,
	const char * tx_channel_name
);

/**
 * Remove a channel from the list of channels that will be associated
 * with the flow as part of its initial configuration.
 * 
 * @param config the configuration object
 * @param channel the channel to add to the configuration
 *
 * @return AUD_SUCCESS if the channel was successfully removed from the
 *   set of initial associations or an error otherwise.
 */
aud_error_t
dr_rxflow_config_remove_associated_channel
(
	dr_rxflow_config_t * config,
	const dr_rxchannel_t * channel
);

/**
 * Get the initial subscription for the given channel that is part of the
 * initial association list for the flow.
 *
 * @param config the configuration object
 * @param channel the channel whose channel subscription is to be obtained.
 *
 * @return the subscription that will be given to the channel as part of
 *   its initial association, or NULL if the channel is not in the list
 *   of initial associations.
 */
const char *
dr_rxflow_config_get_associated_channel_subscription
(
	dr_rxflow_config_t * config,
	const dr_rxchannel_t * channel
);

/**
 * Commit the changes specified by the 'config' object.
 *
 * @note When this function completes successfully,
 *   (a) the configuration is sent but not yet applied to the device,
 *   (b) the config object handle is released
 *
 * @param config the configuration object
 * @param response_fn the function to call when a response is received
 *   for this operation.
 * @param request_id the request id for this operation
 *
 * @return AUD_SUCCESS if the configuration was sent to the device, or an
 *   error otherwise.
 */
aud_error_t
dr_rxflow_config_commit
(
	dr_rxflow_config_t * config,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Discard the changes specified by the 'config' object
 * This operation also releases the config object handle.
 *
 * @param config the configuration object
 *
 * @return AUD_SUCCESS
 */
aud_error_t
dr_rxflow_config_discard
(
	dr_rxflow_config_t * config
);

/**
 * Delete a user-configured flow. This function also clears the
 * subscriptions and associations for any channels that were associated
 * with this flow
 *
 * @note Successful completion of this function does not mean that the
 *   configuration change was successfully appliedon on the device, just
 *   that the message was successfully sent to the device.
 *
 * @param flow_ptr A pointer to the flow handle
 * @param response_fn the function to call upon completion of this
 *   operation
 * @param request_id the id for this operation
 *
 * @return AUD_SUCCESS if the message was successfully sent to the device
 *   or an error otherwise
 */
aud_error_t
dr_rxflow_delete
(
	dr_rxflow_t ** flow_ptr,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Delete one or more flows identified by their ids
 *
 * @note Successful completion of this function does not mean that the
 *   configuration change was successfully applied on on the device, just
 *   that the message was successfully sent to the device.
 *
 * @param device The device to be configured
 * @param response_fn the function to call upon completion of this
 *   operation
 * @param request_id the id for this operation
 * @param num_flow_ids the number of flow ids in the flow_ids array
 * @param flow_ids the ids of the flows to be deleted
 *
 * @return AUD_SUCCESS if the message was successfully sent to the device
 *   or an error otherwise
 */
aud_error_t
dr_device_delete_rxflows
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	unsigned int num_flow_ids,
	const dante_id_t * flow_ids
);

/**
 * Delete all user-configured flows on the device.
 * This function also clears all subscriptions and all associations
 * for any channels associated with any user-configured flow.
 *
 * @note Successful completion of this function does not mean that the
 *   configuration change was successfully appliedon on the device, just
 *   that the message was successfully sent to the device.
 *
 * @param device The device to be configured
 * @param response_fn the function to call upon completion of this
 *   operation
 * @param request_id the id for this operation
 *
 * @return AUD_SUCCESS if the message was successfully sent to the device
 *   or an error otherwise
 */
aud_error_t
dr_device_delete_all_rxflows
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

#ifdef __cplusplus
}
#endif

#endif

