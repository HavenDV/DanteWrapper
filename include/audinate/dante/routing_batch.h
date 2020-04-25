/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Batch-oriented routing types and functions.
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file routing_batch.h
 *
 * Batch-oriented routing types and definitions. These functions allow
 * efficient configuration of multiple aspects of a device. Batch functions
 * have slightly different requirements to normal operations in the routing api.
 *
 * Firstly, it is *not* necessary to have queried a device's capabilities before sending
 * a batch configuration message. However, if the device's capabilities are not known
 * the API will not provide any error checking (for channel ids etc) or attempt to
 * update any internal state. Note that for remote devices it is still necessary to
 * have resolved the device's addresses before undertaking any batch operations.
 *
 * Secondly, the API does *not* make any internal copies of the data batch data structures
 * provided by the host application, but assumes that this information will remain in existence
 * and remain valid until the request has been completed. Thus, the host application must not
 * alter or free the batch data provided until either the completion callback is called or the 
 * the host application has explicitly cancelled the request via the function dr_device_cancel_request.
 * This requirement is necessary as the API may need to send and receive multiple messages
 * in order to completely configure the device.
 */
#ifndef _ROUTING_BATCH_H
#define _ROUTING_BATCH_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/routing.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Batch subscription
//----------------------------------------------------------

/**
 * Structure describing multiple subscriptions
 */
typedef struct dr_batch_subscription
{
	/** 
	 * The id of the channel to be configured
	 */
	dante_id_t rxchannel_id;
	
	/** 
	 * The name of the transmit channel to which the recieve channel will be subscribed.
	 * If this field and the device_name field are both set to the empty string then
	 * the receive channel will be unsubscribed.
	 */
	dante_name_t channel;

	/** 
	 * The name of the transmit device to which the recieve channel will be subscribed.
	 * If this field and the device_name field are both set to the empty string then
	 * the receive channel will be unsubscribed.
	 */
	dante_name_t device;

} dr_batch_subscription_t;

/**
 * Issue a batch subscription to a device
 *
 * @param device the device to be configured
 * @param response_fn the function to be called upon completion of this operation
 * @param request_id A request identifier for this operation
 * @param num_subscriptions the number of subscriptions in this batch operation
 * @param subscriptions the subscriptions in this batch operation. Must not be 'NULL'.
 * 
 * @note The API makes a copy of the 'subscriptions' pointer but does not copy the array contents.
 *       The memory pointed to by this variable must *not* be freed or modified while
 *       the request is in progress as the API may refer to the contents of this array at any time
 *       during the request process.
 */
aud_error_t
dr_device_batch_subscribe
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	uint16_t num_subscriptions,
	const dr_batch_subscription_t * subscriptions
);

//----------------------------------------------------------
// Batch Labels
//----------------------------------------------------------

/**
 * Structure describing a single TX channel label.
 * Intepretation is as follows:
 * - non-zero label id:
 *   - non-zero channel and non-empty name: configure a specific label (identified by id) with the given name / channel
 *   - zero channel or empty name: delete a specific label (identified by id)
 * - zero label id, non-zero channel, non-empty name: configure a label with the given name / channel using an unused label id
 * - zero label id, zero channel, non-empty name: delete the label with the given name
 * - zero label id, empty name: invalid configuration,
 */
typedef struct dr_batch_txlabel
{
	/** 
	 * The id of the label to be configured.
	 * A non-zero value forces the device to configure a particular label,
	 * while a '0' allows the device to chaoose a free label as appropriate.
	 */
	dante_id_t txlabel_id;

	/** 
	 * The id of the channel to which the label will be attached.
	 */
	dante_id_t txchannel_id;

	/** 
	 * The label
	 */
	dante_name_t label;

} dr_batch_txlabel_t;

/**
 * Issue a batch tx label command to a device
 *
 * @param device the device to be configured
 * @param response_fn the function to be called upon completion of this operation
 * @param request_id A request identifier for this operation
 * @param num_labels the number of labels in this batch operation
 * @param labels the labels in this batch operation. Must not be 'NULL'.
 * 
 * @note The API makes a copy of the 'labels' pointer but does not copy the array contents.
 *       The memory pointed to by this variable must *NOT* be freed or modified while
 *       the request is in progress as the API may refer to the contents of this array at any time
 *       during the request process.
 */
aud_error_t
dr_device_batch_txlabel
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	uint16_t num_labels,
	const dr_batch_txlabel_t * labels
);


//----------------------------------------------------------
// Batch Labels
//----------------------------------------------------------

/**
 * Structure describing a single RX channel label.
 */
typedef struct dr_batch_rxlabel
{
	/** 
	 * The id of the channel.
	 */
	dante_id_t rxchannel_id;

	/** 
	 * The label
	 */
	dante_name_t label;

} dr_batch_rxlabel_t;

/**
 * Issue a batch tx label command to a device
 *
 * @param device the device to be configured
 * @param response_fn the function to be called upon completion of this operation
 * @param request_id A request identifier for this operation
 * @param num_labels the number of labels in this batch operation
 * @param labels the labels in this batch operation. Must not be 'NULL'.
 * 
 * @note The API makes a copy of the 'labels' pointer but does not copy the array contents.
 *       The memory pointed to by this variable must *NOT* be freed or modified while
 *       the request is in progress as the API may refer to the contents of this array at any time
 *       during the request process.
 */
aud_error_t
dr_device_batch_rxlabel
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	uint16_t num_labels,
	const dr_batch_rxlabel_t * labels
);


//----------------------------------------------------------
// Batch TX Flows
//----------------------------------------------------------

enum { DR_BATCH_TX_FLOW_MAX_SLOTS = 16 };

/** 
 * Structure describing a single TX flow
 *
 * @note: The only flow element currently configurable via the batch commands
 *   is channel assignments.
 */
typedef struct dr_batch_txflow
{
	/**
	 * ID of the flow.  If 0, will be auto-allocated (not recommended).
	 */
	dante_id_t txflow_id;

	/**
	 * Number of slots (channels) in the flow
	 * If 0 and a non-0 flow ID, the flow will be deleted if it exists
	 */
	uint16_t num_slots;
	
	/**
	 * Array of channel IDs, one per slot.  Null ID (0) is legal, and indicates
	 * a slot without an associated channel.
	 */
	dante_id_t channels[DR_BATCH_TX_FLOW_MAX_SLOTS];

} dr_batch_txflow_t;

/**
 * Issue a batch TX flow command to a device
 *	
 * @param device the device to configure
 * @param response_fn function called upon completion of this operation
 * @param request_id request identifier for this operation
 * @param num_flows number of flows in this batch operation
 * @param flows flows in this batch operation. Must be non-NULL.
 *	
 * @note The API makes a copy of the 'flows' pointer but does not copy the array contents.
 *  The memory pointed to by this variable must *NOT* be freed or modified while the
 *  request is in progess, as the API may refer to the contents of the array at any time.
 */
aud_error_t
dr_device_batch_txflow
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	uint16_t num_flows,
	const dr_batch_txflow_t * flows
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
