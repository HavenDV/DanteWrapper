/*
 * File     : $RCSfile$
 * Created  : July 2008
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Time handling functions for the cross-platform API
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file platform_time.h
 *
 * Cross-platform time definitions and functions used throughout the Dante API.
 *
 * Do not include this file directly, instead include "audinate/dante_api.h"
 */

#ifndef _AUD_PLATFORM_TIME_H
#define _AUD_PLATFORM_TIME_H

#ifndef AUD_PLATFORM
#error platform_time.h should never be included directly
#endif

#ifdef __cplusplus
extern "C" {
#endif



/**
 * The number of milliseconds in a second
 */
#define AUD_MSEC_PER_SEC 1000UL

/**
 * The number of nanoseconds in a second
 */
#define AUD_NSEC_PER_SEC 1000000000UL

/**
 * The number of microseconds in a second
 */
#define AUD_USEC_PER_SEC 1000000UL

/**
 * The number of microseconds in a millisecond
 */
#define AUD_USEC_PER_MSEC 1000UL

/**
 * The number of nanoseconds in a millisecond
 */
#define AUD_NSEC_PER_MSEC 1000000UL

/**
 * The number of nanoseconds in a second
 */
#define AUD_NSEC_PER_SEC 1000000000UL

/**
 * The number of 100 nanoseconds in a second
 */
#define AUD_100NSEC_PER_SEC 10000000UL

/**
 * The number of 100 nanoseconds in a millisecond
 */
#define AUD_100NSEC_PER_MSEC 10000UL

/**
 * The number of 100 nanoseconds in a microsecond
 */
#define AUD_100NSEC_PER_USEC 10UL

/**
 * Time zero (utime)
 */
extern const aud_utime_t AUD_UTIME_ZERO;

/**
 * Get the current system time for the local machine
 * in seconds/microseconds.
 * This is equivalent to the POSIX gettimeofday() function.
 *
 * @param at the time structure that will contain the current time
 *
 * @return AUD_SUCCESS if the current time was successfully obtained, an error otherwise
 */
aud_error_t
aud_utime_get
(
	aud_utime_t * at
);

/**
 * Add 'add' to 'at and store in 'at'
 *
 * @param add the time value to be added to 'at'
 * @param at the time value that will have 'add' added to it
 *
 * @return AUD_SUCCESS if the times were successfully added, an error otherwise
 */
AUD_INLINE aud_error_t
aud_utime_add
(
	aud_utime_t * at, 
	const aud_utime_t * add
) {
	at->tv_sec += add->tv_sec;
	at->tv_usec += add->tv_usec;
	while (at->tv_usec >= (aud_tv_usec_t) AUD_USEC_PER_SEC)
	{
		at->tv_sec ++;
		at->tv_usec -= (aud_tv_usec_t) AUD_USEC_PER_SEC;
	}
		// 'while' rather than 'if' handles un-normalised values

	return AUD_SUCCESS;
}

/**
 * Subtract 'sub' from 'at' and store in 'at'
 *
 * @param sub the time value to be subtracted to 'at'
 * @param at the time value that will have 'add' substracted from it
 *
 * @return AUD_SUCCESS if the times were successfully subtracted, an error otherwise
 */
AUD_INLINE aud_error_t
aud_utime_sub
(
	aud_utime_t * at,
	const aud_utime_t * sub
) {
	// need to avoid wrapping on an unsigned value
	while (at->tv_usec < sub->tv_usec)
	{
		at->tv_usec += (aud_tv_usec_t) AUD_USEC_PER_SEC;
		at->tv_sec --;
	}
		// 'while' rather than 'if' handles un-normalised values
	at->tv_usec -= sub->tv_usec;
	at->tv_sec  -= sub->tv_sec;

	return AUD_SUCCESS;
}

/**
 * Compare two times
 *
 * @param at1 the first time value 
 * @param at2 the second time value
 *
 * @return < 0 if at1 is before (less) than at2,
 *   0 if at1 == at2 and > 0 if at1 is after (greater than) at2
 */
AUD_INLINE int32_t
aud_utime_compare
(
	const aud_utime_t * at1,
	const aud_utime_t * at2
) {
	if (at1->tv_sec == at2->tv_sec)
		return at1->tv_usec - at2->tv_usec;
	else
	{
		return (int32_t)(at1->tv_sec - at2->tv_sec);
	}
}


/**
 * The ctime string format alsways has a maximum length of 26, including
 * the trailing null terminator.
 */
#define AUD_CTIME_BUF_LEN 26

/**
 * A buffer long enough to store a ctime string
 */
typedef char aud_ctime_buf_t[AUD_CTIME_BUF_LEN];

/**
 * C89 ctime is NOT thread-safe. This function implements
 * a thread-safe version akin to ctime_r (the C99 standard).
 *
 * @param at the time value to be printed
 * @param buf the buffer to contrain the formatted time string
 *
 * @note Passing NULL for the time will use the current time from aud_utime_get.
 * 
 * @return buf or NULL if an error occurred
 */
char *
aud_utime_ctime
(
	const aud_utime_t * at,
	aud_ctime_buf_t buf
);

/**
 * Helper function that gets a ctime string but removes the trailing newline
 *
 * @param at the time value to be printed
 * @param buf the buffer to constrain the formatted time string
 * 
 * @return buf or NULL if an error occurred
 */
AUD_INLINE char *
aud_utime_ctime_no_newline
(
	const aud_utime_t * at,
	aud_ctime_buf_t buf
) {
	char * rbuf = aud_utime_ctime(at, buf);
	if (rbuf)
	{
		rbuf[24] = '\0';
	}
	return rbuf;
}


#ifdef __cplusplus
}
#endif

#endif

