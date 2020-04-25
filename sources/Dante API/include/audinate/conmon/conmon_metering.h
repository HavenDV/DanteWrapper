/*
 * File     : $RCSfile$
 * Created  : August 2010
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : ConMon software metering channel implementation
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file conmon_metering.h 
 * ConMon metering channel types and definitions
 */
#ifndef _CONMON_METERING_H
#define _CONMON_METERING_H

#include "conmon.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Metering message definitions
//----------------------------------------------------------

enum
{
	CONMON_METERING_MESSAGE_VERSION_1 = 1,
	CONMON_METERING_MESSAGE_VERSION_2 = 2,
	CONMON_METERING_MESSAGE_VERSION_3 = 3,
	CONMON_METERING_MESSAGE_VERSION_CURRENT = CONMON_METERING_MESSAGE_VERSION_3
};

/** A typedef for the metering version */
typedef uint8_t conmon_metering_message_version_t;

/** A typedef for a peak value */
typedef uint8_t conmon_metering_message_peak_t;

enum
{
	CONMON_METERING_PEAK_CLIP             = 0x00,
	CONMON_METERING_PEAK_0_DB             = 0x01,
	// values between 0 and -126dB are laid out
	// sequentially in increments of -0.5dB
	CONMON_METERING_PEAK_MINUS_126_DB     = 0xFD,
	CONMON_METERING_PEAK_MUTE             = 0xFE,
	CONMON_METERING_PEAK_START_OF_MESSAGE = 0xFF
};


/**
 * Version 1 format is:
 * VERSION
 * NUM_TX
 * NUM_RX
 * 1..N TX PEAK
 * 1..N RX PEAK
 * 1..N TX RMS
 * 1..N RX RMS
 */
typedef struct conmon_metering_message_v1_v2
{
	uint8_t version;
	uint8_t num_tx;
	uint8_t num_rx;
} conmon_metering_message_v1_v2_t;

/**
 * Version 3 format is:
 * VERSION
 * NUM_TX (2bytes)
 * NUM_RX (2bytes)
 * 1..N TX PEAK
 * 1..N RX PEAK
 * 1..N TX RMS
 * 1..N RX RMS
 */
typedef struct conmon_metering_message_v3
{
	uint8_t version;
	uint8_t pad;
	uint16_t num_tx;
	uint16_t num_rx;
} conmon_metering_message_v3_t;

/**
 * Initialise a metering message. Sets the version and the
 * number of tx and rx channels, and returns the total size
 * of the message for the given settings
 *
 * @param msg A conmon message body that will hold the message
 * @param version the metering message format version
 * @param num_txchannels the number of transmit channels 
 * @param num_rxchannels the number of transmit channels 
 * @param size_ptr a pointer to a variable that will be set to the size of the metering message
 */
aud_error_t
conmon_metering_message_init
(
	conmon_message_body_t * msg,
	conmon_metering_message_version_t version,
	uint16_t num_txchannels,
	uint16_t num_rxchannels,
	uint16_t * size_ptr
);

/**
 * Parse a metering message, extrating pointers to the version and
 * the number of tx and rx channels
 *
 * @param msg a conmon message body containing the metering message
 * @param version_ptr a pointer to the version field in the packet
 * @param num_txchannels_ptr a pointer to the txchannel count field in the packet
 * @param num_rxchannels_ptr a pointer to the rxchannel count field in the packet
 */
aud_error_t
conmon_metering_message_parse
(
	const conmon_message_body_t * msg,
	conmon_metering_message_version_t * version_ptr,
	uint16_t * num_txchannels_ptr,
	uint16_t * num_rxchannels_ptr
);

/**
 * Get a pointer the the array of peak values for the given channel direction
 *
 * @param msg the metering message
 * @param dir the channel direction
 */
conmon_metering_message_peak_t *
conmon_metering_message_get_peaks
(
	conmon_message_body_t * msg,
	conmon_channel_direction_t dir
);

/**
 * Get a const pointer the the array of peak values for the given channel direction
 *
 * @param msg the metering message
 * @param dir the channel direction
 */
const conmon_metering_message_peak_t *
conmon_metering_message_get_peaks_const
(
	const conmon_message_body_t * msg,
	conmon_channel_direction_t dir
);

/**
 * convert a peak db value to a floating point value
 * in the range 0.0..-126.0
 *
 * @param peak the value to convert
 */
AUD_INLINE float
conmon_metering_message_peak_to_float
(
	conmon_metering_message_peak_t peak
) {
	// bounds checking
	if (peak < CONMON_METERING_PEAK_0_DB || peak > CONMON_METERING_PEAK_MINUS_126_DB)
	{
		return 0.0F; // have to return something...
	}
	// 1..253 -> 0..252 -> 0..126 -> 0..-126
	return -((peak-1)/2.0F);
}


/**
 * Convert float in range 0..-126db to 
 * a metering peak value.
 *
 * @param db the value to convert
 */
AUD_INLINE conmon_metering_message_peak_t
conmon_metering_message_peak_from_float
(
	 float db
) {
	// bounds checking
	if (db > 0.0F)
	{
		return CONMON_METERING_PEAK_CLIP;
	}
	else if (db < -126.0F)
	{
		return CONMON_METERING_PEAK_MUTE;
	}
	// 0..-126 -> 0..126 -> 0..252 -> 1..253
	return (conmon_metering_message_peak_t) (((-db)*2) + 1);
}

#ifdef __cplusplus
}
#endif

#endif

