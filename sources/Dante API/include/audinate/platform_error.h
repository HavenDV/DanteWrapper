/*
 * File     : $RCSfile$
 * Created  : March 2007
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Error codes for the Audinate Cross-Platform API
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */
/**
 * @file platform_error.h
 *
 * Cross-platform error types and definitions used by the Dante API
 *
 * Do not include this file directly, instead include "audinate/dante_api.h"
 */
#ifndef _AUD_PLATFORM_ERROR_H
#define _AUD_PLATFORM_ERROR_H

#ifndef AUD_PLATFORM
#error platform_error.h must not be included directly.
#endif

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------
// Error types
//----------------------------------------------------------

/**
 * typedef for an Audinate error code
 */
typedef unsigned int aud_error_t;

/**
 * typedef for a system error code
 */
typedef unsigned int aud_system_error_t;

/**
 * A string buffer used for printing human-readable error strings
 */
typedef char aud_errbuf_t[AUD_ENV_ERROR_STRLEN+1];

/**
 * Create an error message for the given error. The string format
 * will be:
 * For non-system errors, a message of the form:
 *   "<description>(<code>)"
 * For AUD_ERR_SYSTEM, a message of the form:
 *   "system error: <description>(<code>)"
 * based on the last system error.
 *
 * @param error the error code 
 * @param errbuf the buffer to contain the printed error message
 *
 * @return errbuf
 */
const char *
aud_error_message
(
	aud_error_t error,
	aud_errbuf_t errbuf
);

//----------
// More specific error functions below here
//----------

/**
 * convert a system error info an audinate API error
 *
 * @param error the system error to be converted to a Dante platform error
 *
 * @return the aud_error_t representation of the given system error
 */
aud_error_t
aud_error_from_system_error
(
	aud_system_error_t error
);

/**
 * Get the last system error
 *
 * @return the last system error 
 */
aud_system_error_t
aud_system_error_get_last (void);

/**
 * Convert the last system error into an API error if possible.
 * This is a convenience method that uses the other functions
 *
 * @return the last system error converted to an aud_error_t
 */
AUD_INLINE aud_error_t
aud_error_get_last (void)
{
	return aud_error_from_system_error(aud_system_error_get_last());
}

/**
 * Get the name of the error code as a string
 * returns NULL if not a valid error
 *
 * @param error the error code
 * 
 * @return a string with the name of the given error 
 */
const char *
aud_error_get_name
(
	aud_error_t error
);

/**
 * Get the description of the error code
 * returns NULL if not a valid error
 *
 * @param error the error code
 *
 * @return a string with a description of the given error
 */
const char *
aud_error_get_description
(
	aud_error_t error
);

/**
 * Get a string description of the last system error.
 * This method should always return a string of some sort
 * even if the error code is unknown
 *
 * @param error the system error code
 *
 * @return a string with a description of the given system error
 */
const char *
aud_system_error_get_description
(
	aud_system_error_t error
);


//----------------------------------------------------------
// generic errors
//----------------------------------------------------------

enum {
	AUD_SUCCESS,              // no error, all good
	AUD_ERR_DONE,             // operation done (eg EOF)
						      //  Use this to halt loops or similar where 'success'
							  //  means "all good, keep going'.
	AUD_ERR_SYSTEM,           // error was a system error not handled by API
	AUD_ERR_INVALIDPARAMETER, // invalid function parameter
	AUD_ERR_INVALIDDATA,      // internal data is incompatible with function
	AUD_ERR_INVALIDSTATE,     // wrong state to perform action
	AUD_ERR_NOMEMORY,         // out of memory
	AUD_ERR_INTERRUPTED,      // the operation was interrupted
	AUD_ERR_TRUNCATED,        // data truncation occured 
	AUD_ERR_NOTSUPPORTED,     // the operation is not supported on this platform
	AUD_ERR_TIMEDOUT,         // a time-restricted operation timed out
	AUD_ERR_NOTFOUND,         // a value was searched for but not found,
                              // e.g. in event_loop_socket_event_update
	AUD_ERR_DNSSDFAIL,        // DNS-SD failed
	AUD_ERR_RANGE,            // data out of range
	AUD_ERR_POLICY,           // administrative policy violation
	AUD_ERR_VERSION,          // version mismatch

	// Socket Errors, Mainly BSD-style mappings from E<ERR> to AUD_ERR_<ERR>
	// commented out values are codes which have not yet been required
	// or can be mapped to other settings
	AUD_ERR_ACCES,
	AUD_ERR_ADDRINUSE,
	AUD_ERR_ADDRNOTAVAIL,
	AUD_ERR_AFNOSUPPORT,
	// AUD_ERR_AGAIN,       // map to AUD_ERR_TIMEDOUT
	AUD_ERR_ALREADY,
	AUD_ERR_BADF,
	AUD_ERR_CONNABORTED,
	AUD_ERR_CONNREFUSED,
	AUD_ERR_CONNRESET,
	AUD_ERR_DESTADDRREQ,
	AUD_ERR_FAULT,
	AUD_ERR_HOSTUNREACH,
	// AUD_ERR_HOST_NOT_FOUND,
	AUD_ERR_INPROGRESS,
	//AUD_ERR_INTR,          // <- map to AUD_ERR_INTERRUPTED
	// AUD_ERR_INVAL,        // <- map to AUD_ERR_INVALIDPARAMETER
	//AUD_ERR_IO,
	AUD_ERR_ISCONN,
	AUD_ERR_MFILE,
	AUD_ERR_MSGSIZE,
	AUD_ERR_NETDOWN,
	AUD_ERR_NETRESET,
	AUD_ERR_NETUNREACH,
	// AUD_ERR_NFILE,
	// AUD_ERR_NOADDRESS,
	AUD_ERR_NOBUFS,
	AUD_ERR_NODATA,
	AUD_ERR_NODEV,
	// AUD_ERR_NOMEM,      // <- map to AUD_ERR_NOMEMORY
	AUD_ERR_NOPROTOOPT,
	AUD_ERR_NORECOVERY,
	// AUD_ERR_NOSUPPORT,
	AUD_ERR_NOTCONN,
	AUD_ERR_NOTINITIALISED,
	AUD_ERR_NOTSOCK,
	//AUD_ERR_OPNOTSUPP, // <- map to AUD_ERR_NOTSUPPORTED
	// AUD_ERR_PERM,
	//AUD_ERR_PIPE,
	AUD_ERR_PROTONOSUPPORT,
	AUD_ERR_PROTOTYPE,
	AUD_ERR_SHUTDOWN,
	AUD_ERR_SOCKTNOSUPPORT,
	//AUD_ERR_TIMEDOUT,  // <- map to AUD_ERR_TIMEDOUT
	AUD_ERR_TRYAGAIN,
	//AUD_ERR_WOULDBLOCK, // <- map to AUD_ERR_TIMEDOUT

	AUD_ERR_NOTRUNNING,

	// useful for iterating through all known error codes
	AUD_NUM_ERRORS
};

#ifdef __cplusplus
}
#endif

#endif
