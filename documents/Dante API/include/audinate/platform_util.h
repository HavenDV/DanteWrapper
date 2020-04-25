/*
 * Created  : June 2013
 * Author   : Andrew White
 * Synopsis : Useful OS-related functions
 *
 * This software is copyright (c) 2004-2017 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

#ifndef _AUD_PLATFORM_UTIL_H
#define _AUD_PLATFORM_UTIL_H

#ifndef AUD_PLATFORM
#error platform_util.h should never be included directly
#endif

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------
// String Utilities

/*
	Different platforms disagree on the existence of some pseudo-standard utilities
	like strlcpy.
	Rather than try and exhaustively determine when we have strlcpy, we
	will namespace our own. Its not pretty but at least we can count on it
	being there.
*/
size_t
aud_strlcpy(char * dest, const char * src, size_t size);


//----------------------------------------------------------
// Interfaces

/*
	These functions map between OS interface names and OS interface indices.

	Note: These are not the same as Dante primary / secondary interface indices.
 */

enum
{
	AUD_INTERFACE_IDENTIFIER_FLAG_NAME  = 0x0001,
	AUD_INTERFACE_IDENTIFIER_FLAG_INDEX = 0x0002
};


// Interface names
#ifndef AUD_INTERFACE_NAME_LENGTH
	#define AUD_INTERFACE_NAME_LENGTH 63
#endif

#ifdef WIN32
	#define AUD_INTERFACE_NAME_IS_WCHAR 1
	typedef wchar_t aud_intf_char_t;
		// windows uses wchar for interface names
#else
	#define AUD_INTERFACE_NAME_IS_WCHAR 0
	typedef char aud_intf_char_t;
#endif

/**
	Array to hold name of an interface.
 */
typedef aud_intf_char_t aud_interface_name_t[AUD_INTERFACE_NAME_LENGTH + 1];

/**
	Simple struct for identifying an interface by name, index or both.
 */
typedef struct
{
	uint32_t flags;
	aud_interface_name_t name;           // the interface name
	uint32_t index;
} aud_interface_identifier_t;


/**
	Get all available identifiers for a list of interfaces.

	@param env the environment. May be null.
	@param intfs array of interface identifier structures.  Each should be
		initialised with some form of interface identifier.
	@param n number of entries in array.

	On calling, each element in the array should be initialised with a single
	identifier (ie exactly one bit should be set in flags, and the matching
	value should be set).  If more than one identifier is set, then one will
	arbitrarily be chosen to be matched against.

	On completion, all elements will be updated to contain the matching
	identifiers.  Identifiers that could not be found will have their flags cleared.
 */
aud_error_t
aud_interface_get_identifiers
(
	aud_env_t * env,
	aud_interface_identifier_t * intfs,
	unsigned n
);

//----------------------------------------------------------
// Socket address utilities
// aton and ntoa are custom aud_ functions to avoid
// the need for the "in_addr" struct
//----------------------------------------------------------

/**
 * Parse an IP addres in ASCII form into a uint32 form. Analogous to inet_aton
 *
 * @param str the input string
 * @param addr a pointer to the value to hold the parsed string
 */
aud_error_t
aud_inet_aton
(
	const char * str,
	uint32_t * addr
);

/**
 * Get a string representation of an IPv4 address
 * Returns NULL if the buffer is less than AUD_INET_ADDRSTRLEN
 *
 * @param addr the address to convert
 * @param str the buffer to hold the string
 * @param len the length of the buffer
 */
const char * 
aud_inet_ntoa
(
	uint32_t addr,
	char * str,
	size_t len
);


//----------------------------------------------------------
// Stdin / select() integration
// Windows doesn't let you put stdin into a select(). This makes
// it hard to write portable, non-blocking, single-threaded code
// that can process sockets and stdin.
//
// On Windows this structure "redirects" stdin to a socket,
// using a thread and a pair of local sockets.
//
// on *nix platforms this structure is a stub that just
// returns the stdin FD directly
//----------------------------------------------------------

#if AUD_ENV_HAS_STDIN == 1
#if AUD_ENV_HAS_SELECTABLE_STDIN == 0
/**
 * A structre that abstracts getting stdin on a socket
 */
typedef struct aud_stdin_socket aud_stdin_socket_t;

/**
 * Create a stdin socket wrapper
 */
aud_error_t aud_stdin_socket_new(aud_env_t * env, aud_stdin_socket_t ** out_ss);

/**
 * Delete the stdin socket wrapper
 */
void aud_stdin_socket_delete(aud_stdin_socket_t * ss);

aud_socket_t aud_stdin_socket_get_socket(const aud_stdin_socket_t * ss);

#endif
#endif

//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
