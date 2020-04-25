/*
 * File     : $RCSfile$
 * Created  : December 2012
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Audio Error Reporting types and Functions
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file routing_error_reporting.h
 *
 * Dante supports reporting of various types of audio errors. Different
 * devices support differing subsets of these available types. Each device
 * can describe which of the available types it provides via its error
 * reporting capabilities fields.
 *
 * Error types can be categorized as follows:
 * 
 * 1. Per-flow-interface qualitative information
 *
 *  These fields indicated that given type of error occurred for the
 *  given flow on a given interface.
 *  When an error of this type occurs the field remains set until
 *  explicitly cleared by the user.
 *  Changes to these fields are always notified via conmon using Audinate
 *  event notification messages.
 *
 *  Errors include:
 *  - Early packets (The packet arrived too early for the audio to be
 *    meaningfully processed)
 *  - Late packets (The packet arrived too late for the audio to be
 *    meaningfully processed)
 *  - Dropped packets (A packet didn't arrive at all)
 *  - Out-of-order packets (Packets arrived out-of-sequence)
 *
 * 2. Per-flow-interface quantitative information:
 *
 *  These are counters, minima and maxima for aspects of a given flow
 *  on a given interface. 
 *  These fields can update continually; they may be explicitly reset by the user.
 *  Changes to these error types are not notified via conmon
 *    
 *  Information includes:
 *  - Early packet counters
 *  - Late packet counters
 *  - Dropped packet counters
 *  - Worst case packet delay: what is the worst case delay for a packet,
 *    ie how close is this flow to having packets arrive late
 */

#ifndef _ROUTING_ERROR_REPORTING_H
#define _ROUTING_ERROR_REPORTING_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/routing.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// RX Flow error types. The DR_* values are deprecated
// Use the DANTE_* types instead
//----------------------------------------------------------

enum
{
	DR_RXFLOW_ERROR_FLAG_INDEX_EARLY_PACKETS = DANTE_RXFLOW_ERROR_TYPE_EARLY_PACKETS,
	DR_RXFLOW_ERROR_FLAG_INDEX_LATE_PACKETS = DANTE_RXFLOW_ERROR_TYPE_LATE_PACKETS,
	DR_RXFLOW_ERROR_FLAG_INDEX_DROPPED_PACKETS = DANTE_RXFLOW_ERROR_TYPE_DROPPED_PACKETS,
	DR_RXFLOW_ERROR_FLAG_INDEX_OUT_OF_ORDER_PACKETS = DANTE_RXFLOW_ERROR_TYPE_OUT_OF_ORDER_PACKETS,
	DR_RXFLOW_ERROR_FLAG_COUNT = 4
};

enum
{
	DR_RXFLOW_ERROR_FLAG_EARLY_PACKETS        = DANTE_RXFLOW_ERROR_FLAG_EARLY_PACKETS,
	DR_RXFLOW_ERROR_FLAG_LATE_PACKETS         = DANTE_RXFLOW_ERROR_FLAG_LATE_PACKETS,
	DR_RXFLOW_ERROR_FLAG_DROPPED_PACKETS      = DANTE_RXFLOW_ERROR_FLAG_DROPPED_PACKETS,
	DR_RXFLOW_ERROR_FLAG_OUT_OF_ORDER_PACKETS = DANTE_RXFLOW_ERROR_FLAG_OUT_OF_ORDER_PACKETS
};

typedef dante_rxflow_error_flags_t dr_rxflow_error_flags_t;

enum
{
	DR_RXFLOW_ERROR_FIELD_INDEX_NUM_EARLY_PACKETS        = DANTE_RXFLOW_ERROR_TYPE_EARLY_PACKETS,
	DR_RXFLOW_ERROR_FIELD_INDEX_NUM_LATE_PACKETS         = DANTE_RXFLOW_ERROR_TYPE_LATE_PACKETS,
	DR_RXFLOW_ERROR_FIELD_INDEX_NUM_DROPPED_PACKETS      = DANTE_RXFLOW_ERROR_TYPE_DROPPED_PACKETS,
	DR_RXFLOW_ERROR_FIELD_INDEX_NUM_OUT_OF_ORDER_PACKETS = DANTE_RXFLOW_ERROR_TYPE_OUT_OF_ORDER_PACKETS,
	DR_RXFLOW_ERROR_FIELD_INDEX_MAX_LATENCY_US           = DANTE_RXFLOW_ERROR_TYPE_MAX_LATENCY,
	DR_RXFLOW_ERROR_FIELD_COUNT = 5
};

typedef dante_rxflow_error_type_t dr_rxflow_error_field_t;

enum
{
	DR_RXFLOW_ERROR_FIELD_FLAG_NUM_EARLY_PACKETS        = DANTE_RXFLOW_ERROR_FLAG_EARLY_PACKETS,
	DR_RXFLOW_ERROR_FIELD_FLAG_NUM_LATE_PACKETS         = DANTE_RXFLOW_ERROR_FLAG_LATE_PACKETS,
	DR_RXFLOW_ERROR_FIELD_FLAG_NUM_DROPPED_PACKETS      = DANTE_RXFLOW_ERROR_FLAG_DROPPED_PACKETS,
	DR_RXFLOW_ERROR_FIELD_FLAG_NUM_OUT_OF_ORDER_PACKETS = DANTE_RXFLOW_ERROR_FLAG_OUT_OF_ORDER_PACKETS,
	DR_RXFLOW_ERROR_FIELD_FLAG_MAX_LATENCY_US           = DANTE_RXFLOW_ERROR_FLAG_MAX_LATENCY,
};

typedef dante_rxflow_error_flags_t dr_rxflow_error_field_flags_t;

/**
 * Which per-flow-interface error flags are supported by this device?
 *
 * This field is only meaningful after a device's capabilites have been queried
 */
dr_rxflow_error_flags_t 
dr_device_available_rxflow_error_flags
(
	const dr_device_t * device
);

/**
 * Which per-flow-interface error fields are supported by this device?
 *
 * This field is only meaningful after a device's capabilites have been queried
 */
dr_rxflow_error_field_flags_t 
dr_device_available_rxflow_error_fields
(
	const dr_device_t * device
);

/**
 * Get the subseconds range for the timestamps returned by rxflow error reporting
 */
uint32_t
dr_device_rxflow_error_subsecond_range
(
	const dr_device_t * device
);

/**
 * Get all per-rxflow error flags. The flow interface flags are laid out in an array
 * ordered by interface then by flow id, so rx flow 1 primary is first, then
 * then flow 1 secondary, then flow 2 secondary etc.
 * The array contains entries for all flows across all interfaces regardless of whether a
 * flow is configured or not.
 * The total number of flows and interfaces is specified by the device's capabilities 
 *
 * This function will fail if the device's capabilities are not known. The contents of the 
 * array will not be valid until the rxflow error flags have been updated
 */
aud_error_t
dr_device_get_rxflow_error_flags
(
	const dr_device_t * device,
	dante_rxflow_error_flags_t ** error_flags_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
);

/**
 * Get all per-rxflow error fields. The flow interface fields are laid out in an array
 * ordered by interface then by flow id, so rx flow 1 primary is first, then
 * then flow 1 secondary, then flow 2 secondary etc.
 * The array contains entries for all flows across all interfaces regardless of whether a
 * flow is configured or not.
 * The total number of flows and interfaces is specified by the device's capabilities 
 *
 * This function will fail if the device's capabilities are not known. The contents of the 
 * array will not be valid until the rxflow error field has been updated
 */
aud_error_t
dr_device_get_rxflow_error_fields
(
	const dr_device_t * device,
	dante_rxflow_error_type_t field_type,
	uint32_t ** error_fields_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
);

/**
 * update and optionally clear the rxflow error flags
 */
aud_error_t
dr_device_update_rxflow_error_flags
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	aud_bool_t clear
);

/**
 * update and optionally clear the rxflow error fields
 */
aud_error_t
dr_device_update_rxflow_error_fields
(
	dr_device_t * device,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id,
	dante_rxflow_error_type_t field_type,
	aud_bool_t clear
);

/**
 * Get the error flags for the given flow interface.
 * These flags are only valid after updating the rxflow error flags
 *
 * @note this function is inefficient. If dealing with all flows it is faster to access the array directly
 * using dr_device_get_per_rxflow_error_flags
 */
aud_error_t
dr_rxflow_get_error_flags
(
	const dr_rxflow_t * flow,
	unsigned int intf,
	dr_rxflow_error_flags_t * error_flags_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
);


/**
 * Get the error field for the given flow interface.
 * These flags are only valid after updating the rxflow error flags
 *
 * @param flow the rxflow handle
 * @param intf the interface to query
 * @param field the index of the field to query. The flag for this field must be set in either dr_device_get_per_rxflow_error_fields or dr_device_get_one_rxflow_error_fields
 * @param value_ptr a pointer to the location that will receive the current value. Must be non-NULL.
 * @param timestamp_ptr a pointer to the location that will receive the timestamp associated with the current value. May be NULL.
 *
 * @note this function is inefficient. If dealing with all flows it is faster to access the array directly
 * using dr_device_get_per_rxflow_error_fields
 */
aud_error_t
dr_rxflow_get_error_field_uint32
(
	const dr_rxflow_t * flow,
	unsigned int intf,
	dante_rxflow_error_type_t field_type, 
	uint32_t * value_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
);

/**
 * Get the number of early packets for this flow
 * This value is only valid after the device's RX_ERROR_REPORTING component has been updated
 */
AUD_INLINE aud_error_t
dr_rxflow_num_early_packets
(
	const dr_rxflow_t * flow,
	unsigned int intf,
	uint32_t * num_early_packets_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
) {
	return dr_rxflow_get_error_field_uint32(flow, intf, DR_RXFLOW_ERROR_FIELD_INDEX_NUM_EARLY_PACKETS, num_early_packets_ptr, timestamp_ptr);
}

/**
 * Get the number of late packets for this flow
 * This value is only valid after the device's RX_ERROR_REPORTING component has been updated
 */
AUD_INLINE aud_error_t
dr_rxflow_num_late_packets
(
	const dr_rxflow_t * flow,
	unsigned int intf,
	uint32_t * num_late_packets_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
) {
	return dr_rxflow_get_error_field_uint32(flow, intf, DR_RXFLOW_ERROR_FIELD_INDEX_NUM_LATE_PACKETS, num_late_packets_ptr, timestamp_ptr);
}

/**
 * Get the number of dropped packets for this flow
 * This value is only valid after the device's RX_ERROR_REPORTING component has been updated
 */
AUD_INLINE aud_error_t
dr_rxflow_num_dropped_packets
(
	const dr_rxflow_t * flow,
	unsigned int intf,
	uint32_t * num_dropped_packets_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
) {
	return dr_rxflow_get_error_field_uint32(flow, intf, DR_RXFLOW_ERROR_FIELD_INDEX_NUM_DROPPED_PACKETS, num_dropped_packets_ptr, timestamp_ptr);
}

/**
 * Get the number of dropped packets for this flow
 * This value is only valid after the device's RX_ERROR_REPORTING component has been updated
 */
AUD_INLINE aud_error_t
dr_rxflow_num_out_of_order_packets
(
	const dr_rxflow_t * flow,
	unsigned int intf,
	uint32_t * num_out_of_order_packets_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
) {
	return dr_rxflow_get_error_field_uint32(flow, intf, DR_RXFLOW_ERROR_FIELD_INDEX_NUM_OUT_OF_ORDER_PACKETS, num_out_of_order_packets_ptr, timestamp_ptr);
}

/**
 * Get the maximum recevied packet latency for this flow interface, in microseconds
 * This value is only valid after the device's RX_ERROR_REPORTING component has been updated
 */
AUD_INLINE 
aud_error_t
dr_rxflow_max_latency_us
(
	const dr_rxflow_t * flow,
	unsigned int intf,
	dante_latency_us_t * max_latency_us_ptr,
	dante_rxflow_error_timestamp_t * timestamp_ptr
) {
	return dr_rxflow_get_error_field_uint32(flow, intf, DR_RXFLOW_ERROR_FIELD_INDEX_MAX_LATENCY_US, max_latency_us_ptr, timestamp_ptr);
}

#ifdef __cplusplus
}
#endif

#endif
