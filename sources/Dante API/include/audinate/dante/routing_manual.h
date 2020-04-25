/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Manual routing types and functions.
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file routing_manual.h
 *
 * Manual routing types and definitions. These functions bypass the standard Zen 
 * configuration process. If these functions are used then
 * correct management of routing addresses, conflict detection etc... 
 * becomes the responsibility of the host application.
 */
#ifndef _ROUTING_MANUAL_H
#define _ROUTING_MANUAL_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/routing.h"
#include "dante/routing_flows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Manual TX flow interrogation
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

/**
 * Is this flow intended to be advertised?
 * If this value is AUD_TRUE then the flow would like to be advertised.
 *
 * @note In order to be advertised, the device's tx advertisement scope must also be set to an appropriate value.
 */
aud_error_t
dr_txflow_is_advertised
(
	dr_txflow_t * flow,
	aud_bool_t * advertised_ptr
);

/**
 * Is this flow persistent?
 *
 * @param flow the flow handle
 * @param manual_ptr a pointer to a location to store the flow's 'manual' status
 */
aud_error_t
dr_txflow_is_persistent
(
	dr_txflow_t * flow,
	aud_bool_t * manual_ptr
);

//----------------------------------------------------------
// Manual TX flow configuration
//----------------------------------------------------------

/**
 * Explicitly set the destination address and port for this flow on the
 * specified interface. Calling this on any interface implicitly turns
 * off automatic address selection on all interfaces. In general, if an
 * address is explicitly provided for one interface then addresses should
 * be explicitly provided for all interfaces. If automatic address
 * selection is turned off and an interface does not have an explicitly
 * provided address, the flow will not be transmitted on that interface.
 *
 * @param config the tx flow configuration object 
 * @param intf the interface in which to set the address
 * @param address the address to set for the given interface
 */
aud_error_t
dr_txflow_config_set_address
(
	dr_txflow_config_t * config,
	uint16_t intf,
	const dante_ipv4_address_t * address
);

/**
 * Specify whether or not a flow should be advertised. This value must be set at 
 * flow creation time. This flag is always set for flows with manually configured
 * unicast addresses. It is optional for flows with multicast addresses.
 *
 * @param config the tx flow configuration object
 * @param advertised should the flow be advertised
 */
aud_error_t
dr_txflow_config_set_advertised
(
	dr_txflow_config_t * config,
	aud_bool_t advertised
);

/**
 * Specify whether or not a flow should be persistent. This value must be set at 
 * flow creation time.
 *
 * @param config the tx flow configuration object
 * @param persistent should the flow be persistent
 */
aud_error_t
dr_txflow_config_set_persistent
(
	dr_txflow_config_t * config,
	aud_bool_t persistent
);

//----------------------------------------------------------
// Manual RX flow interrogation
//----------------------------------------------------------

/**
 * Was this flow manually configured?
 *
 * @param flow the flow handle
 * @param is_manual_ptr a pointer to a location to store the flow's 'manual' status
 */
aud_error_t
dr_rxflow_is_manual
(
	dr_rxflow_t * flow,
	aud_bool_t * is_manual_ptr
);

/**
 * Get the latency for this rx flow, in microseconds. Rx flow latency is only
 * meaningful for static rx flows and should be ignored for dynamically
 * allocated rx flows.
 *
 * @param flow the flow handle
 * @param latency_us_ptr a pointer to a location to store the flow's latency in microseconds
 */
aud_error_t
dr_rxflow_get_latency_us
(
	dr_rxflow_t * flow,
	dante_latency_us_t * latency_us_ptr
);

/**
 * Is this flow persistent?
 *
 * @param flow the flow handle
 * @param manual_ptr a pointer to a location to store the flow's 'manual' status
 */
aud_error_t
dr_rxflow_is_persistent
(
	dr_rxflow_t * flow,
	aud_bool_t * manual_ptr
);

//----------------------------------------------------------
// Manual RX flow configuration
//----------------------------------------------------------

/**
 * Get the range of legal values for manually-configured unicast receive flows.
 * Dante devices may have restrictions on the which ports are allowed to be 
 * used for audio flows. Manually configured unicast flows must use ports
 * within this range. The legal ranges includes both the min and max values.
 * This value is part of a device's 'PROPERTIES' component.
 *
 * Older Dante devices do not support this function. For these devices the
 * function returns AUD_ERR_VERSION.
 *
 * @param device the device 
 * @param min_port a pointer to the value that will contain the minimum legal
 *   value for unicast ports on this device
 * @param max_port a pointer to the value that will contain the maximum legal
 *   value for unicast ports on this device
 */
aud_error_t
dr_device_get_manual_unicast_receive_port_range
(
	const dr_device_t * device,
	uint16_t * min_port,
	uint16_t * max_port
);

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
 */
aud_error_t
dr_rxflow_config_new_manual
(
	dr_device_t * device,
	uint16_t id,
	uint16_t num_slots,
	dr_rxflow_config_t ** config_ptr
);

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
 *   It is illegal to modify an automatically-configured flows.
 */
aud_error_t
dr_rxflow_modify_manual_channels
(
	const dr_rxflow_t * flow,
	dr_rxflow_config_t ** config_ptr
);

/**
 * Get the number of channels that will receive audio from the given slot for this
 * configuration
 *
 * @param config the configuration object
 * @param slot the slot being queried.
 */
uint16_t
dr_rxflow_config_num_slot_channels
(
	dr_rxflow_config_t * config,
	uint16_t slot
);

/**
 * Get the rx channel at the given index for the given slot in this configuration
 *
 * @param config the configuration object
 * @param slot the slot being queried.
 * @param index the index within the slot
 */
dr_rxchannel_t *
dr_rxflow_config_slot_channel_at_index
(
	dr_rxflow_config_t * config,
	uint16_t slot,
	uint16_t index
);


/**
 * Add a channel to the flow at the given slot. If the channel is already in the flow,
 * it will be moved

 * @param config the configuration object
 * @param channel The channel to be added. must be non-null.
 * @param slot the slot from which this channel will receive audio.
 */
aud_error_t
dr_rxflow_config_add_manual_channel
(
	dr_rxflow_config_t * config,
	dr_rxchannel_t * channel,
	uint16_t slot
);

/**
 * Remove the channel from the given flow
 *
 * @param config the configuration object
 * @param channel the channel to be removed, must be non-null.
 */
aud_error_t
dr_rxflow_config_remove_manual_channel
(
	dr_rxflow_config_t * config,
	dr_rxchannel_t * channel
);


/**
 * Remove all channels from a slot
 *
 * @param config the configuration object
 * @param slot the slot to be cleared
 */
aud_error_t
dr_rxflow_config_clear_slot
(
	dr_rxflow_config_t * config,
	uint16_t slot
);


/**
 * Explicitly set the destination address and port for this flow on the
 * specified interface. If 'address' is non-null it must be either:
 * - a multicast host and non-zero port: the flow will be configured using
 *   the given host and port
 * - a multicast host and zero port: the flow will be configured using the 
 *   the given host and the default multicast audio port
 * - a zero host and non-zero port: the flow will be configured using the
 *   device's unicast ip address and the given port
 * - a NULL address: the flow will not be configured on the given interface
 *
 * @param config the configuration object
 * @param intf the index of the interface on which the address will be used
 * @param address the address for the interface
 *
 * @note a non-NULL address with zero host and zero port is an illegal parameter
 *
 * @note Certain Dante devices only support multicast flow configuration
 *       in range 239.255.*.* 
 */
aud_error_t
dr_rxflow_config_set_address
(
	dr_rxflow_config_t * config,
	uint16_t intf,
	const dante_ipv4_address_t * address
);

/**
 * Set the latency for this rx flow config object, in microseconds. This value must
 * be less than or equal to the device's maximum rx latency.
 *
 * @param config the configuration object
 * @param latency_us the latency for this flow, in microseconds
 */
aud_error_t
dr_rxflow_config_set_latency_us
(
	dr_rxflow_config_t * config,
	dante_latency_us_t latency_us
);

/**
 * Get the latency for this rx flow config object, in microseconds.
 *
 * @param config the configuration object
 */
dante_latency_us_t
dr_rxflow_config_get_latency_us
(
	dr_rxflow_config_t * config
);

/**
 * Specify whether or not a flow should be persistent. This value must be set at
 * flow creation time.
 *
 * @param config the rx flow configuration object
 */
aud_error_t
dr_rxflow_config_set_persistent
(
	dr_rxflow_config_t * config,
	aud_bool_t persistent
);

/**
* Specify whether or not a flow is manually subscribed. This value may be set at
* flow creation time.
*
* @param config the rx flow configuration object
*/
aud_error_t
dr_rxflow_config_set_flow_sub
(
	dr_rxflow_config_t * config,
	aud_bool_t subscription
);

//----------------------------------------------------------
// Wide-Area advert management
//----------------------------------------------------------

typedef enum
{
	DR_ADVERT_SCOPE_NONE,
	DR_ADVERT_SCOPE_MULTICAST,
	DR_ADVERT_SCOPE_UNICAST,
	DR_ADVERT_SCOPE_ALL,
	DR_NUM_ADVERT_SCOPES      
} dr_advert_scope_t;

/**
 * Does the given device support configuration of local and wide area adverts?
 *
 * This function is only valid after the device's capabilities have been queried
 * @param device the device object
 */
aud_bool_t
dr_device_can_set_advert_scope
(
	const dr_device_t * device
);

/**
 * What is the current advert scope for dbcp adverts?
 * This value is part of the device's 'properties' component
 *
 * @param device the device object
 */
dr_advert_scope_t
dr_device_get_dbcp_advert_scope
(
	const dr_device_t * device
);

/**
 * What is the current advert scope for tx adverts?
 * This value is applied to tx channel, label and flow adverts.
 *
 * This value is part of the device's 'properties' component
 * 
 * @param device the device object
 */
dr_advert_scope_t
dr_device_get_tx_advert_scope
(
	const dr_device_t * device
);

/**
 * Set advert scope for dbcp advert and / or txchannel adverts
 *
 * @param device the device object
 * @param dbcp_advert_scope if non-null, set dbcp advert scope to the value pointed to
 * @param tx_advert_scope if non-null, set txchannel advert scope to the value pointed to
 * @param response_fn the function to call upon completion of this operation
 * @param request_id the id for this operation
 */
aud_error_t
dr_device_set_advert_scope
(
	dr_device_t * device,
	dr_advert_scope_t * dbcp_advert_scope,
	dr_advert_scope_t * tx_advert_scope,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);



#ifdef __cplusplus
}
#endif


#ifdef DR_LEGACY
#include "routing_manual_legacy.h"
#endif

#endif
